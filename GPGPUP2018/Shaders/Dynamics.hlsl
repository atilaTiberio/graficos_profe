
cbuffer PARAMS:register(b0)
{
	float4 Color;
	float  t;
};
struct PARTICLE
{
	float4 Position;
	float4 Velocity;
	float4 Acceleration;
};
RWStructuredBuffer<PARTICLE> g_Particles:register(u0);
RWTexture2D<float4>          g_Target:register(u1);
//Procesamiento de cinemática de particulas
[numthreads(64,1,1)]
void DynamicsMain(uint3 id:SV_DispatchThreadID)
{
	float4 Velocity = g_Particles[id.x].Velocity + g_Particles[id.x].Acceleration * t;
	float4 Position = g_Particles[id.x].Position + Velocity * t;
	g_Particles[id.x].Velocity = Velocity;
	g_Particles[id.x].Position = Position;
}

//Trazo de las particulas en el render target asignado
[numthreads(64,1,1)]
void DrawMain(uint3 id:SV_DispatchThreadID)
{
	int2 OutputPosition = int2(g_Particles[id.x].Position.xy);
	g_Target[OutputPosition] = Color;
}