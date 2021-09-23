namespace xresource_pipeline::config
{
    //
    // This resource pipeline config file can be found in project config directories.
    // The editor will also have a config file in its directory and is the one 
    // Clone when creating new projects.
    // 
    //  + Project Name.lion_project                 // Project can be located any where (This can go to source control)
    //      + Configs                               // Configuration files
    //  + Version_23.lion_editor                    // Editor folder
    //      + Configs                               // Configuration files
    //
    struct resource_type
    {
        xcore::guid::rcfull<>       m_FullGuid;                 // Full guid of the resource pipeline
        xcore::cstring              m_ResourceTypeName;         // Human readable name of the pipeline name
        bool                        m_bDefaultSettingInEditor;  // Location of the pipeline if empty is assumed to be in the EditorPath
    };

    struct info
    {
        xcore::guid::rcfull<>       m_ProjectFullGuid;          // Full guid of the project
        std::vector<resource_type>  m_ResourceTypes;            // List of resource types that we can handle
    };

    inline
    xcore::err Serialize( info& Info, std::string_view FilePath, bool isRead ) noexcept
    {
        xcore::textfile::stream Stream;
        xcore::err              Error;

        // Open file
        if( auto Err = Stream.Open(isRead, FilePath, xcore::textfile::file_type::TEXT ); Err )
            return Err;

        // Deal with general information about the pipeline
        if( Stream.Record(Error, "ResourcePipeline"
            , [&]( std::size_t, xcore::err& Err)
            {
                xcore::cstring  FullGuid;
                if (false == isRead) FullGuid = Info.m_ProjectFullGuid.getPath();

                Err = Stream.Field( "ProjectFullGuid", FullGuid );

                if (isRead && !Err) Err = Info.m_ProjectFullGuid.setupFromPath(FullGuid);
            })) return Error;

        // Deal with each type of resource
        if( Stream.Record( Error, "ResourceTypes"
        , [&]( std::size_t& Count, xcore::err& Err )
        {
            if( isRead ) Info.m_ResourceTypes.resize(Count);
            else         Count = Info.m_ResourceTypes.size();
        }
        , [&]( std::size_t I, xcore::err& Err )
        {
            auto&           RT          = Info.m_ResourceTypes[I];
            xcore::cstring  FullGuid;

            if( false == isRead ) FullGuid = RT.m_FullGuid.getPath();

                (Err = Stream.Field( "FullGuid", FullGuid ))
            ||  (Err = Stream.Field( "ResourceTypeName", RT.m_ResourceTypeName ))
            ||  (Err = Stream.Field( "bDefaultSettingInEditor", RT.m_bDefaultSettingInEditor ));

            if( isRead && !Err ) Err = RT.m_FullGuid.setupFromPath(FullGuid);

        }) ) return Error;

        return {};
    }
}