#include "Func.hlsl"

//  circle

#define  circleTexFormat float4
RWTexture2D<circleTexFormat> tex : register(u0);
 
struct Circle 
{
    uint2  size ;
    float  maxIntensity ;
    float  outerEdge ;
    float  InnerEdge ;
};
static const Circle crcl  =
{
 float2( 280.f, 280.f),
 0.2f,
 0.96f,
 0.93f,
};

//                                                                                                                          CircleCompute
[numthreads( crcl.size.x, 1, 1)] void CircleCompute( 
      uint3 dispatchThreadId
    : SV_DispatchThreadID     )
{
    if (dispatchThreadId.x >= crcl.size.x || dispatchThreadId.y >= crcl.size.y)
        return;
    float2 pos = float2(( float2(dispatchThreadId.xy) / crcl.size) * 2.f - 1.f );                        

    float2 blick1 = float2(0.2,.4);  
    float2 blick2 = float2(-0.61,-.4);
    float intens = ring(pos, 0.77,0.97);
    float innerring = ring(pos, .1,0.97);
    float outerring = ring(float2(1.7,.7)*pos-float2(0.05,-0.05), -1.,0.95);
    intens+=3.*smoothstep(0.78,1., innerring-outerring);
    intens+=8.*circle(pos-blick1,0.1 );
    intens+=2.*circle(pos-blick2,0.2  );
    float3  color = lerp(float3(0.6123,0.624,.9324), 1.0,.05)*intens;
        
    tex[dispatchThreadId.xy] = float4(color,  intens);
 
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
//                                                                                                                          mainCompute
[numthreads(1, 1, 1)] void mainCompute(
      uint3 dispatchThreadId
    : SV_DispatchThreadID     )
{
    //test
    ComputeInstancies[0].size = .2f ;
    ComputeInstancies[0].color = 5;
    ComputeInstancies[0].pos = float2(0.0f,0.0f);
   
    
};

//Instance buffer view for vertex shader
StructuredBuffer<InstData> VertexInstancies : register(t0);

struct VertexOut
{
    float4 pos : SV_Position;
    uint   color : COLOR0;
	float2 uv : TEXCOORD0;
}; 
//                                                                                                                          mainVertex
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

Texture2D <circleTexFormat>  CircleTex     :TEXTURE     :register(t0);
SamplerState       Sampler       :SAMPLER     :register(s0);
//                                                                                                                          mainPixel
float4 mainPixel(VertexOut In) : SV_Target
{
    float4 tex = CircleTex.Sample( Sampler, In.uv ) ;
    
    return    float4( lerp(colorBuffer[In.color].xyz,tex.rgb ,tex.a), tex.a) ;  
    
};

 



  