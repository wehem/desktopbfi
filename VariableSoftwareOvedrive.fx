#include "ReShade.fxh"
#include "ReShadeUI.fxh"

uniform float frame_time < source = "frametime"; >;

uniform float OVERDRIVE_STRENGTH_40 <
    ui_type = "slider";
    ui_min = 0.0; ui_max = 1.0; ui_step = 0.01;
    ui_label = "Overdrive Strength at 40 Hz";
> = 0.5;

uniform float OVERDRIVE_STRENGTH_50 <
    ui_type = "slider";
    ui_min = 0.0; ui_max = 1.0; ui_step = 0.01;
    ui_label = "Overdrive Strength at 50 Hz";
> = 0.5;

uniform float OVERDRIVE_STRENGTH_60 <
    ui_type = "slider";
    ui_min = 0.0; ui_max = 1.0; ui_step = 0.01;
    ui_label = "Overdrive Strength at 60 Hz";
> = 0.5;

uniform float OVERDRIVE_STRENGTH_70 <
    ui_type = "slider";
    ui_min = 0.0; ui_max = 1.0; ui_step = 0.01;
    ui_label = "Overdrive Strength at 70 Hz";
> = 0.5;

uniform float OVERDRIVE_STRENGTH_80 <
    ui_type = "slider";
    ui_min = 0.0; ui_max = 1.0; ui_step = 0.01;
    ui_label = "Overdrive Strength at 80 Hz";
> = 0.5;

uniform float OVERDRIVE_STRENGTH_90 <
    ui_type = "slider";
    ui_min = 0.0; ui_max = 1.0; ui_step = 0.01;
    ui_label = "Overdrive Strength at 90 Hz";
> = 0.5;

uniform float OVERDRIVE_STRENGTH_100 <
    ui_type = "slider";
    ui_min = 0.0; ui_max = 1.0; ui_step = 0.01;
    ui_label = "Overdrive Strength at 100 Hz";
> = 0.5;

uniform float OVERDRIVE_STRENGTH_110 <
    ui_type = "slider";
    ui_min = 0.0; ui_max = 1.0; ui_step = 0.01;
    ui_label = "Overdrive Strength at 110 Hz";
> = 0.5;

uniform float OVERDRIVE_STRENGTH_120 <
    ui_type = "slider";
    ui_min = 0.0; ui_max = 1.0; ui_step = 0.01;
    ui_label = "Overdrive Strength at 120 Hz";
> = 0.5;

uniform float OVERDRIVE_STRENGTH_130 <
    ui_type = "slider";
    ui_min = 0.0; ui_max = 1.0; ui_step = 0.01;
    ui_label = "Overdrive Strength at 130 Hz";
> = 0.5;

uniform float OVERDRIVE_STRENGTH_140 <
    ui_type = "slider";
    ui_min = 0.0; ui_max = 1.0; ui_step = 0.01;
    ui_label = "Overdrive Strength at 140 Hz";
> = 0.5;

uniform float OVERDRIVE_STRENGTH_150 <
    ui_type = "slider";
    ui_min = 0.0; ui_max = 1.0; ui_step = 0.01;
    ui_label = "Overdrive Strength at 150 Hz";
> = 0.5;

uniform float OVERDRIVE_STRENGTH_160 <
    ui_type = "slider";
    ui_min = 0.0; ui_max = 1.0; ui_step = 0.01;
    ui_label = "Overdrive Strength at 160 Hz";
> = 0.5;

uniform int INTERPOLATION_ALGORITHM <
    ui_type = "combo";
    ui_label = "Interpolation Algorithm";
    ui_items = "Linear\0Cubic\0";
> = 0;


texture PrevFrameTex { Width = BUFFER_WIDTH; Height = BUFFER_HEIGHT; Format = RGBA8; };
sampler PrevFrameSampler { Texture = PrevFrameTex; };

texture PrevFrameTimeTex { Width = 1; Height = 1; Format = R32F; };
sampler PrevFrameTimeSampler { Texture = PrevFrameTimeTex; };

texture BackBufferTex : COLOR;
sampler BackBuffer { Texture = BackBufferTex; };

float LinearInterpolation(float x, float x0, float x1, float y0, float y1)
{
    return y0 + (x - x0) * (y1 - y0) / (x1 - x0);
}

float CubicInterpolation(float x, float x0, float x1, float y0, float y1)
{
    float t = (x - x0) / (x1 - x0);
    float t2 = t * t;
    float t3 = t2 * t;
    return y0 * (1 - 3 * t2 + 2 * t3) + y1 * (3 * t2 - 2 * t3);
}

