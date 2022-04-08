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
    uint  color: COLOR;  //  12

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

   
    float aspect= size.x/size.y   ;
    Input.pos.x/= aspect;
    
    Input.pos*=Input.size;// *(100.f/ (FrameTime.z%5+FrameTime.w) );

    Input.pos +=Input.trans ;
    VertexOutput.pos= float4(    Input.pos ,0.0f,1.0f);
    
    return VertexOutput;
};
 
float4   pmain(VertexOut Input) : SV_Target
{
  return   CircleTex.Sample(Sampler,Input.uv ) +float4(colors[Input.color],1.f);
  
};

 struct VSOut
{
	float3 color : Color;
	float4 pos : SV_Position;
};



 /////////////////
VSOut trianglevmain( float2 pos : Position,float3 color : Color )
{
	VSOut vso;
	vso.pos = float4(pos.x,pos.y,0.0f,1.0f);
	vso.color = color;
	return vso;
}
 
 
 
float4 trianglepmain( float3 color : Color ) : SV_Target
{
	return float4( color,1.0f );
}
 