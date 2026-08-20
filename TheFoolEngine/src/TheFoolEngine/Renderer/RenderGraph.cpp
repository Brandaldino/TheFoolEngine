#include "tfpch.h"
#include "RenderGraph.h"

namespace TheFoolEngine
{
    TextureHandle RenderGraph::CreateTexture(const TextureDesc& desc, const char* name)
    {
        TextureHandle handle;
        handle.Desc = desc;
        handle.PoolIndex = m_Pool.Allocate(desc);
        handle.Name = name ? name : "";

        m_Resources.push_back(handle);  // for debug
        return handle;
    }

    TextureHandle RenderGraph::RegisterTexture(const TextureDesc& desc, Ref<Texture2D> texture, const char* name)
    {
        TextureHandle handle;
        handle.Desc = desc;
        handle.PoolIndex = m_Pool.Register(desc, texture);
        handle.Name = name;

        return handle;
    }

    void RenderGraph::AddPass(Scope<Pass> pass)
    {
        m_Passes.emplace_back(std::move(pass));
    }

    void RenderGraph::Execute(RenderContext& context)
    {
        // Resource Allocation: Allocate pool slots for each output
        for (auto& pass : m_Passes)
        {
            for (auto& out : pass->GetOutputs())
            {
                if (!out.IsValid())  // PoolIndex is not allocated
                    out.PoolIndex = m_Pool.Allocate(out.Desc);
            }
        }

        // Re-sorting: At this point, PoolIndex is ready and the dependency graph is correct.
        auto order = TopologicalSort();

        // Execute in topological order
        for (auto idx : order)
            m_Passes[idx]->Execute(context);

        m_Pool.ResetFrame();
    }

    std::vector<uint32_t> RenderGraph::TopologicalSort()
    {
        // 1. Build the dependency graph: for each resource read by pass i, find the pass j that wrote it → i depends on j
        std::unordered_map<uint32_t, uint32_t> writerOf; // res -> pass
        for (uint32_t i = 0; i < m_Passes.size(); ++i)
        {
            for (auto& out : m_Passes[i]->GetOutputs())
                writerOf[out.PoolIndex] = i;
        }

        // 2. Topological Sorting (Kahn's Algorithm: Execute nodes with zero in-degree first)
        std::vector<uint32_t> inDegree(m_Passes.size(), 0);
        std::vector <std::vector<uint32_t>> adj(m_Passes.size());
        for (uint32_t i = 0; i < m_Passes.size(); ++i)
        {
            for (auto& in : m_Passes[i]->GetInputs())
            {
                auto it = writerOf.find(in.PoolIndex);
                if (it != writerOf.end() && it->second != i)
                {
                    adj[it->second].push_back(i);   // j -> i
                    inDegree[i]++;
                }
            }
        }

        // Kahn: Enqueue nodes with in-degree 0, dequeue one by one, and update in-degrees
        std::queue<uint32_t> q;
        for (uint32_t i = 0;i < inDegree.size(); ++i)
            if (inDegree[i] == 0)
                q.push(i);

        std::vector<uint32_t> order;
        while (!q.empty())
        {
            uint32_t cur = q.front();
            q.pop();
            order.push_back(cur);
            for (auto next : adj[cur])
                if (--inDegree[next] == 0)
                    q.push(next);
        }

        TF_CORE_ASSERT(order.size() == m_Passes.size(), "RenderGraph: cyclic dependency detected!");
        return order;
    }

}