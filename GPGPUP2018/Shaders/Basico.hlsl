
/* Un sombreador básico con salida gráfica sobre una textura 2D

	Este ejemplo tiene como objetivo presentar la estructura basica de un HLSL para DirectCompute

*/

//Recursos Globales (Los recursos globales son accesibles por todos los grupos de hilos)
RWTexture2D<float4> Output;  //Recurso de salida global de tipo Textura2D
cbuffer PARAMS
{
	float4 Color;
};


//Recursos Locales (información que compartirán los hilos de éste grupo)
//<declarar aquí recursos locales>


//Estructura del grupo de hilos, un grupo de hilos comparte información entre hilos, no entre grupos.
[numthreads(32,16,1)]        //Un grupo de hilos que consiste en 16x16x1 = 256 hilos
void main(uint3 id:SV_DispatchThreadID,uint3 idlocal:SV_GroupThreadID) //Cada hilo recibe su IDentificador de hilo
{
	//r^2=x^2 + y^2
	//r^2<(x-cx)^2+(y-cy)^2
	int x = id.x;
	int y = id.y;
	int r = 100;
	x -= 320;
	y -= 240;
	if (r*r >= (x*x) + (y*y))
		Output[id.xy] = Color;
	else
		Output[id.xy] = float4(0, 0, 0, 1);
}