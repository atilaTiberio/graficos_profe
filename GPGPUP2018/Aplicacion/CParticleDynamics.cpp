#include "stdafx.h"
#include "CParticleDynamics.h"

CParticleDynamics::CParticleDynamics(CDXManager* pManager)
{
	m_pDX = pManager;
}
bool CParticleDynamics::Initialize(size_t uParticles)
{
	m_System.resize(uParticles);
	m_Params.t = 0.01f;
	m_Params.r = 0.3f;
	m_Params.g = 0.7f;
	m_Params.b = 0.7f;
	m_Params.a = 1.0f;
	int i = 0;
	for (auto& p : m_System)
	{
		memset(&p, 0, sizeof(p));
		p.px = 200.0f;
		p.py = 200.0f;
		p.pz = 0.0f;
		p.pw = 1.0f;
		float rspeed = (100.0f*rand() / 32768.0f)+10;
		p.vx = rspeed * cos(2*i*(3.141592f / uParticles));
		p.vy = rspeed * sin(2*i*(3.141592f / uParticles));
		i++;
	}
	m_pCSDynamics = m_pDX->CompileCS(
		L"..\\Shaders\\Dynamics.hlsl", "DynamicsMain");
	if (!m_pCSDynamics)
		return false;
	m_pCSDraw = m_pDX->CompileCS(
		L"..\\Shaders\\Dynamics.hlsl", "DrawMain");
	if (!m_pCSDraw)
	{
		m_pCSDynamics->Release();
		m_pCSDynamics = nullptr;
		return false;
	}
	D3D11_BUFFER_DESC dbd;
	memset(&dbd, 0, sizeof(dbd));
	dbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	dbd.ByteWidth = 16 * ((sizeof(PARAMS) + 15) / 16);
	dbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	dbd.Usage = D3D11_USAGE_DYNAMIC;
	m_pDX->GetDevice()->CreateBuffer(&dbd, NULL, &m_pCB);

	//Creación de un buffer estructurado
	memset(&dbd, 0, sizeof(dbd));
	dbd.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
	dbd.ByteWidth = sizeof(PARTICLE)*m_System.size();
	dbd.StructureByteStride = sizeof(PARTICLE);
	dbd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	dbd.Usage = D3D11_USAGE_DEFAULT;
	m_pDX->GetDevice()->CreateBuffer(&dbd, 0, &m_pParticles);
	if (!m_pParticles)
	{
		m_pCB->Release();
		m_pCB = nullptr;
		m_pCSDraw->Release();
		m_pCSDraw = nullptr;
		m_pCSDynamics->Release();
		m_pCSDynamics = nullptr;
		return false;
	}
	m_pDX->GetDevice()->CreateUnorderedAccessView(m_pParticles, NULL, &m_pUAVParticles);
	m_pDX->Upload(&m_System[0], m_System.size() * sizeof(PARTICLE), m_pParticles);
	return true;

}
void CParticleDynamics::Run()
{
	D3D11_MAPPED_SUBRESOURCE ms;
	m_pDX->GetContext()->Map(m_pCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
	memcpy(ms.pData, &m_Params, sizeof(PARAMS));
	m_pDX->GetContext()->Unmap(m_pCB, 0);
	m_pDX->GetContext()->CSSetConstantBuffers(0, 1, &m_pCB);
	m_pDX->GetContext()->CSSetShader(m_pCSDynamics, 0, 0);
	m_pDX->GetContext()->CSSetUnorderedAccessViews(0, 1, &m_pUAVParticles, 0);
	m_pDX->GetContext()->Dispatch((m_System.size() + 63) / 64, 1, 1);
}
void CParticleDynamics::Draw(ID3D11UnorderedAccessView* pUAVTarget)
{
	D3D11_MAPPED_SUBRESOURCE ms;
	m_pDX->GetContext()->Map(m_pCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
	memcpy(ms.pData, &m_Params, sizeof(PARAMS));
	m_pDX->GetContext()->Unmap(m_pCB, 0);
	m_pDX->GetContext()->CSSetConstantBuffers(0, 1, &m_pCB);
	m_pDX->GetContext()->CSSetShader(m_pCSDraw, 0, 0);
	m_pDX->GetContext()->CSSetUnorderedAccessViews(0, 1, &m_pUAVParticles, 0);
	m_pDX->GetContext()->CSSetUnorderedAccessViews(1, 1, &pUAVTarget,0);
	m_pDX->GetContext()->Dispatch((m_System.size() + 63) / 64, 1, 1);
}

void CParticleDynamics::Uninitialize()
{
	if(m_pCB) m_pCB->Release();
	m_pCB = nullptr;
	if(m_pCSDynamics) m_pCSDynamics->Release();
	m_pCSDynamics = nullptr;
	if(m_pCSDraw) m_pCSDraw->Release();
	m_pCSDraw = nullptr;
	if (m_pUAVParticles) m_pUAVParticles->Release();
	m_pUAVParticles = nullptr;
	if (m_pParticles) m_pParticles->Release();
	m_pParticles = nullptr;
}

CParticleDynamics::~CParticleDynamics()
{
}
