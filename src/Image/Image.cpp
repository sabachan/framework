#include "stdafx.h"

#include "Image.h"

#include "Brush.h"
#include "DebugImage.h"
#include "Draw.h"
#include <Core/Assert.h>
#include <Core/Config.h>
#include <Core/FilePath.h>
#include <Core/FileSystem.h>
#include <Core/TestFramework.h>
#include <Core/WinUtils.h>
#include <FileFormats/pnm.h>
#include <vector>

#if SG_ENABLE_UNIT_TESTS
namespace sg {
namespace image {
//=============================================================================
// force instanciation to reveal compile errors
template ImageView<u8>;
template ImageView<u8 const>;
template ImageView<ubyte3>;
template Image<u8>;
template Image<ubyte3>;
//=============================================================================
SG_TEST((sg, image), Image, (Image, quick))
{
    filesystem::Init();
    filesystem::MountDeclaredMountingPoints();

    {
        ArrayView<image::BitMapFont const> fonts = image::GetAlwaysAvailableBitmapFonts();

        image::Image<ubyte3> img1(uint2(400, 260), ubyte3(160, 160, 160));
        for_range(size_t,y,0,img1.Size().y())
            for_range(size_t,x,0,img1.Size().x())
            { if((x/2 + y/2) % 2) {img1(x,y) = ubyte3(128, 128, 128);} }

        image::Image<ubyte3> img2 = img1.Clone();
        image::ImageView<ubyte3> img3 = img2;
        image::ImageView<ubyte3> img4(img3, box2u::FromMinMax(uint2(50, 80), uint2(310, 140)));
        img3 = img1;
        for_range(size_t,y,110,130) for_range(size_t,x,120,280) {img2(x,y) = ubyte3(128, 64, 32);}
        for_range(size_t,y,80,160) for_range(size_t,x,80,160) {img3(x,y) = ubyte3(32, 64, 96);}

        auto const brush = image::brush::Blend<image::blend::Classic>().Fill(ubyte3(128, 128, 128));
        image::DrawRect(img2, box2f::FromMinMax(float2(60,60), float2(120, 180)), brush);
        image::DrawRect(img1, box2f::FromMinMax(float2(60,60), float2(120, 180)), image::brush::Fill(ubyte4(128, 128, 0, 64)).Stroke(ubyte3(0,0,0)).Blend<image::blend::Classic>());
        image::DrawRect(img2, box2f::FromMinMax(float2(100,50), float2(150, 100)), image::brush::Stroke(ubyte3(0,0,0)));

        auto const aabrush = image::brush::Blend<image::blend::Classic>().Antialiased();
        image::DrawRect(img2, box2f::FromMinMax(float2(300.5f, 120.f), float2(350.75f, 140.25f)), aabrush.Fill(ubyte4(255, 255, 255, 255)).Stroke(ubyte3(0,0,0), 3));
        image::DrawRect(img2, box2f::FromMinMax(float2(300.5f, 150.f), float2(350.75f, 170.25f)), aabrush.Fill(ubyte4(255, 255, 255, 255)).Stroke(ubyte3(0,0,0), 2));
        image::DrawRect(img2, box2f::FromMinMax(float2(300.5f, 180.f), float2(350.75f, 200.25f)), aabrush.Fill(ubyte4(255, 255, 255, 255)).Stroke(ubyte3(0,0,0), 1));
        image::DrawRect(img2, box2f::FromMinMax(float2(300.5f, 210.f), float2(350.75f, 230.25f)), aabrush.Fill(ubyte4(255, 255, 255, 255)).Stroke(ubyte3(0,0,0), 0.5f));

        image::DrawRect(img2, box2i::FromMinMax(int2(200, 120), int2(250, 140)), brush.Fill(ubyte4(255, 255, 255, 255)).Stroke(ubyte3(0,0,0), 0.5f));
        image::DrawRect(img2, box2i::FromMinMax(int2(200, 150), int2(250, 170)), brush.Fill(ubyte4(255, 255, 255, 255)).Stroke(ubyte3(0,0,0), 1));
        image::DrawRect(img2, box2i::FromMinMax(int2(200, 180), int2(250, 200)), image::brush::Fill(ubyte3(255, 255, 255)).Stroke(ubyte3(0,0,0), 1));
        image::DrawRect(img2, box2i::FromMinMax(int2(200, 210), int2(250, 230)), image::brush::Fill(ubyte3(255, 255, 255)));

        image::DrawRect(img2, box2i::FromMinMax(int2(20, 195), int2(180, 255)), image::brush::Fill(ubyte3(220, 220, 220)));
        image::DrawText(img2, int2(30, 205), "Hello World!", image::brush::Fill(ubyte3(0)));
        image::DrawText(img2, int2(30, 220), "Hello World!", image::brush::Fill(ubyte3(0)).Monospace());
        image::DrawText(img2, int2(30, 235), "Hello World!", image::brush::Fill(ubyte3(0)), fonts[1 % fonts.size()]);
        image::DrawText(img2, int2(30, 250), "Hello World!", image::brush::Fill(ubyte3(0)), fonts[2 % fonts.size()]);

        image::DrawRect(img2, box2i::FromMinMax(int2(160, 20), int2(360, 100)), image::brush::Fill(ubyte3(255, 255, 255)));
        //image::DrawRect(img2, box2i::FromMinMax(int2(160, 30), int2(360, 45)), image::brush::Fill(ubyte3(220, 220, 220)));
        image::DrawText(img2, int2(165, 30), "abcdefghijklmnopqrstuvwxyz", image::brush::Fill(ubyte3(0)));
        image::DrawText(img2, int2(165, 45), "ABCDEFGHIJKLMNOPQRSTUVWXYZ", image::brush::Fill(ubyte3(0)));
        image::DrawText(img2, int2(165, 60), "0123456789.,?!:;*+-=/\\()[]", image::brush::Fill(ubyte3(0)));
        image::DrawText(img2, int2(165, 75), "{}%#&|_\"'°^ιθωΰηµ@", image::brush::Fill(ubyte3(0)));
        image::DrawText(img2, int2(165, 90), "3*2+1=7: 15ms", image::brush::Fill(ubyte3(0)));

        for_range(int, i, -5, 10)
            image::DrawLine1pxNoAA(img4, int2(-10,50) + i * int2(4,8), int2(85,-4) + i * int2(5,3), image::brush::Stroke(ubyte3(255,120,190)));

        image::Image<ubyte3> img5(uint2(3000, 400), ubyte3(0));
        //std::string str = "The quick brown fox jumps over a lazy dog.";
        //std::string str = "Portez ce vieux wisky au juge blond qui fume.";
        //std::string str = "Portez ce vieux whisky au juge blond qui fume la pipe.";
        std::string str = "Portez ce vieux whisky au juge blond qui fume la pipe."
            " ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789.,?!:;*+-=/\\()[]{}<>%#&|_\"'°^ιθωΰηµ@";

        for_range(size_t, i, 0, fonts.size())
        {
            image::DrawText(img5, int2(5, 30 + 20*int(i)), str, image::brush::Fill(ubyte3(255,255,255)), fonts[i]);
        }

        bool ok = false;
        ok = pnm::WriteRGBROK(FilePath("tmp:/TestImage1.pnm"), img1.Buffer(), img1.BufferSizeInBytes(), img1.Size().x(), img1.Size().y(), img1.StrideInBytes());
        if(!ok) { winutils::PrintWinLastError("WriteRGBROK"); }
        ok = pnm::WriteRGBROK(FilePath("tmp:/TestImage2.pnm"), img2.Buffer(), img2.BufferSizeInBytes(), img2.Size().x(), img2.Size().y(), img2.StrideInBytes());
        if(!ok) { winutils::PrintWinLastError("WriteRGBROK"); }
        ok = pnm::WriteRGBROK(FilePath("tmp:/TestImage3.pnm"), img3.Buffer(), img3.BufferSizeInBytes(), img3.Size().x(), img3.Size().y(), img3.StrideInBytes());
        if(!ok) { winutils::PrintWinLastError("WriteRGBROK"); }
        ok = pnm::WriteRGBROK(FilePath("tmp:/TestImage4.pnm"), img4.Buffer(), img4.BufferSizeInBytes(), img4.Size().x(), img4.Size().y(), img4.StrideInBytes());
        if(!ok) { winutils::PrintWinLastError("WriteRGBROK"); }
        ok = pnm::WriteRGBROK(FilePath("tmp:/TestImage5.pnm"), img5.Buffer(), img5.BufferSizeInBytes(), img5.Size().x(), img5.Size().y(), img5.StrideInBytes());
        if(!ok) { winutils::PrintWinLastError("WriteRGBROK"); }

        SG_DEBUG_IMAGE_STEP_INTO(img2);
    }

    filesystem::Shutdown();
}
//=============================================================================
}
}
#endif
