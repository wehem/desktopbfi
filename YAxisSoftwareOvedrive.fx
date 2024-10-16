#include "ReShade.fxh"

uniform float TOP_OD_GAIN <
    ui_type = "slider";
    ui_min = 0.0; ui_max = 1.0; ui_step = 0.01;
    ui_label = "Top Overdrive Gain";
    ui_tooltip = "Overdrive gain for the top of the screen";
> = 0.3;

uniform float CENTER_OD_GAIN <
    ui_type = "slider";
    ui_min = 0.0; ui_max = 1.0; ui_step = 0.01;
    ui_label = "Center Overdrive Gain";
    ui_tooltip = "Overdrive gain for the center of the screen";
> = 0.5;

uniform float BOTTOM_OD_GAIN <
    ui_type = "slider";
    ui_min = 0.0; ui_max = 1.0; ui_step = 0.01;
    ui_label = "Bottom Overdrive Gain";
    ui_tooltip = "Overdrive gain for the bottom of the screen";
> = 0.7;

texture PrevFrameTex { Width = BUFFER_WIDTH; Height = BUFFER_HEIGHT; Format = RGBA8; };
sampler PrevFrameSampler { Texture = PrevFrameTex; };

texture BackBufferTex : COLOR;
sampler BackBuffer { Texture = BackBufferTex; };

// Calculate overdrive gain based on vertical position
float CalculateODGain(float y)
{
    if (y < 0.5)
    {
        return lerp(TOP_OD_GAIN, CENTER_OD_GAIN, y * 2);
    }
    else
    {
        return lerp(CENTER_OD_GAIN, BOTTOM_OD_GAIN, (y - 0.5) * 2);
    }
}

// Overdrive lookup function with variable gain
float3 OverdriveLookup(float3 prev, float3 current, float gain)
{
    float3 diff = current - prev;
    float3 overdrive = current + diff * gain;
    return clamp(overdrive, 0.0, 1.0);
}

float4 PS_Overdrive(float4 vpos : SV_Position, float2 texcoord : TEXCOORD) : SV_Target
{
    float4 current = tex2D(BackBuffer, texcoord);
    float4 prev = tex2D(PrevFrameSampler, texcoord);
    
    float odGain = CalculateODGain(texcoord.y);
    float3 overdriven = OverdriveLookup(prev.rgb, current.rgb, odGain);
    
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
