#include "Func.hlsl"

//  circle

#define  circleTexFormat float4
RWTexture2D<circleTexFormat> tex : register(u0);
 
struct Circle 
{
    uint2  size ;           //8
    float  maxIntensity ;   //4
    float  InnerEdge ;      //4
    float  outerEdge ;      //4
};
static const Circle crcl  =
{
 uint2( 280 , 280 ),
 0.2f,  0.93f,0.96f,
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
    float2 blick2 = float2(-0.31,-.6);
    
    float outerRing = ring(pos, 0.77,0.97);
    float innerRing = ring(pos, .1,0.9);
    float2 posCircleCover =float2(0.3,-0.05);
    float circleCover  = circle(float2(0.7,.8)*pos-posCircleCover,.95);
    outerRing +=1.4*smoothstep(0.1,.5,innerRing-2.5*circleCover);
    outerRing+=3.*circle(pos-blick1,0.1 );
    outerRing+=2.*circle(pos-blick2,0.2  );
    float3 bluishColor=float3(0.6123,0.624,.9324);
    float3 whiteColor=float3(1.0,1.0,1.0);
    float3  color = lerp(bluishColor,whiteColor,outerRing);
        
    tex[dispatchThreadId.xy] = float4(color,  outerRing);
 
}
// end circle

cbuffer ViewPortBuffer : register(b0){ float2 viewPortSize; };

//    t - time since application start 
//  t milisec from start (1.f = 1 milisec)
cbuffer FrameBuffer : register(b1){ float4 FrameTime; }; //   ( t, t / 1000.f, (t % (1000 * 60 * 10)) / 1000.f, t % 1000)

struct Instance
{   
    // no vertex data  (quadPos is used )
	//per instance data 
    float2 pos; // 8
    float  size; //  4
};

struct InstComputeData
{   
    float2 direction; // 8
    float2 pos; // 8
    float velosity; // 8
};

//Instance buffer view for compute shader
RWStructuredBuffer<Instance> ComputeInst  : register(u0);
RWStructuredBuffer<InstComputeData> ComputeData : register(u1);
//                                                                                                                          mainCompute
#define DRAWINSTANCECOUNT 32
[numthreads(DRAWINSTANCECOUNT, 1, 1)] void mainCompute(
      uint3 dispatchThreadId
    : SV_DispatchThreadID 
    )
{
   
    InstComputeData This =  ComputeData[dispatchThreadId.x] ;
     
    float2 pos =float2(0.0,0.0 ); 
    pos+=  (FrameTime.y%20.)/10.0-1.0;
    pos*= float2(1.8,0.6);//sqush along x axis to make rise less steep
    float2 modulator =  This.direction;
    modulator*=0.05* sin(FrameTime.y*dispatchThreadId.x*.1 );

    pos=mul(pos , float2x2(This.direction.x,-This.direction.y,This.direction.y,This.direction.x  ));
    pos +=modulator;
   
    ComputeInst[dispatchThreadId.x].pos= This.velosity*(pos- This.pos  )  ;
    ComputeInst[dispatchThreadId.x].size =0.2f;
    
};

//Instance buffer view for vertex shader
StructuredBuffer<Instance> VertexInstancies : register(t0);

struct VertexOut
{
    float4 pos : SV_Position;
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
//                                                                                                                          mainVertex
VertexOut mainVertex(uint VertID:SV_VertexID, uint InstID:SV_InstanceID)
{
    
    Instance In=VertexInstancies[InstID];
     
    VertexOut Out;
    Out.pos= float4(quadPos[VertID], 0.0f, 1.0f);
    Out.uv = (Out.pos.xy+1.f)/2.f;
    Out.pos.x/= viewPortSize.x/viewPortSize.y ; 
    Out.pos.xy*=In.size;
    Out.pos.xy +=In.pos;
    return  Out;
};

Texture2D <circleTexFormat>  CircleTex     :TEXTURE     :register(t0);
SamplerState       Sampler       :SAMPLER     :register(s0);
//                                                                                                                          mainPixel
float4 mainPixel(VertexOut In) : SV_Target
{
    float4 tex = CircleTex.Sample( Sampler, In.uv ) ;
    
    return    float4(  tex.rgb  , tex.a) ;  
    
};

 



  