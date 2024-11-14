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

// Textures and samplers
texture Prev2FrameTex { Width = BUFFER_WIDTH; Height = BUFFER_HEIGHT; Format = RGBA8; };
sampler Prev2FrameSampler { Texture = Prev2FrameTex; };

texture PrevFrameTex { Width = BUFFER_WIDTH; Height = BUFFER_HEIGHT; Format = RGBA8; };
sampler PrevFrameSampler { Texture = PrevFrameTex; };

texture Prev2FrameTimeTex { Width = 1; Height = 1; Format = R32F; };
sampler Prev2FrameTimeSampler { Texture = Prev2FrameTimeTex; };

texture PrevFrameTimeTex { Width = 1; Height = 1; Format = R32F; };
sampler PrevFrameTimeSampler { Texture = PrevFrameTimeTex; };

texture BackBufferTex : COLOR;
sampler BackBuffer { Texture = BackBufferTex; };

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
    
    if (refreshRate <= 40) 
        strength = OVERDRIVE_STRENGTH_40;
    else if (refreshRate <= 50) 
        strength = CubicInterpolation(refreshRate, 40, 50, OVERDRIVE_STRENGTH_40, OVERDRIVE_STRENGTH_50);
    else if (refreshRate <= 60) 
        strength = CubicInterpolation(refreshRate, 50, 60, OVERDRIVE_STRENGTH_50, OVERDRIVE_STRENGTH_60);
    else if (refreshRate <= 70) 
        strength = CubicInterpolation(refreshRate, 60, 70, OVERDRIVE_STRENGTH_60, OVERDRIVE_STRENGTH_70);
    else if (refreshRate <= 80) 
        strength = CubicInterpolation(refreshRate, 70, 80, OVERDRIVE_STRENGTH_70, OVERDRIVE_STRENGTH_80);
    else if (refreshRate <= 90) 
        strength = CubicInterpolation(refreshRate, 80, 90, OVERDRIVE_STRENGTH_80, OVERDRIVE_STRENGTH_90);
    else if (refreshRate <= 100) 
        strength = CubicInterpolation(refreshRate, 90, 100, OVERDRIVE_STRENGTH_90, OVERDRIVE_STRENGTH_100);
    else if (refreshRate <= 110) 
        strength = CubicInterpolation(refreshRate, 100, 110, OVERDRIVE_STRENGTH_100, OVERDRIVE_STRENGTH_110);
    else if (refreshRate <= 120) 
        strength = CubicInterpolation(refreshRate, 110, 120, OVERDRIVE_STRENGTH_110, OVERDRIVE_STRENGTH_120);
    else if (refreshRate <= 130) 
        strength = CubicInterpolation(refreshRate, 120, 130, OVERDRIVE_STRENGTH_120, OVERDRIVE_STRENGTH_130);
    else if (refreshRate <= 140) 
        strength = CubicInterpolation(refreshRate, 130, 140, OVERDRIVE_STRENGTH_130, OVERDRIVE_STRENGTH_140);
    else if (refreshRate <= 150) 
        strength = CubicInterpolation(refreshRate, 140, 150, OVERDRIVE_STRENGTH_140, OVERDRIVE_STRENGTH_150);
    else if (refreshRate <= 160) 
        strength = CubicInterpolation(refreshRate, 150, 160, OVERDRIVE_STRENGTH_150, OVERDRIVE_STRENGTH_160);
    else 
        strength = OVERDRIVE_STRENGTH_160;
    
    return strength;
}

float3 CalculateQuadraticCoefficients(float3 y0, float3 y1, float3 y2, float t0, float t1, float t2)
{
    float t0_2 = t0 * t0;
    float t1_2 = t1 * t1;
    float t2_2 = t2 * t2;
    
    float3 a, b, c;
    
    [unroll]
    for(int i = 0; i < 3; i++)
    {
        float numerator_a = (y2[i] - y0[i]) * (t1 - t0) - (y1[i] - y0[i]) * (t2 - t0);
        float denominator_a = (t2_2 - t0_2) * (t1 - t0) - (t1_2 - t0_2) * (t2 - t0);
        a[i] = numerator_a / max(denominator_a, 0.0001); // Prevent division by zero
        
        float numerator_b = (y1[i] - y0[i]) - a[i] * (t1_2 - t0_2);
        float denominator_b = max(t1 - t0, 0.0001); // Prevent division by zero
        b[i] = numerator_b / denominator_b;
        
        c[i] = y0[i] - (a[i] * t0_2) - (b[i] * t0);
    }
    
    return float3(a.x, b.x, c.x);
}

