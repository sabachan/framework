#ifndef ObjectScript_Reader_Include_TestImpl
#error "this file must be used only by Reader.cpp"
#endif

{
    // description
    "function references external constants",
    // file content
    "namespace ns {"                                            "\n"
    "   const a = 2"                                            "\n"
    "   const b = -1"                                           "\n"
    "   const c = 1"                                            "\n"
    "   function f(x)"                                          "\n"
    "   {"                                                      "\n"
    "       return a*x*x + b*x + c"                             "\n"
    "   }"                                                      "\n"
    "}"                                                         "\n"
    "f1 is sg::reflectionTest::TestClass_A { u : ns::f(1) }"    "\n"
    "f2 is sg::reflectionTest::TestClass_A { u : ns::f(2) }"    "\n"
    "f3 is sg::reflectionTest::TestClass_A { u : ns::f(3) }"    "\n"
    "f4 is sg::reflectionTest::TestClass_A { u : ns::f(4) }"    "\n"
    "f5 is sg::reflectionTest::TestClass_A { u : ns::f(5) }"    "\n",
    // equivalent file
    "f1 is sg::reflectionTest::TestClass_A { u : 2 }"           "\n"
    "f2 is sg::reflectionTest::TestClass_A { u : 7 }"           "\n"
    "f3 is sg::reflectionTest::TestClass_A { u : 16 }"          "\n"
    "f4 is sg::reflectionTest::TestClass_A { u : 29 }"          "\n"
    "f5 is sg::reflectionTest::TestClass_A { u : 46 }"          "\n",
    // return value
    true,
    // first error
    ErrorType::unknown
},
{
    // description
    "function references external variables",
    // file content
    "namespace ns {"                                            "\n"
    "   const a = 2"                                            "\n"
    "   const b = -1"                                           "\n"
    "   var c = 1"                                              "\n"
    "   function f(x)"                                          "\n"
    "   {"                                                      "\n"
    "       return a*x*x + b*x + c"                             "\n"
    "   }"                                                      "\n"
    "}"                                                         "\n"
    "f1 is sg::reflectionTest::TestClass_A { u : ns::f(1) }"    "\n"
    "f2 is sg::reflectionTest::TestClass_A { u : ns::f(2) }"    "\n"
    "f3 is sg::reflectionTest::TestClass_A { u : ns::f(3) }"    "\n"
    "f4 is sg::reflectionTest::TestClass_A { u : ns::f(4) }"    "\n"
    "f5 is sg::reflectionTest::TestClass_A { u : ns::f(5) }"    "\n",
    // equivalent file
    "",
    // return value
    false,
    // first error
    ErrorType::read_non_const_variable_outside_function_or_template
},
{
    // description
    "function default arguments",
    // file content
    "namespace ns {"                                                "\n"
    "   function f(x, a = 2, b = -1, c = 1)"                        "\n"
    "   {"                                                          "\n"
    "       return a*x*x + b*x + c"                                 "\n"
    "   }"                                                          "\n"
    "}"                                                             "\n"
    "f1 is sg::reflectionTest::TestClass_A { u : ns::f(1) }"        "\n"
    "f2 is sg::reflectionTest::TestClass_A { u : ns::f(2) }"        "\n"
    "f3 is sg::reflectionTest::TestClass_A { u : ns::f(3) }"        "\n"
    "f4 is sg::reflectionTest::TestClass_A { u : ns::f(4) }"        "\n"
    "f5 is sg::reflectionTest::TestClass_A { u : ns::f(5, 0,2,4) }" "\n",
    // equivalent file
    "f1 is sg::reflectionTest::TestClass_A { u : 2 }"               "\n"
    "f2 is sg::reflectionTest::TestClass_A { u : 7 }"               "\n"
    "f3 is sg::reflectionTest::TestClass_A { u : 16 }"              "\n"
    "f4 is sg::reflectionTest::TestClass_A { u : 29 }"              "\n"
    "f5 is sg::reflectionTest::TestClass_A { u : 14 }"              "\n",
    // return value
    true,
    // first error
    ErrorType::unknown
},
{
    // description
    "function missing argument",
    // file content
    "namespace ns {"                                            "\n"
    "   function f(x, a = 2, b = -1, c = 1)"                    "\n"
    "   {"                                                      "\n"
    "       return a*x*x + b*x + c"                             "\n"
    "   }"                                                      "\n"
    "}"                                                         "\n"
    "f1 is sg::reflectionTest::TestClass_A { u : ns::f(1) }"    "\n"
    "f2 is sg::reflectionTest::TestClass_A { u : ns::f() }"     "\n",
    // equivalent file
    "",
    // return value
    false,
    // first error
    ErrorType::missing_argument_in_function_call
},
{
    // description
    "recurssive function",
    // file content
    "function factorial(n)"                                         "\n"
    "{"                                                             "\n"
    "   if(n < 0) return 0"                                         "\n"
    "   if(n == 0) return 1"                                        "\n"
    "   return factorial(n-1) * n"                                  "\n"
    "}"                                                             "\n"
    "f1 is sg::reflectionTest::TestClass_A { u : factorial(1) }"    "\n"
    "f2 is sg::reflectionTest::TestClass_A { u : factorial(2) }"    "\n"
    "f3 is sg::reflectionTest::TestClass_A { u : factorial(3) }"    "\n"
    "f4 is sg::reflectionTest::TestClass_A { u : factorial(4) }"    "\n"
    "f5 is sg::reflectionTest::TestClass_A { u : factorial(5) }"    "\n",
    // equivalent file
    "f1 is sg::reflectionTest::TestClass_A { u : 1 }"               "\n"
    "f2 is sg::reflectionTest::TestClass_A { u : 2 }"               "\n"
    "f3 is sg::reflectionTest::TestClass_A { u : 6 }"               "\n"
    "f4 is sg::reflectionTest::TestClass_A { u : 24 }"              "\n"
    "f5 is sg::reflectionTest::TestClass_A { u : 120 }"             "\n",
    // return value
    true,
    // first error
    ErrorType::unknown
},
{
    // description
    "unknown function call",
    // file content
    "function f(x) { return 2*x+1 }"                        "\n"
    "obj is sg::reflectionTest::TestClass_A { u : F(1) }"   "\n",
    // equivalent file
    "",
    // return value
    false,
    // first error
    ErrorType::callable_not_found
},
{
    // description
    "unknown function call in function",
    // file content
    "function f(x) { return 2*x+1 }"                        "\n"
    "function g(x) { return 2*F(x)+1 }"                     "\n"
    "obj is sg::reflectionTest::TestClass_A { u : g(1) }"   "\n",
    // equivalent file
    "",
    // return value
    false,
    // first error
    ErrorType::callable_not_found
},
{
    // description
    "missing function keyword in function declaration",
    // file content
    "f(x) { return 2*x+1 }"                                 "\n"
    "obj is sg::reflectionTest::TestClass_A { u : f(1) }"   "\n",
    // equivalent file
    "",
    // return value
    false,
    // first error
    ErrorType::callable_not_found
},
{
    // description
    "trying to call a variable",
    // file content
    "const f = 2"                                           "\n"
    "obj is sg::reflectionTest::TestClass_A { u : f(1) }"   "\n",
    // equivalent file
    "",
    // return value
    false,
    // first error
    ErrorType::expression_is_not_callable
},
{
    // description
    "trying to call an object",
    // file content
    "f is sg::reflectionTest::TestClass_A { u : 1 }"        "\n"
    "obj is sg::reflectionTest::TestClass_A { u : f(1) }"   "\n",
    // equivalent file
    "",
    // return value
    false,
    // first error
    ErrorType::callable_not_found
},
{
    // description
    "function cannot use gobal non-const variable",
    // file content
    "var a = 10"                                                "\n"
    "function f(x) { return x + a }"                            "\n"
    "a = 20"                                                    "\n"
    "obj1 is sg::reflectionTest::TestClass_C { u : f(5) }"      "\n",
    // equivalent file
    "",
    // return value
    false,
    // first error
    ErrorType::read_non_const_variable_outside_function_or_template
},
{
    // description
    "function can use gobal const variable",
    // file content
    "const a = 10"                                              "\n"
    "function f(x) { return x + a }"                            "\n"
    "obj1 is sg::reflectionTest::TestClass_C { u : f(5) }"      "\n",
    // equivalent file
    "obj1 is sg::reflectionTest::TestClass_C { u : 15 }"        "\n",
    // return value
    true,
    // first error
    ErrorType::unknown
},
{
    // description
    "asserts in function",
    // file content
    "const a = 10"                                              "\n"
    "function f(x) {"                                           "\n"
    "    assert a > 0"                                          "\n"
    "    const y = x + a"                                       "\n"
    "    assert y > 0"                                          "\n"
    "    return y"                                              "\n"
    "}"                                                         "\n"
    "obj1 is sg::reflectionTest::TestClass_C { u : f(5) }"      "\n",
    // equivalent file
    "obj1 is sg::reflectionTest::TestClass_C { u : 15 }"        "\n",
    // return value
    true,
    // first error
    ErrorType::unknown
},
{
    // description
    "indexing in function",
    // file content
    "const p = [ 2, 3, 5, 7, 11, 13, 17, 19 ]"                  "\n"
    "function f(x) {"                                           "\n"
    "    const a = p[3]"                                        "\n"
    "    const b = p[x % 8]"                                    "\n"
    "    return a * b"                                          "\n"
    "}"                                                         "\n"
    "obj1 is sg::reflectionTest::TestClass_C { u : f(5) }"      "\n",
    // equivalent file
    "obj1 is sg::reflectionTest::TestClass_C { u : 91 }"        "\n",
    // return value
    true,
    // first error
    ErrorType::unknown
},
{
    // description
    "for in function",
    // file content
    "const p = [ 2, 3, 5, 7, 11, 13, 17, 19 ]"                  "\n"
    "function f(x) {"                                           "\n"
    "    var a = 1"                                             "\n"
    "    for(var i = 0; i < 8; ++i)"                            "\n"
    "    {"                                                     "\n"
    "        const b = p[i]"                                    "\n"
    "        if(b > x) break"                                   "\n"
    "        a *= b"                                            "\n"
    "    }"                                                     "\n"
    "    return a"                                              "\n"
    "}"                                                         "\n"
    "obj1 is sg::reflectionTest::TestClass_C { u : f(5) }"      "\n"
    "obj2 is sg::reflectionTest::TestClass_C { u : f(10) }"     "\n",
    // equivalent file
    "obj1 is sg::reflectionTest::TestClass_C { u : 30 }"        "\n"
    "obj2 is sg::reflectionTest::TestClass_C { u : 210 }"       "\n",
    // return value
    true,
    // first error
    ErrorType::unknown
},
{
    // description
    "for (list) in function",
    // file content
    "const p = [ 2, 3, 5, 7, 11, 13, 17, 19 ]"                  "\n"
    "function f(x) {"                                           "\n"
    "    var a = 1"                                             "\n"
    "    for(b in p)"                                           "\n"
    "    {"                                                     "\n"
    "        if(b > x) break"                                   "\n"
    "        a *= b"                                            "\n"
    "    }"                                                     "\n"
    "    return a"                                              "\n"
    "}"                                                         "\n"
    "obj1 is sg::reflectionTest::TestClass_C { u : f(5) }"      "\n"
    "obj2 is sg::reflectionTest::TestClass_C { u : f(10) }"     "\n",
    // equivalent file
    "obj1 is sg::reflectionTest::TestClass_C { u : 30 }"        "\n"
    "obj2 is sg::reflectionTest::TestClass_C { u : 210 }"       "\n",
    // return value
    true,
    // first error
    ErrorType::unknown
},
{
    // description
    "while in function",
    // file content
    "const p = [ 2, 3, 5, 7, 11, 13, 17, 19 ]"                  "\n"
    "function f(x) {"                                           "\n"
    "    var a = 1"                                             "\n"
    "    var b = 0"                                             "\n"
    "    while(b < 8 && p[b] <= x)"                             "\n"
    "    {"                                                     "\n"
    "        a *= p[b]"                                         "\n"
    "        ++b"                                               "\n"
    "    }"                                                     "\n"
    "    return a"                                              "\n"
    "}"                                                         "\n"
    "obj1 is sg::reflectionTest::TestClass_C { u : f(5) }"      "\n"
    "obj2 is sg::reflectionTest::TestClass_C { u : f(10) }"     "\n",
    // equivalent file
    "obj1 is sg::reflectionTest::TestClass_C { u : 30 }"        "\n"
    "obj2 is sg::reflectionTest::TestClass_C { u : 210 }"       "\n",
    // return value
    true,
    // first error
    ErrorType::unknown
},
{
    // description
    "scope in function",
    // file content
    "function f(x) {"                                           "\n"
    "    var a = 1"                                             "\n"
    "    {"                                                     "\n"
    "        const p = [ 2, 3, 5, 7, 11, 13, 17, 19 ]"          "\n"
    "        a = p[x]"                                          "\n"
    "    }"                                                     "\n"
    "    const p = a * 2"                                       "\n"
    "    return p"                                              "\n"
    "}"                                                         "\n"
    "obj1 is sg::reflectionTest::TestClass_C { u : f(2) }"      "\n"
    "obj2 is sg::reflectionTest::TestClass_C { u : f(5) }"      "\n",
    // equivalent file
    "",
    // return value
    false,
    // first error
    ErrorType::object_definition_forbidden_in_context
},
{
    // description
    "functions in nested namespace",
    // file content
    "namespace n {"                                             "\n"
    "    function f(x) {"                                       "\n"
    "        return x+1"                                        "\n"
    "    }"                                                     "\n"
    "    namespace m {"                                         "\n"
    "        function f(x) {"                                   "\n"
    "            return ::n::f(x + 2)"                          "\n"
    "        }"                                                 "\n"
    "    }"                                                     "\n"
    "}"                                                         "\n"
    "obj1 is sg::reflectionTest::TestClass_C { u : ::n::f(1) }" "\n"
    "obj2 is sg::reflectionTest::TestClass_C { u : n::m::f(1) }""\n",
    // equivalent file
    "obj1 is sg::reflectionTest::TestClass_C { u : 2 }"         "\n"
    "obj2 is sg::reflectionTest::TestClass_C { u : 4 }"         "\n",
    // return value
    true,
    // first error
    ErrorType::unknown
},
