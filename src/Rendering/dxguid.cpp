// cf. http://xboxforums.create.msdn.com/forums/t/32885.aspx
// Used to resolve "error LNK2001: unresolved external symbol _IID_ID3D11ShaderReflection"
#define INITGUID
#include <d3d11shader.h>
