#define PI 3.14159265358979323846264338327950288419716939937510
float InverseLerp (float a,float b,float v   )
{
return (v-a)/(b-a);
};

//   
float smootherstep (float a0, float a1, float w) 
{
  return (a1 - a0) * ((w * (w * 6.0 - 15.0) + 10.0) * w * w * w) + a0;
}

   
float2 smootherstep (float2 a0, float2 a1, float2 w) 
{
  return (a1 - a0) * ((w * (w * 6.0 - 15.0) + 10.0) * w * w * w) + a0;
}
   
float3 smootherstep (float3 a0, float3 a1, float3 w) 
{
  return (a1 - a0) * ((w * (w * 6.0 - 15.0) + 10.0) * w * w * w) + a0;
}

float Psudorand (float a  )
{
    return 1;
};
 
float plot (float2 st, float pct){
  return  smoothstep( pct-0.01, pct, st.y) -
          smoothstep( pct, pct+0.01, st.y);
}

float plot (float2 st, float pct,float width ){
  return  smoothstep( pct-width, pct, st.y) -
          smoothstep( pct, pct+width, st.y);
}