float GetOverdriveStrength(float refreshRate)
{
    float strength = 0.0;
    
    if (refreshRate <= 40) strength = OVERDRIVE_STRENGTH_40;
    else if (refreshRate <= 50) strength = INTERPOLATION_ALGORITHM == 0 ? 
        LinearInterpolation(refreshRate, 40, 50, OVERDRIVE_STRENGTH_40, OVERDRIVE_STRENGTH_50) :
        CubicInterpolation(refreshRate, 40, 50, OVERDRIVE_STRENGTH_40, OVERDRIVE_STRENGTH_50);
    else if (refreshRate <= 60) strength = INTERPOLATION_ALGORITHM == 0 ? 
        LinearInterpolation(refreshRate, 50, 60, OVERDRIVE_STRENGTH_50, OVERDRIVE_STRENGTH_60) :
        CubicInterpolation(refreshRate, 50, 60, OVERDRIVE_STRENGTH_50, OVERDRIVE_STRENGTH_60);
    else if (refreshRate <= 70) strength = INTERPOLATION_ALGORITHM == 0 ? 
        LinearInterpolation(refreshRate, 60, 70, OVERDRIVE_STRENGTH_60, OVERDRIVE_STRENGTH_70) :
        CubicInterpolation(refreshRate, 60, 70, OVERDRIVE_STRENGTH_60, OVERDRIVE_STRENGTH_70);
    else if (refreshRate <= 80) strength = INTERPOLATION_ALGORITHM == 0 ? 
        LinearInterpolation(refreshRate, 70, 80, OVERDRIVE_STRENGTH_70, OVERDRIVE_STRENGTH_80) :
        CubicInterpolation(refreshRate, 70, 80, OVERDRIVE_STRENGTH_70, OVERDRIVE_STRENGTH_80);
	else if (refreshRate <= 90) strength = INTERPOLATION_ALGORITHM == 0 ? 
        LinearInterpolation(refreshRate, 80, 90, OVERDRIVE_STRENGTH_80, OVERDRIVE_STRENGTH_90) :
        CubicInterpolation(refreshRate, 80, 90, OVERDRIVE_STRENGTH_80, OVERDRIVE_STRENGTH_90);
    else if (refreshRate <= 100) strength = INTERPOLATION_ALGORITHM == 0 ? 
        LinearInterpolation(refreshRate, 90, 100, OVERDRIVE_STRENGTH_90, OVERDRIVE_STRENGTH_100) :
        CubicInterpolation(refreshRate, 90, 100, OVERDRIVE_STRENGTH_90, OVERDRIVE_STRENGTH_100);
    else if (refreshRate <= 110) strength = INTERPOLATION_ALGORITHM == 0 ? 
        LinearInterpolation(refreshRate, 100, 110, OVERDRIVE_STRENGTH_100, OVERDRIVE_STRENGTH_110) :
        CubicInterpolation(refreshRate, 100, 110, OVERDRIVE_STRENGTH_100, OVERDRIVE_STRENGTH_110);
    else if (refreshRate <= 120) strength = INTERPOLATION_ALGORITHM == 0 ? 
        LinearInterpolation(refreshRate, 110, 120, OVERDRIVE_STRENGTH_110, OVERDRIVE_STRENGTH_120) :
        CubicInterpolation(refreshRate, 110, 120, OVERDRIVE_STRENGTH_110, OVERDRIVE_STRENGTH_120);
    else if (refreshRate <= 130) strength = INTERPOLATION_ALGORITHM == 0 ? 
        LinearInterpolation(refreshRate, 120, 130, OVERDRIVE_STRENGTH_120, OVERDRIVE_STRENGTH_130) :
        CubicInterpolation(refreshRate, 120, 130, OVERDRIVE_STRENGTH_120, OVERDRIVE_STRENGTH_130);
    else if (refreshRate <= 140) strength = INTERPOLATION_ALGORITHM == 0 ? 
        LinearInterpolation(refreshRate, 130, 140, OVERDRIVE_STRENGTH_130, OVERDRIVE_STRENGTH_140) :
        CubicInterpolation(refreshRate, 130, 140, OVERDRIVE_STRENGTH_130, OVERDRIVE_STRENGTH_140);
    else if (refreshRate <= 150) strength = INTERPOLATION_ALGORITHM == 0 ? 
        LinearInterpolation(refreshRate, 140, 150, OVERDRIVE_STRENGTH_140, OVERDRIVE_STRENGTH_150) :
        CubicInterpolation(refreshRate, 140, 150, OVERDRIVE_STRENGTH_140, OVERDRIVE_STRENGTH_150);
    else if (refreshRate <= 160) strength = INTERPOLATION_ALGORITHM == 0 ? 
        LinearInterpolation(refreshRate, 150, 160, OVERDRIVE_STRENGTH_150, OVERDRIVE_STRENGTH_160) :
        CubicInterpolation(refreshRate, 150, 160, OVERDRIVE_STRENGTH_150, OVERDRIVE_STRENGTH_160);
    else strength = OVERDRIVE_STRENGTH_160;
    
    return strength;
}

float3 VariableOverdriveLookup(float3 prev, float3 current, float prevFrameTime, float currentFrameTime)
{
    float3 diff = current - prev;
    
    float prevRefreshRate = 1000.0 / prevFrameTime;
    float currentRefreshRate = 1000.0 / currentFrameTime;
    
    float prevStrength = GetOverdriveStrength(prevRefreshRate);
    float currentStrength = GetOverdriveStrength(currentRefreshRate);
    
    float adjustedStrength = lerp(prevStrength, currentStrength, 0.5);
    
    float3 overdrive = current + diff * adjustedStrength;
    return clamp(overdrive, 0.0, 1.0);
}

float4 PS_Overdrive(float4 vpos : SV_Position, float2 texcoord : TEXCOORD) : SV_Target
{
    float4 current = tex2D(BackBuffer, texcoord);
    float4 prev = tex2D(PrevFrameSampler, texcoord);
    float prevFrameTime = tex2D(PrevFrameTimeSampler, float2(0, 0)).r;
    float currentFrameTime = frame_time;
    
    float3 overdriven = VariableOverdriveLookup(prev.rgb, current.rgb, prevFrameTime, currentFrameTime);
    
    return float4(overdriven, current.a);
}

float4 PS_StorePrevFrame(float4 vpos : SV_Position, float2 texcoord : TEXCOORD) : SV_Target
{
    return tex2D(BackBuffer, texcoord);
}

float4 PS_StorePrevFrameTime(float4 vpos : SV_Position, float2 texcoord : TEXCOORD) : SV_Target
{
    return float4(frame_time, 0, 0, 0);
}

technique VariablePanelOverdrive
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
    
    pass StorePrevFrameTime
    {
        VertexShader = PostProcessVS;
        PixelShader = PS_StorePrevFrameTime;
        RenderTarget = PrevFrameTimeTex;
    }
}