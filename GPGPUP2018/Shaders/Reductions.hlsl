

groupshared float Sums[64]; //no hacer asumption de inicializcion
static const int BlockSize = 128; //es volatil, no se conserva entre dispatchers
RWStructuredBuffer<float> Vector;
[numthreads(64,1,1)]
void VectorReductionMain(uint id:SV_DispatchThreadID, uint3 lid : SV_GroupThreadID, uint3 grpid: SV_GroupID)
{

	Sums[lid.x] = Vector[grpid.x * 128 + lid.x] + Vector[grpid.x * 128 + 64 + lid.x];
	GroupMemoryBarrierWithGroupSync();

	if (lid.x < 32) {
		Sums[lid.x] += Sums[lid.x + 32];
	}
	GroupMemoryBarrierWithGroupSync();
	if (lid.x < 16) {
		Sums[lid.x] += Sums[lid.x + 16];
	}
	GroupMemoryBarrierWithGroupSync();
	if (lid.x < 8) {
		Sums[lid.x] += Sums[lid.x + 8];
	}
	GroupMemoryBarrierWithGroupSync();
	if (lid.x < 4) {
		Sums[lid.x] += Sums[lid.x + 4];
	}
	GroupMemoryBarrierWithGroupSync();

	if (lid.x < 2) {
		Sums[lid.x] += Sums[lid.x + 2];
	}
	GroupMemoryBarrierWithGroupSync();
	if (lid.x < 1) {
		Sums[lid.x] += Sums[lid.x + 1];
		Vector[grpid.x * 128] = Sums[0];
	}

}