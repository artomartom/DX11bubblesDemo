#include "Func.hlsl"

//Texture2DMS<float>     CircleTex   :TEXTURE    :register(t0);
Texture2D      CircleTex   :TEXTURE    :register(t0);
SamplerState  Sampler  :SAMPLER  :register(s0);

cbuffer ViewPortBuffer : register(b0)
{
    float2 size ;   
};

cbuffer FrameBuffer : register(b1)
{
    float4 FrameTime;
    // st / 20., st, st / 1000, st % 1000 // st milisec from start (1.f = 1 milisec)
};


cbuffer ColorsBuffer : register(b2)
{
    float4 colors[6] ;   //6*(4*4)
};

struct VertexIn
{   
	/*vertex
          {{-1.f, +1.f}}
          {{+1.f, +1.f}}
          {{+1.f, -1.f}}
          {{-1.f, +1.f}}
          {{+1.f, -1.f}}
          {{-1.f, -1.f}}
      */
    float2 pos: POSITION;  // 8
	//instance
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


VertexOut vmain(VertexIn In)
{
    VertexOut VertexOut;
    VertexOut.uv =float2(In.pos.x >0,In.pos.y <0);
    VertexOut.color=In.color;
    In.pos.x/= size.x/size.y ;
     
    float  scalar= 0;
    float FromStart=FrameTime.y-In.startTime;
     
    if(FromStart < In.period )  // if the period is not over 
    {
     
        if(FromStart > In.period/2.f) // if it is second half of period
        scalar =1- FromStart /In.period ;  //scale the circle down
        else
        scalar =FromStart /In.period ;  // scale the circle up
    }; 
        
    In.pos*=In.size   *scalar;
    In.pos +=In.trans ;
    VertexOut.pos= float4(    In.pos ,0.0f,1.0f);
    return VertexOut;
};
 
float4   pmain(VertexOut In) : SV_Target
{
    
    //return    float4( colors[In.color].xyz,CircleTex.Load(size.x * In.uv.x,size.y* In.uv.y ).x ) ;  
    return    float4( colors[In.color].xyz,CircleTex.Sample( Sampler, In.uv ).x ) ;  
};

 



  