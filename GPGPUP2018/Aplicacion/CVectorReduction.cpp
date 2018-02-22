#include "stdafx.h"
#include "CVectorReduction.h"


bool CVectorReduction::Initialize(size_t n_Elements)
{
	//m_pVector.resize(nElements);

	m_Vector.resize(n_Elements);
	
	D3D11_BUFFER_DESC dbd;
	memset(&dbd, 0, sizeof(dbd));
	dbd.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
	dbd.ByteWidth = n_Elements * sizeof(float);
	dbd.StructureByteStride = sizeof(float);
	dbd.Usage = D3D11_USAGE_DEFAULT;
	dbd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	m_pDXM->GetDevice()->CreateBuffer(&dbd, NULL, &m_pVector);
	m_pCS = m_pDXM->CompileCS(L"..\\Shaders\\Reductions.hlsl", "VectorReductionMain");

	if (!m_pCS) {
		m_pVector->Release();
		m_pVector = nullptr;
		return false;
	}
	m_pDXM->GetDevice()->CreateUnorderedAccessView(m_pVector, NULL, &m_pUAVVector);

	return true;
}

void CVectorReduction::Uninitialize()
{
}

float CVectorReduction::Run()
{

	m_pDXM->Upload(&m_Vector[0], m_Vector.size() * sizeof(float),m_pVector);
	m_pDXM->GetContext()->CSSetShader(m_pCS, 0, 0);
	m_pDXM->GetContext()->CSSetUnorderedAccessViews(0, 1, &m_pUAVVector,0);
	m_pDXM->GetContext()->Dispatch((m_Vector.size() + 127) / 128, 1, 1);
	m_pDXM->Download(&m_Vector[0], m_Vector.size() * sizeof(float), m_pVector);

	float x = 0.0f;
	
	for (int i= 0; i < m_Vector.size(); i += 128) {
		x += m_Vector[i];
	}

	return x;
}

CVectorReduction::CVectorReduction(CDXManager * pOwner)
{
	m_pDXM = pOwner;
}

CVectorReduction::~CVectorReduction(){
}
