#ifndef Image_ImageIteration_H
#define Image_ImageIteration_H

#include "Image.h"
#include "ImageView.h"

namespace sg {
namespace image {
//=============================================================================
namespace impl {
//=============================================================================
template <typename T>
uint2 GetWidthHeightHelper(ImageView<T> const& img) { return img.WidthHeight(); }
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template <typename T, typename... U>
uint2 GetWidthHeightHelper(ImageView<T> const& img, ImageView<U> const&... imgs)
{
    uint2 const wh = img.WidthHeight();
    SG_ASSERT_AND_UNUSED(wh == GetWidthHeightHelper(imgs...));
    return wh;
}
//=============================================================================
// From http://en.cppreference.com/w/cpp/utility/integer_sequence
template<typename F, typename Tup, std::size_t... index>
void CoInvokeAtRowPosHelper(F&& f, Tup&& rows, size_t x, std::index_sequence<index...>)
{
    return f(std::get<index>(std::forward<Tup>(rows))[x]...);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename F, typename Tup>
void CoInvokeAtRowPos(F&& f, Tup&& rows, size_t x)
{
    constexpr auto size = std::tuple_size<typename std::decay<Tup>::type>::value;
    return CoInvokeAtRowPosHelper(std::forward<F>(f),
                                  std::forward<Tup>(rows),
                                  x,
                                  std::make_index_sequence<size>{});
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename F, typename Tup, std::size_t... index>
void CoInvokeWithPositionAtRowPosHelper(F&& f, Tup&& rows, uint2 xy, std::index_sequence<index...>)
{
    return f(xy, std::get<index>(std::forward<Tup>(rows))[xy.x()]...);
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
template<typename F, typename Tup>
void CoInvokeWithPositionAtRowPos(F&& f, Tup&& rows, uint2 xy)
{
    constexpr auto size = std::tuple_size<typename std::decay<Tup>::type>::value;
    return CoInvokeWithPositionAtRowPosHelper(
        std::forward<F>(f),
        std::forward<Tup>(rows),
        xy,
        std::make_index_sequence<size>{});
}
//=============================================================================
}
//=============================================================================
template <typename F, typename T, typename... Ts>
void CoIterate(F&& f, ImageView<T> img, ImageView<Ts>... imgs)
{
    uint2 const wh = impl::GetWidthHeightHelper(img, imgs...);
    for_range(size_t, y, 0, wh.y())
    {
        auto rows = std::make_tuple(img.Row(y), imgs.Row(y)...);
        for_range(size_t, x, 0, wh.x())
        {
            impl::CoInvokeAtRowPos(std::forward<F>(f), rows, x);
        }
    }
}
//=============================================================================
template <typename F, typename T, typename... Ts>
void CoIterateWithPosition(F&& f, ImageView<T> img, ImageView<Ts>... imgs)
{
    uint2 const wh = impl::GetWidthHeightHelper(img, imgs...);
    for_range(unsigned int, y, 0, wh.y())
    {
        auto rows = std::make_tuple(img.Row(y), imgs.Row(y)...);
        for_range(unsigned int, x, 0, wh.x())
        {
            impl::CoInvokeWithPositionAtRowPos(std::forward<F>(f), rows, uint2(x,y));
        }
    }
}
//=============================================================================
namespace impl {
//=============================================================================
template <typename F>
struct CoIterator
{
    CoIterator(F& f) : m_f(f) {}
    template <typename U, typename... T>
    void operator()(U& o, T... v) { o = m_f(v...); }
private:
    F& m_f;
};
template <typename F>
CoIterator<F> MakeCoIterator(F&& f) { return CoIterator<F>(std::forward<F>(f)); }
//=============================================================================
}
//=============================================================================
template <typename F, typename T, typename... Ts>
void ComputeFromCoIteration(Image<decltype(std::declval<F>()(std::declval<T>(), std::declval<Ts>()...))>& oImage, F&& f, ImageView<T> img, ImageView<Ts>... imgs)
{
    typedef decltype(std::declval<F>()(std::declval<T>(), std::declval<Ts>()...)) out_t;
    uint2 const wh = impl::GetWidthHeightHelper(img, imgs...);
    oImage = Image<out_t>(wh);
    CoIterate(impl::MakeCoIterator(f), oImage.View(), img, imgs...);
}
//=============================================================================
}
}

#endif
