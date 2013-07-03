#ifndef Math_Vector_Include_SwizzleImpl
#error "this file must be used only by Vector.h"
#endif

private: enum class Members : size_t { x = 0, y, z, w };
public:

#define DEFINE_SWIZZLE2(_0, _1) \
    Vector<T, 2> const _0##_1() const { return Vector<T, 2>(_0(), _1()); } \
    Vector_SwizzledSubVector<T, dim, 2, size_t(Members::_0), size_t(Members::_1)> _0##_1() \
    { return Vector_SwizzledSubVector<T, dim, 2, size_t(Members::_0), size_t(Members::_1)>(*this); }
#define DEFINE_SWIZZLE2_EX(_0) \
    Vector<T, 2> const _0##0() const { return Vector<T, 2>(_0(), T(0)); } \
    Vector<T, 2> const _0##1() const { return Vector<T, 2>(_0(), T(1)); }
#define DEFINE_SWIZZLE3(_0, _1, _2) \
    Vector<T, 3> const _0##_1##_2() const { return Vector<T, 3>(_0(), _1(), _2()); } \
    Vector_SwizzledSubVector<T, dim, 3, size_t(Members::_0), size_t(Members::_1), size_t(Members::_2)> _0##_1##_2() \
    { return Vector_SwizzledSubVector<T, dim, 3, size_t(Members::_0), size_t(Members::_1), size_t(Members::_2)>(*this); }
#define DEFINE_SWIZZLE3_2EX(_0, _1) \
    Vector<T, 3> const _0##_1##0() const { return Vector<T, 3>(_0(), _1(), T(0)); } \
    Vector<T, 3> const _0##_1##1() const { return Vector<T, 3>(_0(), _1(), T(1)); }
#define DEFINE_SWIZZLE3_1EX(_0) \
    Vector<T, 3> const _0##00() const { return Vector<T, 3>(_0(), T(0), T(0)); } \
    Vector<T, 3> const _0##01() const { return Vector<T, 3>(_0(), T(0), T(1)); } \
    Vector<T, 3> const _0##10() const { return Vector<T, 3>(_0(), T(1), T(0)); } \
    Vector<T, 3> const _0##11() const { return Vector<T, 3>(_0(), T(1), T(1)); }
#define DEFINE_SWIZZLE4(_0, _1, _2, _3) \
    Vector<T, 4> const _0##_1##_2##_3() const { return Vector<T, 4>(_0(), _1(), _2(), _3()); } \
    Vector_SwizzledSubVector<T, dim, 4, size_t(Members::_0), size_t(Members::_1), size_t(Members::_2), size_t(Members::_3)> _0##_1##_2##_3() \
    { return Vector_SwizzledSubVector<T, dim, 4, size_t(Members::_0), size_t(Members::_1), size_t(Members::_2), size_t(Members::_3)>(*this); }
#define DEFINE_SWIZZLE4_3EX(_0, _1, _2) \
    Vector<T, 4> const _0##_1##_2##0() const { return Vector<T, 4>(_0(), _1(), _2(), T(0)); } \
    Vector<T, 4> const _0##_1##_2##1() const { return Vector<T, 4>(_0(), _1(), _2(), T(1)); }
#define DEFINE_SWIZZLE4_2EX(_0, _1) \
    Vector<T, 4> const _0##_1##00() const { return Vector<T, 4>(_0(), _1(), T(0), T(0)); } \
    Vector<T, 4> const _0##_1##01() const { return Vector<T, 4>(_0(), _1(), T(0), T(1)); } \
    Vector<T, 4> const _0##_1##10() const { return Vector<T, 4>(_0(), _1(), T(1), T(0)); } \
    Vector<T, 4> const _0##_1##11() const { return Vector<T, 4>(_0(), _1(), T(1), T(1)); }
#define DEFINE_SWIZZLE4_1EX(_0) \
    Vector<T, 4> const _0##000() const { return Vector<T, 4>(_0(), T(0), T(0), T(0)); } \
    Vector<T, 4> const _0##001() const { return Vector<T, 4>(_0(), T(0), T(0), T(1)); } \
    Vector<T, 4> const _0##010() const { return Vector<T, 4>(_0(), T(0), T(1), T(0)); } \
    Vector<T, 4> const _0##011() const { return Vector<T, 4>(_0(), T(0), T(1), T(1)); } \
    Vector<T, 4> const _0##100() const { return Vector<T, 4>(_0(), T(1), T(0), T(0)); } \
    Vector<T, 4> const _0##101() const { return Vector<T, 4>(_0(), T(1), T(0), T(1)); } \
    Vector<T, 4> const _0##110() const { return Vector<T, 4>(_0(), T(1), T(1), T(0)); } \
    Vector<T, 4> const _0##111() const { return Vector<T, 4>(_0(), T(1), T(1), T(1)); }

