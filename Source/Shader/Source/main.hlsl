#include "Func.hlsl"

//  circle
RWTexture2D<float> tex : register(u0);
 
#define circleSizex 280
#define circleSizey 280
#define maxIntensity 0.5f
#define maxDistance 1.41421
#define edge 0.96f
#define edgeWidth (1.f - edge)

[numthreads(circleSizex, 1, 1)] void CircleCompute(
      uint3 dispatchThreadId
    : SV_DispatchThreadID     )
{
    if (dispatchThreadId.x >= circleSizex || dispatchThreadId.y >= circleSizey)
        return;

    float2 Pos = float2((float(dispatchThreadId.x) / circleSizex) * 2.f - 1.f,
                        (float(dispatchThreadId.y) / circleSizey) * 2.f - 1.f);

    float distance = length(Pos);

    if (distance > edge + edgeWidth)
    {
        tex[dispatchThreadId.xy] = 0.f;
        return;
    }
    else
    {

        distance = (distance < edge) ? distance : (1.0f - (distance - edge) / edgeWidth);
        tex[dispatchThreadId.xy] = maxIntensity * distance;
        return;
    };
}
// end circle

cbuffer ViewPortBuffer : register(b0){ float2 viewPortSize; };

//    t - time since application start 
//  t milisec from start (1.f = 1 milisec)
cbuffer FrameBuffer : register(b1){ float4 FrameTime; }; //   ( t, t / 1000.f, (t % (1000 * 60 * 10)) / 1000.f, t % 1000)

struct InstData
{   
    // no vertex data  (quadPos is used )
	//per instance data 
    float2 pos; // 8
    float  size; //  4
    uint   color; //  4
};

static float2 quadPos[6] = 
{
    {-1.f, +1.f},
    {+1.f, +1.f},
    {+1.f, -1.f},
    {-1.f, +1.f},
    {+1.f, -1.f},
    {-1.f, -1.f},
};

static float4 colorBuffer[6] = 
{
{0xFB / 256.f, 0xB5 / 256.f, 0xE0 / 256.f, 1.f},
{0xB9 / 256.f, 0x80 / 256.f, 0xCE / 256.f, 1.f},
{0x49 / 256.f, 0x0A / 256.f, 0xBA / 256.f, 1.f},
{0x4B / 256.f, 0x37 / 256.f, 0x8E / 256.f, 1.f},
{0x2B / 256.f, 0x6D / 256.f, 0xE2 / 256.f, 1.f},
{0x1C / 256.f, 0x29 / 256.f, 0xB8 / 256.f, 1.f},
};

//Instance buffer view for compute shader
RWStructuredBuffer<InstData> ComputeInstancies : register(u0);

[numthreads(1, 1, 1)] void mainCompute(
      uint3 dispatchThreadId
    : SV_DispatchThreadID     )
{
    //test
    ComputeInstancies[0].size=  0.1f ;
    ComputeInstancies[0].color=5;
    ComputeInstancies[0].pos =float2(0.0f,0.0f);
   
    
};

//Instance buffer view for vertex shader
StructuredBuffer<InstData> VertexInstancies : register(t0);

struct VertexOut
{
    float4 pos : SV_Position;
    uint   color : COLOR0;
	float2 uv : TEXCOORD0;
}; 

VertexOut mainVertex(uint VertID:SV_VertexID, uint InstID:SV_InstanceID)
{
    
    InstData In=VertexInstancies[InstID];
     
    VertexOut Out;
    Out.pos= float4(quadPos[VertID], 0.0f, 1.0f);
    Out.uv = (Out.pos.xy+1.f)/2.f;
    Out.pos.x/= viewPortSize.x/viewPortSize.y ; 
    Out.pos.xy*=In.size;
    Out.pos.xy +=In.pos;
    Out.color=In.color;
    return  Out;
};

Texture2D <float>  CircleTex     :TEXTURE     :register(t0);
SamplerState       Sampler       :SAMPLER     :register(s0);
 
float4 mainPixel(VertexOut In) : SV_Target
{
    return    float4( colorBuffer[In.color].xyz,CircleTex.Sample( Sampler, In.uv )  ) ;  
    
};

 



  