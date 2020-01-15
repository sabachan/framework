#ifndef ObjectScript_Reader_Include_TestImpl
#error "this file must be used only by Reader.cpp"
#endif

{
    // description
    "template",
    // file content
    "namespace myNamespace {"                                   "\n"
    "   template tplObj("                                       "\n"
    "       x,"                                                 "\n"
    "       y = 1"                                              "\n"
    "   ) is sg::reflectionTest::TestClass_C {"                 "\n"
    "       u : x"                                              "\n"
    "       i : y"                                              "\n"
    "       f : 1.*x/y"                                         "\n"
    "   }"                                                      "\n"
    "}"                                                         "\n"
    "object1 is myNamespace::tplObj {"                          "\n"
    "    x : 3"                                                 "\n"
    "    y : 2"                                                 "\n"
    "}"                                                         "\n"
    "object2 is myNamespace::tplObj {"                          "\n"
    "    x : 4"                                                 "\n"
    "}"                                                         "\n",
    //"object3 is myNamespace::tplObj ( 5, 0.5 )"                 "\n",
    // equivalent file
    "object1 is sg::reflectionTest::TestClass_C {"              "\n"
    "   u : 3 i : 2 f : 3./2"                                   "\n"
    "}"                                                         "\n"
    "object2 is sg::reflectionTest::TestClass_C {"              "\n"
    "   u : 4 i : 1 f : 4"                                      "\n"
    "}"                                                         "\n",
    //"object3 is sg::reflectionTest::TestClass_C {"              "\n"
    //"   u : 5 i : 0.5 f : 10"                                   "\n"
    //"}"                                                         "\n",
    // return value
    true,
    // first error
    ErrorType::unknown
},
#if 0
{
    // description
    "function-style template",
    // file content
    "template tplObj("                                          "\n"
    "    x = 1,"                                                "\n"
    ") is sg::reflectionTest::TestClass_C {"                    "\n"
    "    u : x"                                                 "\n"
    "}"                                                         "\n"
    "obj is tplObj ( 5 )"                                       "\n",
    // equivalent file
    "obj is sg::reflectionTest::TestClass_C { u: 5 }"           "\n",
    // return value
    false,
    // first error
    ErrorType::unknown
},
#endif
{
    // description
    "template parameter collides with property name",
    // file content
    "template tplObj("                                          "\n"
    "    u = 1,"                                                "\n"
    ") is sg::reflectionTest::TestClass_C {"                    "\n"
    "    u : u"                                                 "\n"
    "}"                                                         "\n"
    "obj1 is tplObj {}"                                         "\n"
    "obj2 is tplObj { u : 5 }"                                  "\n",
    // equivalent file
    "obj1 is sg::reflectionTest::TestClass_C { u: 1 }"          "\n"
    "obj2 is sg::reflectionTest::TestClass_C { u: 5 }"          "\n",
    // return value
    true,
    // first error
    ErrorType::unknown
},
{
    // description
    "template parameter collides with property name (2)",
    // file content
    "template tplObj("                                          "\n"
    "    i = 0,"                                                "\n"
    "    u = 1,"                                                "\n"
    ") is sg::reflectionTest::TestClass_C {"                    "\n"
    "    u : i"                                                 "\n"
    "    i : u"                                                 "\n"
    "    f : u + i"                                             "\n"
    "}"                                                         "\n"
    "obj1 is tplObj {}"                                         "\n"
    "obj2 is tplObj { i : 5 }"                                  "\n"
    "obj3 is tplObj { u : 7 i : 2 }"                            "\n",
    // equivalent file
    "obj1 is sg::reflectionTest::TestClass_C { u: 0 i: 1 f: 1 }"    "\n"
    "obj2 is sg::reflectionTest::TestClass_C { u: 5 i: 1 f: 6 }"    "\n"
    "obj3 is sg::reflectionTest::TestClass_C { u: 2 i: 7 f: 9 }"    "\n",
    // return value
    true,
    // first error
    ErrorType::unknown
},
{
    // description
    "template with sub objects",
    // file content
    "template MyTemplate(x) is sg::reflectionTest::TestClass_D"     "\n"
    "{"                                                             "\n"
    "    object: sg::reflectionTest::TestClass_C { i: x }"          "\n"
    "}"                                                             "\n"
    "obj1 is MyTemplate { x : 4 }"                                  "\n"
    "obj2 is MyTemplate { x : 10 }"                                 "\n",
    // equivalent file
    "obj1 is sg::reflectionTest::TestClass_D {"                     "\n"
    "    object: sg::reflectionTest::TestClass_C { i: 4 }"          "\n"
    "}"                                                             "\n"
    "obj2 is sg::reflectionTest::TestClass_D {"                     "\n"
    "    object: sg::reflectionTest::TestClass_C { i: 10 }"         "\n"
    "}"                                                             "\n",
    // return value
    true,
    // first error
    ErrorType::unknown
},
{
    // description
    "constant template",
    // file content
    "template MyTemplate() is sg::reflectionTest::TestClass_C{ i : 4}"  "\n"
    "obj1 is MyTemplate {}"                                             "\n",
    // equivalent file
    "obj1 is sg::reflectionTest::TestClass_C { i: 4 }"              "\n",
    // return value
    true,
    // first error
    ErrorType::unknown
},
{
    // description
    "template with constant subobject",
    // file content
    "template MyTemplate() is sg::reflectionTest::TestClass_D"     "\n"
    "{"                                                             "\n"
    "    object: sg::reflectionTest::TestClass_C { i: 4 }"          "\n"
    "}"                                                             "\n"
    "obj1 is MyTemplate {}"                                         "\n",
    // equivalent file
    "obj1 is sg::reflectionTest::TestClass_D {"                     "\n"
    "    object: sg::reflectionTest::TestClass_C { i: 4 }"          "\n"
    "}"                                                             "\n",
    // return value
    true,
    // first error
    ErrorType::unknown
},
{
    // description
    "template with named sub objects",
    // file content
    "template MyTemplate(x) is sg::reflectionTest::TestClass_D"         "\n"
    "{"                                                                 "\n"
    "    object: subobj is sg::reflectionTest::TestClass_C { i: x }"    "\n"
    "}"                                                                 "\n"
    "obj1 is MyTemplate { x : 4 }"                                      "\n"
    "obj2 is MyTemplate { x : 10 }"                                     "\n"
    "obj3 is sg::reflectionTest::TestClass_D { object : obj1::subobj }" "\n",
    // equivalent file
    "obj1 is sg::reflectionTest::TestClass_D {"                         "\n"
    "    object: subobj is sg::reflectionTest::TestClass_C { i: 4 }"    "\n"
    "}"                                                                 "\n"
    "obj2 is sg::reflectionTest::TestClass_D {"                         "\n"
    "    object: subobj is sg::reflectionTest::TestClass_C { i: 10 }"   "\n"
    "}"                                                                 "\n"
    "obj3 is sg::reflectionTest::TestClass_D { object : obj1::subobj }" "\n",
    // return value
    true,
    // first error
    ErrorType::unknown
},
{
    // description
    "template with inter sub-object references",
    // file content
    "template MyTemplate(x) is sg::reflectionTest::TestClass_D"         "\n"
    "{"                                                                 "\n"
    "    object: subobj is sg::reflectionTest::TestClass_C { i: x }"    "\n"
    "    objectlist: ["                                                 "\n"
    "       subobj,"                                                    "\n"
    "       sg::reflectionTest::TestClass_C { i: x+1 }"                 "\n"
    "    ]"                                                             "\n"
    "}"                                                                 "\n"
    "obj1 is MyTemplate { x : 4 }"                                      "\n",
    // equivalent file
    "obj1 is sg::reflectionTest::TestClass_D {"                         "\n"
    "    object: subobj is sg::reflectionTest::TestClass_C { i: 4 }"    "\n"
    "    objectlist: ["                                                 "\n"
    "       subobj,"                                                    "\n"
    "       sg::reflectionTest::TestClass_C { i: 5 }"                   "\n"
    "    ]"                                                             "\n"
    "}"                                                                 "\n",
    // return value
    true,
    // first error
    ErrorType::unknown
},
{
    // description
    "template with nested objects",
    // file content
    "template MyTemplate(x) is sg::reflectionTest::TestClass_D"         "\n"
    "{"                                                                 "\n"
    "    object: subobj1"                                               "\n"
    "    protected subobj1 is sg::reflectionTest::TestClass_C { i: x }" "\n"
    "    objectlist: ["                                                 "\n"
    "       subobj1,"                                                   "\n"
    "       subobj2"                                                    "\n"
    "    ]"                                                             "\n"
    "    private subobj2 is sg::reflectionTest::TestClass_C { i: x+1 }" "\n"
    "}"                                                                 "\n"
    "obj1 is MyTemplate { x : 4 }"                                      "\n"
    "obj2 is sg::reflectionTest::TestClass_D { object : obj1::subobj1 }""\n",
    // equivalent file
    "obj1 is sg::reflectionTest::TestClass_D {"                         "\n"
    "    protected subobj1 is sg::reflectionTest::TestClass_C { i: 4 }" "\n"
    "    object: subobj1"                                               "\n"
    "    objectlist: ["                                                 "\n"
    "       subobj1,"                                                   "\n"
    "       private subobj2 is sg::reflectionTest::TestClass_C { i: 5 }""\n"
    "    ]"                                                             "\n"
    "}"                                                                 "\n"
    "obj2 is sg::reflectionTest::TestClass_D { object : obj1::subobj1 }""\n",
    // return value
    true,
    // first error
    ErrorType::unknown
},
{
    // description
    "function call on template",
    // file content
    "namespace myNamespace {"                                   "\n"
    "   template tplObj("                                       "\n"
    "       x,"                                                 "\n"
    "       y = 1"                                              "\n"
    "   ) is sg::reflectionTest::TestClass_C {"                 "\n"
    "       u : x"                                              "\n"
    "       i : y"                                              "\n"
    "       f : 1.*x/y"                                         "\n"
    "   }"                                                      "\n"
    "}"                                                         "\n"
    "object1 is myNamespace::tplObj(3, 2)"                      "\n",
    // equivalent file
    "",
    // return value
    false,
    // first error
    ErrorType::expression_is_not_callable
},
{
    // description
    "break in template",
    // file content
    "template tplObj("                                          "\n"
    "    x"                                                     "\n"
    ") is sg::reflectionTest::TestClass_C {"                    "\n"
    "    u : x"                                                 "\n"
    "    break"                                                 "\n"
    "}"                                                         "\n"
    "object1 is tplObj(1)"                                      "\n",
    // equivalent file
    "",
    // return value
    false,
    // first error
    ErrorType::unexpected_use_of_jump_statement
},
{
    // description
    "if/else in template",
    // file content
    "template MyTemplate(x) is sg::reflectionTest::TestClass_D"         "\n"
    "{"                                                                 "\n"
    "    object: subobj is sg::reflectionTest::TestClass_C { i: x }"    "\n"
    "    var list = []"                                                 "\n"
    "    if(x < 100)"                                                   "\n"
    "    {"                                                             "\n"
    "        const y = x+1"                                             "\n"
    "        list += ["                                                 "\n"
    "           subobj,"                                                "\n"
    "           sg::reflectionTest::TestClass_C { i: y }"               "\n"
    "        ]"                                                         "\n"
    "    }"                                                             "\n"
    "    else"                                                          "\n"
    "    {"                                                             "\n"
    "        const y = x-100"                                           "\n"
    "        list += ["                                                 "\n"
    "           sg::reflectionTest::TestClass_C { i: y }"               "\n"
    "        ]"                                                         "\n"
    "    }"                                                             "\n"
    "    objectlist: list"                                              "\n"
    "}"                                                                 "\n"
    "obj1 is MyTemplate { x : 4 }"                                      "\n"
    "obj2 is MyTemplate { x : 104 }"                                    "\n",
    // equivalent file
    "obj1 is sg::reflectionTest::TestClass_D {"                         "\n"
    "    object: subobj is sg::reflectionTest::TestClass_C { i: 4 }"    "\n"
    "    objectlist: ["                                                 "\n"
    "       subobj,"                                                    "\n"
    "       sg::reflectionTest::TestClass_C { i: 5 }"                   "\n"
    "    ]"                                                             "\n"
    "}"                                                                 "\n"
    "obj2 is sg::reflectionTest::TestClass_D {"                         "\n"
    "    object: subobj is sg::reflectionTest::TestClass_C { i: 104 }"  "\n"
    "    objectlist: ["                                                 "\n"
    "       sg::reflectionTest::TestClass_C { i: 4 }"                   "\n"
    "    ]"                                                             "\n"
    "}"                                                                 "\n",
    // return value
    true,
    // first error
    ErrorType::unknown
},
{
    // description
    "template namespace",
    // file content
    "template tpl("                                             "\n"
    "    x, y"                                                  "\n"
    ") is namespace {"                                          "\n"
    "    A is sg::reflectionTest::TestClass_C {"                "\n"
    "        u : x"                                             "\n"
    "    }"                                                     "\n"
    "    B is sg::reflectionTest::TestClass_C {"                "\n"
    "        u : y"                                             "\n"
    "    }"                                                     "\n"
    "}"                                                         "\n"
    "ns1 is tpl { x:1 y:2 }"                                    "\n"
    "ns2 is tpl { x:3 y:4 }"                                    "\n",
    // equivalent file
    "namespace ns1 {"                                                       "\n"
    "    protected A is ::sg::reflectionTest::TestClass_C { u: 1 }"         "\n"
    "    protected B is ::sg::reflectionTest::TestClass_C { u: 2 }"         "\n"
    "}"                                                                     "\n"
    "namespace ns2 {"                                                       "\n"
    "    protected A is ::sg::reflectionTest::TestClass_C { u: 3 }"         "\n"
    "    protected B is ::sg::reflectionTest::TestClass_C { u: 4 }"         "\n"
    "}"                                                                     "\n",
    // return value
    true,
    // first error
    ErrorType::unknown
},
{
    // description
    "template of template",
    // file content
    "namespace myNamespace {"                                   "\n"
    "   template tplObj1("                                       "\n"
    "       x,"                                                 "\n"
    "       y = 1"                                              "\n"
    "   ) is sg::reflectionTest::TestClass_C {"                 "\n"
    "       u : x"                                              "\n"
    "       i : y"                                              "\n"
    "       f : 1.*x/y"                                         "\n"
    "   }"                                                      "\n"
    "   template tplObj2("                                      "\n"
    "       a"                                                  "\n"
    "   ) is tplObj1 {"                                         "\n"
    "       x : 2*a"                                            "\n"
    "       y : a"                                              "\n"
    "   }"                                                      "\n"
    "   template tplObj3("                                      "\n"
    "       b"                                                  "\n"
    "   ) is tplObj2 {"                                         "\n"
    "       a : 3 * b"                                          "\n"
    "   }"                                                      "\n"
    "}"                                                         "\n"
    "object1 is myNamespace::tplObj1 {"                         "\n"
    "    x : 3"                                                 "\n"
    "    y : 2"                                                 "\n"
    "}"                                                         "\n"
    "object2 is myNamespace::tplObj2 {"                         "\n"
    "    a : 4"                                                 "\n"
    "}"                                                         "\n"
    "object3 is myNamespace::tplObj3 {"                         "\n"
    "    b : 2"                                                 "\n"
    "}"                                                         "\n",
    // equivalent file
    "object1 is sg::reflectionTest::TestClass_C {"              "\n"
    "   u : 3 i : 2 f : 3./2"                                   "\n"
    "}"                                                         "\n"
    "object2 is sg::reflectionTest::TestClass_C {"              "\n"
    "   u : 8 i : 4 f : 2"                                      "\n"
    "}"                                                         "\n"
    "object3 is sg::reflectionTest::TestClass_C {"              "\n"
    "   u : 12 i : 6 f : 2"                                     "\n"
    "}"                                                         "\n",
    // return value
    true,
    // first error
    ErrorType::unknown
},
