#include "stdafx.h"
#include "CDXManager.h"


IDXGIAdapter * CDXManager::EnumAndChooseAdapter(HWND hWnd)
{
	IDXGIFactory* pDXGIFactory = nullptr;
	CreateDXGIFactory(IID_IDXGIFactory, (void**)&pDXGIFactory);
	IDXGIAdapter* pDXGIAdapter = nullptr;
	unsigned long iAdapter = 0;
	while (1)
	{
		DXGI_ADAPTER_DESC dad;
		pDXGIFactory->EnumAdapters(iAdapter, &pDXGIAdapter);
		if (!pDXGIAdapter) break;
		pDXGIAdapter->GetDesc(&dad);
		wchar_t szMessage[1024];
		wsprintf(szMessage, L"Descripción:%s\r\nMemoria Dedicada:%d MB",
			dad.Description, dad.DedicatedVideoMemory / (1024 * 1024));
		switch (MessageBox(hWnd, szMessage, L"Selección de dispositivo",
			MB_ICONQUESTION | MB_YESNOCANCEL))
		{
		case IDYES:
			pDXGIFactory->Release();
			return pDXGIAdapter;
		case IDNO:
			pDXGIAdapter->Release();
			iAdapter++;
			break;
		case IDCANCEL:
			pDXGIAdapter->Release();
			pDXGIFactory->Release();
			return nullptr;
		}
	}
	return nullptr;
}

CDXManager::CDXManager()
{
}


CDXManager::~CDXManager()
{
}


bool CDXManager::Initialize(HWND hWnd, IDXGIAdapter* pAdapter, int sx, int sy, bool b_UseSoftware)
{
	DXGI_SWAP_CHAIN_DESC dscd;
	memset(&dscd, 0, sizeof(dscd));
	dscd.BufferCount = 2;
	dscd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dscd.BufferDesc.Height = sy;
	dscd.BufferDesc.Width = sx;
	dscd.BufferDesc.RefreshRate.Numerator = 0;
	dscd.BufferDesc.RefreshRate.Denominator = 0;
	dscd.BufferDesc.Scaling = DXGI_MODE_SCALING_STRETCHED;
	dscd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
	dscd.OutputWindow = hWnd;
	dscd.BufferUsage =
		DXGI_USAGE_RENDER_TARGET_OUTPUT |
		DXGI_USAGE_UNORDERED_ACCESS;
	dscd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	dscd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	dscd.SampleDesc.Count = 1;
	dscd.SampleDesc.Quality = 0;
	dscd.Windowed = true;
	D3D_FEATURE_LEVEL Requested=D3D_FEATURE_LEVEL_11_0, Serviced;
	HRESULT hr;
	if (!b_UseSoftware) {
		hr = D3D11CreateDeviceAndSwapChain(pAdapter,
			D3D_DRIVER_TYPE_UNKNOWN, NULL, 0, &Requested, 1,
			D3D11_SDK_VERSION, &dscd,
			&m_pSwapChain, &m_pDevice, &Serviced, &m_pContext);
	}
	else {
		hr = D3D11CreateDeviceAndSwapChain(NULL,
			D3D_DRIVER_TYPE_WARP, NULL, 0, &Requested, 1,
			D3D11_SDK_VERSION, &dscd,
			&m_pSwapChain, &m_pDevice, &Serviced, &m_pContext);
	}
	return SUCCEEDED(hr);
}
#include <d3dcompiler.h>
ID3D11ComputeShader* CDXManager::CompileCS(const wchar_t * pwszFileName, const char * pszEntryPoint)
{
	//D3DBlob (D3D Binary Large Object = Buffer, Arreglo, Bloque de memoria)
	ID3DBlob* pDXIL=nullptr; //Lenguaje Intermedio de DirectX (resultado de la compilación)
	ID3DBlob* pErrors = nullptr; //Errores y advertencias resultado de la compilación
#ifdef _DEBUG
	//Banderas de compilación depurable
	int Flags = D3DCOMPILE_DEBUG | D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	//Banderas de compilación optimizada (no depurable)
	int Flags = D3DCOMPILE_OPTIMIZATION_LEVEL3 | D3DCOMPILE_ENABLE_STRICTNESS;
#endif
	//Compilar el archivo de HLSL a DXIL
	HRESULT hr = D3DCompileFromFile(pwszFileName, 
		NULL, //Sin definiciones previas 
		D3D_COMPILE_STANDARD_FILE_INCLUDE, //Sistema de pila de inclusión de archivos estándar
		pszEntryPoint,  //
		"cs_5_0",   //Shader Model de Compute Shader 5.0 
		Flags, 0, 
		&pDXIL, &pErrors);
	if (pErrors)
	{
		MessageBoxA(NULL, (char*)pErrors->GetBufferPointer(), "Errores y avisos generados", MB_ICONEXCLAMATION);
		pErrors->Release();
	}
	if (pDXIL)
	{
		//Compilar de DXIL a Nativo para ese GPU/CPU
		ID3D11ComputeShader* pCS = nullptr;
		m_pDevice->CreateComputeShader(pDXIL->GetBufferPointer(), pDXIL->GetBufferSize(), NULL, &pCS);
		pDXIL->Release();
		return pCS;
	}
	return nullptr;
}

void CDXManager::Upload(void* pData, size_t uSize, ID3D11Buffer* pDest)
{
	ID3D11Buffer* pStaging=nullptr;
	D3D11_BUFFER_DESC dbd;
	pDest->GetDesc(&dbd);
	dbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	dbd.Usage = D3D11_USAGE_STAGING;
	dbd.BindFlags = 0;
	m_pDevice->CreateBuffer(&dbd, NULL, &pStaging);
	D3D11_MAPPED_SUBRESOURCE dms;
	m_pContext->Map(pStaging, 0, D3D11_MAP_WRITE, 0, &dms);
	memcpy(dms.pData, pData, uSize);
	m_pContext->Unmap(pStaging, 0);
	m_pContext->CopyResource(pDest, pStaging);
	pStaging->Release();
}
void CDXManager::Download(void* pData, size_t uSize,
	ID3D11Buffer* pSource)
{
	ID3D11Buffer* pStaging = nullptr;
	D3D11_BUFFER_DESC dbd;
	pSource->GetDesc(&dbd);
	dbd.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	dbd.Usage = D3D11_USAGE_STAGING;
	dbd.BindFlags = 0;
	m_pDevice->CreateBuffer(&dbd, NULL, &pStaging);
	m_pContext->CopyResource(pStaging, pSource);
	D3D11_MAPPED_SUBRESOURCE dms;
	m_pContext->Map(pStaging, 0, D3D11_MAP_READ, 0, &dms);
	memcpy(pData,dms.pData, uSize);
	m_pContext->Unmap(pStaging, 0);
	pStaging->Release();
}

