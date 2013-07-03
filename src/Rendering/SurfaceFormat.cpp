#include "stdafx.h"

#include "SurfaceFormat.h"

namespace sg {
namespace rendering {
//=============================================================================
#define APPEND_DXGI(FORMAT) \
    , DXGI_FORMAT_##FORMAT
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
namespace {
SurfaceFormatProperties const surfaceFormatProperties[] =
{
    //                                                                                      byteCountPerPixel
    //                                                                                       |  channelCount
    //                                                                                       |   |  bitCountPerChannel
    //                                                                                       |   |   | isUniformLayout
    //       surfaceFormat                             typelessFormat                        |   |   |    |   isSRGB   isBC             dxgiFormat
    //             |                                          |                              |   |   |    |      |      |                 |
    { SurfaceFormat::UNKNOWN                   , SurfaceFormat::UNKNOWN                   ,  0,  0,  0, false, false, false   APPEND_DXGI(UNKNOWN                   ) },
    { SurfaceFormat::R32G32B32A32_TYPELESS     , SurfaceFormat::R32G32B32A32_TYPELESS     , 16,  4, 32,  true, false, false   APPEND_DXGI(R32G32B32A32_TYPELESS     ) },
    { SurfaceFormat::R32G32B32A32_FLOAT        , SurfaceFormat::R32G32B32A32_TYPELESS     , 16,  4, 32,  true, false, false   APPEND_DXGI(R32G32B32A32_FLOAT        ) },
    { SurfaceFormat::R32G32B32A32_UINT         , SurfaceFormat::R32G32B32A32_TYPELESS     , 16,  4, 32,  true, false, false   APPEND_DXGI(R32G32B32A32_UINT         ) },
    { SurfaceFormat::R32G32B32A32_SINT         , SurfaceFormat::R32G32B32A32_TYPELESS     , 16,  4, 32,  true, false, false   APPEND_DXGI(R32G32B32A32_SINT         ) },
    { SurfaceFormat::R32G32B32_TYPELESS        , SurfaceFormat::R32G32B32_TYPELESS        , 12,  3, 32,  true, false, false   APPEND_DXGI(R32G32B32_TYPELESS        ) },
    { SurfaceFormat::R32G32B32_FLOAT           , SurfaceFormat::R32G32B32_TYPELESS        , 12,  3, 32,  true, false, false   APPEND_DXGI(R32G32B32_FLOAT           ) },
    { SurfaceFormat::R32G32B32_UINT            , SurfaceFormat::R32G32B32_TYPELESS        , 12,  3, 32,  true, false, false   APPEND_DXGI(R32G32B32_UINT            ) },
    { SurfaceFormat::R32G32B32_SINT            , SurfaceFormat::R32G32B32_TYPELESS        , 12,  3, 32,  true, false, false   APPEND_DXGI(R32G32B32_SINT            ) },
    { SurfaceFormat::R16G16B16A16_TYPELESS     , SurfaceFormat::R16G16B16A16_TYPELESS     ,  8,  4, 16,  true, false, false   APPEND_DXGI(R16G16B16A16_TYPELESS     ) },
    { SurfaceFormat::R16G16B16A16_FLOAT        , SurfaceFormat::R16G16B16A16_TYPELESS     ,  8,  4, 16,  true, false, false   APPEND_DXGI(R16G16B16A16_FLOAT        ) },
    { SurfaceFormat::R16G16B16A16_UNORM        , SurfaceFormat::R16G16B16A16_TYPELESS     ,  8,  4, 16,  true, false, false   APPEND_DXGI(R16G16B16A16_UNORM        ) },
    { SurfaceFormat::R16G16B16A16_UINT         , SurfaceFormat::R16G16B16A16_TYPELESS     ,  8,  4, 16,  true, false, false   APPEND_DXGI(R16G16B16A16_UINT         ) },
    { SurfaceFormat::R16G16B16A16_SNORM        , SurfaceFormat::R16G16B16A16_TYPELESS     ,  8,  4, 16,  true, false, false   APPEND_DXGI(R16G16B16A16_SNORM        ) },
    { SurfaceFormat::R16G16B16A16_SINT         , SurfaceFormat::R16G16B16A16_TYPELESS     ,  8,  4, 16,  true, false, false   APPEND_DXGI(R16G16B16A16_SINT         ) },
    { SurfaceFormat::R32G32_TYPELESS           , SurfaceFormat::R32G32_TYPELESS           ,  8,  2, 32,  true, false, false   APPEND_DXGI(R32G32_TYPELESS           ) },
    { SurfaceFormat::R32G32_FLOAT              , SurfaceFormat::R32G32_TYPELESS           ,  8,  2, 32,  true, false, false   APPEND_DXGI(R32G32_FLOAT              ) },
    { SurfaceFormat::R32G32_UINT               , SurfaceFormat::R32G32_TYPELESS           ,  8,  2, 32,  true, false, false   APPEND_DXGI(R32G32_UINT               ) },
    { SurfaceFormat::R32G32_SINT               , SurfaceFormat::R32G32_TYPELESS           ,  8,  2, 32,  true, false, false   APPEND_DXGI(R32G32_SINT               ) },
    { SurfaceFormat::R32G8X24_TYPELESS         , SurfaceFormat::R32G8X24_TYPELESS         ,  8,  3,  0, false, false, false   APPEND_DXGI(R32G8X24_TYPELESS         ) },
    { SurfaceFormat::D32_FLOAT_S8X24_UINT      , SurfaceFormat::D32_FLOAT_S8X24_UINT      ,  8,  3,  0, false, false, false   APPEND_DXGI(D32_FLOAT_S8X24_UINT      ) },
    { SurfaceFormat::R32_FLOAT_X8X24_TYPELESS  , SurfaceFormat::R32_FLOAT_X8X24_TYPELESS  ,  8,  3,  0, false, false, false   APPEND_DXGI(R32_FLOAT_X8X24_TYPELESS  ) },
    { SurfaceFormat::X32_TYPELESS_G8X24_UINT   , SurfaceFormat::X32_TYPELESS_G8X24_UINT   ,  8,  3,  0, false, false, false   APPEND_DXGI(X32_TYPELESS_G8X24_UINT   ) },
    { SurfaceFormat::R10G10B10A2_TYPELESS      , SurfaceFormat::R10G10B10A2_TYPELESS      ,  4,  4,  0, false, false, false   APPEND_DXGI(R10G10B10A2_TYPELESS      ) },
    { SurfaceFormat::R10G10B10A2_UNORM         , SurfaceFormat::R10G10B10A2_TYPELESS      ,  4,  4,  0, false, false, false   APPEND_DXGI(R10G10B10A2_UNORM         ) },
    { SurfaceFormat::R10G10B10A2_UINT          , SurfaceFormat::R10G10B10A2_TYPELESS      ,  4,  4,  0, false, false, false   APPEND_DXGI(R10G10B10A2_UINT          ) },
    { SurfaceFormat::R11G11B10_FLOAT           , SurfaceFormat::R11G11B10_FLOAT           ,  4,  3,  0, false, false, false   APPEND_DXGI(R11G11B10_FLOAT           ) },
    { SurfaceFormat::R8G8B8A8_TYPELESS         , SurfaceFormat::R8G8B8A8_TYPELESS         ,  4,  4,  8,  true, false, false   APPEND_DXGI(R8G8B8A8_TYPELESS         ) },
    { SurfaceFormat::R8G8B8A8_UNORM            , SurfaceFormat::R8G8B8A8_TYPELESS         ,  4,  4,  8,  true, false, false   APPEND_DXGI(R8G8B8A8_UNORM            ) },
    { SurfaceFormat::R8G8B8A8_UNORM_SRGB       , SurfaceFormat::R8G8B8A8_TYPELESS         ,  4,  4,  8,  true,  true, false   APPEND_DXGI(R8G8B8A8_UNORM_SRGB       ) },
    { SurfaceFormat::R8G8B8A8_UINT             , SurfaceFormat::R8G8B8A8_TYPELESS         ,  4,  4,  8,  true, false, false   APPEND_DXGI(R8G8B8A8_UINT             ) },
    { SurfaceFormat::R8G8B8A8_SNORM            , SurfaceFormat::R8G8B8A8_TYPELESS         ,  4,  4,  8,  true, false, false   APPEND_DXGI(R8G8B8A8_SNORM            ) },
    { SurfaceFormat::R8G8B8A8_SINT             , SurfaceFormat::R8G8B8A8_TYPELESS         ,  4,  4,  8,  true, false, false   APPEND_DXGI(R8G8B8A8_SINT             ) },
    { SurfaceFormat::R16G16_TYPELESS           , SurfaceFormat::R16G16_TYPELESS           ,  4,  2, 16,  true, false, false   APPEND_DXGI(R16G16_TYPELESS           ) },
    { SurfaceFormat::R16G16_FLOAT              , SurfaceFormat::R16G16_TYPELESS           ,  4,  2, 16,  true, false, false   APPEND_DXGI(R16G16_FLOAT              ) },
    { SurfaceFormat::R16G16_UNORM              , SurfaceFormat::R16G16_TYPELESS           ,  4,  2, 16,  true, false, false   APPEND_DXGI(R16G16_UNORM              ) },
    { SurfaceFormat::R16G16_UINT               , SurfaceFormat::R16G16_TYPELESS           ,  4,  2, 16,  true, false, false   APPEND_DXGI(R16G16_UINT               ) },
    { SurfaceFormat::R16G16_SNORM              , SurfaceFormat::R16G16_TYPELESS           ,  4,  2, 16,  true, false, false   APPEND_DXGI(R16G16_SNORM              ) },
    { SurfaceFormat::R16G16_SINT               , SurfaceFormat::R16G16_TYPELESS           ,  4,  2, 16,  true, false, false   APPEND_DXGI(R16G16_SINT               ) },
    { SurfaceFormat::R32_TYPELESS              , SurfaceFormat::R32_TYPELESS              ,  4,  1, 32,  true, false, false   APPEND_DXGI(R32_TYPELESS              ) },
    { SurfaceFormat::D32_FLOAT                 , SurfaceFormat::R32_TYPELESS              ,  4,  1, 32,  true, false, false   APPEND_DXGI(D32_FLOAT                 ) },
    { SurfaceFormat::R32_FLOAT                 , SurfaceFormat::R32_TYPELESS              ,  4,  1, 32,  true, false, false   APPEND_DXGI(R32_FLOAT                 ) },
    { SurfaceFormat::R32_UINT                  , SurfaceFormat::R32_TYPELESS              ,  4,  1, 32,  true, false, false   APPEND_DXGI(R32_UINT                  ) },
    { SurfaceFormat::R32_SINT                  , SurfaceFormat::R32_TYPELESS              ,  4,  1, 32,  true, false, false   APPEND_DXGI(R32_SINT                  ) },
    { SurfaceFormat::R24G8_TYPELESS            , SurfaceFormat::R24G8_TYPELESS            ,  4,  2,  0, false, false, false   APPEND_DXGI(R24G8_TYPELESS            ) },
    { SurfaceFormat::D24_UNORM_S8_UINT         , SurfaceFormat::D24_UNORM_S8_UINT         ,  4,  2,  0, false, false, false   APPEND_DXGI(D24_UNORM_S8_UINT         ) },
    { SurfaceFormat::R24_UNORM_X8_TYPELESS     , SurfaceFormat::R24_UNORM_X8_TYPELESS     ,  4,  2,  0, false, false, false   APPEND_DXGI(R24_UNORM_X8_TYPELESS     ) },
    { SurfaceFormat::X24_TYPELESS_G8_UINT      , SurfaceFormat::X24_TYPELESS_G8_UINT      ,  4,  2,  0, false, false, false   APPEND_DXGI(X24_TYPELESS_G8_UINT      ) },
    { SurfaceFormat::R8G8_TYPELESS             , SurfaceFormat::R8G8_TYPELESS             ,  2,  2,  8,  true, false, false   APPEND_DXGI(R8G8_TYPELESS             ) },
    { SurfaceFormat::R8G8_UNORM                , SurfaceFormat::R8G8_TYPELESS             ,  2,  2,  8,  true, false, false   APPEND_DXGI(R8G8_UNORM                ) },
    { SurfaceFormat::R8G8_UINT                 , SurfaceFormat::R8G8_TYPELESS             ,  2,  2,  8,  true, false, false   APPEND_DXGI(R8G8_UINT                 ) },
    { SurfaceFormat::R8G8_SNORM                , SurfaceFormat::R8G8_TYPELESS             ,  2,  2,  8,  true, false, false   APPEND_DXGI(R8G8_SNORM                ) },
    { SurfaceFormat::R8G8_SINT                 , SurfaceFormat::R8G8_TYPELESS             ,  2,  2,  8,  true, false, false   APPEND_DXGI(R8G8_SINT                 ) },
    { SurfaceFormat::R16_TYPELESS              , SurfaceFormat::R16_TYPELESS              ,  2,  1, 16,  true, false, false   APPEND_DXGI(R16_TYPELESS              ) },
    { SurfaceFormat::R16_FLOAT                 , SurfaceFormat::R16_TYPELESS              ,  2,  1, 16,  true, false, false   APPEND_DXGI(R16_FLOAT                 ) },
    { SurfaceFormat::D16_UNORM                 , SurfaceFormat::R16_TYPELESS              ,  2,  1, 16,  true, false, false   APPEND_DXGI(D16_UNORM                 ) },
    { SurfaceFormat::R16_UNORM                 , SurfaceFormat::R16_TYPELESS              ,  2,  1, 16,  true, false, false   APPEND_DXGI(R16_UNORM                 ) },
    { SurfaceFormat::R16_UINT                  , SurfaceFormat::R16_TYPELESS              ,  2,  1, 16,  true, false, false   APPEND_DXGI(R16_UINT                  ) },
    { SurfaceFormat::R16_SNORM                 , SurfaceFormat::R16_TYPELESS              ,  2,  1, 16,  true, false, false   APPEND_DXGI(R16_SNORM                 ) },
    { SurfaceFormat::R16_SINT                  , SurfaceFormat::R16_TYPELESS              ,  2,  1, 16,  true, false, false   APPEND_DXGI(R16_SINT                  ) },
    { SurfaceFormat::R8_TYPELESS               , SurfaceFormat::R8_TYPELESS               ,  1,  1,  8,  true, false, false   APPEND_DXGI(R8_TYPELESS               ) },
    { SurfaceFormat::R8_UNORM                  , SurfaceFormat::R8_TYPELESS               ,  1,  1,  8,  true, false, false   APPEND_DXGI(R8_UNORM                  ) },
    { SurfaceFormat::R8_UINT                   , SurfaceFormat::R8_TYPELESS               ,  1,  1,  8,  true, false, false   APPEND_DXGI(R8_UINT                   ) },
    { SurfaceFormat::R8_SNORM                  , SurfaceFormat::R8_TYPELESS               ,  1,  1,  8,  true, false, false   APPEND_DXGI(R8_SNORM                  ) },
    { SurfaceFormat::R8_SINT                   , SurfaceFormat::R8_TYPELESS               ,  1,  1,  8,  true, false, false   APPEND_DXGI(R8_SINT                   ) },
    { SurfaceFormat::A8_UNORM                  , SurfaceFormat::R8_TYPELESS               ,  1,  1,  8,  true, false, false   APPEND_DXGI(A8_UNORM                  ) },
    { SurfaceFormat::R1_UNORM                  , SurfaceFormat::R1_UNORM                  ,  0,  1,  1, false, false, false   APPEND_DXGI(R1_UNORM                  ) },
    { SurfaceFormat::R9G9B9E5_SHAREDEXP        , SurfaceFormat::R9G9B9E5_SHAREDEXP        ,  4,  0,  0, false, false, false   APPEND_DXGI(R9G9B9E5_SHAREDEXP        ) },
    { SurfaceFormat::R8G8_B8G8_UNORM           , SurfaceFormat::R8G8_B8G8_UNORM           ,  2,  0,  0, false, false, false   APPEND_DXGI(R8G8_B8G8_UNORM           ) },
    { SurfaceFormat::G8R8_G8B8_UNORM           , SurfaceFormat::G8R8_G8B8_UNORM           ,  2,  0,  0, false, false, false   APPEND_DXGI(G8R8_G8B8_UNORM           ) },
    // TODO:
    { SurfaceFormat::BC1_TYPELESS              , SurfaceFormat::BC1_TYPELESS              ,  0,  0,  0, false, false,  true   APPEND_DXGI(BC1_TYPELESS              ) },
    { SurfaceFormat::BC1_UNORM                 , SurfaceFormat::BC1_TYPELESS              ,  0,  0,  0, false, false,  true   APPEND_DXGI(BC1_UNORM                 ) },
    { SurfaceFormat::BC1_UNORM_SRGB            , SurfaceFormat::BC1_TYPELESS              ,  0,  0,  0, false,  true,  true   APPEND_DXGI(BC1_UNORM_SRGB            ) },
    { SurfaceFormat::BC2_TYPELESS              , SurfaceFormat::BC2_TYPELESS              ,  0,  0,  0, false, false,  true   APPEND_DXGI(BC2_TYPELESS              ) },
    { SurfaceFormat::BC2_UNORM                 , SurfaceFormat::BC2_TYPELESS              ,  0,  0,  0, false, false,  true   APPEND_DXGI(BC2_UNORM                 ) },
    { SurfaceFormat::BC2_UNORM_SRGB            , SurfaceFormat::BC2_TYPELESS              ,  0,  0,  0, false,  true,  true   APPEND_DXGI(BC2_UNORM_SRGB            ) },
    { SurfaceFormat::BC3_TYPELESS              , SurfaceFormat::BC3_TYPELESS              ,  0,  0,  0, false, false,  true   APPEND_DXGI(BC3_TYPELESS              ) },
    { SurfaceFormat::BC3_UNORM                 , SurfaceFormat::BC3_TYPELESS              ,  0,  0,  0, false, false,  true   APPEND_DXGI(BC3_UNORM                 ) },
    { SurfaceFormat::BC3_UNORM_SRGB            , SurfaceFormat::BC3_TYPELESS              ,  0,  0,  0, false,  true,  true   APPEND_DXGI(BC3_UNORM_SRGB            ) },
    { SurfaceFormat::BC4_TYPELESS              , SurfaceFormat::BC4_TYPELESS              ,  0,  0,  0, false, false,  true   APPEND_DXGI(BC4_TYPELESS              ) },
    { SurfaceFormat::BC4_UNORM                 , SurfaceFormat::BC4_TYPELESS              ,  0,  0,  0, false, false,  true   APPEND_DXGI(BC4_UNORM                 ) },
    { SurfaceFormat::BC4_SNORM                 , SurfaceFormat::BC4_TYPELESS              ,  0,  0,  0, false, false,  true   APPEND_DXGI(BC4_SNORM                 ) },
    { SurfaceFormat::BC5_TYPELESS              , SurfaceFormat::BC5_TYPELESS              ,  0,  0,  0, false, false,  true   APPEND_DXGI(BC5_TYPELESS              ) },
    { SurfaceFormat::BC5_UNORM                 , SurfaceFormat::BC5_TYPELESS              ,  0,  0,  0, false, false,  true   APPEND_DXGI(BC5_UNORM                 ) },
    { SurfaceFormat::BC5_SNORM                 , SurfaceFormat::BC5_TYPELESS              ,  0,  0,  0, false, false,  true   APPEND_DXGI(BC5_SNORM                 ) },
    { SurfaceFormat::B5G6R5_UNORM              , SurfaceFormat::B5G6R5_UNORM              ,  0,  0,  0, false, false, false   APPEND_DXGI(B5G6R5_UNORM              ) },
    { SurfaceFormat::B5G5R5A1_UNORM            , SurfaceFormat::B5G5R5A1_UNORM            ,  0,  0,  0, false, false, false   APPEND_DXGI(B5G5R5A1_UNORM            ) },
    { SurfaceFormat::B8G8R8A8_UNORM            , SurfaceFormat::B8G8R8A8_UNORM            ,  0,  0,  0,  true, false, false   APPEND_DXGI(B8G8R8A8_UNORM            ) },
    { SurfaceFormat::B8G8R8X8_UNORM            , SurfaceFormat::B8G8R8X8_UNORM            ,  0,  0,  0,  true, false, false   APPEND_DXGI(B8G8R8X8_UNORM            ) },
    { SurfaceFormat::R10G10B10_XR_BIAS_A2_UNORM, SurfaceFormat::R10G10B10_XR_BIAS_A2_UNORM,  0,  0,  0, false, false, false   APPEND_DXGI(R10G10B10_XR_BIAS_A2_UNORM) },
    { SurfaceFormat::B8G8R8A8_TYPELESS         , SurfaceFormat::B8G8R8A8_TYPELESS         ,  0,  0,  0,  true, false, false   APPEND_DXGI(B8G8R8A8_TYPELESS         ) },
    { SurfaceFormat::B8G8R8A8_UNORM_SRGB       , SurfaceFormat::B8G8R8A8_TYPELESS         ,  0,  0,  0,  true,  true, false   APPEND_DXGI(B8G8R8A8_UNORM_SRGB       ) },
    { SurfaceFormat::B8G8R8X8_TYPELESS         , SurfaceFormat::B8G8R8X8_TYPELESS         ,  0,  0,  0,  true, false, false   APPEND_DXGI(B8G8R8X8_TYPELESS         ) },
    { SurfaceFormat::B8G8R8X8_UNORM_SRGB       , SurfaceFormat::B8G8R8X8_TYPELESS         ,  0,  0,  0,  true,  true, false   APPEND_DXGI(B8G8R8X8_UNORM_SRGB       ) },
    { SurfaceFormat::BC6H_TYPELESS             , SurfaceFormat::BC6H_TYPELESS             ,  0,  0,  0, false, false,  true   APPEND_DXGI(BC6H_TYPELESS             ) },
    { SurfaceFormat::BC6H_UF16                 , SurfaceFormat::BC6H_TYPELESS             ,  0,  0,  0, false, false,  true   APPEND_DXGI(BC6H_UF16                 ) },
    { SurfaceFormat::BC6H_SF16                 , SurfaceFormat::BC6H_TYPELESS             ,  0,  0,  0, false, false,  true   APPEND_DXGI(BC6H_SF16                 ) },
    { SurfaceFormat::BC7_TYPELESS              , SurfaceFormat::BC7_TYPELESS              ,  0,  0,  0, false, false,  true   APPEND_DXGI(BC7_TYPELESS              ) },
    { SurfaceFormat::BC7_UNORM                 , SurfaceFormat::BC7_TYPELESS              ,  0,  0,  0, false, false,  true   APPEND_DXGI(BC7_UNORM                 ) },
    { SurfaceFormat::BC7_UNORM_SRGB            , SurfaceFormat::BC7_TYPELESS              ,  0,  0,  0, false,  true,  true   APPEND_DXGI(BC7_UNORM_SRGB            ) },
    { SurfaceFormat::AYUV                      , SurfaceFormat::AYUV                      ,  0,  0,  0, false, false, false   APPEND_DXGI(AYUV                      ) },
    { SurfaceFormat::Y410                      , SurfaceFormat::Y410                      ,  0,  0,  0, false, false, false   APPEND_DXGI(Y410                      ) },
    { SurfaceFormat::Y416                      , SurfaceFormat::Y416                      ,  0,  0,  0, false, false, false   APPEND_DXGI(Y416                      ) },
    { SurfaceFormat::NV12                      , SurfaceFormat::NV12                      ,  0,  0,  0, false, false, false   APPEND_DXGI(NV12                      ) },
    { SurfaceFormat::P010                      , SurfaceFormat::P010                      ,  0,  0,  0, false, false, false   APPEND_DXGI(P010                      ) },
    { SurfaceFormat::P016                      , SurfaceFormat::P016                      ,  0,  0,  0, false, false, false   APPEND_DXGI(P016                      ) },
 // { SurfaceFormat::420_OPAQUE                , SurfaceFormat::420_OPAQUE                ,  0,  0,  0, false, false, false   APPEND_DXGI(420_OPAQUE                ) },
    { SurfaceFormat::UNKNOWN /* TO REPLACE */  , SurfaceFormat::UNKNOWN                   ,  0,  0,  0, false, false, false   APPEND_DXGI(420_OPAQUE                ) },
    { SurfaceFormat::YUY2                      , SurfaceFormat::YUY2                      ,  0,  0,  0, false, false, false   APPEND_DXGI(YUY2                      ) },
    { SurfaceFormat::Y210                      , SurfaceFormat::Y210                      ,  0,  0,  0, false, false, false   APPEND_DXGI(Y210                      ) },
    { SurfaceFormat::Y216                      , SurfaceFormat::Y216                      ,  0,  0,  0, false, false, false   APPEND_DXGI(Y216                      ) },
    { SurfaceFormat::NV11                      , SurfaceFormat::NV11                      ,  0,  0,  0, false, false, false   APPEND_DXGI(NV11                      ) },
    { SurfaceFormat::AI44                      , SurfaceFormat::AI44                      ,  0,  0,  0, false, false, false   APPEND_DXGI(AI44                      ) },
    { SurfaceFormat::IA44                      , SurfaceFormat::IA44                      ,  0,  0,  0, false, false, false   APPEND_DXGI(IA44                      ) },
    { SurfaceFormat::P8                        , SurfaceFormat::P8                        ,  0,  0,  0, false, false, false   APPEND_DXGI(P8                        ) },
    { SurfaceFormat::A8P8                      , SurfaceFormat::A8P8                      ,  0,  0,  0, false, false, false   APPEND_DXGI(A8P8                      ) },
    { SurfaceFormat::B4G4R4A4_UNORM            , SurfaceFormat::B4G4R4A4_UNORM            ,  0,  0,  0, false, false, false   APPEND_DXGI(B4G4R4A4_UNORM            ) },
};
}
//=============================================================================
SurfaceFormatProperties const& GetProperties(SurfaceFormat iSurfaceFormat)
{
    SG_ASSERT(size_t(iSurfaceFormat) < SG_ARRAYSIZE(surfaceFormatProperties));
    SurfaceFormatProperties const& surfaceProperties = surfaceFormatProperties[size_t(iSurfaceFormat)];
    SG_ASSERT(surfaceProperties.surfaceFormat == iSurfaceFormat);
    return surfaceProperties;
}
//'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ArrayView<SurfaceFormatProperties const> GetAllSurfaceFormatProperties()
{
    return AsArrayView(surfaceFormatProperties);
}
//=============================================================================
}
}
