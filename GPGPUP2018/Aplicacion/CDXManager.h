#pragma once
#include <dxgi.h>
#include <d3d11.h>
class CDXManager
{
protected:
	IDXGISwapChain *    m_pSwapChain;  //Infraestructura de render
	ID3D11Device*       m_pDevice;     //Fabrica de recursos 
	ID3D11DeviceContext* m_pContext;   //Fachada funcionalidad
public:
	IDXGIAdapter * EnumAndChooseAdapter(HWND hWnd);
	IDXGISwapChain* GetSwapChain() { return m_pSwapChain; };
	ID3D11Device* GetDevice() { return m_pDevice; }
	ID3D11DeviceContext* GetContext(){return m_pContext; }
	CDXManager();
	~CDXManager();
	bool Initialize(HWND hWnd, IDXGIAdapter* pAdapter, int sx, int sy, bool b_UseSoftware = true);
	ID3D11ComputeShader* CompileCS(const wchar_t* pwszFileName, const char* pszEntryPoint);
	void Upload(void* pData, size_t uSize, ID3D11Buffer* pDest);
	void Download(void* pData, size_t uSize, ID3D11Buffer* pSource);
};

