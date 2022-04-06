 #include "Func.hlsl"
/*
cbuffer FrameConstBuffer : register(b0)
{
    float4 FrameTime;
    // st / 20., st, st / 1000, st % 1000
    float2 Cursor;
};
*/

//Texture2D    CircleTex   :TEXTURE    :register(t0);
//SamplerState  Sampler  :SAMPLER  :register(s0);

struct VertexIn
{
    float2 uv: TEXCOORD;
 // float2 pos :POSITION;
   
};

 

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
   VertexOutput.Pos=float4(Input.uv,0.99f,0.99f);
   //  VertexOutput.Pos=float4(Input.pos+(Input.uv-0.5f),0.99f,0.99f);
    VertexOutput.uv=float2(0.99f,0.99f);
    return VertexOutput;
};
 
PixelOut  pmain(VertexOut Input)
{

    PixelOut output;
  //   output.Col = CircleTex.Sample(Sampler,Input.uv );
    output.Col = float4(0.99f,0.99f,0.99f,0.99f);

    return output;
};