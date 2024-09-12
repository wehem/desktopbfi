#include "ReShade.fxh"

uniform float FRAME_RATE <
    ui_type = "slider";
    ui_min = 30.0; ui_max = 240.0; ui_step = 1.0;
    ui_label = "Frame Rate";
    ui_tooltip = "The frame rate of the alternation effect";
> = 100.0;

uniform float CYCLE_LENGTH <
    ui_type = "slider";
    ui_min = 2.0; ui_max = 8.0; ui_step = 1.0;
    ui_label = "Cycle Length";
    ui_tooltip = "The number of frames in each alternation cycle";
> = 4.0;

uniform float2 CHECKERBOARD_SIZE <
    ui_type = "slider";
    ui_min = 1.0; ui_max = 100.0; ui_step = 1.0;
    ui_label = "Checkerboard Size";
    ui_tooltip = "Size of the checkerboard squares (width, height)";
> = float2(2.0, 2.0);

uniform float2 CHECKERBOARD_OFFSET <
    ui_type = "slider";
    ui_min = 0.0; ui_max = 100.0; ui_step = 1.0;
    ui_label = "Checkerboard Offset";
    ui_tooltip = "Offset of the checkerboard pattern (x, y)";
> = float2(0.0, 0.0);

uniform float Timer < source = "timer"; >;

texture BackBufferTex : COLOR;
sampler BackBuffer { Texture = BackBufferTex; };

float4 PS_Main(float4 vpos : SV_Position, float2 texcoord : TEXCOORD) : SV_Target
{
    // Calculate current frame in the cycle
    float time = Timer * 0.001 * FRAME_RATE; // Convert milliseconds to seconds
    float currentFrame = floor(time) % CYCLE_LENGTH;

    // Calculate checkerboard pattern with size and offset
    float2 checkerCoord = floor((texcoord * ReShade::ScreenSize + CHECKERBOARD_OFFSET) / CHECKERBOARD_SIZE);
    float checker = (checkerCoord.x + checkerCoord.y) % 2.0;

    // Determine which quadrant we're in
    float2 quadrantCoord = floor((texcoord * ReShade::ScreenSize + CHECKERBOARD_OFFSET) / CHECKERBOARD_SIZE);
    float quadrant = (quadrantCoord.x + quadrantCoord.y * 2.0) % CYCLE_LENGTH;

    // Determine if this pixel should be on or off
    bool isOn = (quadrant == currentFrame);

    // Sample the input texture
    float4 texColor = tex2D(BackBuffer, texcoord);

    // Output color
    if (isOn)
    {
        return texColor;
    }
    else
    {
        return float4(0.0, 0.0, 0.0, 1.0); // Black for off pixels
    }
}

technique QuadruntFlash
{
    pass
    {
        VertexShader = PostProcessVS;
        PixelShader = PS_Main;
    }
}