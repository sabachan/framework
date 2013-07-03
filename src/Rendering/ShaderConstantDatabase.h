#ifndef Rendering_ShaderConstantDatabase_H
#define Rendering_ShaderConstantDatabase_H

#include <Core/Assert.h>
#include <Core/FastSymbol.h>
#include <Core/IntTypes.h>
#include <Core/SmartPtr.h>
#include <Math/Matrix.h>
#include <Math/Vector.h>

#include <unordered_map>

namespace sg {
namespace rendering {
//=============================================================================
class IShaderVariable : public RefAndSafeCountable
{
public:
public:
    virtual ~IShaderVariable() {}
    //virtual size_t Rows() = 0;
    //virtual size_t Columns() = 0;
    //virtual size_t Elements() = 0;
    virtual void WriteInBuffer(void* iBuffer, size_t iMaxSize) const = 0;
};
//=============================================================================
template <typename T> struct ShaderValueTrait { static const bool is_valid_type = false; };
template <> struct ShaderValueTrait<bool>  { static const bool is_valid_type = true; typedef u32   base_type; typedef u32   storage_type; inline static storage_type ToStorageType(base_type v) { return v?1:0; } inline static base_type FromStorageType(storage_type v) { return v!=0; } };
template <> struct ShaderValueTrait<u32>   { static const bool is_valid_type = true; typedef u32   base_type; typedef u32   storage_type; inline static storage_type ToStorageType(base_type v) { return v; }     inline static base_type FromStorageType(storage_type v) { return v; }    };
template <> struct ShaderValueTrait<i32>   { static const bool is_valid_type = true; typedef i32   base_type; typedef i32   storage_type; inline static storage_type ToStorageType(base_type v) { return v; }     inline static base_type FromStorageType(storage_type v) { return v; }    };
template <> struct ShaderValueTrait<float> { static const bool is_valid_type = true; typedef float base_type; typedef float storage_type; inline static storage_type ToStorageType(base_type v) { return v; }     inline static base_type FromStorageType(storage_type v) { return v; }    };
//=============================================================================
template <typename T>
class ShaderVariable : public IShaderVariable
{
    typedef typename ShaderValueTrait<T>::storage_type storage_type;
public:
    void Set(T v) { m_value = ShaderValueTrait<T>::ToStorageType(v); }
    T Get() { return ShaderValueTrait<T>::FromStorageType(m_value); }
    virtual void WriteInBuffer(void* iBuffer, size_t iMaxSize) const override
    {
        SG_ASSERT_AND_UNUSED(iMaxSize == sizeof(storage_type));
        SG_ASSERT_AND_UNUSED(iMaxSize >= sizeof(storage_type));
        memcpy(iBuffer, &m_value, sizeof(storage_type));
    }
private:
    storage_type m_value;
};
//=============================================================================
template <typename T, size_t dim>
class ShaderVariable <math::Vector<T, dim> > : public IShaderVariable
{
    typedef math::Vector<T, dim> base_type;
    typedef math::Vector<typename ShaderValueTrait<T>::storage_type, dim> storage_type;
public:
    void Set(size_t i, T v) { m_value[i] = ShaderValueTrait<T>::ToStorageType(v); }
    T Get(size_t i) { return ShaderValueTrait<T>::FromStorageType(m_value[i]); }
    storage_type const& Get() { return m_values; }
    void Set(base_type const& v) { for(size_t i=0; i<dim; ++i) { m_value[i] = ShaderValueTrait<T>::ToStorageType(v[i]); } }
    virtual void WriteInBuffer(void* iBuffer, size_t iMaxSize) const override
    {
        SG_ASSERT_AND_UNUSED(iMaxSize == sizeof(storage_type));
        SG_ASSERT_AND_UNUSED(iMaxSize >= sizeof(storage_type));
        memcpy(iBuffer, &m_value, sizeof(storage_type));
    }
private:
    storage_type m_value;
};
//=============================================================================
template <typename T, size_t dim_m, size_t dim_n, math::MatrixOrder order>
class ShaderVariable <math::Matrix<T, dim_m, dim_n, order> > : public IShaderVariable
{
    typedef math::Matrix<T, dim_m, dim_n, order> base_type;
    typedef math::Matrix<typename ShaderValueTrait<T>::storage_type, dim_m, dim_n, order> storage_type;
public:
    void Set(size_t i, size_t j, T v) { m_value(i,j) = ShaderValueTrait<T>::ToStorageType(v); }
    T Get(size_t i, size_t j) { return ShaderValueTrait<T>::FromStorageType(m_value(i,j)); }
    storage_type const& Get() { return m_value; }
    void Set(base_type const& v) { for(size_t i=0; i<dim_m; ++i) { for(size_t j=0; j<dim_n; ++j) { m_value(i,j) = ShaderValueTrait<T>::ToStorageType(v(i,j)); } } }
    virtual void WriteInBuffer(void* iBuffer, size_t iMaxSize) const override
    {
        SG_UNUSED(iMaxSize);
        SG_ASSERT(iMaxSize == sizeof(storage_type));
        SG_ASSERT(iMaxSize >= sizeof(storage_type));
        memcpy(iBuffer, &m_value, sizeof(storage_type));
    }
private:
    storage_type m_value;
};
//=============================================================================
FAST_SYMBOL_TYPE_HEADER(ShaderConstantName)
//=============================================================================
class IShaderConstantDatabase : public RefAndSafeCountable
{
public:
    virtual ~IShaderConstantDatabase() {}
    IShaderVariable const* GetConstant(ShaderConstantName iName) const;
    virtual IShaderVariable const* GetConstantIFP(ShaderConstantName iName) const = 0;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
class ShaderConstantDatabase : public IShaderConstantDatabase
{
public:
    ShaderConstantDatabase();
    virtual ~ShaderConstantDatabase() override;
    void AddVariable(ShaderConstantName iName, IShaderVariable* iValue);
    IShaderVariable* GetConstantForWriting(ShaderConstantName iName);
    virtual IShaderVariable const* GetConstantIFP(ShaderConstantName iName) const override;
    std::unordered_map<ShaderConstantName, refptr<IShaderVariable> > const& Constants() const { return m_constants; }
private:
    std::unordered_map<ShaderConstantName, refptr<IShaderVariable> > m_constants;
};
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <bool keepRefOnFirst, bool keepRefOnSecond>
class ShaderConstantDatabasePair : public IShaderConstantDatabase
{
    SG_NON_COPYABLE(ShaderConstantDatabasePair)
public:
    ShaderConstantDatabasePair(IShaderConstantDatabase const* iFirstDatabase, IShaderConstantDatabase const* iSecondDatabase);
    virtual ~ShaderConstantDatabasePair() override;
    virtual IShaderVariable const* GetConstantIFP(ShaderConstantName iName) const override;
private:
    std::unordered_map<ShaderConstantName, refptr<IShaderVariable> > m_constants;
    reforsafeptr<IShaderConstantDatabase const, keepRefOnFirst> const m_firstDatabase;
    reforsafeptr<IShaderConstantDatabase const, keepRefOnSecond> const m_secondDatabase;
};
//=============================================================================
}
}

#endif
