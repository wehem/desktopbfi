#include "ReShade.fxh"

uniform float OVERDRIVE_STRENGTH <
    ui_type = "slider";
    ui_min = 0.0; ui_max = 1.0; ui_step = 0.01;
    ui_label = "Overdrive Strength";
    ui_tooltip = "Strength of the overdrive effect";
> = 0.5;

texture PrevFrameTex { Width = BUFFER_WIDTH; Height = BUFFER_HEIGHT; Format = RGBA8; };
sampler PrevFrameSampler { Texture = PrevFrameTex; };

texture BackBufferTex : COLOR;
sampler BackBuffer { Texture = BackBufferTex; };

// Simple overdrive lookup function
float3 OverdriveLookup(float3 prev, float3 current)
{
    float3 diff = current - prev;
    float3 overdrive = current + diff * OVERDRIVE_STRENGTH;
    return clamp(overdrive, 0.0, 1.0);
}

float4 PS_Overdrive(float4 vpos : SV_Position, float2 texcoord : TEXCOORD) : SV_Target
{
    float4 current = tex2D(BackBuffer, texcoord);
    float4 prev = tex2D(PrevFrameSampler, texcoord);
    
    float3 overdriven = OverdriveLookup(prev.rgb, current.rgb);
    
    return float4(overdriven, current.a);
}

float4 PS_StorePrevFrame(float4 vpos : SV_Position, float2 texcoord : TEXCOORD) : SV_Target
{
    return tex2D(BackBuffer, texcoord);
}

technique PanelOverdrive
{
    pass Overdrive
    {
        VertexShader = PostProcessVS;
        PixelShader = PS_Overdrive;
    }
    
    pass StorePrevFrame
    {
        VertexShader = PostProcessVS;
        PixelShader = PS_StorePrevFrame;
        RenderTarget = PrevFrameTex;
    }
}
