#ifndef Image_ImageView_H
#define Image_ImageView_H

#include <Core/ArrayView.h>
#include <Core/For.h>
#include <Core/SmartPtr.h>
#include <Core/TemplateUtils.h>
#include <Math/Box.h>
#include <Math/Vector.h>

namespace sg {
namespace image {
//=============================================================================
#if SG_ENABLE_ASSERT
class CheckerForImageView
{
public:
    CheckerForImageView() { SetInvalid(); }
    void OnAcquire() { SG_ASSERT(!IsValid()); new(&m_mappedSafeCountable) SafeCountable; }
    void OnRelease() { SG_ASSERT(IsValid()); reinterpret_cast<SafeCountable*>(&m_mappedSafeCountable)->~SafeCountable(); SetInvalid(); SG_ASSERT(!IsValid()); }
    SafeCountable const* GetSafeCountable() const { SG_ASSERT(IsValid()); return reinterpret_cast<SafeCountable const*>(&m_mappedSafeCountable); }
private:
    bool IsValid() const { static_assert(sizeof(intptr_t) == sizeof(m_mappedSafeCountable), ""); return all_ones != *reinterpret_cast<intptr_t const*>(&m_mappedSafeCountable); }
    void SetInvalid() { *reinterpret_cast<intptr_t*>(&m_mappedSafeCountable) = all_ones; }
private:
    typename std::aligned_storage<sizeof(SafeCountable), alignof(SafeCountable)>::type m_mappedSafeCountable;
};
#endif
//=============================================================================
template <typename T> class ImageView;
template <typename T>
class AbstractImage
{
protected:
    template <typename T2> friend class AbstractImage;
    typedef T value_type;
    typedef typename ConstPassing<value_type>::type value_type_for_const_passing;
    typedef typename std::conditional<std::is_const<T>::value, void const, void>::type correct_constness_void_type;
    typedef typename std::conditional<std::is_const<T>::value, u8 const, u8>::type correct_constness_byte_type;
public:
    uint2 const& WidthHeight() const { return m_size; }
    void Fill(value_type_for_const_passing iValue);
    value_type_for_const_passing operator() (size_t x, size_t y) const { SG_ASSERT(x < m_size.x()); SG_ASSERT(y < m_size.y()); return reinterpret_cast<value_type const*>(m_data + y * m_strideInBytes)[x]; }
    value_type& operator() (size_t x, size_t y) { SG_ASSERT(x < m_size.x()); SG_ASSERT(y < m_size.y()); return reinterpret_cast<value_type*>(m_data + y * m_strideInBytes)[x]; }
    value_type_for_const_passing operator() (uint2 const& p) const { return operator()(p.x(), p.y()); }
    value_type& operator() (uint2 const& p) { return operator()(p.x(), p.y()); }
    ArrayView<value_type const> Row(size_t y) const { SG_ASSERT(y < m_size.y()); return ArrayView<value_type const>(reinterpret_cast<value_type const*>(m_data + y * m_strideInBytes), m_size.x()); }
    ArrayView<value_type> Row(size_t y) { SG_ASSERT(y < m_size.y()); return ArrayView<value_type>(reinterpret_cast<value_type*>(m_data + y * m_strideInBytes), m_size.x()); }
    correct_constness_byte_type* Buffer() { return m_data; }
    u8 const* Buffer() const { return m_data; }
    uint2 const& Size() const { return m_size; }
    size_t StrideInBytes() const { return m_strideInBytes; }
    size_t BufferSizeInBytes() const { return (m_size.y() - 1) * m_strideInBytes + m_size.x() * sizeof(T); }
    bool Empty() const { return !AllGreaterStrict(m_size, uint2(0)); }
protected:
    AbstractImage();
    AbstractImage(correct_constness_byte_type* iData, uint2 const& iSize, size_t iStrideInBytes);
    AbstractImage(ArrayView<correct_constness_byte_type> iData, uint2 const& iSize, size_t iStrideInBytes);
    AbstractImage(AbstractImage const& iOther, box2u const& iRect);

    template<typename T2, typename = std::enable_if<std::is_const<T>::value && std::is_same<std::add_const<T2>::type, T>::value>::type>
    AbstractImage(AbstractImage<T2> const& iOther);

