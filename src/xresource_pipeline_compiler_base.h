namespace xresource_pipeline::compiler
{
    //
    // Input to compilers
    // ------------------
    // -BUILDTYPE Oz -TARGET WINDOWS -EDITOR "Path to the editor root" -PROJECT "Path to the project root" -RESOURCE ff.ff/ff -OUTPUT "Path to a resource .dbase"
    //
    class base
    {
    public:
        
        using imported_file_fn = xcore::func< void( const xcore::cstring& FilePath ) >;

        enum class build_type
        { INVALID
        , O0                // Compiles the asset as fast as possible no real optimization
        , O1                // Build with optimizations
        , Oz                // Take all the time you like to optimize this resource
        };
            
        struct platform
        {
            bool                    m_bValid            { false };                              // If we need to build for this platform
            xcore::target::platform m_Platform          { xcore::target::platform::WINDOWS };   // Platform that we need to compile for
            xcore::cstring          m_DataPath          {};                                     // This is where the compiler need to drop all the compiled data
        };

    public:
        
        virtual                                        ~base                        ( void )                                                        noexcept = default;
                                                        base                        ( void )                                                        noexcept;

                            xcore::err                  Compile                     ( void )                                                        noexcept;
                            xcore::err                  Parse                       ( int argc, const char* argv[] )                                noexcept;

    protected:
        
                            xcore::err                  InternalParse               ( const int argc, const char *argv[] )                          noexcept;
                            xcore::cstring              getDestinationPath          ( xcore::target::platform p )                           const   noexcept;

        virtual             xcore::guid::rcfull<>       getResourcePipelineFullGuid ( void )                                                const   noexcept = 0;
        virtual             xcore::err                  onCompile                   ( void )                                                        noexcept = 0;
                            xcore::err                  setupPaths                  ( void )                                                        noexcept;
                            xcore::err                  CreatePath                  ( const xcore::cstring& Path )                          const   noexcept;

    protected:
        
        build_type                                              m_BuildType             {};
        std::chrono::steady_clock::time_point                   m_Timmer                {};
        xcore::cstring                                          m_ProjectPath           {}; // Project or Library that contains the resource (note that this is the root)
        xcore::guid::rcfull<>                                   m_RscGuid               {}; // GUID of the resource
        xcore::cstring                                          m_EditorPath            {};
        xcore::cstring                                          m_OutputRootPath        {};
        xcore::cstring                                          m_OutputProjectPath     {};
        xcore::cstring                                          m_CompilerConfigPath    {};
        xcore::cstring                                          m_CompilerConfigPathFile{};

        xcore::cstring                                          m_ProjectConfigPath     {};

        xcore::cstring                                          m_ResourcePath          {}; // Path to the asset 
        xcore::cstring                                          m_ResourceDescriptorPath{}; // Path to the asset 

        xcore::cstring                                          m_GeneratedPath         {};
   //     xcore::cstring                                          m_CompiledPath          {}; // Compiled asset path for the given project/library
   //     xcore::cstring                                          m_ExternalAssetsPath    {};

        xcore::cstring                                          m_BrowserPath           {};
        xcore::file::stream                                     m_LogFile               {};
        std::array<platform, xcore::target::getPlatformCount()> m_Target                {};
        xcore::log::channel                                     m_LogChannel            { "COMPILER" };
        config::info                                            m_ConfigInfo            {};
        int                                                     m_ConfigInfoIndex       {-1};
    
    protected:

        friend void LogFunction(const xcore::log::channel& Channel, xcore::log::msg_type Type, const char* String, int Line, const char* file) noexcept;
    };
}
