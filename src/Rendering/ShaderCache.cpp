#include "stdafx.h"

#include "ShaderCache.h"

#include "RenderDevice.h"
#include <Core/Assert.h>
#include <Core/ComPtr.h>
#include <Core/Log.h>
#include <Core/PerfLog.h>
#include <Core/SimpleFileReader.h>
#include <Core/StringUtils.h>
#include <Core/WinUtils.h>
#include <d3d11.h>
#include <d3d11sdklayers.h>
#include <d3dcommon.h>
#include <D3Dcompiler.h>
#include <sstream>
#include <unordered_map>


namespace sg {
namespace rendering {
namespace shadercache {
//=============================================================================
namespace {
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
struct ShaderBaseInstance
{
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
struct InputLayoutInstance : public RefCountable
{
    ArrayView<D3D11_INPUT_ELEMENT_DESC const> desc;
    comptr<ID3D11InputLayout> inputlayout;
    // TODO: add vs used by validation ? (they are probably a lot)
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
struct VertexShaderInstance : public RefCountable
{
    safeptr<VertexShaderDescriptor const> desc;
    comptr<ID3D11VertexShader> shader;
    comptr<ID3DBlob> blob;
    comptr<ID3D11ShaderReflection> reflection;
#if SHADER_CACHE_ENABLE_RELOADABLE_SHADERS
    u64 fileModificationTimestamp;
    std::vector<std::pair<FilePath, u64> > headerFilesAndTs;
#endif
    // TODO: add binded input layouts for shader update validation ? (they are probably one or a few)
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
struct PixelShaderInstance : public RefCountable
{
    safeptr<PixelShaderDescriptor const> desc;
    comptr<ID3D11PixelShader> shader;
    comptr<ID3D11ShaderReflection> reflection;
#if SHADER_CACHE_ENABLE_RELOADABLE_SHADERS
    u64 fileModificationTimestamp;
    std::vector<std::pair<FilePath, u64> > headerFilesAndTs;
#endif
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class IncludeManager : public ID3D10Include
{
public:
    IncludeManager(FilePath const& iFile)
        : m_pathStack()
        , m_filesStack()
#if SHADER_CACHE_ENABLE_RELOADABLE_SHADERS
        , m_filesAndTimestamps()
#endif
    {
        m_pathStack.push_back(iFile.ParentDirectory().GetSystemFilePath());
    }
    ~IncludeManager()
    {
        SG_ASSERT(m_filesStack.empty());
        SG_ASSERT(m_pathStack.size() == 1);
    }
#if SHADER_CACHE_ENABLE_RELOADABLE_SHADERS
    std::vector<std::pair<FilePath, u64> >& FilesAndTimstamps()
    {
        return m_filesAndTimestamps;
    }
#endif
private:
    DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE Close(LPCVOID pData) override
    {
        SG_UNUSED(pData);
        SG_ASSERT(!m_filesStack.empty());
        SG_ASSERT(pData == m_filesStack.back()->data());
        delete m_filesStack.back();
        m_filesStack.pop_back();
        m_pathStack.pop_back();
        return S_OK;
    }
    DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE Open(
        D3D10_INCLUDE_TYPE IncludeType,
        LPCSTR pFileName,
        LPCVOID pParentData,
        LPCVOID *ppData,
        UINT *pBytes
    ) override
    {
        SG_UNUSED(pParentData);
        switch(IncludeType)
        {
        case D3D10_INCLUDE_LOCAL:
            {
                std::string const& parentpath = m_pathStack.back();
                std::ostringstream oss;
                oss << parentpath << "/" << pFileName;
                FilePath file = FilePath::CreateFromFullSystemPath(oss.str().c_str());
#if SHADER_CACHE_ENABLE_RELOADABLE_SHADERS
                u64 ts = winutils::GetFileModificationTimestamp(file.GetSystemFilePath().c_str());
                m_filesAndTimestamps.push_back(std::make_pair(file, ts));
#endif
                SimpleFileReader* reader = new SimpleFileReader(file);
                if(0 == reader->size())
                    return E_FAIL;
                *ppData = (void*)reader->data();
                *pBytes = (UINT)reader->size();
                m_filesStack.push_back(reader);
                m_pathStack.push_back(file.ParentDirectory().GetSystemFilePath());
            }
            break;
        case D3D10_INCLUDE_SYSTEM:
            SG_ASSERT_NOT_IMPLEMENTED();
            break;
        default:
            SG_ASSERT_NOT_REACHED();
        }
        return S_OK;
    }
    std::vector<std::string> m_pathStack;
    std::vector<SimpleFileReader*> m_filesStack;
#if SHADER_CACHE_ENABLE_RELOADABLE_SHADERS
    std::vector<std::pair<FilePath, u64> > m_filesAndTimestamps;
#endif
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void CreateInputLayout(ID3D11Device* iD3DDevice, ArrayView<D3D11_INPUT_ELEMENT_DESC const> iDescriptor, VertexShaderInstance const* iVSForValidation, InputLayoutInstance* ioInstance)
{
    ID3DBlob* blob = iVSForValidation->blob.get();
    SG_ASSERT(nullptr != blob);

    const void *bytecode = blob->GetBufferPointer();
    size_t bytecodeLength = blob->GetBufferSize();

    HRESULT hr = iD3DDevice->CreateInputLayout(
        iDescriptor.data(),
        (UINT)iDescriptor.size(),
        bytecode,
        bytecodeLength,
        ioInstance->inputlayout.GetPointerForInitialisation()
    );
    SG_ASSERT_AND_UNUSED(SUCCEEDED(hr));
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void CompileFromFile(FilePath const& iFile, char const* iEntryPoint, char const* target, ID3DBlob **opBlob, IncludeManager* includeManager)
{
    std::string srcFile = iFile.GetSystemFilePath();
    std::wstring srcFileW = ConvertUTF8ToUCS2(srcFile);

    D3D10_SHADER_MACRO defines[] = {
#if SG_ENABLE_ASSERT
        { "DEBUG", "true" },
#endif
        { 0, 0 },
    };

    comptr<ID3D10Blob> errorMsgs;

    HRESULT hr;
    bool retry;
    size_t nRetry = 0;
    do
    {
        retry = false;
        {
            SIMPLE_CPU_PERF_LOG_SCOPE("D3DCompileFromFile");
            hr = D3DCompileFromFile(
                srcFileW.c_str(),
                defines,
                includeManager,
                iEntryPoint,
                target,
                D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_OPTIMIZATION_LEVEL3 | D3DCOMPILE_WARNINGS_ARE_ERRORS,
                0,
                opBlob,
                errorMsgs.GetPointerForInitialisation()
                );
        }
        if(FAILED(hr))
        {
            if(nullptr != errorMsgs)
            {
                void* errdata = errorMsgs->GetBufferPointer();
                size_t errsize = errorMsgs->GetBufferSize();
                std::string errstr((char*)errdata, errsize);
                SG_LOG_ERROR("Rendering/Shader", errstr.c_str());
                retry = winutils::ShowModalErrorReturnRetry("Shader compilation failed!", errstr.c_str());
                nRetry = 0;
            }
            else
            {
                DWORD lastErr = GetLastError();
                if(ERROR_SHARING_VIOLATION == lastErr)
                    retry = true;
                else
                    retry = false;

                if(!retry || 5 < ++nRetry)
                {
                    std::ostringstream oss;
                    oss << "D3DCompileFromFile(" << iFile.GetPrintableString() << ") failed with error:" << std::endl;
                    oss << winutils::GetWinLastError() << std::endl;
                    retry = winutils::ShowModalErrorReturnRetry("Shader compilation failed!", oss.str().c_str());
                    nRetry = 0;
                }
            }
        }
    } while(retry);
    SG_ASSERT(SUCCEEDED(hr));

    comptr<ID3DBlob> disassembly;

    hr = D3DDisassemble(
        (*opBlob)->GetBufferPointer(),
        (*opBlob)->GetBufferSize(),
        D3D_DISASM_ENABLE_INSTRUCTION_NUMBERING | D3D_DISASM_ENABLE_DEFAULT_VALUE_PRINTS,
        NULL,
        disassembly.GetPointerForInitialisation()
        );
    SG_ASSERT(SUCCEEDED(hr));

    if(nullptr != disassembly)
    {
        void* disassemblydata = disassembly->GetBufferPointer();
        size_t disassemblysize = disassembly->GetBufferSize();
        std::string disassemblystr((char*)disassemblydata, disassemblysize);
        SG_CODE_FOR_LOG(char const* nInstructionsstr = strstr(disassemblystr.c_str(), "// Approximately");)
        SG_LOG_INFO("Rendering/Shader", "=============================================================================");
        SG_LOG_INFO("Rendering/Shader", iFile.GetPrintableString().c_str());
        //SG_LOG_INFO("Rendering/Shader", disassemblystr.c_str());
        SG_LOG_INFO("Rendering/Shader", nInstructionsstr);
        SG_LOG_INFO("Rendering/Shader", "=============================================================================");
    }
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void CompileShader(ID3D11Device* iD3DDevice, VertexShaderDescriptor const* iDescriptor, VertexShaderInstance *ioShaderInstance)
{
    HRESULT hr = S_OK;

    FilePath const& file = iDescriptor->GetFilePath();
    char const* entryPoint = iDescriptor->EntryPoint().c_str();

    char* target = "vs_4_1";

#if SHADER_CACHE_ENABLE_RELOADABLE_SHADERS
    u64 fileModificationTimestamp = winutils::GetFileModificationTimestamp(file.GetSystemFilePath());
#endif

    IncludeManager includeManager = IncludeManager(file);

    CompileFromFile(file, entryPoint, target, ioShaderInstance->blob.GetPointerForInitialisation(), &includeManager);

    const void *bytecode = ioShaderInstance->blob->GetBufferPointer();
    size_t bytecodeLength = ioShaderInstance->blob->GetBufferSize();
    ID3D11ClassLinkage* classLinkage = nullptr;

    hr = iD3DDevice->CreateVertexShader(
        bytecode,
        bytecodeLength,
        classLinkage,
        ioShaderInstance->shader.GetPointerForInitialisation());
    SG_ASSERT(SUCCEEDED(hr));

    hr = D3DReflect( ioShaderInstance->blob->GetBufferPointer(), ioShaderInstance->blob->GetBufferSize(),
        IID_ID3D11ShaderReflection, (void**)ioShaderInstance->reflection.GetPointerForInitialisation());
    SG_ASSERT(SUCCEEDED(hr));

#if SHADER_CACHE_ENABLE_RELOADABLE_SHADERS
    ioShaderInstance->fileModificationTimestamp = fileModificationTimestamp;
    using std::swap;
    swap(ioShaderInstance->headerFilesAndTs, includeManager.FilesAndTimstamps());
#endif

#if 0
    // Shader introspection example code, not exhaustive.
    {
        ID3D11ShaderReflection* reflector = ioShaderInstance->reflection.get();
        D3D_FEATURE_LEVEL featureLevel;
        hr = reflector->GetMinFeatureLevel(&featureLevel);
        SG_ASSERT(SUCCEEDED(hr));
        D3D11_SHADER_DESC shDesc;
        hr = reflector->GetDesc(&shDesc);
        SG_ASSERT(SUCCEEDED(hr));
        for(UINT i = 0; i < shDesc.ConstantBuffers; ++i)
        {
            ID3D11ShaderReflectionConstantBuffer* cb = reflector->GetConstantBufferByIndex(i);
            SG_ASSERT(nullptr != cb);
            D3D11_SHADER_BUFFER_DESC cbDesc;
            hr = cb->GetDesc(&cbDesc);
            SG_ASSERT(SUCCEEDED(hr));
            for(UINT v = 0; v < cbDesc.Variables; ++v)
            {
                ID3D11ShaderReflectionVariable* var = cb->GetVariableByIndex(v);
                D3D11_SHADER_VARIABLE_DESC varDesc;
                hr = var->GetDesc(&varDesc);
                SG_ASSERT(SUCCEEDED(hr));
                ID3D11ShaderReflectionType* varType = var->GetType();
                SG_ASSERT(nullptr != varType);
                D3D11_SHADER_TYPE_DESC varTypeDesc;
                hr = varType->GetDesc(&varTypeDesc);
                SG_ASSERT(SUCCEEDED(hr));
                for(UINT m = 0; m < varTypeDesc.Members; ++m)
                {
                    ID3D11ShaderReflectionType* memberType = varType->GetMemberTypeByIndex(m);
                    SG_ASSERT(nullptr != memberType);
                    D3D11_SHADER_TYPE_DESC memberTypeDesc;
                    hr = varType->GetDesc(&memberTypeDesc);
                    SG_ASSERT(SUCCEEDED(hr));
                }
            }
        }
        for(UINT i = 0; i < shDesc.InputParameters; ++i)
        {
            D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
            hr = reflector->GetInputParameterDesc(i, &paramDesc);
            SG_ASSERT(SUCCEEDED(hr));
        }
        for(UINT i = 0; i < shDesc.OutputParameters; ++i)
        {
            D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
            hr = reflector->GetOutputParameterDesc(i, &paramDesc);
            SG_ASSERT(SUCCEEDED(hr));
        }
        for(UINT i = 0; i < shDesc.BoundResources; ++i)
        {
            D3D11_SHADER_INPUT_BIND_DESC bindDesc;
            hr = reflector->GetResourceBindingDesc(i, &bindDesc);
            SG_ASSERT(SUCCEEDED(hr));
        }
    }
#endif
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void CompileShader(ID3D11Device* iD3DDevice, PixelShaderDescriptor const* iDescriptor, PixelShaderInstance *ioShaderInstance)
{
    HRESULT hr = S_OK;

    FilePath const& file = iDescriptor->GetFilePath();
    char const* entryPoint = iDescriptor->EntryPoint().c_str();

    char* target = "ps_4_1";

#if SHADER_CACHE_ENABLE_RELOADABLE_SHADERS
    u64 fileModificationTimestamp = winutils::GetFileModificationTimestamp(file.GetSystemFilePath());
#endif

    comptr<ID3DBlob> blob;
    IncludeManager includeManager = IncludeManager(file);

    CompileFromFile(file, entryPoint, target, blob.GetPointerForInitialisation(), &includeManager);

    const void *bytecode = blob->GetBufferPointer();
    size_t bytecodeLength = blob->GetBufferSize();
    ID3D11ClassLinkage* classLinkage = nullptr;
    ID3D11PixelShader** ppPixelShader = ioShaderInstance->shader.GetPointerForInitialisation();

    hr = iD3DDevice->CreatePixelShader(bytecode, bytecodeLength, classLinkage, ppPixelShader);
    SG_ASSERT(SUCCEEDED(hr));

    hr = D3DReflect( blob->GetBufferPointer(), blob->GetBufferSize(),
        IID_ID3D11ShaderReflection, (void**)ioShaderInstance->reflection.GetPointerForInitialisation());
    SG_ASSERT(SUCCEEDED(hr));

#if SHADER_CACHE_ENABLE_RELOADABLE_SHADERS
    ioShaderInstance->fileModificationTimestamp = fileModificationTimestamp;
    using std::swap;
    swap(ioShaderInstance->headerFilesAndTs, includeManager.FilesAndTimstamps());
#endif
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
}
//=============================================================================
class ShaderCache
{
public:
    ShaderCache()
        : m_renderDevice()
        , m_vertexShadersMap()
        , m_vertexShaders()
        , m_pixelShadersMap()
        , m_pixelShaders()
    {
    }
    ~ShaderCache()
    {
    }
    void SetRenderDevice(RenderDevice* iRenderDevice)
    {
        SG_ASSERT(nullptr == iRenderDevice || nullptr == m_renderDevice);
        Clear();
        m_renderDevice = iRenderDevice;
    }
    ShaderInputLayoutProxy GetProxy(ArrayView<D3D11_INPUT_ELEMENT_DESC const> iDescriptor, VertexShaderProxy iVertexShaderForValidation)
    {
        auto f = m_inputLayoutsMap.find(iDescriptor.data());
        if(f != m_inputLayoutsMap.end())
        {
            size_t const id = f->second;
            InputLayoutInstance* inst = m_inputLayouts[id].get();
            if(nullptr == inst->inputlayout)
            {
                VertexShaderInstance* vsInst = GetInstance(iVertexShaderForValidation);
                CreateInputLayout(m_renderDevice->D3DDevice(), inst->desc, vsInst, inst);
            }
            return ShaderInputLayoutProxy(id);
        }
        else
        {
            size_t const id = m_inputLayouts.size();
            VertexShaderInstance* vsInst = GetInstance(iVertexShaderForValidation);
            InputLayoutInstance* inst = new InputLayoutInstance();
            inst->desc = iDescriptor;
            CreateInputLayout(m_renderDevice->D3DDevice(), inst->desc, vsInst, inst);
            m_inputLayouts.push_back(inst);
            auto r = m_inputLayoutsMap.insert(std::make_pair(iDescriptor.data(), id));
            SG_ASSERT(r.second);
            return ShaderInputLayoutProxy(id);
        }
    }
    VertexShaderProxy GetProxy(VertexShaderDescriptor const* iDescriptor)
    {
        auto f = m_vertexShadersMap.find(iDescriptor);
        if(f != m_vertexShadersMap.end())
        {
            size_t const id = f->second;
            return VertexShaderProxy(id);
        }
        else
        {
            size_t const id = m_vertexShaders.size();
            VertexShaderInstance* inst = new VertexShaderInstance();
            inst->desc = iDescriptor;
            m_vertexShaders.push_back(inst);
            refptr<VertexShaderDescriptor const> refdesc(iDescriptor); // TODO: comment ne pas construire de refptr inutile ?
            auto r = m_vertexShadersMap.insert(std::make_pair(refdesc, id));
            SG_ASSERT(r.second);
            return VertexShaderProxy(id);
        }
    }
    PixelShaderProxy GetProxy(PixelShaderDescriptor const* iDescriptor)
    {
        auto f = m_pixelShadersMap.find(iDescriptor);
        if(f != m_pixelShadersMap.end())
        {
            size_t const id = f->second;
            return PixelShaderProxy(id);
        }
        else
        {
            size_t const id = m_pixelShaders.size();
            PixelShaderInstance* inst = new PixelShaderInstance();
            inst->desc = iDescriptor;
            m_pixelShaders.push_back(inst);
            refptr<PixelShaderDescriptor const> refdesc(iDescriptor); // TODO: comment ne pas construire de refptr inutile ?
            auto r = m_pixelShadersMap.insert(std::make_pair(refdesc, id));
            SG_ASSERT(r.second);
            return PixelShaderProxy(id);
        }
    }
    ID3D11InputLayout* GetInputLayout(ShaderInputLayoutProxy iProxy)
    {
        SG_ASSERT(m_inputLayouts.size() > iProxy.id());
        InputLayoutInstance* inst = m_inputLayouts[iProxy.id()].get();
        SG_ASSERT(nullptr != inst);
        /*
        if(nullptr == inst->inputlayout)
            CompileShader(m_renderDevice->D3DDevice(), inst->desc.get(), inst);
            */
        SG_ASSERT(nullptr != inst->inputlayout);
        return inst->inputlayout.get();
    }
    ArrayView<D3D11_INPUT_ELEMENT_DESC const> GetInputLayoutDescriptor(ShaderInputLayoutProxy iProxy)
    {
        SG_ASSERT(m_inputLayouts.size() > iProxy.id());
        InputLayoutInstance* inst = m_inputLayouts[iProxy.id()].get();
        SG_ASSERT(nullptr != inst);
        return inst->desc;
    }
    VertexShaderInstance* GetInstance(VertexShaderProxy iProxy)
    {
        SG_ASSERT(m_vertexShaders.size() > iProxy.id());
        VertexShaderInstance* inst = m_vertexShaders[iProxy.id()].get();
        SG_ASSERT(nullptr != inst);
        if(nullptr == inst->shader)
            CompileShader(m_renderDevice->D3DDevice(), inst->desc.get(), inst);
        SG_ASSERT(nullptr != inst->shader);
        return inst;
    }
    PixelShaderInstance* GetInstance(PixelShaderProxy iProxy)
    {
        SG_ASSERT(m_pixelShaders.size() > iProxy.id());
        PixelShaderInstance* inst = m_pixelShaders[iProxy.id()].get();
        SG_ASSERT(nullptr != inst);
        if(nullptr == inst->shader)
            CompileShader(m_renderDevice->D3DDevice(), inst->desc.get(), inst);
        SG_ASSERT(nullptr != inst->shader);
        return inst;
    }
    ID3D11VertexShader* GetShader(VertexShaderProxy iProxy)
    {
        VertexShaderInstance* inst = GetInstance(iProxy);
        SG_ASSERT(nullptr != inst);
        SG_ASSERT(nullptr != inst->shader);
        return inst->shader.get();
    }
    ID3D11PixelShader* GetShader(PixelShaderProxy iProxy)
    {
        PixelShaderInstance* inst = GetInstance(iProxy);
        SG_ASSERT(nullptr != inst);
        SG_ASSERT(nullptr != inst->shader);
        return inst->shader.get();
    }
    ID3D11ShaderReflection* GetReflection(VertexShaderProxy iProxy)
    {
        VertexShaderInstance* inst = GetInstance(iProxy);
        SG_ASSERT(nullptr != inst);
        SG_ASSERT(nullptr != inst->reflection);
        return inst->reflection.get();
    }
    ID3D11ShaderReflection* GetReflection(PixelShaderProxy iProxy)
    {
        SG_ASSERT(m_pixelShaders.size() > iProxy.id());
        PixelShaderInstance* inst = m_pixelShaders[iProxy.id()].get();
        SG_ASSERT(nullptr != inst);
        if(nullptr == inst->reflection)
            CompileShader(m_renderDevice->D3DDevice(), inst->desc.get(), inst);
        SG_ASSERT(nullptr != inst->reflection);
        return inst->reflection.get();
    }
#if SHADER_CACHE_ENABLE_RELOADABLE_SHADERS
    void InvalidateOutdatedShaders()
    {
        for(auto it : m_vertexShaders)
        {
            FilePath const& file = it->desc->GetFilePath();
            u64 const ts = winutils::GetFileModificationTimestamp(file.GetSystemFilePath().c_str());
            if(ts != it->fileModificationTimestamp)
            {
                it->shader = nullptr;
                continue;
            }
            for(auto h : it->headerFilesAndTs)
            {
                u64 const hts = winutils::GetFileModificationTimestamp(h.first.GetSystemFilePath().c_str());
                if(hts != h.second)
                {
                    it->shader = nullptr;
                    break;
                }
            }
        }
        for(auto it : m_pixelShaders)
        {
            FilePath const& file = it->desc->GetFilePath();
            u64 const ts = winutils::GetFileModificationTimestamp(file.GetSystemFilePath().c_str());
            if(ts != it->fileModificationTimestamp)
            {
                it->shader = nullptr;
                continue;
            }
            for(auto h : it->headerFilesAndTs)
            {
                u64 const hts = winutils::GetFileModificationTimestamp(h.first.GetSystemFilePath().c_str());
                if(hts != h.second)
                {
                    it->shader = nullptr;
                    break;
                }
            }
        }
    }
#endif
    void InvalidateAllShaders()
    {
        for(auto it : m_vertexShaders)
            it->shader = nullptr;
        for(auto it : m_pixelShaders)
            it->shader = nullptr;
    }
private:
    void Clear()
    {
        m_renderDevice = nullptr;
        m_inputLayouts.clear();
        m_inputLayoutsMap.clear();
        m_vertexShaders.clear();
        m_vertexShadersMap.clear();
        m_pixelShaders.clear();
        m_pixelShadersMap.clear();
    }
private:
    struct HashShaderDesciptor
    {
        size_t operator()(refptr<VertexShaderDescriptor const> const& a) const { return a->Hash(); }
        size_t operator()(VertexShaderDescriptor const* a) const { return a->Hash(); }
        size_t operator()(refptr<PixelShaderDescriptor const> const& a) const { return a->Hash(); }
        size_t operator()(PixelShaderDescriptor const* a) const { return a->Hash(); }
    };
    struct CompareShaderDescriptor
    {
        size_t operator()(refptr<VertexShaderDescriptor const> const& a, refptr<VertexShaderDescriptor const> const& b) const { return a->Equals(*b); }
        size_t operator()(refptr<VertexShaderDescriptor const> const& a, VertexShaderDescriptor const* b) const { return a->Equals(*b); }
        size_t operator()(VertexShaderDescriptor const* a, refptr<VertexShaderDescriptor const> const& b) const { return a->Equals(*b); }
        size_t operator()(VertexShaderDescriptor const* a, VertexShaderDescriptor const* b) const { return a->Equals(*b); }
        size_t operator()(refptr<PixelShaderDescriptor const> const& a, refptr<PixelShaderDescriptor const> const& b) const { return a->Equals(*b); }
        size_t operator()(refptr<PixelShaderDescriptor const> const& a, PixelShaderDescriptor const* b) const { return a->Equals(*b); }
        size_t operator()(PixelShaderDescriptor const* a, refptr<PixelShaderDescriptor const> const& b) const { return a->Equals(*b); }
        size_t operator()(PixelShaderDescriptor const* a, PixelShaderDescriptor const* b) const { return a->Equals(*b); }
    };
    safeptr<RenderDevice> m_renderDevice;
    std::unordered_map<D3D11_INPUT_ELEMENT_DESC const*, size_t> m_inputLayoutsMap;
    std::vector<refptr<InputLayoutInstance> > m_inputLayouts;
    std::unordered_map<refptr<VertexShaderDescriptor const>, size_t, HashShaderDesciptor, CompareShaderDescriptor> m_vertexShadersMap;
    std::vector<refptr<VertexShaderInstance> > m_vertexShaders;
    std::unordered_map<refptr<PixelShaderDescriptor const>, size_t, HashShaderDesciptor, CompareShaderDescriptor> m_pixelShadersMap;
    std::vector<refptr<PixelShaderInstance> > m_pixelShaders;
};
//=============================================================================
namespace {
ShaderCache* g_shaderCache = nullptr;
}
//=============================================================================
void Init()
{
    SG_ASSERT(nullptr == g_shaderCache);
    g_shaderCache = new ShaderCache();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void SetRenderDevice(RenderDevice* iRenderDevice)
{
    SG_ASSERT(nullptr != g_shaderCache);
    return g_shaderCache->SetRenderDevice(iRenderDevice);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ShaderInputLayoutProxy GetProxy(ArrayView<D3D11_INPUT_ELEMENT_DESC const> iDescriptor, VertexShaderProxy iVertexShaderForValidation)
{
    SG_ASSERT(nullptr != g_shaderCache);
    return g_shaderCache->GetProxy(iDescriptor, iVertexShaderForValidation);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
VertexShaderProxy GetProxy(VertexShaderDescriptor const* iDescriptor)
{
    SG_ASSERT(nullptr != g_shaderCache);
    return g_shaderCache->GetProxy(iDescriptor);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
PixelShaderProxy GetProxy(PixelShaderDescriptor const* iDescriptor)
{
    SG_ASSERT(nullptr != g_shaderCache);
    return g_shaderCache->GetProxy(iDescriptor);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ArrayView<D3D11_INPUT_ELEMENT_DESC const> GetInputLayoutDescriptor(ShaderInputLayoutProxy iProxy)
{
    SG_ASSERT(nullptr != g_shaderCache);
    return g_shaderCache->GetInputLayoutDescriptor(iProxy);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ID3D11InputLayout* GetInputLayout(ShaderInputLayoutProxy iProxy)
{
    SG_ASSERT(nullptr != g_shaderCache);
    return g_shaderCache->GetInputLayout(iProxy);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ID3D11VertexShader* GetShader(VertexShaderProxy iProxy)
{
    SG_ASSERT(nullptr != g_shaderCache);
    return g_shaderCache->GetShader(iProxy);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ID3D11PixelShader* GetShader(PixelShaderProxy iProxy)
{
    SG_ASSERT(nullptr != g_shaderCache);
    return g_shaderCache->GetShader(iProxy);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#if SHADER_CACHE_ENABLE_RELOADABLE_SHADERS
u64 GetShaderTimestamp(VertexShaderProxy iProxy)
{
    SG_ASSERT(nullptr != g_shaderCache);
    VertexShaderInstance* inst = g_shaderCache->GetInstance(iProxy);
    return inst->fileModificationTimestamp;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
u64 GetShaderTimestamp(PixelShaderProxy iProxy)
{
    SG_ASSERT(nullptr != g_shaderCache);
    PixelShaderInstance* inst = g_shaderCache->GetInstance(iProxy);
    return inst->fileModificationTimestamp;
}
#endif
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ID3D11ShaderReflection* GetReflection(VertexShaderProxy iProxy)
{
    SG_ASSERT(nullptr != g_shaderCache);
    return g_shaderCache->GetReflection(iProxy);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ID3D11ShaderReflection* GetReflection(PixelShaderProxy iProxy)
{
    SG_ASSERT(nullptr != g_shaderCache);
    return g_shaderCache->GetReflection(iProxy);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
#if SHADER_CACHE_ENABLE_RELOADABLE_SHADERS
void InvalidateOutdatedShaders()
{
    SG_ASSERT(nullptr != g_shaderCache);
    return g_shaderCache->InvalidateOutdatedShaders();
}
#endif
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void InvalidateAllShaders()
{
    SG_ASSERT(nullptr != g_shaderCache);
    return g_shaderCache->InvalidateAllShaders();
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
void Shutdown()
{
    SG_ASSERT(nullptr != g_shaderCache);
    delete g_shaderCache;
    g_shaderCache = nullptr;
}
//=============================================================================
}
}
}
