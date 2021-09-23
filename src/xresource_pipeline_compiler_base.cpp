
#include "xresource_pipeline.h"

namespace xresource_pipeline::compiler {

base* g_pBase = nullptr;

//---------------------------------------------------------------------------------------

void LogFunction( const xcore::log::channel& Channel, xcore::log::msg_type Type, const char* pString, int Line, const char* file ) noexcept
{
    auto Message = xcore::string::Fmt
    ( "%s(%d) [%s] %s/n"
    , file ? file : "Unkown file"
    , Line
    , xcore::log::msg_type::L_INFO    == Type ? "Info"
    : xcore::log::msg_type::L_WARNING == Type ? "Warning"
    :                                           "Error"
    , pString ? pString : "Unkown" 
    );

    printf( Message.data() );

    if( false == g_pBase->m_LogFile.isOpen() ) 
    {
        if( auto Err = g_pBase->m_LogFile.Printf( Message.data() ); Err )
        {
            printf( "Fail to save data in log\n" );
            Err.clear();
        }
    }
}

//--------------------------------------------------------------------------

base::base( void ) noexcept
{
    // Install the log function
    xcore::get().m_MainLogger.m_pOutput = LogFunction;
    g_pBase = this;
}


//--------------------------------------------------------------------------

xcore::err base::setupPaths( void ) noexcept
{
    m_ProjectConfigPath = xcore::string::Fmt("%s/Config", m_ProjectPath.data() );

    //
    // Lets read the config info file
    //
    if( auto Err = xresource_pipeline::config::Serialize( m_ConfigInfo, xcore::string::Fmt("%s/ResourcePipeline.config", m_ProjectConfigPath.data()).data(), true ); Err )
        return Err;

    //
    // Check if we can see our resource type in the list...
    //
    {
        for( const auto& E : m_ConfigInfo.m_ResourceTypes )
        {
            auto RCFull = getResourcePipelineFullGuid();
            if( E.m_FullGuid.m_Instance.m_Value == RCFull.m_Instance.m_Value 
             && E.m_FullGuid.m_Type.m_Value     == RCFull.m_Type.m_Value )
            {
                m_ConfigInfoIndex = (int)static_cast<std::size_t>(&E - m_ConfigInfo.m_ResourceTypes.data());
                break;
            }
        }
        if( m_ConfigInfoIndex == -1 )
            return xerr_failure_s("Fail to find the resource type inside ResourcePipeline.config");
    }
    
    //
    // Set the OutputProjectPath
    //
    {
        auto ProjectFullGuidString = m_ConfigInfo.m_ProjectFullGuid.getPath();
        xcore::string::ReplaceChar(ProjectFullGuidString, '/', '-');
        m_OutputProjectPath = xcore::string::Fmt( "%s/%s", m_OutputRootPath.data(), ProjectFullGuidString.data() );
    }

    //
    // Set the Browser path
    //
    m_BrowserPath = xcore::string::Fmt("%s/Browser.dbase/%s/%s", m_OutputProjectPath.data()
                                                               , m_ConfigInfo.m_ResourceTypes[m_ConfigInfoIndex].m_ResourceTypeName.data()
                                                               , m_RscGuid.m_Instance.getStringHex<char>().data() 
                                                               );    
    if( auto Err = m_LogFile.open( xcore::string::To<wchar_t>(xcore::string::Fmt( "%s/Compilation.log.txt", m_BrowserPath.data() )), "wt"); Err )
        return xerr_failure_s("Fail to create the log file");

    //
    // Set path to the resource
    //
    m_ResourcePath = xcore::string::Fmt( "%s/Resources/%s/%s", m_ProjectPath.data()
                                                             , m_ConfigInfo.m_ResourceTypes[m_ConfigInfoIndex].m_ResourceTypeName.data()
                                                             , m_RscGuid.m_Instance.getStringHex<char>().data()
                                                             );

    m_ResourceDescriptorPath = xcore::string::Fmt("%s/Descriptor.txt", m_ResourcePath.data() );

    //
    // Set the output path of the resource
    //
    for( auto& E : m_Target )
    {
        if( E.m_bValid )
        {
            E.m_DataPath = xcore::string::Fmt( "%s/%s.platform/Data", m_OutputProjectPath.data(), xcore::target::getPlatformString(E.m_Platform) );
        }
    }

    //
    // Set the Generated path
    //
    m_GeneratedPath = xcore::string::Fmt( "%s/Generated.dbase", m_OutputProjectPath.data() );



    return {};
}

//--------------------------------------------------------------------------

xcore::err base::InternalParse( const int argc, const char *argv[] ) noexcept
{
    xcore::cmdline::parser CmdLineParser;
    
    //
    // Create the switches and their rules
    //
    {
        CmdLineParser.AddCmdSwitch( xcore::string::const_crc("BUILDTYPE"),  1,  1, 0, 1, false );
        CmdLineParser.AddCmdSwitch( xcore::string::const_crc("TARGET"),     1, -1, 1, 1, false );
        CmdLineParser.AddCmdSwitch( xcore::string::const_crc("EDITOR"),     1,  1, 1, 1, false );
        CmdLineParser.AddCmdSwitch( xcore::string::const_crc("OUTPUT"),     1,  1, 1, 1, false );
        CmdLineParser.AddCmdSwitch( xcore::string::const_crc("PROJECT"),    1,  1, 1, 1, true );
        CmdLineParser.AddCmdSwitch( xcore::string::const_crc("INPUT"),      1,  1, 1, 1, true );
    }
    
    //
    // Start parsing the arguments
    //
    if( auto Err =  CmdLineParser.Parse( argc, argv ); Err )
        return Err;
    
    //
    // Deal with the help message
    //
    if( argc < 2 || CmdLineParser.DoesUserNeedsHelp() )
    {
        std::printf( "\n"
                  "-------------------------------------------------------------\n"
                  "LION - Compiler system.                                      \n"
                  "%s [ %s - %s ]                                               \n"
                  "Switches: (Make sure they are in order)                      \n"
                  "     -BUILDTYPE  O0 - Compile as fast as possible            \n"
                  "                 Q1 - Compile with optimizations             \n"
                  "                 Qz - Maximum performance for asset          \n"
                  "     -TARGET     <WINDOWS OSX IOS ANDROID PS3 TOOLS>         \n"
                  "     -EDITOR     <NetworkFriendlyPathToEditor.lion_editor>   \n"
                  "     -PROJECT    <NetworkFriendlyPathToProject.lion_project> \n"
                  "     -INPUT      <ASSET_GUID_FULL> Must comes after -PROJECT \n"
                  "     -OUTPUT     <NetworkFriendlyPathToResource.lion_rcdbase>\n"
                  "-------------------------------------------------------------\n",
                 argv[0],
                 __TIME__, __DATE__ );
        
        return xerr_code_s( error::DISPLAY_HELP, "User requested help" );
    }
    
    //
    // Go through the parameters of the command line
    //
    xcore::cstring InputName;
    for( int i=0; i< CmdLineParser.getCommandCount(); i++ )
    {
        const auto  Cmd     = CmdLineParser.getCommand(i);
        const auto  CmdCRC  = Cmd.getCRC();
        if( CmdCRC == xcore::types::value<xcore::crc<32>::FromString( "TARGET" )> )
        {
            const auto Count = Cmd.getArgumentCount(); 
            
            for( int i=0; i<Count; i++ )
            {
                const char* pString = Cmd.getArgument(i);
                bool        bFound  = false;
                
                // Go through all the platforms and pick up the right one
                constexpr auto TargetCount = xcore::target::getPlatformCount(); 
                for( int ip = 0; ip < TargetCount; ip++ )
                {
                    const auto p = xcore::types::static_cast_safe<xcore::target::platform>( ip );

                    if( xcore::string::CompareI( pString, xcore::target::getPlatformString(p) ) == 0 )
                    {
                        platform& NewTarget   = m_Target[ip];
                        
                        if( NewTarget.m_bValid )
                            return xerr_failure_s("The same platform was enter multiple times");
                        
                        NewTarget.m_Platform  = p;
                        NewTarget.m_bValid    = true;
                        
                        bFound = true;
                        break;
                    }
                }
                
               if( bFound == false )
                   return xerr_failure_s("Platform not supported");
            }
        }
        else if( CmdCRC == xcore::types::value<xcore::crc<32>::FromString( "EDITOR" )> )
        {
            if( -1 == xcore::string::FindStr( Cmd.getArgument(0), ".lion_editor") )
                return xerr_failure_s("I got a path in the EDITOR switch that is not a .lion_editor path");

            xcore::string::Copy( m_EditorPath, Cmd.getArgument( 0 ) );
            xcore::string::CleanPath(m_EditorPath);
        }
        else if( CmdCRC == xcore::types::value<xcore::crc<32>::FromString( "OUTPUT" )> )
        {
            if (-1 == xcore::string::FindStr( Cmd.getArgument(0), ".lion_rcdata"))
                return xerr_failure_s("I got a path in the OUTPUT switch that is not a .lion_rcdata path");

            xcore::string::Copy(m_OutputRootPath, Cmd.getArgument( 0 ) );
            xcore::string::CleanPath(m_OutputRootPath);
        }
        else if( CmdCRC == xcore::types::value<xcore::crc<32>::FromString( "PROJECT" )> )
        {
            if (-1 == xcore::string::FindStr( Cmd.getArgument(0), ".lion_project"))
                return xerr_failure_s("I got a path in the PROJECT switch that is not a .lion_project path");

            xcore::string::Copy( m_ProjectPath, Cmd.getArgument( 0 ) );
            xcore::string::CleanPath(m_ProjectPath);
        }
        else if( CmdCRC == xcore::types::value<xcore::crc<32>::FromString( "INPUT" )> )
        {
            if (auto Err = m_RscGuid.setupFromPath(Cmd.getArgument(0)); Err)
                return Err;

            if( xcore::string::findLastInstance(Cmd.getArgument(0), '/') == -1 )
                return xerr_failure_s("Badly formatted input resource expecting ttttt.ttttt/iiiiiiiii but got ttttt.ttttt.iiiiiiiii");
        }
        else if( CmdCRC == xcore::types::value<xcore::crc<32>::FromString( "BUILDTYPE" )> )
        {
            xcore::cstring BuildType;

            xcore::string::Copy( BuildType, Cmd.getArgument( 0 ) );
            
            if( BuildType == xcore::string::constant("O0") )
            {
                m_BuildType = build_type::O0;
            }
            else if( BuildType == xcore::string::constant("O1") )
            {
                m_BuildType = build_type::O1;
            }
            else if( BuildType == xcore::string::constant("Oz") )
            {
                m_BuildType = build_type::Oz;
            }
            else
            {
                return xerr_failure_s("Build Type not supported");
            }
        }
        else
        {
            // We must have forgotten a switch because we should not be able to reach this point
            return xerr_failure_s("Compiler found unknown arguments");
        }
    }

    //
    // Logs data base
    //
    if( auto Err = setupPaths(); Err )
        return Err;

    //
    // Make sure that the paths exists
    //    


    return {};
}

//--------------------------------------------------------------------------

xcore::cstring base::getDestinationPath( xcore::target::platform p ) const noexcept
{
    const int Index = static_cast<int>(p);
    xassert( m_Target[Index].m_bValid );
    xassert( m_Target[Index].m_Platform == p );
    return m_Target[Index].m_DataPath;
}

//--------------------------------------------------------------------------

xcore::err base::Parse( int argc, const char *argv[] ) noexcept
{
    if( auto Err = InternalParse( argc, argv ); Err )
    {
        if( Err.getCode().getState<xresource_pipeline::error>() != xresource_pipeline::error::DISPLAY_HELP )
        {
            // TODO: We should open the log file before this???
            // XLOG_CHANNEL_ERROR(m_LogChannel, "Parsing error (%s)", Err.getCode().m_pString);
        }
        else
        {
            // If we are displaying help we just return...
            Err.clear();
        }
        
        return Err;
    }
    
    return {};
}

//--------------------------------------------------------------------------

xcore::err base::Compile( void ) noexcept
{
    m_Timmer = std::chrono::steady_clock::now();

    //
    // Create the log folder
    //
    /*
    {
        std::error_code         ec;
        std::filesystem::path   LogPath { xcore::string::To<wchar_t>(m_BrowserPath).data() };

        std::filesystem::create_directories( LogPath, ec );
        if( ec )
        {
            XLOG_CHANNEL_ERROR( m_LogChannel, "Fail to create the log directory [%s]", ec.message().c_str() );
            return xerr_failure_s( "Fail to create the log directory" );
        }
    }
    */
    //
    // Create log file per platform
    //
    /*
    for( auto& Entry : m_Target )
    {
        if( Entry.m_bValid == false )
            continue;

        const auto  p           = x_static_cast<x_target::platform>( m_Target.getIndexByEntry(Entry) );

        xstring Path;
        Path.setup( "%s/%s.txt"
                    , m_LogsPath.getPtr()
                    , x_target::getPlatformString(p) );

        if( auto Err = Entry.m_LogFile.Open( Path.To<xwchar>(), "wt" ); Err.isError() )
        {
            X_LOG_CHANNEL_ERROR( m_LogChannel, "Fail to create the log file [%s]", Path.To<xchar>().getPtr() );
            return { err::state::FAILURE, "Fail to create the log file" };
        }
    }
    */

    //
    // Create the log 
    //
    /*
    {
        xcore::cstring Path;
        Path = xcore::string::Fmt( "%s/Log.txt", m_LogsPath.data() );
        if( auto Err = m_LogFile.open( xcore::string::To<wchar_t>(Path), "wt" ); Err )
        {
            XLOG_CHANNEL_ERROR( m_LogChannel, "Fail to create the log file [%s]", Path.data() );
            return xerr_failure_s( "Fail to create the log file" );
        }
    }
    */
    //
    // Get the timer
    //
    if( m_LogFile.isOpen() )
    {
        auto Err = m_LogFile.Printf( "------------------------------------------------------------------\n"
                                     " Start Compilation\n"
                                     "------------------------------------------------------------------\n" );
        Err.clear();
    }

    //
    // Do the actual compilation
    //
    if( auto Err = onCompile(); Err )
        return Err;

    //
    // Get the timer
    //
    if( m_LogFile.isOpen() )
    {
        auto Err = m_LogFile.Printf( "------------------------------------------------------------------\n"
                                     " Compilation Time: %fs\n"
                                     "------------------------------------------------------------------\n"
                                     , std::chrono::duration<float>(std::chrono::steady_clock::now() - m_Timmer).count() );
        Err.clear();
    }

    return {};
}

} //namespace xasset_pipeline::compiler