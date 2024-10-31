#include "ReShade.fxh"

uniform float UNDERDRIVE_STRENGTH <
    ui_type = "slider";
    ui_min = 0.0; ui_max = 1.0; ui_step = 0.01;
    ui_label = "Underdrive Strength";
    ui_tooltip = "Strength of the underdrive effect";
> = 0.5;

texture PrevFrameTex { Width = BUFFER_WIDTH; Height = BUFFER_HEIGHT; Format = RGBA8; };
sampler PrevFrameSampler { Texture = PrevFrameTex; };

texture BackBufferTex : COLOR;
sampler BackBuffer { Texture = BackBufferTex; };

// Simple underdrive lookup function
float3 UnderdriveLookup(float3 prev, float3 current)
{
    float3 diff = current - prev;
    float3 underdrive = current - diff * UNDERDRIVE_STRENGTH;
    return clamp(underdrive, 0.0, 1.0);
}

float4 PS_Underdrive(float4 vpos : SV_Position, float2 texcoord : TEXCOORD) : SV_Target
{
    float4 current = tex2D(BackBuffer, texcoord);
    float4 prev = tex2D(PrevFrameSampler, texcoord);
    
    float3 underdriven = UnderdriveLookup(prev.rgb, current.rgb);
    
    return float4(underdriven, current.a);
}

float4 PS_StorePrevFrame(float4 vpos : SV_Position, float2 texcoord : TEXCOORD) : SV_Target
{
    return tex2D(BackBuffer, texcoord);
}

technique PanelUnderdrive
{
    pass Underdrive
    {
        VertexShader = PostProcessVS;
        PixelShader = PS_Underdrive;
    }
    
    pass StorePrevFrame
    {
        VertexShader = PostProcessVS;
        PixelShader = PS_StorePrevFrame;
        RenderTarget = PrevFrameTex;
    }
}