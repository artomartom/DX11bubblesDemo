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

cbuffer ViewPortBuffer : register(b0){ float2 viewPortSize; float  resolution; };

//  t - time since application start 
//  t milisec from start (1.f = 1 sec)
//  deltaT - time since last frame  
cbuffer FrameBuffer : register(b1){ float4 FrameTime; }; //   (  t  , (t % (1000 * 60 * 10))  , deltaT,deltaT)

struct Instance
{   
    // no vertex data  (quadPos is used )
	//per instance data 
    float2 pos; // 8
};

struct ComputeData
{   
    float2 velocity; // 8
    float2 pos; // 8
};

float2 unit2(float angle)
{
    return float2(cos(angle),sin(angle));
}
 

//Instance buffer view for compute shader
RWStructuredBuffer<Instance> ComputeOut  : register(u0);
RWStructuredBuffer<ComputeData> ComputeIn : register(u1);
//                                                                                                                          mainCompute
#define DRAWINSTANCECOUNT 1
[numthreads(DRAWINSTANCECOUNT, 1, 1)] void mainCompute(
      uint3 dispatchThreadId
    : SV_DispatchThreadID 
    )
{
    ComputeData This =  ComputeIn[dispatchThreadId.x] ;
    float deltaT=FrameTime.z *0.7f ;
 
    This.pos+=This.velocity*deltaT;
    
    float radius=0.2;
    if(This.pos.x>(1.0-radius) )
    {
    ComputeIn[dispatchThreadId.x].velocity =reflect(This.velocity,float2(-1.0,0.0));
    }
    else if(This.pos.x<-(1.0-radius) )
    {
    ComputeIn[dispatchThreadId.x].velocity =reflect(This.velocity,float2(1.0,0.0));
     }
    else if(This.pos.y>(1.0/resolution-radius))
    { 
    ComputeIn[dispatchThreadId.x].velocity =reflect(This.velocity,float2( 0.0,-1.0));
    }
    else if(This.pos.y<-(1.0/resolution-radius))
    {
    ComputeIn[dispatchThreadId.x].velocity =reflect(This.velocity,float2(0.0,1.0));
    }
    


    ComputeIn[dispatchThreadId.x].pos =This.pos;
    ComputeOut[dispatchThreadId.x].pos=  This.pos ;
     
    
};

//Instance buffer view for vertex shader
StructuredBuffer<Instance> VertexIn : register(t0);

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
    
    Instance In=VertexIn[InstID];
     
    VertexOut Out;
    Out.pos= float4(quadPos[VertID], 0.0f, 1.0f);
    Out.uv = (Out.pos.xy+1.f)/2.f;
    Out.pos.xy*=0.2f; //scale texture down;
    Out.pos.xy +=In.pos;
    Out.pos.y*=resolution;
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

 



  