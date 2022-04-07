 #include "Func.hlsl"
 

Texture2D    CircleTex   :TEXTURE    :register(t0);
SamplerState  Sampler  :SAMPLER  :register(s0);

struct VertexIn

{   
	//vertex
    float2 pos : POSITION;
	//instance
    float2 trans : TRANSLATION;
    float  size: SIZE;
    float3  color: COLOR;

};

 

struct VertexOut
{
    float4 pos : SV_Position;
    float4 color : COLOR0;
	float2 uv : TEXCOORD0;
};

struct PixelOut
{     
    float4 color : SV_TARGET;
};


VertexOut vmain(VertexIn Input)
{
    VertexOut VertexOutput;
	
    VertexOutput.pos= float4( (Input.pos*Input.size)  +Input.trans,0.0f,1.0f);
    
    VertexOutput.uv =float2(Input.pos.x >0,Input.pos.y <0);
   
    VertexOutput.color=float4(Input.color ,0.99f );


     
    return VertexOutput;
};
 
PixelOut  pmain(VertexOut Input)
{

    PixelOut output;
    float4 e =  CircleTex.Sample(Sampler,Input.uv ) *Input.color;
	output.color =e;
	output.color.w=0.0f*output.color.x ;
	return output;
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
 