#define DEFINE_SWIZZLE2_BATCH(_0) \
    DEFINE_SWIZZLE2(_0,x) DEFINE_SWIZZLE2(_0,y) DEFINE_SWIZZLE2(_0,z) DEFINE_SWIZZLE2(_0,w) DEFINE_SWIZZLE2_EX(_0)

    DEFINE_SWIZZLE2_BATCH(x) DEFINE_SWIZZLE2_BATCH(y) DEFINE_SWIZZLE2_BATCH(z) DEFINE_SWIZZLE2_BATCH(w)

#define DEFINE_SWIZZLE3_BATCH2(_0,_1) \
    DEFINE_SWIZZLE3(_0,_1,x) DEFINE_SWIZZLE3(_0,_1,y) DEFINE_SWIZZLE3(_0,_1,z) DEFINE_SWIZZLE3(_0,_1,w) DEFINE_SWIZZLE3_2EX(_0,_1)
#define DEFINE_SWIZZLE3_BATCH1(_0) \
    DEFINE_SWIZZLE3_BATCH2(_0,x) DEFINE_SWIZZLE3_BATCH2(_0,y) DEFINE_SWIZZLE3_BATCH2(_0,z) DEFINE_SWIZZLE3_BATCH2(_0,w) DEFINE_SWIZZLE3_1EX(_0)

    DEFINE_SWIZZLE3_BATCH1(x) DEFINE_SWIZZLE3_BATCH1(y) DEFINE_SWIZZLE3_BATCH1(z) DEFINE_SWIZZLE3_BATCH1(w)

#define DEFINE_SWIZZLE4_BATCH3(_0,_1,_2) \
    DEFINE_SWIZZLE4(_0,_1,_2,x) DEFINE_SWIZZLE4(_0,_1,_2,y) DEFINE_SWIZZLE4(_0,_1,_2,z) DEFINE_SWIZZLE4(_0,_1,_2,w) DEFINE_SWIZZLE4_3EX(_0,_1,_2)
#define DEFINE_SWIZZLE4_BATCH2(_0,_1) \
    DEFINE_SWIZZLE4_BATCH3(_0,_1,x) DEFINE_SWIZZLE4_BATCH3(_0,_1,y) DEFINE_SWIZZLE4_BATCH3(_0,_1,z) DEFINE_SWIZZLE4_BATCH3(_0,_1,w) DEFINE_SWIZZLE4_2EX(_0,_1)
#define DEFINE_SWIZZLE4_BATCH1(_0) \
    DEFINE_SWIZZLE4_BATCH2(_0,x) DEFINE_SWIZZLE4_BATCH2(_0,y) DEFINE_SWIZZLE4_BATCH2(_0,z) DEFINE_SWIZZLE4_BATCH2(_0,w) DEFINE_SWIZZLE4_1EX(_0)

    DEFINE_SWIZZLE4_BATCH1(x) DEFINE_SWIZZLE4_BATCH1(y) DEFINE_SWIZZLE4_BATCH1(z) DEFINE_SWIZZLE4_BATCH1(w)

#undef DEFINE_SWIZZLE2
#undef DEFINE_SWIZZLE2_EX
#undef DEFINE_SWIZZLE3
#undef DEFINE_SWIZZLE3_2EX
#undef DEFINE_SWIZZLE3_1EX
#undef DEFINE_SWIZZLE4
#undef DEFINE_SWIZZLE4_3EX
#undef DEFINE_SWIZZLE4_2EX
#undef DEFINE_SWIZZLE4_1EX
#undef DEFINE_SWIZZLE2_BATCH
#undef DEFINE_SWIZZLE3_BATCH2
#undef DEFINE_SWIZZLE3_BATCH1
#undef DEFINE_SWIZZLE4_BATCH3
#undef DEFINE_SWIZZLE4_BATCH2
#undef DEFINE_SWIZZLE4_BATCH1

