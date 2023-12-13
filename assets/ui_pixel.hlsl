Texture2D g_Texture;
SamplerState g_Texture_sampler; // By convention, texture samplers must use the '_sampler' suffix

struct PSInput 
{ 
    float4 Pos   : SV_POSITION;
    float4 Color : COLOR0;
    float2 UV : TEX_COORD;
};

struct PSOutput
{ 
    float4 Color : SV_TARGET; 
};

void main(in  PSInput  PSIn,
          out PSOutput PSOut)
{
    
    // for font only 
    float alpha = g_Texture.Sample(g_Texture_sampler, PSIn.UV).r;
    clip(alpha > 0.2 ? 1.0 : -1.0);
    PSOut.Color = PSIn.Color;
            //PSIn.Color * alpha;
    //PSOut.Color.a = alpha;
    //PSOut.Color.rgb *= g_Texture.Sample(g_Texture_sampler, PSIn.UV).r;
    //PSOut.Color.a = g_Texture.Sample(g_Texture_sampler, PSIn.UV).r;

        }
