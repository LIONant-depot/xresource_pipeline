namespace xresource_pipeline::key
{
    struct base
    {
        xcore::cstring              m_ResourceName;
        xcore::guid::rcfull<>       m_FullGuid;
        xcore::cstring              m_Description;
        xcore::vector<tag::guid>    m_Tags;
    };
}