namespace xresource_pipeline::compiler
{
    //
    // Input to compilers
    // ------------------
    // -BUILDTYPE Oz -TARGET WINDOWS -LIBRARY "Path to the library" -RESOURCE ff.ff/ff
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
            xcore::cstring          m_CompiledBinPath   {};                                     // This is where the compiler need to drop all the compiled data
            xcore::file::stream     m_LogFile           {};
        };

    public:
        
        virtual                                        ~base                        ( void )                                                        noexcept = default;
                                                        base                        ( void )                                                        noexcept;
                            xcore::err                  Compile                     ( void )                                                        noexcept;
                            xcore::err                  Parse                       ( int argc, const char* argv[] )                                noexcept;

    protected:
        
                            xcore::err                  InternalParse               ( const int argc, const char *argv[] )                          noexcept;
                            xcore::cstring              getDestinationPath          ( xcore::target::platform p )                           const   noexcept;

        virtual             xcore::err                  onCompile                   ( void )                                                        noexcept = 0;

    protected:
        
        build_type                                              m_BuildType             {};
        std::chrono::steady_clock::time_point                   m_Timmer                {};
        xcore::cstring                                          m_LibraryPath           {}; // Where the resource is located
        xcore::cstring                                          m_ResourcePath          {}; // Path to the asset 
        xcore::guid::rcfull<>                                   m_RscGuid               {}; // GUID of the resource
        xcore::guid::rcfull<>                                   m_LibraryGuid           {}; // GUID of the library or project that has the asset
        xcore::cstring                                          m_QLionPath             {}; // ??????
        xcore::cstring                                          m_CompiledPath          {}; // Compiled asset path for the given project/library
        xcore::cstring                                          m_ExternalAssetsPath    {};
        xcore::cstring                                          m_LogsPath              {};
        xcore::file::stream                                     m_LogFile               {};
        std::array<platform, xcore::target::getPlatformCount()> m_Target                {};
        xcore::log::channel                                     m_LogChannel            { "COMPILER" };

    protected:

        friend void LogFunction(const xcore::log::channel& Channel, xcore::log::msg_type Type, const char* String, int Line, const char* file) noexcept;
    };
}
