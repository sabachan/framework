#include "stdafx.h"

#include "RenderStateDico.h"

#include "RenderDevice.h"
#include <Core/ArrayList.h>
#include <Core/Assert.h>
#include <Core/ComPtr.h>
#include <unordered_map>

#include "WTF/IncludeD3D11.h"

namespace sg {
namespace rendering {
//=============================================================================
FAST_SYMBOL_TYPE_IMPL(RenderStateName)
//=============================================================================
namespace renderstatedico {
//=============================================================================
namespace {
    template<typename RS, typename RSDescriptor, typename Creator>
    class RenderStateDicoImpl
    {
    public:
        void clear()
        {
            m_renderStatesFromName.clear();
            m_renderStatesFromDesc.clear();
            m_renderStates.clear();
        }
        RS* GetRenderState(ID3D11Device* iD3DDevice, RenderStateName iName, RSDescriptor const* iDescIfNeedCreation)
        {
            auto insertResult = m_renderStatesFromName.insert(std::make_pair(iName, -1));
            if(-1 == insertResult.first->second)
            {
#if SG_ENABLE_ASSERT
                if(insertResult.second) // fails only first time
                {
                    SG_ASSERT_MSG(nullptr != iDescIfNeedCreation, "Render state not found!");
                }
#endif
                if(nullptr == iDescIfNeedCreation)
                    return nullptr;

                auto f = m_renderStatesFromDesc.find(iDescIfNeedCreation);
                if(m_renderStatesFromDesc.end() == f)
                {
                    size_t const index = m_renderStates.size();

                    m_renderStates.emplace_back();
                    auto& back = m_renderStates.back();
                    back.first.reset(new RSDescriptor(*iDescIfNeedCreation));
                    Creator creator;
                    creator(iD3DDevice, back.second, *back.first);
                    SG_ASSERT(m_renderStates[index].first->Equals(*iDescIfNeedCreation));
#if SG_ENABLE_ASSERT
                    SetDebugName(back.second.get(), iName.Value());
#endif

                    insertResult.first->second = index;
                    auto r = m_renderStatesFromDesc.insert(std::make_pair(back.first.get(), index));
                    SG_ASSERT(r.second);

                    return back.second.get();
                }
                else
                {
                    size_t const index = f->second;
                    auto r = m_renderStatesFromDesc.insert(std::make_pair(m_renderStates[index].first.get(), index));
                    SG_ASSERT(r.second);
                    SG_ASSERT(index < m_renderStates.size());
                    SG_ASSERT(m_renderStates[index].first->Equals(*iDescIfNeedCreation));
                    return m_renderStates[index].second.get();

                }
            }
            else
            {
                size_t const index = insertResult.first->second;
                SG_ASSERT(index < m_renderStates.size());
                SG_ASSERT(nullptr == iDescIfNeedCreation || m_renderStates[index].first->Equals(*iDescIfNeedCreation));
                return m_renderStates[index].second.get();
            }
        }
    private:
        struct Hash
        {
            size_t operator() (safeptr<RSDescriptor const> const& k) const { return k->Hash(); }
            size_t operator() (RSDescriptor const* k) const { return k->Hash(); }
        };
        struct Pred
        {
            bool operator() (safeptr<RSDescriptor const> const& a, safeptr<RSDescriptor const> const& b) const { return a->Equals(*b); }
            bool operator() (RSDescriptor const* a, safeptr<RSDescriptor const> const& b) const { return a->Equals(*b); }
            bool operator() (safeptr<RSDescriptor const> const& a, RSDescriptor const* b) const { return a->Equals(*b); }
            bool operator() (RSDescriptor const* a, RSDescriptor const* b) const { return a->Equals(*b); }
        };
        std::unordered_map<RenderStateName, size_t> m_renderStatesFromName;
        std::unordered_map<safeptr<RSDescriptor const>, size_t, Hash, Pred> m_renderStatesFromDesc;
        ArrayList<std::pair<scopedptr<RSDescriptor>, comptr<RS> > > m_renderStates;
    };
    struct BlendStateCreator
    {
        void operator() (ID3D11Device* iD3DDevice, comptr<ID3D11BlendState>& oBlendState, BlendStateDescriptor const& iDesc)
        {
            D3D11_BLEND_DESC const& blendDesc = iDesc.GetDesc();
            HRESULT hr = iD3DDevice->CreateBlendState(&blendDesc, oBlendState.GetPointerForInitialisation());
            SG_ASSERT_AND_UNUSED(SUCCEEDED(hr));
        }
    };
    class RenderStateDico
    {
    public:
        void SetRenderDevice(RenderDevice* iRenderDevice)
        {
            SG_ASSERT(nullptr == iRenderDevice || nullptr == m_renderDevice);
            m_renderDevice = iRenderDevice;
            m_blendStateDico.clear();
        }
        ID3D11BlendState* GetBlendState(RenderStateName iName, BlendStateDescriptor const* iDescIfNeedCreation)
        {
            return m_blendStateDico.GetRenderState(m_renderDevice->D3DDevice(), iName, iDescIfNeedCreation);
        }
        ID3D11DepthStencilState* GetDepthStencilState(RenderStateName iName, DepthStencilDescriptor const* iDescIfNeedCreation)
        {
            SG_UNUSED((iName, iDescIfNeedCreation));
            SG_ASSERT_NOT_IMPLEMENTED();
            return nullptr;
        }
        ID3D11RasterizerState* GetRasterizerState(RenderStateName iName, RasterizerStateDescriptor const* iDescIfNeedCreation)
        {
            SG_UNUSED((iName, iDescIfNeedCreation));
            SG_ASSERT_NOT_IMPLEMENTED();
            return nullptr;
        }
    private:
        safeptr<RenderDevice> m_renderDevice;
        RenderStateDicoImpl<ID3D11BlendState, BlendStateDescriptor, BlendStateCreator> m_blendStateDico;
    };
    RenderStateDico* s_dico = nullptr;
}
//=============================================================================
void Init()
{
    SG_ASSERT(nullptr == s_dico);
    s_dico = new RenderStateDico;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void SetRenderDevice(RenderDevice* iRenderDevice)
{
    SG_ASSERT(nullptr != s_dico);
    s_dico->SetRenderDevice(iRenderDevice);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ID3D11BlendState* GetBlendState(RenderStateName iName, BlendStateDescriptor const* iDescIfNeedCreation)
{
    SG_ASSERT(nullptr != s_dico);
    return s_dico->GetBlendState(iName, iDescIfNeedCreation);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ID3D11DepthStencilState* GetDepthStencilState(RenderStateName iName, DepthStencilDescriptor const* iDescIfNeedCreation)
{
    SG_ASSERT(nullptr != s_dico);
    return s_dico->GetDepthStencilState(iName, iDescIfNeedCreation);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ID3D11RasterizerState* GetRasterizerState(RenderStateName iName, RasterizerStateDescriptor const* iDescIfNeedCreation)
{
    SG_ASSERT(nullptr != s_dico);
    return s_dico->GetRasterizerState(iName, iDescIfNeedCreation);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Shutdown()
{
    SG_ASSERT(nullptr != s_dico);
    delete s_dico;
    s_dico = nullptr;
}
//=============================================================================
}
}
}
