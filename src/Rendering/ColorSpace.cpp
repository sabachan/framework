#include "stdafx.h"

#include "ColorSpace.h"

#if SG_ENABLE_UNIT_TESTS

#include <Core/ArrayView.h>
#include <Core/TestFramework.h>

namespace sg {
//=============================================================================
namespace testcolorspace {
namespace {
template <ColorSpace CS>
void TestConversion(float3 const& color)
{
    ColorSpacePoint<ColorSpace::sRGB> sRGB(color);
    ColorSpacePoint<CS> ccs = ConvertTo<CS>(sRGB);
    ColorSpacePoint<ColorSpace::sRGB> back = ConvertTo<ColorSpace::sRGB>(ccs);
    SG_ASSERT(EqualsWithTolerance(float3(sRGB), float3(back), 0.001f));
}
void TestConversions(float3 const& color)
{
    TestConversion<ColorSpace::sRGB>(color);
    TestConversion<ColorSpace::LinearRGB>(color);
    TestConversion<ColorSpace::CIEXYZ>(color);
    TestConversion<ColorSpace::CIExyY>(color);
    TestConversion<ColorSpace::HueChromaIntensity>(color);
    TestConversion<ColorSpace::HueChromaLightness>(color);
    TestConversion<ColorSpace::HueChromaValue>(color);
    //TestConversion<ColorSpace::HueChromaLuma>(color);
    TestConversion<ColorSpace::HueSaturationIntensity>(color);
    TestConversion<ColorSpace::HueSaturationLightness>(color);
    TestConversion<ColorSpace::HueSaturationValue>(color);
}
}
}
//=============================================================================
SG_TEST((sg), ColorSpace, (Rendering, quick))
{
    {
        float3x3 const A = float3x3(CIEXYZToLinearRGBMatrix);
        float3x3 const B = float3x3(LinearRGBToCIEXYZMatrix);
        float3x3 const C = A*B;
        float3x3 const D = Invert_AssumeInvertible(A);
        float3x3 const E = A*D;
        SG_ASSERT(EqualsWithTolerance(float3x3::Identity(), C, 0.00001f));
        SG_ASSERT(EqualsWithTolerance(float3x3::Identity(), E, 0.00001f));
        SG_DEBUG_LINE;
    }

    float3 colors[] = {
        float3(0.40f, 0.50f, 0.70f),
        float3(0.00f, 0.00f, 0.00f),
        float3(0.50f, 0.50f, 0.50f),
        float3(1.00f, 1.00f, 1.00f),
        float3(1.00f, 0.00f, 0.00f),
        float3(1.00f, 1.00f, 0.00f),
        float3(0.50f, 1.00f, 1.00f),
        float3(0.80f, 0.20f, 0.10f),
    };
    for(auto const& c : AsArrayView(colors))
    {
        testcolorspace::TestConversions(c);
    }
}
//=============================================================================
}
#endif