    AbstractImage(AbstractImage const&) = default;
    AbstractImage& operator= (AbstractImage const&) = default;
protected:
    uint2 m_size;
    size_t m_strideInBytes;
    correct_constness_byte_type* m_data;
};
//=============================================================================
template <typename T>
AbstractImage<T>::AbstractImage()
    : m_size(0,0)
    , m_strideInBytes(0)
    , m_data(nullptr)
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T>
AbstractImage<T>::AbstractImage(correct_constness_byte_type* iData, uint2 const& iSize, size_t iStrideInBytes)
    : m_size(iSize)
    , m_strideInBytes(iStrideInBytes)
    , m_data(iData)
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T>
AbstractImage<T>::AbstractImage(ArrayView<correct_constness_byte_type> iData, uint2 const& iSize, size_t iStrideInBytes)
    : AbstractImage(iData.data(), iSize, iStrideInBytes)
{
    SG_CODE_FOR_ASSERT(size_t const accessibleDataSizeInBytes = iStrideInBytes * (iSize.y() - 1) + iSize.x() * sizeof(T);)
    SG_ASSERT(accessibleDataSizeInBytes <= iData.size());
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T>
AbstractImage<T>::AbstractImage(AbstractImage const& iOther, box2u const& iRect)
    : m_size(iRect.Delta())
    , m_strideInBytes(iOther.m_strideInBytes)
    , m_data(iOther.m_data + iRect.min.x() * sizeof(T) + iRect.min.y() * iOther.m_strideInBytes)
{
    SG_ASSERT(AllLessEqual(iRect.max, iOther.Size()));
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T>
template<typename T2, typename>
AbstractImage<T>::AbstractImage(AbstractImage<T2> const& iOther)
    : m_size(iOther.m_size)
    , m_strideInBytes(iOther.m_strideInBytes)
    , m_data(iOther.m_data)
{
}
//=============================================================================
template <typename T>
void AbstractImage<T>::Fill(value_type_for_const_passing iValue)
{
    for(size_t y = 0; y < m_size.y(); ++y)
    {
        value_type* row = reinterpret_cast<value_type*>(m_data + y * m_strideInBytes);
        for(size_t x = 0; x < m_size.x(); ++x)
        {
            row[x] = iValue;
        }
    }
}
//=============================================================================
template <typename T>
class ImageView : public AbstractImage<T>
{
public:
    template <typename T2> friend class ImageView;
    typedef typename AbstractImage<T>::value_type value_type;
    typedef typename AbstractImage<T>::value_type_for_const_passing value_type_for_const_passing;
    typedef typename std::conditional<std::is_const<T>::value, void const, void>::type correct_constness_void_type;
    typedef typename std::conditional<std::is_const<T>::value, u8 const, u8>::type correct_constness_byte_type;
public:
    ImageView();
    ImageView(correct_constness_void_type* iData, uint2 const& iSize, size_t iStrideInBytes SG_CODE_FOR_ASSERT(SG_COMMA SafeCountable const* iDataOwner = nullptr) );
    ImageView(correct_constness_byte_type* iData, uint2 const& iSize, size_t iStrideInBytes SG_CODE_FOR_ASSERT(SG_COMMA SafeCountable const* iDataOwner = nullptr) );
    ImageView(ArrayView<correct_constness_byte_type> iData, uint2 const& iSize, size_t iStrideInBytes SG_CODE_FOR_ASSERT(SG_COMMA SafeCountable const* iDataOwner = nullptr) );
    ImageView(ImageView const& iOther, box2u const& iRect);

    template<typename T2, typename = std::enable_if<std::is_const<T>::value && std::is_same<std::add_const<T2>::type, T>::value>::type>
    ImageView(ImageView<T2> const& iOther);

    ImageView(ImageView const&) = default;
    ImageView& operator= (ImageView const&) = default;

    ImageView<T> RectView(uintbox2 const& rect);
    ImageView<T const> RectView(uintbox2 const& rect) const { return ConstRectView(rect); }
    ImageView<T const> ConstRectView(uintbox2 const& rect) const;
private:
#if SG_ENABLE_ASSERT
    safeptr<SafeCountable const> m_dataOwner;
#endif
};
//=============================================================================
template <typename T>
ImageView<T>::ImageView()
    : AbstractImage()
    SG_CODE_FOR_ASSERT(SG_COMMA m_dataOwner(nullptr))
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T>
ImageView<T>::ImageView(correct_constness_void_type* iData, uint2 const& iSize, size_t iStrideInBytes SG_CODE_FOR_ASSERT(SG_COMMA SafeCountable const* iDataOwner) )
    : AbstractImage(reinterpret_cast<correct_constness_byte_type*>(iData), iSize, iStrideInBytes)
    SG_CODE_FOR_ASSERT(SG_COMMA m_dataOwner(iDataOwner))
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T>
ImageView<T>::ImageView(correct_constness_byte_type* iData, uint2 const& iSize, size_t iStrideInBytes SG_CODE_FOR_ASSERT(SG_COMMA SafeCountable const* iDataOwner) )
    : AbstractImage(iData, iSize, iStrideInBytes)
    SG_CODE_FOR_ASSERT(SG_COMMA m_dataOwner(iDataOwner))
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T>
ImageView<T>::ImageView(ArrayView<correct_constness_byte_type> iData, uint2 const& iSize, size_t iStrideInBytes SG_CODE_FOR_ASSERT(SG_COMMA SafeCountable const* iDataOwner) )
    : AbstractImage(iData, iSize, iStrideInBytes)
    SG_CODE_FOR_ASSERT(SG_COMMA m_dataOwner(iDataOwner))
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T>
ImageView<T>::ImageView(ImageView const& iOther, box2u const& iRect)
    : AbstractImage(iOther, iRect)
    SG_CODE_FOR_ASSERT(SG_COMMA m_dataOwner(iOther.m_dataOwner))
{
    SG_ASSERT(AllLessEqual(iRect.max, iOther.Size()));
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T>
template<typename T2, typename>
ImageView<T>::ImageView(ImageView<T2> const& iOther)
    : AbstractImage(iOther)
    SG_CODE_FOR_ASSERT(SG_COMMA m_dataOwner(iOther.m_dataOwner))
{
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T>
ImageView<T> ImageView<T>::RectView(uintbox2 const& rect)
{
    SG_ASSERT(all(rect.max <= m_size));
    return ImageView<T>(m_data + rect.min.x() * sizeof(T) + rect.min.y() * m_strideInBytes, rect.Delta(), m_strideInBytes SG_CODE_FOR_ASSERT(SG_COMMA m_dataOwner.get()) );
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T>
ImageView<T const> ImageView<T>::ConstRectView(uintbox2 const& rect) const
{
    SG_ASSERT(all(rect.max < m_size));
    return ImageView<T const>(m_data + rect.min.x() * sizeof(T) + rect.min.y() * m_strideInBytes, rect.Delta(), m_strideInBytes SG_CODE_FOR_ASSERT(SG_COMMA m_dataOwner.get()) );
}
//=============================================================================
typedef ImageView<ubyte3> RGBImageView;
//=============================================================================
}
}

#endif
