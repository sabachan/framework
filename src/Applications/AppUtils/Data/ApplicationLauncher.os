import "data:/scripts/lib/StandardApplication.oslib"

StandardOffscreenCommon{}
StandardToolsUICommon{}

export compositing is sg::renderengine::CompositingDescriptor
{
    inputSurfaces : []
    outputSurfaces : [["render", protected render is sg::renderengine::OutputSurfaceDescriptor{}]]
    inputConstantDatabases : [["input constants", ::inputConstants]]
    instructions : [
        [ "Draw",
            [
            sg::renderengine::SetShaderResourceDatabaseDescriptor { database:shaderResources },
            sg::renderengine::SetConstantDatabaseDescriptor { database:inputConstants },
            sg::renderengine::ClearSurfacesDescriptor { renderTargets:[ render ] color:[0,0,0,1] },
            sg::renderengine::SetRenderTargetsDescriptor { renderTargets:[render] depthStencil:null },
            sg::renderengine::CompositingLayerDescriptor { spec: [ "gui2D" ] },
            ]
        ],
    ]
}

private inputConstants is sg::renderengine::InputConstantDatabaseDescriptor
{
}
private shaderResources is sg::renderengine::ShaderResourceDatabaseDescriptor
{
    resources: [
    ]
    samplers: [
        ["sampler_input", sg::renderengine::SamplerDescriptor{filter: "Linear" adressMode: "Border" borderColor: [0,0,0,0]} ],
    ]
}
private render_depth is sg::renderengine::DepthStencilSurfaceDescriptor
{
    resolution: sg::renderengine::ResolutionFromOutputSurfaceDescriptor{ surface: ::compositing::render }
}

export styleGuide is sg::ApplicationLauncherStyleGuide
{
    colors : [
        [ "BGColor",        [  0,   0,   0, 255] ],
        [ "FillColor",      [ 96,  96, 128, 128] ],
        [ "LineColor",      [255, 240, 200, 255] ],
        [ "HighlightColor", [128, 160, 200, 128] ],
        [ "ClickColor",     [160, 160, 160, 128] ],
    ]
    lengths : [
        [ "LineThickness",      { unit : 1 } ],
        [ "LineThickness2",     { unit : 2 } ],
        [ "TextHMargin",        { magnifiable : 40 } ],
        [ "TextTopMargin",      { magnifiable : 8 } ],
        [ "TextBottomMargin",   { magnifiable : 5 } ],
    ]
    uniformDrawers : [
        [ "Default", sg::ui::UniformDrawerDescriptor { pixelShader: sg::renderengine::PixelShaderDescriptor { file: "src:/UserInterface/Shaders/P_UI_UniformDrawer.hlsl" entryPoint: "pmain" } } ]
    ]
    textureDrawers : [
        [ "Default", sg::ui::TextureDrawerDescriptor { pixelShader: sg::renderengine::PixelShaderDescriptor { file: "src:/UserInterface/Shaders/P_UI_TextureDrawer_Multiply.hlsl" entryPoint: "pmain" } textureBindName : "texture_input" } ]
    ]
    tfs : [
        [ "Default", sg::ui::TextFormatScript {} ],
    ]

    textStyles : [
        [ "Default", {
            fillColor      : [255, 255, 255, 255]
            strokeColor    : [  0,   0,   0,   0]
            strokeSize     : 0.5
            size           : 13
            fontFamilyName : "tech"
            bold           : false
            italic         : false
            superscript    : false
            subscript      : false
            underlined     : false
            strikeThrough  : false
        } ],
    ]

    paragraphStyles : [
        [ "Default", {
            alignment : 0.
            justified : 0.
            lineWidth : 300.
            lineGap   : 0
            balanced  : false
        } ],
    ]
}
