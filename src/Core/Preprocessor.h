#ifndef Core_Preprocessor_H
#define Core_Preprocessor_H

#include "Config.h"

#define Core_Preprocessor_Include_Impl
#include "Preprocessor_impl.h"
#undef Core_Preprocessor_Include_Impl

#define SG_CONCAT(a, b) SG_PP_IMPL_CONCAT(a, b)

#define STRIP_PARENTHESIS(ARG) SG_PP_IMPL_STRIP_PARENTHESIS(ARG)

#define IF_THEN(true_OR_false, TRUE_EXPR) SG_PP_IMPL_IF_THEN_ELSE(true_OR_false, TRUE_EXPR, )
#define IF_THEN_ELSE(true_OR_false, TRUE_EXPR, FALSE_EXPR) SG_PP_IMPL_IF_THEN_ELSE(true_OR_false, TRUE_EXPR, FALSE_EXPR)

#define APPLY_MACRO_FOR_EACH(MACRO, LIST) SG_PP_IMPL_APPLY_MACRO_FOR_EACH(MACRO, LIST)

#define APPLY_MULTIARG_MACRO_FOR_EACH(MACRO, LIST, ...) SG_PP_IMPL_APPLY_MULTIARG_MACRO_FOR_EACH(MACRO, LIST, __VA_ARGS__)

#endif
