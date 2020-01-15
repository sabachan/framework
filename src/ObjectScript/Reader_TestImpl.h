#ifndef ObjectScript_Reader_Include_TestImpl
#error "this file must be used only by Reader.cpp"
#endif

// description                  file content                                    eq. file    return val. firstError
{ "variables",                  "var a=1 const b=1+a*3 a+=b assert a==5",       "",         true,       ErrorType::unknown},
{ "unclosed comment",           "var i = 0 /* unclosed comment /",              "",         false,      ErrorType::unexpected_end_of_file_in_comment_bloc },
{ "invalid identifier",         "var i = 0 j### = 2",                           "",         false,      ErrorType::invalid_character_sequence },
{ "octal",                      "var i = 042",                                  "",         false,      ErrorType::octal_format_is_forbidden },
{ "integer value ok",           "o is sg::reflectionTest::TestClass_A{u: 4294967295}",
                                "o is sg::reflectionTest::TestClass_A{u: 4294967295}",      true,       ErrorType::unknown },
{ "int value too big",          "o is sg::reflectionTest::TestClass_A{u: 4294967296}",
                                                                                "",         false,      ErrorType::integer_value_too_big },
{ "int value too big (hex)",    "o is sg::reflectionTest::TestClass_A{u: 0x100000000}",
                                                                                "",         false,      ErrorType::integer_value_too_big },
{ "unexpected eof",             "var i = 0 var j = i +",                        "",         false,      ErrorType::unexpected_end_of_file_value_expected },
{ "eof in bloc",                "function f(x) { return 2*x+1",                 "",         false,      ErrorType::unexpected_end_of_file_in_bloc },
{ "eof in bracket",             "var tab = [10, 3",                             "",         false,      ErrorType::unexpected_end_of_file_in_bracket },
{ "eof in parenthesis",         "function f(x){return 2*x} var i=f(3",          "",         false,      ErrorType::unexpected_end_of_file_in_parenthesis },
{ "eof in construct",           "template tplt(x, y) is",                       "",         false,      ErrorType::unexpected_end_of_file_in_construct },
{ "eof in ternary",             "var i=0 var j = i>10 ? 3",                     "",         false,      ErrorType::unexpected_end_of_file_in_ternary_op },
{ "missing value",              "var i=0 for(var j=0;j<2;++j) { i-= }",         "",         false,      ErrorType::value_expected },
{ "missing value",              "var i=1 + for(var j=0;j<2;++j) i+=3",          "",         false,      ErrorType::value_expected },
{ "unexpected namespace",       "var a=1 + namespace sg { b = 2} 3",            "",         false,      ErrorType::value_expected },
{ "for too many semicolons",    "var i=1 for(var j=0;j<2;++j;++i) i+=3*j",      "",         false,      ErrorType::for_too_many_semicolons },
{ "for cond too many expr",     "var i=1 for(var j=0;j<2 i<4;++j ++i) i+=3*j",  "",         false,      ErrorType::for_invalid_construct },
{ "for empty parenthesis",      "var i=1 for() i+=3",                           "",         false,      ErrorType::for_empty_construct },
{ "for missing cond incr",      "var i=1 for(var j=0) i+=3",                    "",         false,      ErrorType::for_invalid_construct },
{ "for missing incr",           "var i=1 for(var j=0;j<2) i+=3",                "",         false,      ErrorType::for_invalid_construct },
{ "for(;)",                     "var i=1 for(;) i+=3",                          "",         false,      ErrorType::for_invalid_construct },
{ "for(;;)",                    "var i=1 for(;;) { i+=3 break }",               "",         true,       ErrorType::unknown},
{ "for(;str;)",                 "var i=1 for(;\"hello\";) { i+=3 }",            "",         false,      ErrorType::unexpected_type_for_condition},
{ "for(in) missing set",        "var i=1 for(var j in ) i+=3",                  "",         false,      ErrorType::for_invalid_construct },
{ "while too many expr",        "var i=1 while(0<i i<10) i+=3",                 "",         false,      ErrorType::while_invalid_condition },
{ "while()",                    "var i=1 while() i+=3",                         "",         false,      ErrorType::while_empty_condition },
{ "while(str)",                 "var i=1 while(\"hello\") i+=3",                "",         false,      ErrorType::unexpected_type_for_condition },
{ "if()",                       "var i=0 var j=0 while(++i<10) if() ++j",       "",         false,      ErrorType::if_empty_condition},
{ "if()",                       "var i=0 var j=0 if(i<0 j>0) ++j",              "",         false,      ErrorType::if_invalid_condition},
{ "if(str)",                    "var i=0 var j=0 if(\"Hello world\") ++j",      "",         false,      ErrorType::unexpected_type_for_condition},
{ "int - []",                   "obj is sg::reflectionTest::TestClass_A { u : 1-[2] }",
                                                                                "",         false,      ErrorType::unsupported_types_for_binary_operator },
{ "int /= float",               "var i=1  i /= 2.0",                            "",         false,      ErrorType::unsupported_type_for_compound_assignment },
{ "!int",                       "var i=0  var j = !i",                          "",         false,      ErrorType::unsupported_type_for_unary_operator },
{ "string++",                   "var i=\"hello world!\"  i++",                  "",         false,      ErrorType::unsupported_types_for_increment_operator },
{ "if missing parentheses",     "var i=0 j=0 while(++i<10) if i>5 ++j",         "",         false,      ErrorType::if_missing_parentheses },
{ "for missing parentheses",    "var j=[] for var i in [1,3] j += i*4",         "",         false,      ErrorType::for_missing_parentheses },
{ "while miss. parentheses",    "var i=0 j=0 while ++i<10 if(i>5) ++j",         "",         false,      ErrorType::while_missing_parentheses },
{ "incorrect use of in",        "var i=0 t=[2,3,5] while(j in t) i+=j",         "",         false,      ErrorType::incorrect_use_of_keyword_in },
{ "incorrect use of semicolon", "var i = 1;",                                   "",         false,      ErrorType::incorrect_use_of_semicolon},
{ "unexpected qualifiers",      "var i=0 var j = const i = 2 var k = j+1",      "",         false,      ErrorType::value_expected },
{ "incorrect qualifier",        "private i = 2",                                "",         false,      ErrorType::incorrect_use_of_qualifier_for_variable_definition },
{ "divide by 0",                "var i=3 var j=0 var k=i/j",                    "",         false,      ErrorType::divide_by_zero },
{ "assert ok",                  "var i=0 for(var j=0;j<5;j++){i+=2} assert i==10", "",      true,       ErrorType::unknown },
{ "assert failed",              "var i=0 for(var j=0;j<5;j++){i+=2} assert i<10",  "",      false,      ErrorType::assert_failed },
{ "incorrect alias name",       "alias 1+1",                                    "",         false,      ErrorType::expected_identifier_for_alias_name },
{ "alias missing is",           "alias MyAlias MyClass",                        "",         false,      ErrorType::alias_missing_is },
{ "incorrect alias type",       "alias MyAlias is 1024",                        "",         false,      ErrorType::expected_type_for_alias_type },
{ "invalid ternary cond",       "var b=true var s=\"a\"+b?\"b\":\"c\"",         "",         false,      ErrorType::incorrect_type_for_ternary_operator_condition },
{ "if else too far",            "var a=0 var b=1 if(a==b) a=1 b=2 else b=3",    "",         false,      ErrorType::incorrect_use_of_keyword_else},
{ "incorrect use of function",  "function a(){} b=1+a*3",                       "",         false,      ErrorType::unsupported_types_for_binary_operator},
{ "split identifier",           "var my variable = 1",                          "",         false,      ErrorType::expression_is_not_an_instruction},
{ "value != instruction",       "var a = 1 a + 3",                              "",         false,      ErrorType::expression_is_not_an_instruction},

