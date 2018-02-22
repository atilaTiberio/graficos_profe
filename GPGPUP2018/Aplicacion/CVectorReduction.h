#pragma once
#include<vector>
#include "CDXManager.h"
using namespace std;

class CVectorReduction
{
	CDXManager *m_pDXM;
	ID3D11ComputeShader *m_pCS;
	ID3D11Buffer *m_pCB;
	ID3D11Buffer *m_pVector;
	ID3D11UnorderedAccessView * m_pUAVVector;

public:
	vector<float> m_Vector;
	bool Initialize(size_t n_Elements);
	void Uninitialize();
	float Run();
	CVectorReduction(CDXManager *pOwner);
	~CVectorReduction();
};

