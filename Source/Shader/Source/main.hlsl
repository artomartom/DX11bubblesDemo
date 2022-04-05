 #include "Func.hlsl"

cbuffer FrameConstBuffer : register(b0)
{
    float4 FrameTime;
    // st / 20., st, st / 1000, st % 1000
    float2 Cursor;
};

struct VertexIn
{
    float2 pos : POSITION;
    float2 uv : TEXCOORD;
};
Texture2D    Scene   :TEXTURE    :register(t0);
SamplerState  Sam  :SAMPLER  :register(s0);

struct VertexOut
{
    float4 Pos : SV_Position;
    float2 uv : TEXCOORD;
};

struct PixelOut
{     
    float4 Col : SV_TARGET;
};


VertexOut vmain(VertexIn Input)
{

    VertexOut VertexOutput;
   
   VertexOutput.Pos=float4(Input.pos.x,Input.pos.y,0.99f,0.99f);
     VertexOutput.uv = Input.uv;
//float4(0.99f,0.99f,0.99f,0.99f);

    return VertexOutput;
};
 
PixelOut  pmain(VertexOut Input)
{

    PixelOut output;
    output.Col = Scene.Sample(Sam,Input.uv );

    return output;
};