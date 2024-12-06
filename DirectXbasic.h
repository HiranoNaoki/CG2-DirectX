#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include "WinApp.h"



class DirectXbasic {
public:

	void Intialize(WinApp* winApp);

	void Device();

	void Command();

	void Swap();

	void DepthBuffer();
private:
	Microsoft::WRL::ComPtr<ID3D12Device> device;

	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory;

	WinApp* winApp = nullptr;
};