{ "use of class as value",      "var i=sg::reflectionTest::TestClass_A",        "",         false,      ErrorType::use_of_class_as_value},

{ "empty object",               "obj1 is sg::reflectionTest::TestClass_A {}",
                                      "obj1 is sg::reflectionTest::TestClass_A {}",         true,       ErrorType::unknown },
{ "object without bloc",        "sg::reflectionTest::TestClass_A",              "",         false,      ErrorType::expression_is_not_an_instruction},
{ "named object without bloc",  "obj1 is sg::reflectionTest::TestClass_A",      "",         false,      ErrorType::object_definition_expects_an_object},
{ "assign to class",            "sg::reflectionTest::TestClass_A=1",            "",         false,      ErrorType::assign_to_class},
{ "unresolved identifier 1",    "const a = 0 c = a + b",                        "",         false,      ErrorType::unresolved_identifier_in_expression},
{ "unresolved identifier 2",    "const a = 0 c = b * a",                        "",         false,      ErrorType::unresolved_identifier_in_expression},
{ "unresolved identifier 2",    "const b = -a",                                 "",         false,      ErrorType::unresolved_identifier_in_expression},

{ "lone continue",              "continue",                                     "",         false,      ErrorType::unexpected_use_of_jump_statement},
{ "lone break",                 "break",                                        "",         false,      ErrorType::unexpected_use_of_jump_statement},
{ "lone return",                "const a = 0 return a",                         "",         false,      ErrorType::unexpected_use_of_jump_statement},
{ "return missing value",       "function f(x) { const y = x*x return }",       "",         false,      ErrorType::missing_term_after_prefix_operator},
{ "lone return missing value",  "return",                                       "",         false,      ErrorType::missing_term_after_prefix_operator},
{ "function f() {)",            "function f(x, y) { return x + y )",            "",         false,      ErrorType::unexpected_closing_token},
{ "function f() {]",            "function f(x, y) { return x + y ]",            "",         false,      ErrorType::unexpected_closing_token},
{ "function f(}",               "function f(x, y} { return x + y }",            "",         false,      ErrorType::unexpected_closing_token},
{ "function f(]",               "function f(x, y] { return x + y }",            "",         false,      ErrorType::unexpected_closing_token},
{ "{)",                         "sg::reflectionTest::TestClass_A{u: 0)",        "",         false,      ErrorType::unexpected_closing_token},
{ "[}",                         "var v = [ 0, 1, 2 }",                          "",         false,      ErrorType::unexpected_closing_token},

{ "unknown property name",      "sg::reflectionTest::TestClass_A{zz: 0}",       "",         false,      ErrorType::unknown_property_name},
{ "function as property name",  "sg::reflectionTest::TestClass_A{function: 0}", "",         false,      ErrorType::missing_function_name},
{ "for as property name",       "sg::reflectionTest::TestClass_A{for: 0}",      "",         false,      ErrorType::for_missing_parentheses},
{ "return as property name",    "sg::reflectionTest::TestClass_A{return: 0}",   "",         false,      ErrorType::incorrect_use_of_operator},
{ "missing namespace name",     "namespace{ var a = 0 }",                       "",         false,      ErrorType::missing_namespace_name},

