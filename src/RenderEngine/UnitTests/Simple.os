export compositing is sg::renderengine::CompositingDescriptor
{
    inputSurfaces : []
    outputSurfaces : [["render", protected render is sg::renderengine::OutputSurfaceDescriptor{}]]
    inputConstantDatabases : [["input constants", ::inputConstants]]
    instructions : [
        [ "Init",
            [
            sg::renderengine::SetConstantDatabaseDescriptor { database:constants },
            sg::renderengine::SetShaderResourceDatabaseDescriptor { database:shaderResources },
            sg::renderengine::ClearSurfacesDescriptor { renderTargets:[ pripyat_mipmap ] color:[1,1,0,1] },
            sg::renderengine::SetRenderTargetsDescriptor { renderTargets:[ pripyat_mipmap ] depthStencil:null },
            sg::renderengine::SetShaderResourceDatabaseDescriptor { database: sg::renderengine::ShaderResourceDatabaseDescriptor{ parent:shaderResources resources: [["texture_input", pripyat]] }},
            sg::renderengine::ShaderPassDescriptor { pixelShader: sg::renderengine::PixelShaderDescriptor { file:"src:/RenderEngine/UnitTests/CopyInput.hlsl" entryPoint:"pmain" } },
            sg::renderengine::GenerateMipmapDescriptor { surface: pripyat_mipmap bindingPoint: "texture_input" pixelShader: sg::renderengine::PixelShaderDescriptor { file:"src:/RenderEngine/UnitTests/CopyInput.hlsl" entryPoint:"pmain" } },
            ]
        ],
        [ "Draw",
            [
            sg::renderengine::SetConstantDatabaseDescriptor { database:constants },
            sg::renderengine::ClearSurfacesDescriptor { renderTargets:[ render ] color:[0,0,0,1]  depthStencils: [ ::render_depth ] depth: 1},
            //sg::renderengine::ClearSurfacesDescriptor { renderTargets:[ internalSurface ] color:[1,0,1,1]},
            //sg::renderengine::SetRenderTargetsDescriptor { renderTargets:[internalSurface] depthStencil:null },
            sg::renderengine::SetShaderResourceDatabaseDescriptor { database:shaderResources },
            //sg::renderengine::ShaderPassDescriptor { pixelShader: sg::renderengine::PixelShaderDescriptor { file:"src:/RenderEngine/UnitTests/CopyInput.hlsl" entryPoint:"pmain" } },
            //
            sg::renderengine::SetRenderTargetsDescriptor { renderTargets:[render] depthStencil:null },
            sg::renderengine::SetShaderResourceDatabaseDescriptor { database: sg::renderengine::ShaderResourceDatabaseDescriptor{ parent:shaderResources resources: [["texture_input", pripyat_mipmap ]] }},
            sg::renderengine::ShaderPassDescriptor { pixelShader: sg::renderengine::PixelShaderDescriptor { file:"src:/RenderEngine/UnitTests/CopyInput.hlsl" entryPoint:"pmain" } },
            sg::renderengine::ShaderPassDescriptor { pixelShader: sg::renderengine::PixelShaderDescriptor { file:"src:/RenderEngine/UnitTests/Explosion.hlsl" entryPoint:"pmain" } },
            sg::renderengine::SetConstantDatabaseDescriptor { database:constants },
            sg::renderengine::SetRenderTargetsDescriptor { renderTargets:[render] depthStencil: ::render_depth },
            sg::renderengine::CompositingLayerDescriptor { spec: [ "opaque", "main render" ] },
            sg::renderengine::SetRenderTargetsDescriptor { renderTargets:[render] depthStencil:null },
            sg::renderengine::ShaderPassDescriptor { pixelShader: sg::renderengine::PixelShaderDescriptor { file:"src:/RenderEngine/UnitTests/AnimatedGeometries.hlsl" entryPoint:"pmain" } },
            sg::renderengine::ShaderPassDescriptor { pixelShader: sg::renderengine::PixelShaderDescriptor { file:"src:/RenderEngine/UnitTests/AnimateEarthGolem.hlsl" entryPoint:"pmain" } },
            ]
        ],
        // todo Clear SRVs at end.
    ]
}

private inputConstants is sg::renderengine::InputConstantDatabaseDescriptor
{
    constants: [ ["date_in_frames", 0], ["dummy_int", 0], ["dummy_float", 1.], ]
}

private constants is sg::renderengine::ConstantDatabaseDescriptor
{
    parent: ::inputConstants
    //constants: [ ["date_in_frames", 100], ["dummy_int", 3], ["dummy_float", 4.2], ] // ["dummy_array_float", [1.,2,3,4] ] ]
}

private pripyat is sg::renderengine::TextureSurfaceDescriptor{filename:"src:/RenderEngine/UnitTests/pripyat.png"}
private pripyat_mipmap is sg::renderengine::SurfaceDescriptor
{
    resolution: sg::renderengine::ResolutionFromTextureSurfaceDescriptor{ surface: ::pripyat }
}
private earthGolem is sg::renderengine::TextureSurfaceDescriptor{filename:"src:/RenderEngine/UnitTests/animatedEarthGolem.png"}

private shaderResources is sg::renderengine::ShaderResourceDatabaseDescriptor
{
    resources: [
        //["texture_pripyat", pripyat ],
        //["texture_input", pripyat_mipmap ],
        ["texture_earthGolem", earthGolem ],
    ]
    samplers: [
        ["sampler_input", sg::renderengine::SamplerDescriptor{filter: "Linear" adressMode: "Border" borderColor: [0,0,0,0]} ],
    ]
}
private render_depth is sg::renderengine::DepthStencilSurfaceDescriptor
{
    resolution: sg::renderengine::ResolutionFromOutputSurfaceDescriptor{ surface: ::compositing::render }
}

/*
private internalSurface is sg::renderengine::SurfaceDescriptor
{
    //resolution: sg::renderengine::ResolutionFromOutputSurfaceDescriptor{ surface: ::compositing::render }
    resolution: sg::renderengine::ResolutionDescriptor{
        parentx: private pripyat_res is sg::renderengine::ResolutionFromTextureSurfaceDescriptor{ surface: pripyat }
        parenty: pripyat_res
        scale: [1./2, 1./2]
        roundingMode: "Floor"
        additiveTerm: [0,0]
    }
}
*/
