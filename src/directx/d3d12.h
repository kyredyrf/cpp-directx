#pragma once

#include <dxgi1_6.h>
#include <map>

#define DXGI_ADAPTER_FLAG3_DATA(ID) { DXGI_ADAPTER_FLAG3_##ID, #ID }
#define DXGI_GRAPHICS_PREEMPTION_DATA(ID) { DXGI_GRAPHICS_PREEMPTION_##ID, #ID }
#define DXGI_COMPUTE_PREEMPTION_DATA(ID) { DXGI_COMPUTE_PREEMPTION_##ID, #ID }

inline std::string ToString(DXGI_ADAPTER_FLAG3 e)
{
    static std::map<DXGI_ADAPTER_FLAG3, const char*> data =
    {
        DXGI_ADAPTER_FLAG3_DATA(NONE),
        DXGI_ADAPTER_FLAG3_DATA(REMOTE),
        DXGI_ADAPTER_FLAG3_DATA(SOFTWARE),
        DXGI_ADAPTER_FLAG3_DATA(ACG_COMPATIBLE),
        DXGI_ADAPTER_FLAG3_DATA(SUPPORT_MONITORED_FENCES),
        DXGI_ADAPTER_FLAG3_DATA(SUPPORT_NON_MONITORED_FENCES),
        DXGI_ADAPTER_FLAG3_DATA(KEYED_MUTEX_CONFORMANCE),
    };

    std::string s;

    for (auto i : data)
    {
        if ((e & i.first) != 0)
        {
            if (s.length() > 0)
            {
                s += StringGenerator::SPrintf(" | ");
            }

            s += i.second;
        }
    }

    return std::move(s);
}

inline std::string ToString(DXGI_GRAPHICS_PREEMPTION_GRANULARITY e)
{
    static std::map<DXGI_GRAPHICS_PREEMPTION_GRANULARITY, const char*> data =
    {
        DXGI_GRAPHICS_PREEMPTION_DATA(DMA_BUFFER_BOUNDARY),
        DXGI_GRAPHICS_PREEMPTION_DATA(PRIMITIVE_BOUNDARY),
        DXGI_GRAPHICS_PREEMPTION_DATA(TRIANGLE_BOUNDARY),
        DXGI_GRAPHICS_PREEMPTION_DATA(PIXEL_BOUNDARY),
        DXGI_GRAPHICS_PREEMPTION_DATA(INSTRUCTION_BOUNDARY),
    };

    return std::move(data[e]);
}

inline std::string ToString(DXGI_COMPUTE_PREEMPTION_GRANULARITY e)
{
    static std::map<DXGI_COMPUTE_PREEMPTION_GRANULARITY, const char*> data =
    {
        DXGI_COMPUTE_PREEMPTION_DATA(DMA_BUFFER_BOUNDARY),
        DXGI_COMPUTE_PREEMPTION_DATA(DISPATCH_BOUNDARY),
        DXGI_COMPUTE_PREEMPTION_DATA(THREAD_GROUP_BOUNDARY),
        DXGI_COMPUTE_PREEMPTION_DATA(THREAD_BOUNDARY),
        DXGI_COMPUTE_PREEMPTION_DATA(INSTRUCTION_BOUNDARY),
    };

    return std::move(data[e]);
}
