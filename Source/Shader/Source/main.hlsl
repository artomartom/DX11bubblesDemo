 #include "Func.hlsl"
 

 

struct VertexIn
{   
    float2 uv : TEXCOORD;
    float2 pos : POSITION;

};

 

struct VertexOut
{
    float4 Pos : SV_Position;
    float4 Col : COLOR0;
};

struct PixelOut
{     
    float4 Col : SV_TARGET;
};


VertexOut vmain(VertexIn Input)
{
    VertexOut VertexOutput;
   VertexOutput.Pos=float4(Input.uv+Input.pos,0.0f,1.0f);
     

    VertexOutput.Col=float4(0.99f,0.99f,0.99f,0.99f);
    return VertexOutput;
};
 
PixelOut  pmain(VertexOut Input)
{

    PixelOut output;
  //   output.Col = CircleTex.Sample(Sampler,Input.uv );
    output.Col = float4(0.99f,0.99f,0.99f,1.f);

    return output;
};

 struct VSOut
{
	float3 color : Color;
	float4 pos : SV_Position;
};

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
 