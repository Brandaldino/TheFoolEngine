#include "tfpch.h"
#include "RenderGraph.h"

#include "PointShadowMap.h"

namespace TheFoolEngine
{

    TextureHandle RenderGraph::CreateRenderTarget(const TextureDesc& desc, const char* name)
    {
        RenderTargetResource res;
        res.Desc = desc;

        switch (desc.Type)
        {
            case RenderTargetType::Color:
            {
                FrameBufferSpecification spec;
                spec.Width = desc.Width;
                spec.Height = desc.Height;
                spec.FrameBufferFormat = desc.Format;
                res.FrameBuffer = FrameBuffer::Create(spec);
                break;
            }
            case RenderTargetType::DepthArray:
            {
                FrameBufferSpecification spec;
                spec.Width = desc.Width;
                spec.Height = desc.Height;
                spec.DepthOnly = true;
                spec.LayerCount = desc.LayerCount;
                res.FrameBuffer = FrameBuffer::Create(spec);
                break;
            }
            case RenderTargetType::CubeMapArray:
            {
                res.PointShadowMap = PointShadowMap::Create(desc.Width, desc.LayerCount);
                break;
            }
        }

        m_Resources.push_back(std::move(res));

        TextureHandle handle;
        handle.Desc = desc;
        handle.PoolIndex = (uint32_t)m_Resources.size() - 1;
        handle.Name = name ? name : "";

        return handle;
    }

    Ref<FrameBuffer> RenderGraph::GetFrameBuffer(const TextureHandle& handle) const
    {
        if (handle.PoolIndex >= m_Resources.size())
            return nullptr;

        return m_Resources[handle.PoolIndex].FrameBuffer;
    }

    Ref<PointShadowMap> RenderGraph::GetPointShadowMap(const TextureHandle& handle) const
    {
        if (handle.PoolIndex >= m_Resources.size())
            return nullptr;

        return m_Resources[handle.PoolIndex].PointShadowMap;
    }

    Ref<Texture2D> RenderGraph::GetTexture(const TextureHandle& handle) const
    {
        auto target = GetFrameBuffer(handle);
        if (!target)
            return nullptr;

        return target->GetColorAttachment();
    }

    void RenderGraph::Resize(uint32_t width, uint32_t height)
    {
        for (auto& res : m_Resources)
        {
            if (res.FrameBuffer)
                res.FrameBuffer->Resize(width, height);
        }
    }

    void RenderGraph::AddPass(Scope<Pass> pass)
    {
        m_Passes.emplace_back(std::move(pass));
    }

    void RenderGraph::Execute(RenderContext& context)
    {
        context.RenderGraph = this;

        // Re-sorting: At this point, PoolIndex is ready and the dependency graph is correct.
        auto order = TopologicalSort();

        // Execute in topological order
        for (auto idx : order)
            m_Passes[idx]->Execute(context);
    }

    std::vector<uint32_t> RenderGraph::TopologicalSort()
    {
        // 1. RAW: Resource -> Write its pass
        std::unordered_map<uint32_t, uint32_t> writerOf;
        for (uint32_t i = 0; i < m_Passes.size(); ++i)
            for (auto& out : m_Passes[i]->GetOutputs())
                writerOf[out.PoolIndex] = i;

        // 2. Map construction (merging RAW + WAW)
        std::vector<uint32_t> inDegree(m_Passes.size(), 0);
        std::vector<std::vector<uint32_t>> adj(m_Passes.size());

        // WAW: Resource -> Last pass that wrote to it (in AddPass order, as declared by the developer)
        std::unordered_map<uint32_t, uint32_t> lastWriter;

        for (uint32_t i = 0; i < m_Passes.size(); ++i)
        {
            // RAW: Read resource -> Pass that depends on it for writing
            for (auto& in : m_Passes[i]->GetInputs())
            {
                auto it = writerOf.find(in.PoolIndex);
                if (it != writerOf.end() && it->second != i)
                {
                    adj[it->second].push_back(i);
                    inDegree[i]++;
                }
            }
            // WAW: Write resource -> Pass that depends on the previous pass of writing the same resource
            for (auto& out : m_Passes[i]->GetOutputs())
            {
                auto it = lastWriter.find(out.PoolIndex);
                if (it != lastWriter.end() && it->second != i)
                {
                    adj[it->second].push_back(i);   // the previous writer -> the current writer
                    inDegree[i]++;
                }
                lastWriter[out.PoolIndex] = i;
            }
        }

        // 3. Kahn topological sorting
        std::queue<uint32_t> q;
        for (uint32_t i = 0; i < inDegree.size(); ++i)
            if (inDegree[i] == 0)
                q.push(i);

        std::vector<uint32_t> order;
        while (!q.empty())
        {
            uint32_t cur = q.front(); q.pop();
            order.push_back(cur);
            for (auto next : adj[cur])
                if (--inDegree[next] == 0)
                    q.push(next);
        }

        TF_CORE_ASSERT(order.size() == m_Passes.size(), "RenderGraph: cyclic dependency detected!");
        return order;
    }

}