float3 PredictNextValue(float3 coef, float t)
{
    // Limit the prediction to avoid extreme values
    float3 predicted = coef.x * t * t + coef.y * t + coef.z;
    return clamp(predicted, 0.0, 1.0);
}

float3 VariableOverdriveLookup(float3 prev2, float3 prev1, float3 current, 
                              float prev2FrameTime, float prev1FrameTime, float currentFrameTime)
{
    // Normalize time values to prevent extreme numbers
    float timeScale = 0.001; // Convert to milliseconds
    float t0 = 0;
    float t1 = (prev1FrameTime - prev2FrameTime) * timeScale;
    float t2 = (currentFrameTime - prev2FrameTime) * timeScale;
    
    float3 coef = CalculateQuadraticCoefficients(prev2, prev1, current, t0, t1, t2);
    
    // Predict next value with a smaller time step
    float nextTime = t2 + (currentFrameTime * timeScale * 0.5); // Reduced prediction time
    float3 predicted = PredictNextValue(coef, nextTime);
    
    // Calculate overdrive with more conservative strength
    float refreshRate = 1000.0 / currentFrameTime;
    float strength = GetOverdriveStrength(refreshRate)
    
    // More conservative overdrive calculation
    float3 diff = predicted - current;
    float3 overdrive = current + diff * strength;
    
    // Ensure we're not exceeding valid color range
    return clamp(overdrive, 0.0, 1.0);
}

float4 PS_Overdrive(float4 vpos : SV_Position, float2 texcoord : TEXCOORD) : SV_Target
{
    float4 current = tex2D(BackBuffer, texcoord);
    float4 prev1 = tex2D(PrevFrameSampler, texcoord);
    float4 prev2 = tex2D(Prev2FrameSampler, texcoord);
    
    float prev2FrameTime = tex2D(Prev2FrameTimeSampler, float2(0, 0)).r;
    float prev1FrameTime = tex2D(PrevFrameTimeSampler, float2(0, 0)).r;
    float currentFrameTime = frame_time;
    
    // Add safety checks for frame times
    if (prev2FrameTime <= 0 || prev1FrameTime <= 0 || currentFrameTime <= 0)
    {
        return current;
    }
    
    // Check if the frame times make sense
    if (prev2FrameTime > prev1FrameTime || prev1FrameTime > currentFrameTime)
    {
        return current;
    }
    
    float3 overdriven = VariableOverdriveLookup(prev2.rgb, prev1.rgb, current.rgb,
                                               prev2FrameTime, prev1FrameTime, currentFrameTime);
    
    // Debug output - uncomment to see the effect strength
    // return float4(abs(finalColor - current.rgb) * 5.0, 1.0);
    
    return float4(overdriven, current.a);
}

float4 PS_StorePrevFrame(float4 vpos : SV_Position, float2 texcoord : TEXCOORD) : SV_Target
{
    return tex2D(BackBuffer, texcoord);
}

float4 PS_StorePrev2Frame(float4 vpos : SV_Position, float2 texcoord : TEXCOORD) : SV_Target
{
    return tex2D(PrevFrameSampler, texcoord);
}

float4 PS_StorePrevFrameTime(float4 vpos : SV_Position, float2 texcoord : TEXCOORD) : SV_Target
{
    return float4(frame_time, 0, 0, 0);
}

float4 PS_StorePrev2FrameTime(float4 vpos : SV_Position, float2 texcoord : TEXCOORD) : SV_Target
{
    return tex2D(PrevFrameTimeSampler, float2(0, 0));
}

technique VariablePanelOverdrive
{
    pass StorePrev2Frame
    {
        VertexShader = PostProcessVS;
        PixelShader = PS_StorePrev2Frame;
        RenderTarget = Prev2FrameTex;
    }
    
    pass StorePrevFrame
    {
        VertexShader = PostProcessVS;
        PixelShader = PS_StorePrevFrame;
        RenderTarget = PrevFrameTex;
    }
    
    pass StorePrev2FrameTime
    {
        VertexShader = PostProcessVS;
        PixelShader = PS_StorePrev2FrameTime;
        RenderTarget = Prev2FrameTimeTex;
    }
    
    pass StorePrevFrameTime
    {
        VertexShader = PostProcessVS;
        PixelShader = PS_StorePrevFrameTime;
        RenderTarget = PrevFrameTimeTex;
    }
    pass Overdrive
    {
        VertexShader = PostProcessVS;
        PixelShader = PS_Overdrive;
    }
}