{ "missing comma in array",     "var t = [0, 1 2]",                             "",         false,      ErrorType::syntax_error_missing_comma},

{
    // description
    "assert message failed",
    // file content
    "var i=0 while(i++<5){i+=2} assert i<6, \"i should be less than 6\"",
    // equivalent file
    "",
    // return value
    false,
    // first error
    ErrorType::assert_failed
},
{
    // description
    "visibility keywords",
    // file content
    "export obj1 is sg::reflectionTest::TestClass_A {u:1}"      "\n"
    "public obj2 is sg::reflectionTest::TestClass_A {u:2}"      "\n"
    "protected obj3 is sg::reflectionTest::TestClass_A {u:3}"   "\n"
    "private obj4 is sg::reflectionTest::TestClass_A {u:4}"     "\n"
    "export sg::reflectionTest::TestClass_A {u:5}"              "\n"
    "public sg::reflectionTest::TestClass_A {u:6}"              "\n"
    "protected sg::reflectionTest::TestClass_A {u:7}"           "\n"
    "private sg::reflectionTest::TestClass_A {u:8}"             "\n",
    // equivalent file
    "export obj1 is sg::reflectionTest::TestClass_A {u:1}"      "\n"
    "public obj2 is sg::reflectionTest::TestClass_A {u:2}"      "\n"
    "protected obj3 is sg::reflectionTest::TestClass_A {u:3}"   "\n"
    "private obj4 is sg::reflectionTest::TestClass_A {u:4}"     "\n"
    "export sg::reflectionTest::TestClass_A {u:5}"              "\n"
    "public sg::reflectionTest::TestClass_A {u:6}"              "\n"
    "protected sg::reflectionTest::TestClass_A {u:7}"           "\n"
    "private sg::reflectionTest::TestClass_A {u:8}"             "\n",
    // return value
    true,
    // first error
    ErrorType::unknown
},
{
    // description
    "1 element array",
    // file content
    "sg::reflectionTest::TestClass_C {"         "\n"
    "   vectoru : [ 10, ]"                      "\n"
    "}"                                         "\n",
    // equivalent file
    "sg::reflectionTest::TestClass_C {"         "\n"
    "   vectoru : [ 10 ]"                       "\n"
    "}"                                         "\n",
    // return value
    true,
    // first error
    ErrorType::unknown
},
{
    // description
    "pairs in array",
    // file content
    "sg::reflectionTest::TestClass_C {"         "\n"
    "   vectorPair : [ [10, 0.02], [1, 4.3], ]" "\n"
    "}"                                         "\n",
    // equivalent file
    "sg::reflectionTest::TestClass_C {"         "\n"
    "   vectorPair : [ [10, 0.02], [1, 4.3] ]"  "\n"
    "}"                                         "\n",
    // return value
    true,
    // first error
    ErrorType::unknown
},
{
    // description
    "1 pair in array",
    // file content
    "sg::reflectionTest::TestClass_C {"         "\n"
    "   vectorPair : [ [10, 0.02], ]"           "\n"
    "}"                                         "\n",
    // equivalent file
    "sg::reflectionTest::TestClass_C {"         "\n"
    "   vectorPair : [ [10, 0.02] ]"            "\n"
    "}"                                         "\n",
    // return value
    true,
    // first error
    ErrorType::unknown
},
{
    // description
    "struct property as array",
    // file content
    "sg::reflectionTest::TestClass_C {"         "\n"
    "   structB : [ 2, 0.5 ]"                   "\n"
    "}"                                         "\n",
    // equivalent file
    "sg::reflectionTest::TestClass_C {"         "\n"
    "   structB : [ 2, 0.5 ]"                   "\n"
    "}"                                         "\n",
    // return value
    true,
    // first error
    ErrorType::unknown
},
{
    // description
    "struct property as struct",
    // file content
    "sg::reflectionTest::TestClass_C {"         "\n"
    "   structB : { u:2 f:0.5 }"                "\n"
    "}"                                         "\n",
    // equivalent file
    "sg::reflectionTest::TestClass_C {"         "\n"
    "   structB : [ 2, 0.5 ]"                   "\n"
    "}"                                         "\n",
    // return value
    true,
    // first error
    ErrorType::unknown
},
{
    // description
    "binary operations int int",
    // file content
    "const a = 7 const b = 3"                   "\n"
    "sg::reflectionTest::TestClass_C {"         "\n"
    "   vectoru : ["                            "\n"
    "       a+b,"                               "\n"
    "       a-b,"                               "\n"
    "       a*b,"                               "\n"
    "       a/b,"                               "\n"
    "       a%b,"                               "\n"
    "       a|b,"                               "\n"
    "       a&b,"                               "\n"
    "       a^b,"                               "\n"
    "       a<<b,"                              "\n"
    "       a>>b,"                              "\n"
    "   ]"                                      "\n"
    "}"                                         "\n",
    // equivalent file
    "sg::reflectionTest::TestClass_C {"         "\n"
    "   vectoru : ["                            "\n"
    "       10, //a+b"                          "\n"
    "       4,  //a-b"                          "\n"
    "       21, //a*b"                          "\n"
    "       2,  //a/b"                          "\n"
    "       1,  //a%b"                          "\n"
    "       7,  //a|b"                          "\n"
    "       3,  //a&b"                          "\n"
    "       4,  //a^b"                          "\n"
    "       56, //a<<b"                         "\n"
    "       0,  //a>>b"                         "\n"
    "   ]"                                      "\n"
    "}"                                         "\n",
    // return value
    true,
    // first error
    ErrorType::unknown
},
{
    // description
    "identifierize",
    // file content
    "const i = 10"                                          "\n"
    "$(\"object\") is sg::reflectionTest::TestClass_A {}"   "\n"
    "$(\"object\"+i) is sg::reflectionTest::TestClass_A {}" "\n"
    "const name = \"myobject\""                             "\n"
    "$name is sg::reflectionTest::TestClass_A {}"           "\n"
    "$(name+(i+1)) is sg::reflectionTest::TestClass_A {}"   "\n"
    "$(name+i+1) is sg::reflectionTest::TestClass_A {}"     "\n",
    // equivalent file
    "object is sg::reflectionTest::TestClass_A {}"          "\n"
    "object10 is sg::reflectionTest::TestClass_A {}"        "\n"
    "myobject is sg::reflectionTest::TestClass_A {}"        "\n"
    "myobject11 is sg::reflectionTest::TestClass_A {}"      "\n"
    "myobject101 is sg::reflectionTest::TestClass_A {}"     "\n",
    // return value
    true,
    // first error
    ErrorType::unknown
},
{
    // description
    "prefix operator grammar",
    // file content
    "function sq(x) { return x * x }"           "\n"
    "var a = 0"                                 "\n"
    "const b = [3, 5, 7, 11, 13 ]"              "\n"
    "const c = 0 ++a"                           "\n"
    "const d = b[a] ++a"                        "\n"
    "const e = sq(a) ++a"                       "\n"
    "a++ ++a"                                   "\n"
    "const f = e + a ++a"                       "\n"
    "sg::reflectionTest::TestClass_C {"         "\n"
    "   vectoru : [ c, d, e, f, a ]"            "\n"
    "}"                                         "\n",
    // equivalent file
    "sg::reflectionTest::TestClass_C {"         "\n"
    "   vectoru : [ 0, 5, 4, 9, 6 ]"            "\n"
    "}"                                         "\n",
    // return value
    true,
    // first error
    ErrorType::unknown
},
{
    // description
    "for construct 1",
    // file content
    "var tab = []"                              "\n"
    "for(var i=0; i<6; i++)"                    "\n"
    "   tab += [i*i]"                           "\n"
    "sg::reflectionTest::TestClass_C {"         "\n"
    "   vectoru : tab"                          "\n"
    "}"                                         "\n",
    // equivalent file
    "sg::reflectionTest::TestClass_C {"         "\n"
    "   vectoru : [0,1,4,9,16,25]"              "\n"
    "}"                                         "\n",
    // return value
    true,
    // first error
    ErrorType::unknown
},
{
    // description
    "for with multiple instructions in parentheses",
    // file content
    "var tab = []"                                  "\n"
    "for(var i=0 i--; i<6; i++ tab += [i*i]) {}"    "\n"
    "sg::reflectionTest::TestClass_C {"             "\n"
    "   vectoru : tab"                              "\n"
    "}"                                             "\n",
    // equivalent file
    "sg::reflectionTest::TestClass_C {"             "\n"
    "   vectoru : [0,1,4,9,16,25,36]"               "\n"
    "}"                                             "\n",
    // return value
    true,
    // first error
    ErrorType::unknown
},
{
    // description
    "for construct 2",
    // file content
    "var tab = []"                              "\n"
    "const list = [0,1,2,4,5,7]"                "\n"
    "for(i in list)"                            "\n"
    "   tab += [i*i]"                           "\n"
    "sg::reflectionTest::TestClass_C {"         "\n"
    "   vectoru : tab"                          "\n"
    "}"                                         "\n",
    // equivalent file
    "sg::reflectionTest::TestClass_C {"         "\n"
    "   vectoru : [0,1,4,16,25,49]"             "\n"
    "}"                                         "\n",
    // return value
    true,
    // first error
    ErrorType::unknown
},
{
    // description
    "break/continue",
    // file content
    "var i = 0"                                     "\n"
    "var j = 0"                                     "\n"
    "while(true)"                                   "\n"
    "{"                                             "\n"
    "   i++"                                        "\n"
    "   if(10 == i) break"                          "\n"
    "   if(i%2 == 0) continue"                      "\n"
    "   j += i"                                     "\n"
    "   sg::reflectionTest::TestClass_A { u : j }"  "\n"
    "}"                                             "\n",
    // equivalent file
    "sg::reflectionTest::TestClass_A { u : 1 }"     "\n"
    "sg::reflectionTest::TestClass_A { u : 4 }"     "\n"
    "sg::reflectionTest::TestClass_A { u : 9 }"     "\n"
    "sg::reflectionTest::TestClass_A { u : 16 }"    "\n"
    "sg::reflectionTest::TestClass_A { u : 25 }"    "\n",
    // return value
    true,
    // first error
    ErrorType::unknown
},
{
    // description
    "one instruction loops",
    // file content
    "function abs(x)"                                   "\n"
    "{"                                                 "\n"
    "    if(x < 0)"                                     "\n"
    "        return -x"                                 "\n"
    "    else"                                          "\n"
    "        return x"                                  "\n"
    "}"                                                 "\n"
    "function f(x, y)"                                  "\n"
    "{"                                                 "\n"
    "    var v = x"                                     "\n"
    "    if(v < y)"                                     "\n"
    "    {"                                             "\n"
    "        while(v < y)"                              "\n"
    "            v += 10"                               "\n"
    "    }"                                             "\n"
    "    return v"                                      "\n"
    "}"                                                 "\n"
    "function m(x, y)"                                  "\n"
    "{"                                                 "\n"
    "    var v = 0"                                     "\n"
    "    for(var i = 0; i < x; ++i)"                    "\n"
    "    {"                                             "\n"
    "       for(var j = 0; j < y; ++j)"                 "\n"
    "            v += 1"                                "\n"
    "    }"                                             "\n"
    "    return v"                                      "\n"
    "}"                                                 "\n"
    "function g(x, y)"                                  "\n"
    "{"                                                 "\n"
    "    var v = 0"                                     "\n"
    "    for(var i = 0; i < x; ++i)"                    "\n"
    "    {"                                             "\n"
    "       v += 10"                                    "\n"
    "       if( v > y )"                                "\n"
    "            return v"                              "\n"
    "    }"                                             "\n"
    "    return v"                                      "\n"
    "}"                                                 "\n"
    "sg::reflectionTest::TestClass_A { u : abs(2) }"    "\n"
    "sg::reflectionTest::TestClass_A { u : abs(-6) }"   "\n"
    "sg::reflectionTest::TestClass_A { u : f(4, 55) }"  "\n"
    "sg::reflectionTest::TestClass_A { u : m(4, 3) }"   "\n"
    "sg::reflectionTest::TestClass_A { u : g(6, 34) }"  "\n"
    "sg::reflectionTest::TestClass_A { u : g(6, 99) }"  "\n",
    // equivalent file
    "sg::reflectionTest::TestClass_A { u : 2 }"         "\n"
    "sg::reflectionTest::TestClass_A { u : 6 }"         "\n"
    "sg::reflectionTest::TestClass_A { u : 64 }"        "\n"
    "sg::reflectionTest::TestClass_A { u : 12 }"        "\n"
    "sg::reflectionTest::TestClass_A { u : 40 }"        "\n"
    "sg::reflectionTest::TestClass_A { u : 60 }"        "\n",
    // return value
    true,
    // first error
    ErrorType::unknown
},
{
    // description
    "visibility keywords and templates",
    // file content
    "template tplObj("                                          "\n"
    "    x = 1,"                                                "\n"
    ") is sg::reflectionTest::TestClass_A {"                    "\n"
    "    u : x"                                                 "\n"
    "}"                                                         "\n"
    "export obj1 is tplObj {}"                                  "\n"
    "public obj2 is tplObj { x:2 }"                             "\n"
    "protected obj3 is tplObj { x:3 }"                          "\n"
    "private obj4 is tplObj { x:4 }"                            "\n",
    // equivalent file
    "export obj1 is sg::reflectionTest::TestClass_A {u:1}"      "\n"
    "public obj2 is sg::reflectionTest::TestClass_A {u:2}"      "\n"
    "protected obj3 is sg::reflectionTest::TestClass_A {u:3}"   "\n"
    "private obj4 is sg::reflectionTest::TestClass_A {u:4}"     "\n",
    // return value
    true,
    // first error
    ErrorType::unknown
},
{
    // description
    "incorrect class name",
    // file content
    "obj1 is reflectionTest::TestClass_A { u : 1 }"         "\n"
    "obj2 is sg::reflectionTest::TestClass_A { u : 2 }"     "\n",
    // equivalent file
    "",
    // return value
    false,
    // first error
    ErrorType::class_not_found
},
{
    // description
    "object name collision",
    // file content
    "namespace myNamespace {"                                       "\n"
    "   obj is sg::reflectionTest::TestClass_A { u : 1 }"           "\n"
    "   obj is sg::reflectionTest::TestClass_C { u : 2 f:1.5 }"     "\n"
    "}"                                                             "\n",
    // equivalent file
    "",
    // return value
    false,
    // first error
    ErrorType::object_name_collision
},
{
    // description
    "object name collision with class, exact match",
    // file content
    "namespace sg { namespace reflectionTest {"                     "\n"
    "   TestClass_C is sg::reflectionTest::TestClass_A { u : 1 }"   "\n"
    "} }"                                                           "\n",
    // equivalent file
    "",
    // return value
    false,
    // first error
    ErrorType::object_name_collision_with_class
},
{
    // description
    "object name collision with class",
    // file content
    "namespace sg { namespace reflectionTest { namespace subns {"   "\n"
    "   TestClass_C is sg::reflectionTest::TestClass_A { u : 1 }"   "\n"
    "} } }"                                                         "\n",
    // equivalent file
    "",
    // return value
    false,
    // first error
    ErrorType::object_name_collision_with_class
},
{
    // description
    "object name collision with script object",
    // file content
    "namespace ns {"                                                "\n"
    "   function obj(x) { return x*x + 2 * x - 3 }"                 "\n"
    "   namespace subns {"                                          "\n"
    "       obj is sg::reflectionTest::TestClass_A { u : 1 }"       "\n"
    "} }"                                                           "\n",
    // equivalent file
    "",
    // return value
    false,
    // first error
    ErrorType::object_name_collision_with_script_object
},
{
    // description
    "variable name collision",
    // file content
    "namespace myNamespace {"   "\n"
    "   const i = 0"            "\n"
    "   var i = 2"              "\n"
    "}"                         "\n",
    // equivalent file
    "",
    // return value
    false,
    // first error
    ErrorType::name_already_defined
},
{
    // description
    "variable name collision, non exact",
    // file content
    "namespace myNamespace {"   "\n"
    "   const i = 0"            "\n"
    "   namespace subns {"      "\n"
    "       var i = 2"          "\n"
    "   }"                      "\n"
    "}"                         "\n",
    // equivalent file
    "",
    // return value
    false,
    // first error
    ErrorType::name_already_defined_in_outer_scope
},
{
    // description
    "variable name collision, non exact",
    // file content
    "namespace myNamespace {"   "\n"
    "   const i = 0"            "\n"
    "   function f(n) {"        "\n"
    "       var i = n"          "\n"
    "   }"                      "\n"
    "}"                         "\n",
    // equivalent file
    "",
    // return value
    false,
    // first error
    ErrorType::name_already_defined_in_outer_scope
},
{
    // description
    "namespace name collision with class",
    // file content
    "namespace sg {"                                                "\n"
    "   namespace reflectionTest {"                                 "\n"
    "       namespace TestClass_C {"                                "\n"
    "           obj is sg::reflectionTest::TestClass_A { u : 1 }"   "\n"
    "} } }"                                                         "\n",
    // equivalent file
    "",
    // return value
    false,
    // first error
    ErrorType::namespace_name_collision_with_class
},
{
    // description
    "namespace name collision with script object",
    // file content
    "namespace ns {"                                                "\n"
    "   function f(x) { return x*x + 2 * x - 3 }"                   "\n"
    "   namespace f {"                                              "\n"
    "       obj is sg::reflectionTest::TestClass_A { u : 1 }"       "\n"
    "} }"                                                           "\n",
    // equivalent file
    "",
    // return value
    false,
    // first error
    ErrorType::namespace_name_collision_with_script_object
},
{
    // description
    "function name collision with class",
    // file content
    "namespace sg { namespace reflectionTest {"                     "\n"
    "   function TestClass_C(x) { return x*x + 2 * x - 3 }"         "\n"
    "} }"                                                           "\n",
    // equivalent file
    "",
    // return value
    false,
    // first error
    ErrorType::function_name_collision_with_class
},
{
    // description
    "template name collision with class",
    // file content
    "namespace sg { namespace reflectionTest {"                     "\n"
    "   template TestClass_C(x, y)"                                 "\n"
    "   is ::sg::reflectionTest::TestClass_A"                       "\n"
    "   {"                                                          "\n"
    "       u = x + y"                                              "\n"
    "   }"                                                          "\n"
    "} }"                                                           "\n"
    "obj is sg::reflectionTest::TestClass_C { x : 1 y : 5 }"        "\n",
    // equivalent file
    "",
    // return value
    false,
    // first error
    ErrorType::template_name_collision_with_class
},
{
    // description
    "class in namespace",
    // file content
    "namespace sg {"                                            "\n"
    "   obj1 is reflectionTest::TestClass_A { u : 1 }"          "\n"
    "   obj2 is sg::reflectionTest::TestClass_A { u : 2 }"      "\n"
    "}"                                                         "\n",
    // equivalent file
    "namespace sg {"                                            "\n"
    "   obj1 is sg::reflectionTest::TestClass_A { u : 1 }"      "\n"
    "   obj2 is sg::reflectionTest::TestClass_A { u : 2 }"      "\n"
    "}"                                                         "\n",
    // return value
    true,
    // first error
    ErrorType::unknown
},
{
    // description
    "object reference in variable",
    // file content
    "obj1 is sg::reflectionTest::TestClass_C { u : 1 }"         "\n"
    "var obj1_reference = obj1"                                 "\n"
    "obj2 is sg::reflectionTest::TestClass_D {"                 "\n"
    "   object : obj1_reference"                                "\n"
    "}"                                                         "\n",
    // equivalent file
    "obj1 is sg::reflectionTest::TestClass_C { u : 1 }"         "\n"
    "obj2 is sg::reflectionTest::TestClass_D {"                 "\n"
    "   object : ::obj1"                                        "\n"
    "}"                                                         "\n",
    // return value
    true,
    // first error
    ErrorType::unknown
},
#if 0
{
    // description
    "object in variable",
    // file content
    "var obj1_reference = obj1 is sg::reflectionTest::TestClass_C { u : 1 }"    "\n"
    "obj2 is sg::reflectionTest::TestClass_D {"                                 "\n"
    "   object : obj1_reference"                                                "\n"
    "}"                                                                         "\n",
    // equivalent file
    "obj1 is sg::reflectionTest::TestClass_C { u : 1 }"                         "\n"
    "obj2 is sg::reflectionTest::TestClass_D {"                                 "\n"
    "   object : ::obj1"                                                        "\n"
    "}"                                                                         "\n",
    // return value
    true,
    // first error
    ErrorType::unknown
},
{
    // description
    "anonymous object in variable",
    // file content
    "var obj1 = sg::reflectionTest::TestClass_C { u : 1 }"      "\n"
    "obj2 is sg::reflectionTest::TestClass_D {"                 "\n"
    "   object : obj1"                                          "\n"
    "}"                                                         "\n",
    // equivalent file
    "obj2 is sg::reflectionTest::TestClass_D {"                 "\n"
    "   object : sg::reflectionTest::TestClass_C { u : 1 }"     "\n"
    "}"                                                         "\n",
    // return value
    // return value
    true,
    // first error
    ErrorType::unknown
},
#endif


