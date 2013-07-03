#ifndef Image_Image_H
#define Image_Image_H

#include "ImageView.h"
#include <Core/ArrayView.h>
#include <Core/For.h>
#include <Core/SmartPtr.h>
#include <Core/TemplateUtils.h>
#include <Math/Box.h>
#include <Math/Vector.h>

namespace sg {
namespace image {
//=============================================================================
template <typename T>
class Image : public SafeCountable, public AbstractImage<T>
{
    static_assert(!std::is_const<T>::value,
        "Image of const elements is forbidden.");
    SG_NON_COPYABLE(Image)
    template <typename U>
    friend void swap(Image<U>& a, Image<U>& b);
public:
    typedef T value_type;
    typedef typename ConstPassing<value_type>::type value_type_for_const_passing;
public:
    Image();
    Image(uint2 const& iSize);
    Image(uint2 const& iSize, value_type_for_const_passing iClearValue);
    ~Image();

    Image(Image&& iOther);
    Image& operator= (Image&& iOther);
    Image Clone();

    void Reset();
    void Reset(uint2 const& iSize);
    template <typename... Args> void Reset(uint2 const& iSize, Args... iClearArgs);

    operator ImageView<T>() { return ImageView<T>(m_data, m_size, m_strideInBytes SG_CODE_FOR_ASSERT(SG_COMMA m_safeCountForData.get()) ); }
    operator ImageView<T const>() const { return ImageView<T const>(m_data, m_size, m_strideInBytes SG_CODE_FOR_ASSERT(SG_COMMA m_safeCountForData.get()) ); }
    ImageView<T> View() { return ImageView<T>(m_data, m_size, m_strideInBytes SG_CODE_FOR_ASSERT(SG_COMMA m_safeCountForData.get()) ); }
    ImageView<T const> View() const { return ImageView<T const>(m_data, m_size, m_strideInBytes SG_CODE_FOR_ASSERT(SG_COMMA m_safeCountForData.get()) ); }
    ImageView<T const> ConstView() const { return ImageView<T const>(m_data, m_size, m_strideInBytes SG_CODE_FOR_ASSERT(SG_COMMA m_safeCountForData.get()) ); }
private:
    void Swap(Image& iOther);
private:
    SG_CODE_FOR_ASSERT(refptr<RefAndSafeCountable> m_safeCountForData;)
};
//=============================================================================
template <typename T>
Image<T>::Image()
    : AbstractImage()
    SG_CODE_FOR_ASSERT(SG_COMMA m_safeCountForData())
{}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T>
Image<T>::Image(uint2 const& iSize)
    : AbstractImage(new u8[iSize.x() * iSize.y() * sizeof(T)], iSize, iSize.x() * sizeof(T))
    SG_CODE_FOR_ASSERT(SG_COMMA m_safeCountForData(new RefAndSafeCountable))
{}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T>
Image<T>::Image(uint2 const& iSize, value_type_for_const_passing iClearValue)
    : Image(iSize)
{
    Fill(iClearValue);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T>
Image<T>::Image(Image&& iOther)
    : Image()
{
    swap(*this, iOther);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T>
Image<T>& Image<T>::operator=(Image&& iOther)
{
    swap(*this, iOther);
    return *this;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T>
void Image<T>::Swap(Image& iOther)
{
    using std::swap;
    swap(m_size,          iOther.m_size);
    swap(m_strideInBytes, iOther.m_strideInBytes);
    swap(m_data,          iOther.m_data);
    SG_CODE_FOR_ASSERT(swap(m_safeCountForData, iOther.m_safeCountForData);)
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T>
void swap(Image<T>& a, Image<T>& b)
{
    a.Swap(b);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T>
typename Image<T> Image<T>::Clone()
{
    Image tmp(m_size);
    size_t const rowSizeInBytes = m_size.x() * sizeof(T);
    for_range(size_t, y, 0, m_size.y())
        memcpy(tmp.Row(y).data(), Row(y).data(), rowSizeInBytes);
    return tmp;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T>
void Image<T>::Reset()
{
    if(!std::is_trivially_destructible<T>())
    {
        uint2 const size = m_size;
        size_t const stride = m_strideInBytes;
        u8* data = m_data;
        for_range(size_t, y, 0, size.y())
        {
            u8* scanline = data + y * stride;
            SG_UNUSED(scanline);
            for_range(size_t, x, 0, size.x())
                reinterpret_cast<T*>(scanline)[x].~T();
        }
    }
    delete[] m_data;
    SG_CODE_FOR_ASSERT(m_safeCountForData = nullptr;)
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T>
template <typename... Args>
void Image<T>::Reset(uint2 const& iSize, Args... iClearArgs)
{
    SG_ASSERT(AllGreaterStrict(iSize, uint2(0)));
    Reset();
    size_t const count = iSize.x() * iSize.y();
    m_data = new u8[count * sizeof(T)];
    if(!std::is_trivially_constructible<T>())
    {
        for_range(size_t, i, 0, count)
            new(reinterpret_cast<T*>(m_data) + i) T(iClearArgs...);
    }
    m_size = iSize;
    m_strideInBytes = iSize.x() * sizeof(T);
    SG_CODE_FOR_ASSERT(m_safeCountForData = new RefAndSafeCountable;)
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T>
void Image<T>::Reset(uint2 const& iSize)
{
    SG_ASSERT(AllGreaterStrict(iSize, uint2(0)));
    Reset();
    size_t const count = iSize.x() * iSize.y();
    m_data = new u8[count * sizeof(T)];
    if(!std::is_trivially_constructible<T>())
    {
        for_range(size_t, i, 0, count)
           new(reinterpret_cast<T*>(m_data) + i) T();
    }
    m_size = iSize;
    m_strideInBytes = iSize.x() * sizeof(T);
    SG_CODE_FOR_ASSERT(m_safeCountForData = new RefAndSafeCountable;)
}
//=============================================================================
template <typename T>
Image<T>::~Image()
{
    Reset();
}
//=============================================================================
typedef Image<ubyte3> RGBImage;
//=============================================================================
}
}

#endif
