
import "Test_Import2.os"

//const PI_2 = PI / 2

namespace myNamespace {

alias TestClassC is ::sg::reflectionTest::TestClass_C

template ListSquares(n) is TestClassC
{
    const l = list_squares(n)
    vectoru : l
}

}
