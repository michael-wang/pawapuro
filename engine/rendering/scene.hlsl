cbuffer Camera : register(b0)
{
    row_major float4x4 view_projection;
    float4 translation;
};

struct VertexOutput
{
    float4 position : SV_POSITION;
    float3 color : COLOR;
};

VertexOutput VSMain(float3 position : POSITION, float3 color : COLOR)
{
    VertexOutput output;
    output.position = mul(float4(position + translation.xyz, 1.0), view_projection);
    output.color = color;
    return output;
}

float4 PSMain(VertexOutput input) : SV_TARGET
{
    return float4(input.color, 1.0);
}
