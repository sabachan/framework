#include "stdafx.h"

#include "DebugImage.h"
#include "ImageIteration.h"
#include <Core/WinUtils.h>
#include <FileFormats/pnm.h>
#include <iomanip>
#include <sstream>

#if SG_ENABLE_ASSERT
namespace sg {
namespace image {
//=============================================================================
namespace {
std::string GetFilename(char const* expr, char const* ext)
{
    static size_t imgCount = 0;
    bool isPrintableExpr = true;
    {
        size_t i = 0;
        while(0 != expr[i])
        {
            char const c = expr[i];
            if(   ('a' <= c && c <= 'z')
                || ('A' <= c && c <= 'Z')
                || ('0' <= c && c <= '9')
                || '_' == c)
            { /* ok */}
            else
                isPrintableExpr = false;
            ++i;
        }
    }
    std::ostringstream oss;
    oss << "tmp:/DBG" << std::setw(4) << std::setfill('0') << imgCount;
    if(isPrintableExpr)
        oss <<"_" << expr;
    oss << ext;
    ++imgCount;
    return oss.str();
}
}
//=============================================================================
void WriteDebugImageIFN(ImageView<u8 const> const& img,
                        char const* expr,
                        char const* file,
                        size_t line,
                        char const* fctName,
                        bool& dumpAlways,
                        bool forceDump)
{
    SG_UNUSED((file, line, fctName));
    bool dumpThisTime = (dumpAlways || forceDump) && !(dumpAlways && forceDump); // modify me, or dumpAlways
    if(dumpThisTime)
    {
        bool ok = pnm::WriteGrayROK(FilePath(GetFilename(expr, ".pnm")), img.Buffer(), img.BufferSizeInBytes(), img.Size().x(), img.Size().y(), img.StrideInBytes());
        if(!ok) { winutils::PrintWinLastError("WriteGrayROK"); }
    }
}
//=============================================================================
void WriteDebugImageIFN(ImageView<ubyte3 const> const& img,
                        char const* expr,
                        char const* file,
                        size_t line,
                        char const* fctName,
                        bool& dumpAlways,
                        bool forceDump)
{
    SG_UNUSED((file, line, fctName));
    bool dumpThisTime = (dumpAlways || forceDump) && !(dumpAlways && forceDump); // modify me, or dumpAlways
    if(dumpThisTime)
    {
        bool ok = pnm::WriteRGBROK(FilePath(GetFilename(expr, ".pnm")), img.Buffer(), img.BufferSizeInBytes(), img.Size().x(), img.Size().y(), img.StrideInBytes());
        if(!ok) { winutils::PrintWinLastError("WriteRGBROK"); }
    }
}
//=============================================================================
void WriteDebugImageIFN(ImageView<ubyte4 const> const& img,
                        char const* expr,
                        char const* file,
                        size_t line,
                        char const* fctName,
                        bool& dumpAlways,
                        bool forceDump)
{
    SG_UNUSED((file, line, fctName));
    bool dumpThisTime = (dumpAlways || forceDump) && !(dumpAlways && forceDump); // modify me, or dumpAlways
    if(dumpThisTime)
    {
        Image<ubyte3> imgrgb(img.WidthHeight());
        Image<u8> imga(img.WidthHeight());
        CoIterate([](ubyte3& orgb, u8& oa, ubyte4 const& irgba) { orgb = irgba.xyz(); oa = irgba.w(); }, imgrgb.View(), imga.View(), img);
        bool ok = pnm::WriteRGBROK(FilePath(GetFilename(expr, "_rgb.pnm")), imgrgb.Buffer(), imgrgb.BufferSizeInBytes(), imgrgb.Size().x(), imgrgb.Size().y(), imgrgb.StrideInBytes());
        if(!ok) { winutils::PrintWinLastError("WriteRGBROK"); }
        ok = pnm::WriteGrayROK(FilePath(GetFilename(expr, "_a.pnm")), imga.Buffer(), imga.BufferSizeInBytes(), imga.Size().x(), imga.Size().y(), imga.StrideInBytes());
        if(!ok) { winutils::PrintWinLastError("WriteGrayROK"); }
    }
}
//=============================================================================
}
}
#endif
