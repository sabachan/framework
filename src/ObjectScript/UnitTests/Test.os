import "Test_Import1.os"

function f()
{
    var a = 1
    var b = 2
    assert 2 * a == b
}

f()

public obj0 is ::sg::reflectionTest::TestClass_C
{
    vectoru : [1,2,3]
}
export obj1 is ::myNamespace::ListSquares { n:10 }
export obj2 is ::myNamespace::ListSquares { n:30 }