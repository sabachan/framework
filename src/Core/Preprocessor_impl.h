#ifndef Core_Preprocessor_Include_Impl
#error "this file must be used only by Preprocessor.h"
#endif

#define SG_PP_IMPL_CONCAT_T0(x, y) x##y
#define SG_PP_IMPL_CONCAT_T1(x, y) SG_PP_IMPL_CONCAT_T0(x, y)
#define SG_PP_IMPL_CONCAT_T2(x, y) SG_PP_IMPL_CONCAT_T1(x, y)
#define SG_PP_IMPL_CONCAT(x, y) SG_PP_IMPL_CONCAT_T0(x, y)

#define SG_PP_IMPL_IF_true(TRUE_EXPR, FALSE_EXPR)  TRUE_EXPR
#define SG_PP_IMPL_IF_false(TRUE_EXPR, FALSE_EXPR) FALSE_EXPR
#define SG_PP_IMPL_IF_THEN_ELSE(true_OR_false, TRUE_EXPR, FALSE_EXPR) SG_PP_IMPL_IF_##true_OR_false(TRUE_EXPR, FALSE_EXPR)

#define SG_PP_IMPL_TRY_TO_CALL(MACRO, ARG) MACRO ARG
#define SG_PP_IMPL_COPY_ARGS(...) __VA_ARGS__

#define SG_PP_IMPL_GET_LONE_ARG(_0) _0
#define SG_PP_IMPL_GET_ARG_0(_0, ...) _0
#define SG_PP_IMPL_GET_ARG_2(_0, _1, _2, ...) _2
#define SG_PP_IMPL_GET_ARG_9(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, ...) _9
#define SG_PP_IMPL_GET_ARGS_AFTER_0(_0, ...) __VA_ARGS__

#define SG_PP_IMPL_GENERATE_2_ARGS(...) _,_
#define SG_PP_IMPL_3RD_ARG(...) SG_PP_IMPL_TRY_TO_CALL(SG_PP_IMPL_GET_ARG_2, (__VA_ARGS__))
#define SG_PP_IMPL_HAS_2_OR_1_ARGS(...) SG_PP_IMPL_3RD_ARG(__VA_ARGS__, true, false)
#define SG_PP_IMPL_HAS_PARENTHESIS(ARG) SG_PP_IMPL_HAS_2_OR_1_ARGS(SG_PP_IMPL_TRY_TO_CALL(SG_PP_IMPL_GENERATE_2_ARGS, ARG))

#define SG_PP_IMPL_STRIP_PARENTHESIS_IFN_I(ARG, HAS_PARENTHESIS) SG_PP_IMPL_IF_THEN_ELSE(HAS_PARENTHESIS, SG_PP_IMPL_TRY_TO_CALL(SG_PP_IMPL_COPY_ARGS, ARG), ARG)
#define SG_PP_IMPL_STRIP_PARENTHESIS_IFN(ARG, HAS_PARENTHESIS) SG_PP_IMPL_STRIP_PARENTHESIS_IFN_I(ARG, HAS_PARENTHESIS)

#define SG_PP_IMPL_STRIP_PARENTHESIS(ARG) SG_PP_IMPL_STRIP_PARENTHESIS_IFN(ARG, SG_PP_IMPL_HAS_PARENTHESIS(ARG))

#define SG_PP_IMPL_HAS_0_OR_1_ARGS_TRY_TO_CALL() _, _
#define SG_PP_IMPL_HAS_0_OR_1_ARGS(...) SG_PP_IMPL_HAS_2_OR_1_ARGS(SG_PP_IMPL_HAS_0_OR_1_ARGS_TRY_TO_CALL __VA_ARGS__ (), true, false)

#define SG_PP_IMPL_COUNT_ARGS_CONCAT(x, y)  SG_PP_IMPL_CONCAT_T1(x, y)

#define SG_PP_IMPL_COUNT_ARGS_CASE_0_OR_1_true(...) 0
#define SG_PP_IMPL_COUNT_ARGS_CASE_0_OR_1_false(...) 1
#define SG_PP_IMPL_COUNT_ARGS_CASE_0_OR_1(...) SG_PP_IMPL_COUNT_ARGS_CONCAT( SG_PP_IMPL_COUNT_ARGS_CASE_0_OR_1_, SG_PP_IMPL_HAS_0_OR_1_ARGS(__VA_ARGS__) ) ()

#define SG_PP_IMPL_COUNT_ARGS_CASE_2_OR_LESS_true(N, ARGS) SG_PP_IMPL_COUNT_ARGS_CASE_0_OR_1 ARGS
#define SG_PP_IMPL_COUNT_ARGS_CASE_2_OR_LESS_false(N, ARGS) N
#define SG_PP_IMPL_COUNT_ARGS_HAS_2_OR_1_ARGS(...) SG_PP_IMPL_HAS_2_OR_1_ARGS(__VA_ARGS__)
#define SG_PP_IMPL_COUNT_ARGS_GENERATE_2_ARGS_IF_1_1() _, _
#define SG_PP_IMPL_COUNT_ARGS_IMPL(N, ARGS) \
    SG_PP_IMPL_COUNT_ARGS_CONCAT( \
        SG_PP_IMPL_COUNT_ARGS_CASE_2_OR_LESS_, \
        SG_PP_IMPL_COUNT_ARGS_HAS_2_OR_1_ARGS( \
            SG_PP_IMPL_COUNT_ARGS_CONCAT( \
                SG_PP_IMPL_COUNT_ARGS_GENERATE_2_ARGS_IF_1_, \
                N \
            ) () \
        ) \
    ) (N, ARGS)
