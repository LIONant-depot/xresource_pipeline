namespace xresource_pipeline::descriptor
{
    struct base
    {
        struct version
        {
            std::int32_t    m_Major;
            std::int32_t    m_Minor;
        };    

        version m_Version;

        xcore::err Serialize( xcore::textfile::stream& Stream, bool isRead ) noexcept
        {
            xcore::err Error;

            // Deal with general information about the pipeline
            if( Stream.Record(Error, "Info"
                , [&]( std::size_t, xcore::err& Err)
                {
                    Err = Stream.Field("Version", m_Version.m_Major, m_Version.m_Minor );
                })) return Error;
        
            return {};
        }
    };

}