#if SG_OBJECTSCRIPT_TYPEDEF_IS_DEPRECATED
{
    // description
    "class typedef",
    // file content
    "typedef sg::reflectionTest::TestClass_C ClassC"            "\n"
    "obj1 is ClassC { u : 1 }"                                  "\n",
    // equivalent file
    "",
    // return value
    false,
    // first error
    ErrorType::typedef_is_deprecated
},
#else
{
    // description
    "class typedef",
    // file content
    "typedef sg::reflectionTest::TestClass_C ClassC"            "\n"
    "obj1 is ClassC { u : 1 }"                                  "\n",
    // equivalent file
    "obj1 is sg::reflectionTest::TestClass_C { u : 1 }"         "\n",
    // return value
    true,
    // first error
    ErrorType::unknown
},
#endif
{
    // description
    "class alias",
    // file content
    "alias ClassC is sg::reflectionTest::TestClass_C"           "\n"
    "obj1 is ClassC { u : 1 }"                                  "\n",
    // equivalent file
    "obj1 is sg::reflectionTest::TestClass_C { u : 1 }"         "\n",
    // return value
    true,
    // first error
    ErrorType::unknown
},
{
    // description
    "intrinsic list_length",
    // file content
    "const t = [0, 1, 2, 3, 4]"                                 "\n"
    "obj1 is sg::reflectionTest::TestClass_C {"                 "\n"
    "    u : intrinsic(\"list_length\", t)"                     "\n"
    "}"                                                         "\n",
    // equivalent file
    "obj1 is sg::reflectionTest::TestClass_C { u : 5 }"         "\n",
    // return value
    true,
    // first error
    ErrorType::unknown
},
{
    // description
    "intrinsic is_same_type",
    // file content
    "const t1 = [0, 1, 2, 3, 4]"                                "\n"
    "const t2 = [10, 20, 30]"                                   "\n"
    "const t3 = [5, \"hello\", 2.1, true, false]"               "\n"
    "obj1 is sg::reflectionTest::TestClass_C {"                 "\n"
    "    vectoru : ["                                           "\n"
    "       intrinsic(\"is_same_type\", t1, t2) ? 1 : 0,"       "\n"
    "       intrinsic(\"is_same_type\", t1, t3) ? 1 : 0,"       "\n"
    "       intrinsic(\"is_same_type\", t1[0], t1[1]) ? 1 : 0," "\n"
    "       intrinsic(\"is_same_type\", t3[0], t3[1]) ? 1 : 0," "\n"
    "       intrinsic(\"is_same_type\", t3[0], t3[2]) ? 1 : 0," "\n"
    "       intrinsic(\"is_same_type\", t3[3], t3[4]) ? 1 : 0," "\n"
    "    ]"                                                     "\n"
    "}"                                                         "\n",
    // equivalent file
    "obj1 is sg::reflectionTest::TestClass_C {"                 "\n"
    "    vectoru : [ 1, 1, 1, 0, 0, 1 ]"                        "\n"
    "}"                                                         "\n",
    // return value
    true,
    // first error
    ErrorType::unknown
},
{
    // description
    "intrinsic type_name_of",
    // file content
    "const t1 = [0, 1, 2, 3, 4]"                                "\n"
    "const t2 = [10, 20, 30]"                                   "\n"
    "const t3 = [5, \"hello\", 2.1, true, false]"               "\n"
    "obj1 is sg::reflectionTest::TestClass_C {"                 "\n"
    "    str : intrinsic(\"type_name_of\", t1)"                 "\n"
    "}"                                                         "\n"
    "obj2 is sg::reflectionTest::TestClass_C {"                 "\n"
    "    str : intrinsic(\"type_name_of\", t1[0])"              "\n"
    "}"                                                         "\n"
    "obj3 is sg::reflectionTest::TestClass_C {"                 "\n"
    "    str : intrinsic(\"type_name_of\", t3[2])"              "\n"
    "}"                                                         "\n"
    "obj4 is sg::reflectionTest::TestClass_C {"                 "\n"
    "    str : intrinsic(\"type_name_of\", t3[3])"              "\n"
    "}"                                                         "\n",
    // equivalent file
    "obj1 is sg::reflectionTest::TestClass_C { str: \"List\" }"     "\n"
    "obj2 is sg::reflectionTest::TestClass_C { str: \"Int32\" }"    "\n"
    "obj3 is sg::reflectionTest::TestClass_C { str: \"Float\" }"    "\n"
    "obj4 is sg::reflectionTest::TestClass_C { str: \"Boolean\" }"  "\n",
    // return value
    true,
    // first error
    ErrorType::unknown
},
{
    // description
    "invalid value type for property (1)",
    // file content
    "D is sg::reflectionTest::TestClass_D { vectorStructC : [ A, B, ] }"      "\n"
    "A is ::sg::reflectionTest::TestClass_A { u: 1 }"                         "\n"
    "B is ::sg::reflectionTest::TestClass_A { u: 2 }"                         "\n",
    // equivalent file
    "",
    // return value
    false,
    // first error
    ErrorType::invalid_value_type_for_property
},
{
    // description
    "invalid value type for property (2)",
    // file content
    "D is sg::reflectionTest::TestClass_D { objectlist : [ { u: 1 o: A}, { u: 2 o: B}, ] }"     "\n"
    "A is ::sg::reflectionTest::TestClass_A { u: 1 }"                                           "\n"
    "B is ::sg::reflectionTest::TestClass_A { u: 2 }"                                           "\n",
    // equivalent file
    "",
    // return value
    false,
    // first error
    ErrorType::invalid_value_type_for_property
},
{
    // description
    "object reference in array of structs",
    // file content
    "D is sg::reflectionTest::TestClass_D { vectorStructC : ["      "\n"
    "    { u: 1 o: A},"                                             "\n"
    "    { u: 2 o: B},"                                             "\n"
    "] }"                                                           "\n"
    "A is sg::reflectionTest::TestClass_A { u : 1 }"                "\n"
    "B is sg::reflectionTest::TestClass_A { u : 2 }"                "\n",
    // equivalent file
    "D is ::sg::reflectionTest::TestClass_D { vectorStructC: ["     "\n"
    "    { u: 1 f: 0 o: ::A },"                                     "\n"
    "    { u: 2 f: 0 o: ::B },"                                     "\n"
    "] }"                                                           "\n"
    "A is ::sg::reflectionTest::TestClass_A { u: 1 }"               "\n"
    "B is ::sg::reflectionTest::TestClass_A { u: 2 }"               "\n",
    // return value
    true,
    // first error
    ErrorType::unknown
},
{
    // description
    "object reference in array of structs in variable",
    // file content
    "var T = [ { u: 1 o: A} ]"                                      "\n"
    "T += [ { u: 2 o: B} ]"                                         "\n"
    "D is sg::reflectionTest::TestClass_D { vectorStructC : T }"    "\n"
    "A is sg::reflectionTest::TestClass_A { u : 1 }"                "\n"
    "B is sg::reflectionTest::TestClass_A { u : 2 }"                "\n",
    // equivalent file
    "D is ::sg::reflectionTest::TestClass_D { vectorStructC: ["     "\n"
    "    { u: 1 f: 0 o: ::A },"                                     "\n"
    "    { u: 2 f: 0 o: ::B },"                                     "\n"
    "] }"                                                           "\n"
    "A is ::sg::reflectionTest::TestClass_A { u: 1 }"               "\n"
    "B is ::sg::reflectionTest::TestClass_A { u: 2 }"               "\n",
    // return value
    true,
    // first error
    ErrorType::unknown
},
{
    // description
    "object reference in array in for loop",
    // file content
    "var T = []"                                                    "\n"
    "for(var i = 0; i < 10; ++i)"                                   "\n"
    "{"                                                             "\n"
    "    if(i%2 == 0)      T += [A]"                                "\n"
    "    else if(i%3 == 0) T += [B]"                                "\n"
    "    else              T += [null]"                             "\n"
    "}"                                                             "\n"
    "D is sg::reflectionTest::TestClass_D { objectlist : T }"       "\n"
    "A is sg::reflectionTest::TestClass_A { u : 1 }"                "\n"
    "B is sg::reflectionTest::TestClass_A { u : 2 }"                "\n",
    // equivalent file
    "D is ::sg::reflectionTest::TestClass_D { objectlist: ["         "\n"
    "    A, null, A, B, A, null, A, null, A, B"                      "\n"
    "] }"                                                            "\n"
    "A is ::sg::reflectionTest::TestClass_A { u: 1 }"                "\n"
    "B is ::sg::reflectionTest::TestClass_A { u: 2 }"                "\n",
    // return value
    true,
    // first error
    ErrorType::unknown
},
{
    // description
    "object reference in array of structs in for loop",
    // file content
    "var T = []"                                                    "\n"
    "for(var i = 0; i < 10; ++i)"                                   "\n"
    "{"                                                             "\n"
    "    if(i%2 == 0)      T += [{ u: 1 o: A}]"                     "\n"
    "    else if(i%3 == 0) T += [{ u: 2 o: B}]"                     "\n"
    "    else              T += [{ u: 0 o: null}]"                  "\n"
    "}"                                                             "\n"
    "D is sg::reflectionTest::TestClass_D { vectorStructC : T }"    "\n"
    "A is sg::reflectionTest::TestClass_A { u : 1 }"                "\n"
    "B is sg::reflectionTest::TestClass_A { u : 2 }"                "\n",
    // equivalent file
    "D is ::sg::reflectionTest::TestClass_D { vectorStructC: ["     "\n"
    "    { u: 1 o: A },"                                            "\n"
    "    { u: 0 o: null },"                                         "\n"
    "    { u: 1 o: A },"                                            "\n"
    "    { u: 2 o: B },"                                            "\n"
    "    { u: 1 o: A },"                                            "\n"
    "    { u: 0 o: null },"                                         "\n"
    "    { u: 1 o: A },"                                            "\n"
    "    { u: 0 o: null },"                                         "\n"
    "    { u: 1 o: A },"                                            "\n"
    "    { u: 2 o: B },"                                            "\n"
    "] }"                                                           "\n"
    "A is ::sg::reflectionTest::TestClass_A { u: 1 }"               "\n"
    "B is ::sg::reflectionTest::TestClass_A { u: 2 }"               "\n",
    // return value
    true,
    // first error
    ErrorType::unknown
},

