#pragma once

#include "../Core/Base.h"

namespace TheFoolEngine
{
    class OcclusionQuery
    {
    public:
        OcclusionQuery();
        ~OcclusionQuery();

        OcclusionQuery(const OcclusionQuery&) = delete;
        OcclusionQuery& operator=(const OcclusionQuery&) = delete;

        void Begin();
        void End();
        bool Available();
        uint32_t GetResult();
    private:
        uint32_t m_RendererID = 0;
    };
}