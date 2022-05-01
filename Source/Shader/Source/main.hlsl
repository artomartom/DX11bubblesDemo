#include "Func.hlsl"

//  compute
RWTexture2D<float> tex : register(u0);
 
#define circleSizex 280
#define circleSizey 280
#define maxIntensity 0.5f
#define maxDistance 1.41421
#define edge 0.96f
#define edgeWidth (1.f - edge)

[numthreads(circleSizex, 1, 1)] void mainCircle(
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
// end compute



cbuffer ViewPortBuffer : register(b0){    float2 size ;   };

//   st - time since application start   st / 20., st, st / 1000, st % 1000 // st milisec from start (1.f = 1 milisec)
cbuffer FrameBuffer : register(b1){    float4 FrameTime;};   

struct VertexIn
{   
    // no vertex data  (quadPos is used )
	//per instance data 
    float2 trans: TRANSLATION;  // 8
    float  size: SIZE;  //  4
    float  period: PERIOD;  //  4
    uint   color: COLOR;  //  4
    float startTime: STARTTIME;  //  4
};

 

struct VertexOut
{
    float4 pos : SV_Position;
    uint  color : COLOR0;
	float2 uv : TEXCOORD0;
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

VertexOut mainVertex(VertexIn In,   uint VertID : SV_VertexID)
{

    float2 pos = quadPos[VertID];
    VertexOut VertexOut;
     
    VertexOut.uv = (pos+1.f )/2.f;
    VertexOut.color=In.color;
    pos.x/= size.x/size.y ; 
     
    float  scalar= 0;
    float FromStart=FrameTime.y-In.startTime;
     
    if(FromStart < In.period )  // if the period is not over 
    {
     
        if(FromStart > In.period/2.f) // if it is second half of period
        scalar =1- FromStart /In.period ;  //scale the circle down
        else
        scalar =FromStart /In.period ;  // scale the circle up
    }; 
        
    pos*=In.size   *scalar;
    pos +=In.trans ;
    VertexOut.pos= float4(    pos ,0.0f,1.0f);
    return VertexOut;
};

Texture2D <float>     CircleTex   :TEXTURE    :register(t0);
SamplerState  Sampler  :SAMPLER  :register(s0);
 
float4   mainPixel(VertexOut In) : SV_Target
{
    return    float4( colorBuffer[In.color].xyz,CircleTex.Sample( Sampler, In.uv )  ) ;  
    
};

 



  