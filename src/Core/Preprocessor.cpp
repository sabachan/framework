#include "stdafx.h"

#include "Preprocessor.h"

#if SG_ENABLE_UNIT_TESTS

#include "Assert.h"
#include "Log.h"
#include "TestFramework.h"

#define PRINT_T0(x) #x
#define PRINT_T1(x) PRINT_T0(x)

#define PRINT_REV2ARGS_T0(x, y) #y #x

namespace sg {
//=============================================================================
SG_TEST((sg), Preprocessor, (Core, quick))
{
    SG_ASSERT(SG_PP_IMPL_HAS_0_OR_1_ARGS() == true);
    SG_ASSERT(SG_PP_IMPL_HAS_0_OR_1_ARGS(hello) == false);
    SG_ASSERT(SG_PP_IMPL_HAS_2_OR_1_ARGS(hello) == false);
    SG_ASSERT(SG_PP_IMPL_HAS_2_OR_1_ARGS(hello, world) == true);
    SG_ASSERT(SG_PP_IMPL_HAS_2_OR_1_ARGS((hello, world)) == false);

    SG_ASSERT(SG_PP_IMPL_HAS_PARENTHESIS(hello) == false);
    SG_ASSERT(SG_PP_IMPL_HAS_PARENTHESIS((hello)) == true);
    SG_ASSERT(SG_PP_IMPL_HAS_PARENTHESIS((hello, world)) == true);

    SG_ASSERT(0 == strcmp(PRINT_T1(STRIP_PARENTHESIS(hello world)), "hello world"));
    SG_ASSERT(0 == strcmp(PRINT_T1(STRIP_PARENTHESIS((hello world))), "hello world"));
    SG_ASSERT(0 == strcmp(PRINT_T1(STRIP_PARENTHESIS(((hello world)))), "(hello world)"));

    SG_ASSERT(0 == strcmp("" APPLY_MACRO_FOR_EACH(PRINT_T0, ()), ""));
    SG_ASSERT(0 == strcmp("" APPLY_MACRO_FOR_EACH(PRINT_T0, (hello)), "hello"));
    SG_ASSERT(0 == strcmp("" APPLY_MACRO_FOR_EACH(PRINT_T0, (hello, world)), "helloworld"));
    SG_ASSERT(0 == strcmp("" APPLY_MACRO_FOR_EACH(PRINT_T0, (hello, world, !)), "helloworld!"));
    SG_ASSERT(0 == strcmp("" APPLY_MACRO_FOR_EACH(PRINT_T0, (a,b,c,d,e,f,g,h,i)), "abcdefghi"));

    SG_ASSERT(0 == strcmp("" APPLY_MULTIARG_MACRO_FOR_EACH(PRINT_T0, ()), ""));
    SG_ASSERT(0 == strcmp("" APPLY_MULTIARG_MACRO_FOR_EACH(PRINT_T0, (hello)), "hello"));
    SG_ASSERT(0 == strcmp("" APPLY_MULTIARG_MACRO_FOR_EACH(PRINT_T0, (hello, world)), "helloworld"));
    SG_ASSERT(0 == strcmp("" APPLY_MULTIARG_MACRO_FOR_EACH(PRINT_T0, (hello, world, !)), "helloworld!"));
    SG_ASSERT(0 == strcmp("" APPLY_MULTIARG_MACRO_FOR_EACH(PRINT_T0, (a,b,c,d,e,f,g,h,i)), "abcdefghi"));

    SG_ASSERT(0 == strcmp("" APPLY_MULTIARG_MACRO_FOR_EACH(PRINT_REV2ARGS_T0, (), hello), ""));
    SG_ASSERT(0 == strcmp("" APPLY_MULTIARG_MACRO_FOR_EACH(PRINT_REV2ARGS_T0, (world), hello), "helloworld"));
    SG_ASSERT(0 == strcmp("" APPLY_MULTIARG_MACRO_FOR_EACH(PRINT_REV2ARGS_T0, (world, kitty), hello), "helloworldhellokitty"));
    SG_ASSERT(0 == strcmp("" APPLY_MULTIARG_MACRO_FOR_EACH(PRINT_REV2ARGS_T0, (a,b,c,d,e,f,g,h,i), _), "_a_b_c_d_e_f_g_h_i"));
}
//=============================================================================
}
#endif