#define SG_PP_IMPL_COUNT_ARGS_MAX_9(...) SG_PP_IMPL_COUNT_ARGS_IMPL( SG_PP_IMPL_GET_ARG_9(__VA_ARGS__, 9, 8, 7, 6, 5, 4, 3, 2, 1) , (__VA_ARGS__) )

#define SG_PP_IMPL_COUNT_ARGS(...) SG_PP_IMPL_COUNT_ARGS_MAX_9(__VA_ARGS__)

#define SG_PP_IMPL_APPLY_MACRO_FOR_EACH_CONCAT(x, y) SG_PP_IMPL_CONCAT_T1(x, y)

#define SG_PP_IMPL_CALL_1ARG(MACRO, ARG) MACRO(ARG)

#define SG_PP_IMPL_APPLY_MACRO_FOR_EACH_0(MACRO, LIST)
#define SG_PP_IMPL_APPLY_MACRO_FOR_EACH_1(MACRO, LIST) MACRO LIST
#define SG_PP_IMPL_APPLY_MACRO_FOR_EACH_2(MACRO, LIST) SG_PP_IMPL_CALL_1ARG(MACRO, SG_PP_IMPL_GET_ARG_0 LIST) SG_PP_IMPL_APPLY_MACRO_FOR_EACH_1(MACRO, (SG_PP_IMPL_GET_ARGS_AFTER_0 LIST))
#define SG_PP_IMPL_APPLY_MACRO_FOR_EACH_3(MACRO, LIST) SG_PP_IMPL_CALL_1ARG(MACRO, SG_PP_IMPL_GET_ARG_0 LIST) SG_PP_IMPL_APPLY_MACRO_FOR_EACH_2(MACRO, (SG_PP_IMPL_GET_ARGS_AFTER_0 LIST))
#define SG_PP_IMPL_APPLY_MACRO_FOR_EACH_4(MACRO, LIST) SG_PP_IMPL_CALL_1ARG(MACRO, SG_PP_IMPL_GET_ARG_0 LIST) SG_PP_IMPL_APPLY_MACRO_FOR_EACH_3(MACRO, (SG_PP_IMPL_GET_ARGS_AFTER_0 LIST))
#define SG_PP_IMPL_APPLY_MACRO_FOR_EACH_5(MACRO, LIST) SG_PP_IMPL_CALL_1ARG(MACRO, SG_PP_IMPL_GET_ARG_0 LIST) SG_PP_IMPL_APPLY_MACRO_FOR_EACH_4(MACRO, (SG_PP_IMPL_GET_ARGS_AFTER_0 LIST))
#define SG_PP_IMPL_APPLY_MACRO_FOR_EACH_6(MACRO, LIST) SG_PP_IMPL_CALL_1ARG(MACRO, SG_PP_IMPL_GET_ARG_0 LIST) SG_PP_IMPL_APPLY_MACRO_FOR_EACH_5(MACRO, (SG_PP_IMPL_GET_ARGS_AFTER_0 LIST))
#define SG_PP_IMPL_APPLY_MACRO_FOR_EACH_7(MACRO, LIST) SG_PP_IMPL_CALL_1ARG(MACRO, SG_PP_IMPL_GET_ARG_0 LIST) SG_PP_IMPL_APPLY_MACRO_FOR_EACH_6(MACRO, (SG_PP_IMPL_GET_ARGS_AFTER_0 LIST))
#define SG_PP_IMPL_APPLY_MACRO_FOR_EACH_8(MACRO, LIST) SG_PP_IMPL_CALL_1ARG(MACRO, SG_PP_IMPL_GET_ARG_0 LIST) SG_PP_IMPL_APPLY_MACRO_FOR_EACH_7(MACRO, (SG_PP_IMPL_GET_ARGS_AFTER_0 LIST))
#define SG_PP_IMPL_APPLY_MACRO_FOR_EACH_9(MACRO, LIST) SG_PP_IMPL_CALL_1ARG(MACRO, SG_PP_IMPL_GET_ARG_0 LIST) SG_PP_IMPL_APPLY_MACRO_FOR_EACH_8(MACRO, (SG_PP_IMPL_GET_ARGS_AFTER_0 LIST))

#define SG_PP_IMPL_CALL_MULTIARG(MACRO, ...) SG_PP_IMPL_TRY_TO_CALL(MACRO, (__VA_ARGS__))

