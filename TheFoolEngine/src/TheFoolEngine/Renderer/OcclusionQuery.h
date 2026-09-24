#pragma once

#include "../Core/Base.h"

namespace TheFoolEngine
{

    class OcclusionQuery
    {
    public:
        virtual ~OcclusionQuery() = default;

        OcclusionQuery(const OcclusionQuery&) = delete;
        OcclusionQuery& operator=(const OcclusionQuery&) = delete;

        virtual void Begin() = 0;
        virtual void End() = 0;
        virtual bool Available() = 0;
        virtual uint32_t GetResult() = 0;
        virtual const uint32_t GetRendererID() const = 0;

        static Ref<OcclusionQuery> Create();
    protected:
        OcclusionQuery() = default;
    };
}