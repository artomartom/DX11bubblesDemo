#include "Func.hlsl"

//  circle

#define  circleTexFormat float4
RWTexture2D<circleTexFormat> tex : register(u0);
 
static const uint2  circleSize =uint2( 280, 280);
static const float  maxIntensity= 0.2f;
static const float  maxDistance= 1.41421;
static const float  edge= 0.96f;
static const float  outerEdgeWidth= 1.0f - edge;
static const float  InnerEdgeWidth= edge - 0.83f;
static const float3  ColorA= float3(0.941176534f, 1.000000000f, 1.000000000f );
static const float3  ColorB= float3(0.545098066f, 0.000000000f, 0.000000000f );
static const float3  ColorC= float3(0.862745166f, 0.862745166f, 0.862745166f );

/*

float circle(vec2 pos ,float radius   )
{
    float distance=length(pos);
    return 1.0 - smoothstep(0.0, radius, distance);
}
  
float ring(vec2 pos ,float InnerEdge ,float OuterEdge )
{
float edge= mix(InnerEdge,OuterEdge,0.5);
float distance = length(pos);
return  smoothstep(InnerEdge, edge,distance )-smoothstep( edge, OuterEdge ,distance);
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 uv = fragCoord/iResolution.xy;
    vec2 pos =(uv*2.0-1.0 )*vec2(iResolution.x/iResolution.y,1.0) ;
    
    //_____
    vec2 blick1 = vec2(0.2,.4);  
    vec2 blick2 = vec2(-0.61,-.4);
  
   
    float intens = ring(pos, 0.77,0.97);
    float innerring = ring(pos, .1,0.97);
    float outerring = ring(vec2(1.7,.7)*pos-vec2(0.05,-0.05), -1.,0.95);
    intens+=smoothstep(0.78,1., innerring-outerring);
    intens+=0.7*circle(pos-blick1,0.1 );
    intens+=0.7*circle(pos-blick2,0.2  );
    fragColor = vec4(vec3(intens), 1.0);
     
       
} 
*/


//                                                                                                                          CircleCompute
[numthreads(circleSize.x, 1, 1)] void CircleCompute( 
      uint3 dispatchThreadId
    : SV_DispatchThreadID     )
{
    if (dispatchThreadId.x >= circleSize.x || dispatchThreadId.y >= circleSize.y)
        return;

    float2 pos = float2((float(dispatchThreadId.x) / circleSize.x) * 2.f - 1.f,
                        (float(dispatchThreadId.y) / circleSize.y) * 2.f - 1.f);

    float distance = length(pos);

    if (distance > edge + outerEdgeWidth)
    {
        tex[dispatchThreadId.xy] = float4(0.f,0.f,0.f,0.f);
    }
    else
    {

        //  pct = pow(distance(st,vec2(0.460,0.450)),distance(st,vec2(0.290,0.650)));  
        //float f = abs(sin(a*234.5))*0.55+.3;

        //float intens =  smoothstep( edge - InnerEdgeWidth , edge  ,distance )-smoothstep( edge , edge + outerEdgeWidth ,distance );
        float2 dir= normalize(  pos);
        float intens =  smoothstep( edge - InnerEdgeWidth, edge, distance )-smoothstep( edge , edge + outerEdgeWidth ,distance );
        intens -=(  distance* distance* distance* distance);
      //  float3 col = ColorB* clamp(  -dir.y , dir.x  ,ColorA);
      //  float blick =dot(pos,float2(-cos(0.785398163f),sin(0.785398163f)  ));
        //col+=(lerp(0.9f,1.f, blick.x)-0.9f).x;  

        tex[dispatchThreadId.xy] = float4(lerp(ColorB,ColorA,0.5f), maxIntensity*intens);
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
//                                                                                                                          mainCompute
[numthreads(1, 1, 1)] void mainCompute(
      uint3 dispatchThreadId
    : SV_DispatchThreadID     )
{
    //test
    ComputeInstancies[0].size = 1.0f ;
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

 



  