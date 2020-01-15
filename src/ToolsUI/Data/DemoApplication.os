import "data:/scripts/lib/StandardApplication.oslib"

StandardOffscreenCommon{}
StandardToolsUICommon{}
StandardCommonTools{}

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
            sg::renderengine::SetRenderTargetsDescriptor { renderTargets:[ render ] depthStencil:null },
            sg::renderengine::ShaderPassDescriptor { pixelShader: sg::renderengine::PixelShaderDescriptor { file:"src:/ToolsUI/Data/Shaders/TestBezier.hlsl" entryPoint:"pmain" } },
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
