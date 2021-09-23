namespace xresource_pipeline::tag
{
    using guid = xcore::guid::unit<64>;

    struct info
    {
        xcore::cstring          m_ResourceName;
        guid                    m_Guid;
    };
}