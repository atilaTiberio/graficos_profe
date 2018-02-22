#pragma once
#include "CDXManager.h"
#include <vector>
using namespace std;
class CParticleDynamics
{
protected:
	CDXManager* m_pDX;
	ID3D11ComputeShader* m_pCSDynamics;
	ID3D11ComputeShader* m_pCSDraw;
	ID3D11Buffer* m_pCB;
	ID3D11Buffer* m_pParticles;
	ID3D11UnorderedAccessView* m_pUAVParticles;
public:
	struct PARTICLE
	{
		float px, py, pz, pw;
		float vx, vy, vz, vw;
		float ax, ay, az, aw;
	};
	struct PARAMS
	{
		float r, g, b, a;
		float t;
	}m_Params;
	vector<PARTICLE> m_System;
public:
	CParticleDynamics(CDXManager* pDX);
	bool Initialize(size_t uParicles);
	void Run();
	void Draw(ID3D11UnorderedAccessView* pTarget);
	void Uninitialize();
	~CParticleDynamics();
};