#define SG_PP_IMPL_APPLY_MULTIARG_MACRO_FOR_EACH_0(MACRO, LIST, ...)
#define SG_PP_IMPL_APPLY_MULTIARG_MACRO_FOR_EACH_1(MACRO, LIST, ...) SG_PP_IMPL_CALL_MULTIARG(MACRO, SG_PP_IMPL_GET_LONE_ARG LIST, __VA_ARGS__)
#define SG_PP_IMPL_APPLY_MULTIARG_MACRO_FOR_EACH_2(MACRO, LIST, ...) SG_PP_IMPL_CALL_MULTIARG(MACRO, SG_PP_IMPL_GET_ARG_0 LIST, __VA_ARGS__) SG_PP_IMPL_APPLY_MULTIARG_MACRO_FOR_EACH_1(MACRO, (SG_PP_IMPL_GET_ARGS_AFTER_0 LIST), __VA_ARGS__)
#define SG_PP_IMPL_APPLY_MULTIARG_MACRO_FOR_EACH_3(MACRO, LIST, ...) SG_PP_IMPL_CALL_MULTIARG(MACRO, SG_PP_IMPL_GET_ARG_0 LIST, __VA_ARGS__) SG_PP_IMPL_APPLY_MULTIARG_MACRO_FOR_EACH_2(MACRO, (SG_PP_IMPL_GET_ARGS_AFTER_0 LIST), __VA_ARGS__)
#define SG_PP_IMPL_APPLY_MULTIARG_MACRO_FOR_EACH_4(MACRO, LIST, ...) SG_PP_IMPL_CALL_MULTIARG(MACRO, SG_PP_IMPL_GET_ARG_0 LIST, __VA_ARGS__) SG_PP_IMPL_APPLY_MULTIARG_MACRO_FOR_EACH_3(MACRO, (SG_PP_IMPL_GET_ARGS_AFTER_0 LIST), __VA_ARGS__)
#define SG_PP_IMPL_APPLY_MULTIARG_MACRO_FOR_EACH_5(MACRO, LIST, ...) SG_PP_IMPL_CALL_MULTIARG(MACRO, SG_PP_IMPL_GET_ARG_0 LIST, __VA_ARGS__) SG_PP_IMPL_APPLY_MULTIARG_MACRO_FOR_EACH_4(MACRO, (SG_PP_IMPL_GET_ARGS_AFTER_0 LIST), __VA_ARGS__)
#define SG_PP_IMPL_APPLY_MULTIARG_MACRO_FOR_EACH_6(MACRO, LIST, ...) SG_PP_IMPL_CALL_MULTIARG(MACRO, SG_PP_IMPL_GET_ARG_0 LIST, __VA_ARGS__) SG_PP_IMPL_APPLY_MULTIARG_MACRO_FOR_EACH_5(MACRO, (SG_PP_IMPL_GET_ARGS_AFTER_0 LIST), __VA_ARGS__)
#define SG_PP_IMPL_APPLY_MULTIARG_MACRO_FOR_EACH_7(MACRO, LIST, ...) SG_PP_IMPL_CALL_MULTIARG(MACRO, SG_PP_IMPL_GET_ARG_0 LIST, __VA_ARGS__) SG_PP_IMPL_APPLY_MULTIARG_MACRO_FOR_EACH_6(MACRO, (SG_PP_IMPL_GET_ARGS_AFTER_0 LIST), __VA_ARGS__)
#define SG_PP_IMPL_APPLY_MULTIARG_MACRO_FOR_EACH_8(MACRO, LIST, ...) SG_PP_IMPL_CALL_MULTIARG(MACRO, SG_PP_IMPL_GET_ARG_0 LIST, __VA_ARGS__) SG_PP_IMPL_APPLY_MULTIARG_MACRO_FOR_EACH_7(MACRO, (SG_PP_IMPL_GET_ARGS_AFTER_0 LIST), __VA_ARGS__)
#define SG_PP_IMPL_APPLY_MULTIARG_MACRO_FOR_EACH_9(MACRO, LIST, ...) SG_PP_IMPL_CALL_MULTIARG(MACRO, SG_PP_IMPL_GET_ARG_0 LIST, __VA_ARGS__) SG_PP_IMPL_APPLY_MULTIARG_MACRO_FOR_EACH_8(MACRO, (SG_PP_IMPL_GET_ARGS_AFTER_0 LIST), __VA_ARGS__)

// TODO: check that list is parenthesized
//#define CHECK_PARENTHESIZED(...)

#define SG_PP_IMPL_APPLY_MACRO_FOR_EACH(MACRO, LIST) SG_PP_IMPL_APPLY_MACRO_FOR_EACH_CONCAT(SG_PP_IMPL_APPLY_MACRO_FOR_EACH_, SG_PP_IMPL_COUNT_ARGS LIST )(MACRO, LIST)

#define SG_PP_IMPL_APPLY_MULTIARG_MACRO_FOR_EACH(MACRO, LIST, ...) SG_PP_IMPL_APPLY_MACRO_FOR_EACH_CONCAT(SG_PP_IMPL_APPLY_MULTIARG_MACRO_FOR_EACH_, SG_PP_IMPL_COUNT_ARGS LIST )(MACRO, LIST, __VA_ARGS__)
