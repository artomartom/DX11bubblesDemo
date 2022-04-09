 #include "Func.hlsl"
 

Texture2D    CircleTex   :TEXTURE    :register(t0);
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
     float3 colors[6] ;   
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
    float2 pos : POSITION;  // 8
	//instance
    float2 trans : TRANSLATION;  // 8
    float  size: SIZE;  //  4
    uint  color: COLOR;  //  4

};

 

struct VertexOut
{
    float4 pos : SV_Position;
    uint  color : COLOR0;
	float2 uv : TEXCOORD0;
}; 


VertexOut vmain(VertexIn Input)
{
    VertexOut VertexOutput;
    VertexOutput.uv =float2(Input.pos.x >0,Input.pos.y <0);
    VertexOutput.color=Input.color;
    Input.pos.x/= size.x/size.y ;
     
    float  scalar ;
    float  period= 1800.f;
    if((FrameTime.y%period*2) > period)
    scalar =1-(FrameTime.y %period )  /period ;
    else
      scalar =(FrameTime.y %period )  /period ;
      
    Input.pos*=Input.size  *scalar;

    Input.pos +=Input.trans ;
    VertexOutput.pos= float4(    Input.pos ,0.0f,1.0f);
    return VertexOutput;
};
 
float4   pmain(VertexOut Input) : SV_Target
{
       return    float4(colors[Input.color],CircleTex.Sample(Sampler,Input.uv ).x ) ;  
};

 



  