#ifndef Rendering_Shader_H
#define Rendering_Shader_H

#include <Core/FilePath.h>
#include <Core/SmartPtr.h>
#include <Core/ArrayView.h>
#include <Core/Utils.h>

struct D3D11_INPUT_ELEMENT_DESC;
struct ID3D11InputLayout;
struct ID3D11PixelShader;
struct ID3D11ShaderReflection;
struct ID3D11VertexShader;

namespace sg {
namespace rendering {
namespace shadercache {
    class ShaderCache;
}
//=============================================================================
struct ShaderBaseProxy
{
    SG_NON_NEWABLE
public:
    ShaderBaseProxy (ShaderBaseProxy const&) = default;
#if 0 // en atente du C++1
    ShaderBaseProxy const& operator=(ShaderBaseProxy const&) = default;
#endif
public:
    bool IsValid() const { return -1 != m_id; }
#if SG_ENABLE_ASSERT
    // To prevent incorrect use of id by other client than ShaderCache
    size_t Hash() const { size_t h=m_id^0xf523d954; h^=m_id<<15; h^=m_id>>7; return h; }
#else
    size_t Hash() const { return m_id; }
#endif
protected:
    ShaderBaseProxy() : m_id(all_ones) {}
    ShaderBaseProxy(size_t iId) : m_id(iId) {}
    size_t id() const { return m_id; }
private:
    size_t m_id;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
struct VertexShaderProxy : private ShaderBaseProxy
{
public:
    VertexShaderProxy() {}
    ID3D11VertexShader* GetShader() const;
    ID3D11ShaderReflection* GetReflection() const;
    using ShaderBaseProxy::IsValid;
    using ShaderBaseProxy::Hash;
    friend bool operator==(VertexShaderProxy const& a, VertexShaderProxy const& b) { return a.id() == b.id(); }
private:
    friend class shadercache::ShaderCache;
    VertexShaderProxy(size_t iId) : ShaderBaseProxy(iId) {}
};
inline bool operator!=(VertexShaderProxy const& a, VertexShaderProxy const& b) { return !(a == b); }
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
struct PixelShaderProxy : private ShaderBaseProxy
{
public:
    PixelShaderProxy() {}
    ID3D11PixelShader* GetShader() const;
    ID3D11ShaderReflection* GetReflection() const;
    using ShaderBaseProxy::IsValid;
    using ShaderBaseProxy::Hash;
    friend bool operator==(PixelShaderProxy const& a, PixelShaderProxy const& b) { return a.id() == b.id(); }
private:
    friend class shadercache::ShaderCache;
    PixelShaderProxy(size_t iId) : ShaderBaseProxy(iId) {}
};
inline bool operator!=(PixelShaderProxy const& a, PixelShaderProxy const& b) { return !(a == b); }
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
struct ShaderInputLayoutProxy : private ShaderBaseProxy
{
public:
    ShaderInputLayoutProxy() {}
    ShaderInputLayoutProxy(ArrayView<D3D11_INPUT_ELEMENT_DESC const> iDescriptor, VertexShaderProxy iVertexShaderForValidation);
    ArrayView<D3D11_INPUT_ELEMENT_DESC const> GetDescriptor() const;
    ID3D11InputLayout* GetInputLayout() const;
    using ShaderBaseProxy::Hash;
    friend bool operator==(ShaderInputLayoutProxy const& a, ShaderInputLayoutProxy const& b) { return a.id() == b.id(); }
private:
    friend class shadercache::ShaderCache;
    ShaderInputLayoutProxy(size_t iId) : ShaderBaseProxy(iId) {}
};
inline bool operator!=(ShaderInputLayoutProxy const& a, ShaderInputLayoutProxy const& b) { return !(a == b); }
//=============================================================================
class ShaderBaseDescriptor
{
public:
    FilePath const& GetFilePath() const { return m_filepath; }
    std::string const& EntryPoint() const { return m_entryPoint; }
protected:
    ShaderBaseDescriptor(FilePath const& iFilePath, std::string const& iEntryPoint)
        : m_filepath(iFilePath)
        , m_entryPoint(iEntryPoint)
    { }
    size_t Hash() const;
    bool Equals(ShaderBaseDescriptor const& b) const;
private:
    FilePath m_filepath;
    std::string m_entryPoint;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class VertexShaderDescriptor : public RefAndSafeCountable
                             , public ShaderBaseDescriptor
{
public:
    VertexShaderDescriptor(FilePath const& iFilePath, std::string const& iEntryPoint)
        : ShaderBaseDescriptor(iFilePath, iEntryPoint) {}
    VertexShaderProxy GetProxy() const;
    size_t Hash() const;
    bool Equals(VertexShaderDescriptor const& b) const;
private:
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class PixelShaderDescriptor : public RefAndSafeCountable
                            , public ShaderBaseDescriptor
{
public:
    PixelShaderDescriptor(FilePath const& iFilePath, std::string const& iEntryPoint)
        : ShaderBaseDescriptor(iFilePath, iEntryPoint) {}
    PixelShaderProxy GetProxy() const;
    size_t Hash() const;
    bool Equals(PixelShaderDescriptor const& b) const;
private:
};
//=============================================================================
}
}

#endif
