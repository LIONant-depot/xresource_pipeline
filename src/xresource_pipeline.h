#ifndef XRESOURCE_PIPELINE_H
#define XRESOURCe_PIPELINE_H
#pragma once

#include "xcore.h"

namespace xresource_pipeline
{
    //----------------------------------------------------------------------------
    // Description
    //----------------------------------------------------------------------------
    enum class error : std::uint32_t
    { GUID      = xcore::guid::unit<32>("xresource_pipeline").m_Value
    , OK        = 0
    , FAILURE
    , DISPLAY_HELP
    };
}

//
//  
//
#include "xresource_pipeline_compiler_base.h"


#endif