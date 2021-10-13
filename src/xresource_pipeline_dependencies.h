namespace xresource_pipeline
{
    struct dependencies
    {
        xcore::vector<xcore::guid::rcfull<>>    m_VirtualResources;
        xcore::vector<xcore::cstring>           m_Assets;
        xcore::vector<xcore::cstring>           m_VirtualAssets;

        inline static xcore::err Serialize ( dependencies& Dependencies, const std::string_view FilePath, bool isRead ) noexcept;
    };

    //--------------------------------------------------------------------------------------------
    
    xcore::err dependencies::Serialize( dependencies& Dependencies, const std::string_view FilePath, bool isRead ) noexcept
    {
        xcore::textfile::stream Stream;
        xcore::err              Error;

        // Open file
        if (auto Err = Stream.Open(isRead, FilePath, xcore::textfile::file_type::TEXT, xcore::textfile::flags{ .m_isWriteFloats = true, .m_isWriteEndianSwap = false }); Err)
            return Err;

        if (Stream.Record(Error, "Assets"
            , [&](std::size_t& Count, xcore::err& Err)
            {
                if (isRead) Dependencies.m_Assets.appendList(Count);
                else        Count = Dependencies.m_Assets.size();
            }
            , [&](std::size_t I, xcore::err& Err)
            {
                auto& Entry = Dependencies.m_Assets[I];

                Stream.Field("Asset", Entry ).clear();
            }))
        {
            // If we don't have this record is because there is no clip art (Which is ok)
            if (Error.getCode().getState<xcore::textfile::error_state>() == xcore::textfile::error_state::UNEXPECTED_RECORD) Error.clear();
            else return Error;
        }

        if (Stream.Record(Error, "VirtualResources"
            , [&](std::size_t& Count, xcore::err& Err)
            {
                if (isRead) Dependencies.m_VirtualResources.appendList(Count);
                else        Count = Dependencies.m_VirtualResources.size();
            }
            , [&](std::size_t I, xcore::err& Err)
            {
                auto& Entry = Dependencies.m_VirtualResources[I];

                Stream.Field("Type",    Entry.m_Type.m_Value ).clear();
                Stream.Field("Instance", Entry.m_Instance.m_Value).clear();
            }))
        {
            // If we don't have this record is because there is no clip art (Which is ok)
            if (Error.getCode().getState<xcore::textfile::error_state>() == xcore::textfile::error_state::UNEXPECTED_RECORD) Error.clear();
            else return Error;
        }
    
        if (Stream.Record(Error, "VirtualAssets"
            , [&](std::size_t& Count, xcore::err& Err)
            {
                if (isRead) Dependencies.m_VirtualAssets.appendList(Count);
                else        Count = Dependencies.m_VirtualAssets.size();
            }
            , [&](std::size_t I, xcore::err& Err)
            {
                auto& Entry = Dependencies.m_VirtualAssets[I];

                Stream.Field("VirtualAsset", Entry ).clear();
            }))
        {
            // If we don't have this record is because there is no clip art (Which is ok)
            if (Error.getCode().getState<xcore::textfile::error_state>() == xcore::textfile::error_state::UNEXPECTED_RECORD) Error.clear();
            else return Error;
        }
    
        return {};
    }
}