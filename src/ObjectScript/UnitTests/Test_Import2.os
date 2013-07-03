import "Test_Import3.os"

namespace myNamespace {

function list_squares(n)
{
    var tab = []
    for(var i = 1; i <= n; ++i)
        tab += [sq(i)]
    return tab
}

}