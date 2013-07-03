#ifndef Rendering_Color_H
#define Rendering_Color_H

#include <Core/TemplateUtils.h>
#include <Math/Vector.h>
#include <type_traits>

namespace sg {
namespace rendering {
//=============================================================================
template <typename T, size_t dim>
class Color
{
    static_assert(dim == 3 || dim == 4, "only rgb or rgba colors are supported");
    typedef Color this_type;
    typedef T value_type;
    typedef typename ConstPassing<value_type>::type value_type_for_const_passing;
    typedef math::Vector<T, dim> vector_type;
public:
    this_type() : m_data() {}
    explicit this_type(uninitialized_t) : m_data(uninitialized) {}
    explicit this_type(vector_type const& iData) : m_data(iData) {}
    this_type(value_type_for_const_passing r, value_type_for_const_passing g, value_type_for_const_passing b) : m_data(r,g,b) { static_assert(dim == 3, "incorrect number of constructor parameters."); }
    this_type(value_type_for_const_passing r, value_type_for_const_passing g, value_type_for_const_passing b, value_type_for_const_passing a) : m_data(r,g,b,a) { static_assert(dim == 4, "incorrect number of constructor parameters."); }
    this_type(this_type const& iOther) { m_data = iOther.m_data; }
    this_type & operator=(this_type const& iOther) { m_data = iOther.m_data; return *this; }
    template<typename T2>
    explicit this_type(Color<T2, dim> const& iOther) { m_data = vector_type(iOther.m_data); }

    value_type_for_const_passing r() const { static_assert(1 <= dim, "r not available."); return m_data._[0]; }
    value_type_for_const_passing g() const { static_assert(2 <= dim, "g not available."); return m_data._[1]; }
    value_type_for_const_passing b() const { static_assert(3 <= dim, "b not available."); return m_data._[2]; }
    value_type_for_const_passing a() const { static_assert(4 <= dim, "a not available."); return m_data._[3]; }
    value_type&                  r()       { static_assert(1 <= dim, "r not available."); return m_data._[0]; }
    value_type&                  g()       { static_assert(2 <= dim, "g not available."); return m_data._[1]; }
    value_type&                  b()       { static_assert(3 <= dim, "b not available."); return m_data._[2]; }
    value_type&                  a()       { static_assert(4 <= dim, "a not available."); return m_data._[3]; }

    value_type_for_const_passing operator[] (size_t i) const { SG_ASSERT(i <= dim); return m_data._[i]; }
    value_type&                  operator[] (size_t i)       { SG_ASSERT(i <= dim); return m_data._[i]; }

#define DEFINE_COMPONENTWISE_OP(OP) \
    this_type const& operator OP ## = (this_type const& a) \
    { \
        m_data OP ## = (a.m_data); \
        return *this; \
    } \
    this_type const& operator OP ## = (T s) \
    { \
        m_data OP ## = (s); \
        return *this; \
    } \
    /* Note that a is taken by copy */ \
    /*  (cf. http://stackoverflow.com/questions/4421706/operator-overloading/4421729#4421729) */ \
    friend this_type operator OP (this_type a, this_type const& b) \
    { \
        return a OP ## = b; \
    } \
    friend this_type operator OP (this_type a, T s) \
    { \
        return a OP ## = s; \
    } \
    friend this_type operator OP (T s, this_type a) \
    { \
        return a OP ## = s; \
    }

    DEFINE_COMPONENTWISE_OP(+)
    DEFINE_COMPONENTWISE_OP(-)
    DEFINE_COMPONENTWISE_OP(*)
    DEFINE_COMPONENTWISE_OP(/)
    DEFINE_COMPONENTWISE_OP(%)
    DEFINE_COMPONENTWISE_OP(&)
    DEFINE_COMPONENTWISE_OP(|)
    DEFINE_COMPONENTWISE_OP(>>)
    DEFINE_COMPONENTWISE_OP(<<)
#undef DEFINE_COMPONENTWISE_OP

#define DEFINE_COMPONENTWISE_UNARY_OP(OP) \
    this_type operator OP () \
    { \
        this_type r(uninitialized); \
        for(size_t i = 0; i < 4; ++i) { r.m_data._[i] = OP m_data._[i]; } \
        return r; \
    }

    DEFINE_COMPONENTWISE_UNARY_OP(-)
#undef DEFINE_COMPONENTWISE_UNARY_OP

    explicit operator vector_type() const { return m_data; }
    vector_type const& Data() const { return m_data; }
    vector_type&       Data()       { return m_data; }
public:
    vector_type m_data;
};
//=============================================================================
} // namespace rendering
//=============================================================================
//       What every programmer should know about color (fast version)
//
// Colors are often writen (r, g, b, a) with a being a lerp cofficient to use
// when blending with background color.
// However, a better form is alpha-premultiplication as it is easier to do most
// of the blending operations (cf. [Porter-Duff, 84]):
//     (r*a, g*a, b*a, a)
// Also, RGB colors are often encoded in sRGB, otherwise known as gamma, that
// can be approximated by a pow (but is not, please use the dedicated
// functions if you need to convert them):
//     (r^gamma, g^gamma, b^gamma)
// Mixing both premultiplication and gamma needs to do the operations in a
// correct order:
//     ((r*a)^gamma, (g*a)^gamma, (b*a)^gamma, a)
// alpha-premultiplication should be made before for multiple reasons, one of
// them being that the GPU will decode an sRGB texture by decoding the 3 rgb
// channels as sRGB encodedwhile leaving the a channel untouched.
//
// We hence have 4 potential ways to code a color.
// Here are some recommandations about when to use them:
// - The ideal color representation is linear with alpha premultiplication as
//   it enables correct and fast computations. Hence, Color4f should only
//   represent linear colors with alpha premultiplication.
// - For the same reason, Color3f should contain only linear colors.
// - When presenting a color value to a user, using a non premultiplied SRGBA
//   color is better as he is used to see this kind of values. Also, alpha
//   premultiplication under gamma is difficult to manipulate by hand.
//   We can then assume that most of R8G8B8A8 variables in the code are using
//   sRGB with no alpha premultiplication.
// - Textures should contains alpha premultiplied sRGBA colors. Hence, the type
//   R8G8B8A8 used in a texture should represent that, but the transformation
//   to another encoding should be done as soon as possible.
// Any other usage should be explicit, eg. by adding a hint in variable names.
//
// We will use the following naming convention.
// Use lower case (r,g,b) for color channels that are NOT alpha premultiplied.
// Use upper case (R,G,B) for color channels that ARE alpha premultiplied.
// Use prefix s for standard-RGB color space and l for linear color space.
// Alpha channel should be writen using lower case a.
// - srgba means not alpha premultiplied sRGB colors:
//      (r^gamma, g^gamma, b^gamma, a)
// - lRGBa means alpha premultiplied linear colors:
//      (r*a, g*a, b*a, a)
// - sRGBa means alpha premultiplied sRGB colors:
//      ((r*a)^gamma, (g*a)^gamma, (b*a)^gamma, a)
typedef rendering::Color<float, 3> Color3f;
typedef rendering::Color<float, 4> Color4f;
typedef rendering::Color<u8, 4> R8G8B8A8;
//=============================================================================
}

#endif
