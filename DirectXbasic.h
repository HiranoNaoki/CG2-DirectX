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

	void DescriptorHeap();

	void RenderTargetView();

	D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCPUDescriptorHandle(uint32_t index);

	D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPUDescriptorHandle(uint32_t index);

	void DepthStencil();

	void fence();

	void viewport();

	void scissorRect();

	void DxccomPtr();

	void ImGUI();

	void PreDraw();
	void PostDraw();

private:
	Microsoft::WRL::ComPtr<ID3D12Device> device;

	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory;

	WinApp* winApp = nullptr;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(
		D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);

	static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index);

	static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>& descriptorHeap, uint32_t descriptorSize, uint32_t index);

	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2> swapChainResources;

	UINT64 fenceValue = 0;


};