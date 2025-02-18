#define DIRECTINPUT_VERSION 0x0800


#include<string>
#include<format>
#include<d3d12.h>
#include<dxgi1_6.h>
#include<cassert>
#include <dxgidebug.h>
#include <dxcapi.h>
#include<vector>
#include<cmath>
#include<math.h>
#include<fstream>
#include<sstream>
#include"externals/DirectXTex/DirectXTex.h"

#include"externals//DirectXTex//d3dx12.h"
#include <corecrt_math_defines.h>
#include <dinput.h>
#include <wrl.h>
#include"Input.h"
#include "WinApp.h"
#include"Math.h"
#include"DirectXbasic.h"
#include "Logger.h"
#include<wrl.h>




struct VertexDate
{
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};

struct MaterialData
{
	std::string textureFilePath;
};

struct ModelData
{
	std::vector<VertexDate> vertices;
	MaterialData material;
};



MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename) {

	MaterialData materialData;
	std::string line;

	std::ifstream file(directoryPath + "/" + filename);
	assert(file.is_open());

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;


		if (identifier == "map_Kd") {
			std::string textureFilename;
			s >> textureFilename;

			materialData.textureFilePath = directoryPath + "/" + textureFilename;
		}
	}

	return materialData;
}




ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename) {

	ModelData modelData;
	std::vector<Vector4> positions;
	std::vector<Vector3> normals;
	std::vector<Vector2> texcoords;
	std::string line;


	std::ifstream file(directoryPath + "/" + filename);
	assert(file.is_open());

	while (std::getline(file, line))
	{
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;


		if (identifier == "v") {
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.w = 1.0f;
			positions.push_back(position);
		}
		else if (identifier == "vt") {
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoords.push_back(texcoord);
		}
		else if (identifier == "vn") {
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			normals.push_back(normal);
		}
		else if (identifier == "f") {
			VertexDate triangle[3];


			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
				std::string vertexDefinition;
				s >> vertexDefinition;

				std::istringstream v(vertexDefinition);
				uint32_t elementIndices[3];
				for (int32_t element = 0; element < 3; ++element) {
					std::string index;
					std::getline(v, index, '/');
					elementIndices[element] = std::stoi(index);
				}

				Vector4 position = positions[elementIndices[0] - 1];
				Vector2 texcoord = texcoords[elementIndices[1] - 1];
				Vector3 normal = normals[elementIndices[2] - 1];

				position.x *= -1.0f;
				texcoord.y = 1.0f - texcoord.y;
				normal.x *= -1.0f;

				triangle[faceVertex] = { position, texcoord, normal };
			}

			modelData.vertices.push_back(triangle[2]);
			modelData.vertices.push_back(triangle[1]);
			modelData.vertices.push_back(triangle[0]);
		}
		else if (identifier == "mtllib") {

			std::string materialFilename;
			s >> materialFilename;

			modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
		}
	}

	return modelData;
}






Transform transform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f } };

Transform cameratransform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,-10.0f} };

Transform transformSprite{ {1.0f,1.0f,1.0f,},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };


Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
Matrix4x4 cameraMatrix = MakeAffineMatrix(cameratransform.scale, cameratransform.rotate, cameratransform.translate);
Matrix4x4 viewMatrix = Inverse(cameraMatrix);
Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f,(1280.0f/ 720.0f), 0.1f, 100.0f);
Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));


Matrix4x4 worldMatrixSprite = MakeAffineMatrix(transformSprite.scale, transformSprite.rotate, transformSprite.translate);
Matrix4x4 viewMatrixSprite = MakeIdentity4x4();
Matrix4x4 projectionMatrixSprite = MakeOrthograhicMatrix(0.0f, float(WinApp::kClientWidth), 0.0f, float(WinApp::kClientHeight), 0.0f, 100.0f);
Matrix4x4 worldViewProjectionMatrixSprate = Multiply(worldMatrixSprite, Multiply(viewMatrixSprite, projectionMatrixSprite));




Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(
	Microsoft::WRL::ComPtr<ID3D12Device> device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible)
{
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = heapType;
	descriptorHeapDesc.NumDescriptors = numDescriptors;
	descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));



	assert(SUCCEEDED(hr));
	return descriptorHeap;
}



//String.h
/*std::wstring ConvertString(const std::string& str) {
	if (str.empty()) {
		return std::wstring();
	}

	auto sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), NULL, 0);
	if (sizeNeeded == 0) {
		return std::wstring();
	}
	std::wstring result(sizeNeeded, 0);
	MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), &result[0], sizeNeeded);
	return result;
}

std::string ConvertString(const std::wstring& str) {
	if (str.empty()) {
		return std::string();
	}

	auto sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), NULL, 0, NULL, NULL);
	if (sizeNeeded == 0) {
		return std::string();
	}
	std::string result(sizeNeeded, 0);
	WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), result.data(), sizeNeeded, NULL, NULL);
	return result;
}*/



//Logger.h
//void Log(const std::string& message) {

//}



/*Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(
	const std::wstring& filePath,
	const wchar_t* profile,
	Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils,
	Microsoft::WRL::ComPtr<IDxcCompiler3> dxCompiler,
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler)
{
	Log(ConvertString(std::format(L"Begin CompileShader, path:{}, profile:{}\n", filePath, profile)));

	Microsoft::WRL::ComPtr<IDxcBlobEncoding> ShaderSource = nullptr;

	HRESULT hr = dxcUtils->LoadFile(filePath.c_str(), NULL, &ShaderSource);

	assert(SUCCEEDED(hr));

	DxcBuffer shaderSourceBuffer;
	shaderSourceBuffer.Ptr = ShaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = ShaderSource->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8;



	LPCWSTR arguments[] = {
	filePath.c_str(),
	L"-E",L"main",
	L"-T",profile,
	L"-Zi",L"-Qembed_debug",
	L"-Od",
	L"-Zpr",
	};

	Microsoft::WRL::ComPtr<IDxcResult> shaderResult = nullptr;
	hr = dxCompiler->Compile(
		&shaderSourceBuffer,
		arguments,
		_countof(arguments),
		 includeHandler.Get(),
		IID_PPV_ARGS(&shaderResult)
	);

	assert(SUCCEEDED(hr));

	Microsoft::WRL::ComPtr<IDxcBlobUtf8> shaderError = nullptr;
	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
	if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
		Log(shaderError->GetStringPointer());

		assert(false);
	}
	Microsoft::WRL::ComPtr<IDxcBlob> shaderBlob = nullptr;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr));

	Log(ConvertString(std::format(L"Compile Suceeded,path:{},profile:{}\n", filePath, profile)));

	ShaderSource->Release();
	shaderResult->Release();

	return shaderBlob;

}*/

/*Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(Microsoft::WRL::ComPtr<ID3D12Device> device, size_t sizeInBytes) {


	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC vertexResoureDesc{};

	vertexResoureDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	vertexResoureDesc.Width = sizeInBytes;

	vertexResoureDesc.Height = 1;
	vertexResoureDesc.DepthOrArraySize = 1;
	vertexResoureDesc.MipLevels = 1;
	vertexResoureDesc.SampleDesc.Count = 1;

	vertexResoureDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE,
		&vertexResoureDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));
	return resource;
}*/

/*DirectX::ScratchImage LoadTexture(const std::string& filePath) {
	DirectX::ScratchImage image{};
	std::wstring filePathw = ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathw.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));

	DirectX::ScratchImage mipImges{};
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImges);
	assert(SUCCEEDED(hr));

	return mipImges;

}*/



/*Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(Microsoft::WRL::ComPtr<ID3D12Device> device, const DirectX::TexMetadata& metabete)
{
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = UINT(metabete.width);
	resourceDesc.Height = UINT(metabete.height);
	resourceDesc.MipLevels = UINT16(metabete.mipLevels);
	resourceDesc.DepthOrArraySize = UINT16(metabete.arraySize);
	resourceDesc.Format = metabete.format;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metabete.dimension);



	//Heap
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	//heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	//heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;

	//resource
	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));

	return resource;

}*/



//[[nodiscard]]


/*Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(Microsoft::WRL::ComPtr <ID3D12Device> device, int32_t width, int32_t height) {
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = width;
	resourceDesc.Height = height;
	resourceDesc.MipLevels = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;


	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f;
	depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthClearValue,
		IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));
	return resource;
}*/




bool useMonsterBall = true;

struct D3DResourceLeakChecker
{
	~D3DResourceLeakChecker() {
		Microsoft::WRL::ComPtr<IDXGIDebug1> debug;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
			debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
			debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
			debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);

		}
	}
};

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {


	D3DResourceLeakChecker leakcheker;





#pragma region 



	WinApp* winApp = nullptr;
	Input* input = nullptr;
	DirectXbasic* dxbasic = nullptr;

	winApp = new WinApp();
	winApp->Initialize();

	input = new Input();
	input->Initialize(winApp);

	dxbasic = new DirectXbasic();
	dxbasic->Initialize(winApp);




#pragma endregion 


	/*Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap = CreateDescriptorHeap(dxbasic->GetDevice(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap = CreateDescriptorHeap(dxbasic->GetDevice(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap = CreateDescriptorHeap(dxbasic->GetDevice(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);


*/



	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};

	depthStencilDesc.DepthEnable = true;

	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;

	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;


	//Rootsignature
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;


	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;
	descriptorRange[0].NumDescriptors = 1;
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


	D3D12_ROOT_PARAMETER rootParameters[4] = {};
	//material
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[0].Descriptor.ShaderRegister = 0;
	//transformatrix
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[1].Descriptor.ShaderRegister = 0;
	//texture
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);
	//directionalLight
	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[3].Descriptor.ShaderRegister = 1;

	descriptionRootSignature.pParameters = rootParameters;
	descriptionRootSignature.NumParameters = _countof(rootParameters);

	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
	staticSamplers[0].ShaderRegister = 0;
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);




	Microsoft::WRL::ComPtr<ID3DBlob> sigunatureBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr;
	hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &sigunatureBlob, &errorBlob);
	if (FAILED(hr)) {
		Logger::Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	hr = dxbasic->GetDevice()->CreateRootSignature(0, sigunatureBlob->GetBufferPointer(), sigunatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));

	//InputLayout
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[2].SemanticName = "NORMAL";
	inputElementDescs[2].SemanticIndex = 0;
	inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	D3D12_BLEND_DESC blendDesc{};

	blendDesc.RenderTarget[0].RenderTargetWriteMask =
		D3D12_COLOR_WRITE_ENABLE_ALL;

	D3D12_RASTERIZER_DESC rasterizerDesc{};

	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = dxbasic->CompileShader(L"resource/shaders/Object3d.VS.hlsl",
		L"vs_6_0");
	assert(vertexShaderBlob != nullptr);

	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = dxbasic->CompileShader(L"resource/shaders/Object3d.PS.hlsl",
		L"ps_6_0");
	assert(pixelShaderBlob != nullptr);



	 Material* materialDate = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> materialResource = dxbasic->CreateBufferResource(sizeof(Material));
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialDate));
	materialDate->color = { Vector4(1.0f, 1.0f, 1.0f, 1.0f) };
	materialDate->enableLighting = true;
	materialDate->uvTransform = MakeIdentity4x4();






	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPilelineStateDesc{};
	graphicsPilelineStateDesc.pRootSignature = rootSignature.Get();
	graphicsPilelineStateDesc.InputLayout = inputLayoutDesc;
	graphicsPilelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),
	vertexShaderBlob->GetBufferSize() };//vertexShader
	graphicsPilelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),
	pixelShaderBlob->GetBufferSize() };
	graphicsPilelineStateDesc.BlendState = blendDesc;
	graphicsPilelineStateDesc.RasterizerState = rasterizerDesc;

	graphicsPilelineStateDesc.NumRenderTargets = 1;
	graphicsPilelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	graphicsPilelineStateDesc.PrimitiveTopologyType =
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	graphicsPilelineStateDesc.SampleDesc.Count = 1;
	graphicsPilelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	graphicsPilelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPilelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelinestate = nullptr;
	hr = dxbasic->GetDevice()->CreateGraphicsPipelineState(&graphicsPilelineStateDesc,
		IID_PPV_ARGS(&graphicsPipelinestate));
	assert(SUCCEEDED(hr));





	/*ID3D12Resource* vertexResource = CreateBufferResource(device, sizeof(VertexDate) * 6);

	D3D12_VERTEX_BUFFER_VIEW vertexBufferview{};

	vertexBufferview.BufferLocation = vertexResource->GetGPUVirtualAddress();

	vertexBufferview.SizeInBytes = sizeof(VertexDate) * 6;

	vertexBufferview.StrideInBytes = sizeof(VertexDate);

	VertexDate* vertexDate = nullptr;

	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexDate));

	vertexDate[0].position = { -0.5f,-0.5,0.0f,1.0f };
	vertexDate[0].texcoord = { 0.0f,1.0f };
	vertexDate[0].normal.x = vertexDate[0].position.x;
	vertexDate[0].normal.y = vertexDate[0].position.y;
	vertexDate[0].normal.z = vertexDate[0].position.z;

	vertexDate[1].position = { 0.0f,0.5f,0.0f,1.0f };
	vertexDate[1].texcoord = { 0.5f,0.0f };
	vertexDate[1].normal.x = vertexDate[1].position.x;
	vertexDate[1].normal.y = vertexDate[1].position.y;
	vertexDate[1].normal.z = vertexDate[1].position.z;

	vertexDate[2].position = { 0.5f,-0.5f,0.0f,1.0f };
	vertexDate[2].texcoord = { 1.0f,1.0f };
	vertexDate[2].normal.x = vertexDate[2].position.x;
	vertexDate[2].normal.y = vertexDate[2].position.y;
	vertexDate[2].normal.z = vertexDate[2].position.z;

	vertexDate[3].position = { -0.5f,-0.5f,0.5f,1.0f };
	vertexDate[3].texcoord = { 0.0f,1.0f };
	vertexDate[3].normal.x = vertexDate[3].position.x;
	vertexDate[3].normal.y = vertexDate[3].position.y;
	vertexDate[3].normal.z = vertexDate[3].position.z;

	vertexDate[4].position = { 0.0f,0.0f,0.0f,1.0f };
	vertexDate[4].texcoord = { 0.5f,0.0f };
	vertexDate[4].normal.x = vertexDate[4].position.x;
	vertexDate[4].normal.y = vertexDate[4].position.y;
	vertexDate[4].normal.z = vertexDate[4].position.z;

	vertexDate[5].position = { 0.5f,-0.5f,-0.5f,1.0f };
	vertexDate[5].texcoord = { 1.0f,1.0f, };
	vertexDate[5].normal.x = vertexDate[5].position.x;
	vertexDate[5].normal.y = vertexDate[5].position.y;
	vertexDate[5].normal.z = vertexDate[5].position.z;*/





	DirectX::ScratchImage mipImages = dxbasic->LoadTexture("resource/uvChecker.png");
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource = dxbasic->CreateTextureResource(dxbasic->GetDevice(), metadata);
	Microsoft::WRL::ComPtr<ID3D12Resource> intermeditateResource =
		dxbasic->UploadTextureDate(textureResource.Get(), mipImages);


	DirectX::ScratchImage mipImages2 = dxbasic->LoadTexture("resource/monsterBall.png");
	const DirectX::TexMetadata& metadata2 = mipImages2.GetMetadata();
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource2 = dxbasic->CreateTextureResource(dxbasic->GetDevice(), metadata2);
	Microsoft::WRL::ComPtr<ID3D12Resource> intermeditateResource2 =
		dxbasic->UploadTextureDate(textureResource2.Get(), mipImages2);



	ModelData modelData = LoadObjFile("resource", "axis.obj");

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = dxbasic->CreateBufferResource(sizeof(VertexDate) * modelData.vertices.size());

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};

	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();

	vertexBufferView.SizeInBytes = UINT(sizeof(VertexDate) * modelData.vertices.size());

	vertexBufferView.StrideInBytes = sizeof(VertexDate);

	VertexDate* vertexData = nullptr;

	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexDate) * modelData.vertices.size());








	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource = dxbasic->CreateDepthStencilTextureResource(dxbasic->GetDevice(), WinApp::kClientWidth, WinApp::kClientHeight);


	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourceSprite = dxbasic->CreateBufferResource(sizeof(VertexDate) * 6);


	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite{};
	vertexBufferViewSprite.BufferLocation = vertexResourceSprite->GetGPUVirtualAddress();
	vertexBufferViewSprite.SizeInBytes = sizeof(VertexDate) * 6;
	vertexBufferViewSprite.StrideInBytes = sizeof(VertexDate);


	VertexDate* vertexDateSprite = nullptr;
	vertexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&vertexDateSprite));

	vertexDateSprite[0].position = { 0.0f,360.0f,0.0f,1.0f };
	vertexDateSprite[0].texcoord = { 0.0f,1.0f };
	vertexDateSprite[0].normal = { 0.0f,0.0f,-1.0f };
	vertexDateSprite[1].position = { 0.0f,0.0f,0.0f,1.0f };
	vertexDateSprite[1].texcoord = { 0.0f,0.0f };
	vertexDateSprite[1].normal = { 0.0f,0.0f,-1.0f };
	vertexDateSprite[2].position = { 640.0f,360.0f,0.0f,1.0f };
	vertexDateSprite[2].texcoord = { 1.0f,1.0f };
	vertexDateSprite[2].normal = { 0.0f,0.0f,-1.0f };

	vertexDateSprite[3].position = { 0.0f,0.0f,0.0f,1.0f };
	vertexDateSprite[3].texcoord = { 0.0f,0.0f };
	vertexDateSprite[3].normal = { 0.0f,0.0f,-1.0f };
	vertexDateSprite[4].position = { 640.0f,0.0f,0.0f,1.0f };
	vertexDateSprite[4].texcoord = { 1.0f,0.0f };
	vertexDateSprite[4].normal = { 0.0f,0.0f,-1.0f };
	vertexDateSprite[5].position = { 640.0f,360.0f,0.0f,1.0f };
	vertexDateSprite[5].texcoord = { 1.0f,1.0f };
	vertexDateSprite[5].normal = { 0.0f,0.0f,-1.0f };

	const uint32_t kSubdivision = 16;

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourceSphere = dxbasic->CreateBufferResource(sizeof(VertexDate) * kSubdivision * kSubdivision * 6);

	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSphere{};
	vertexBufferViewSphere.BufferLocation = vertexResourceSphere->GetGPUVirtualAddress();
	vertexBufferViewSphere.SizeInBytes = sizeof(VertexDate) * kSubdivision * kSubdivision * 6;
	vertexBufferViewSphere.StrideInBytes = sizeof(VertexDate);


	VertexDate* vertexDateSphere = nullptr;
	vertexResourceSphere->Map(0, nullptr, reinterpret_cast<void**>(&vertexDateSphere));





	const float kLonEvery = float(M_PI) * 2.0f / float(kSubdivision);
	const float kLatEvery = float(M_PI) / float(kSubdivision);

	uint32_t start = 0;

	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex)
	{
		float lat = -float(M_PI) / 2.0f + kLatEvery * latIndex;

		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex)
		{
			start = (latIndex * kSubdivision + lonIndex) * 6;

			float lon = lonIndex * kLonEvery;


			VertexDate vertA = {
				   {
					std::cosf(lat) * std::cosf(lon),
					std::sinf(lat),
					std::cosf(lat) * std::sinf(lon),
					1.0f},{
				float(lonIndex) / float(kSubdivision),
				1.0f - float(latIndex) / float(kSubdivision)},{
					std::cosf(lat) * std::cosf(lon),
					std::sinf(lat),
					std::cosf(lat) * std::sinf(lon),
				}

			};

			VertexDate vertB = {
				{std::cosf(lat + kLatEvery) * std::cosf(lon),
				std::sinf(lat + kLatEvery),
				std::cosf(lat + kLatEvery) * std::sinf(lon),
			1.0f
				},{
				float(lonIndex) / float(kSubdivision),
				1.0f - float(latIndex + 1) / float(kSubdivision)},{
					std::cosf(lat + kLatEvery) * std::cosf(lon),
				std::sinf(lat + kLatEvery),
				std::cosf(lat + kLatEvery) * std::sinf(lon)

				}

			};

			VertexDate vertC = {
				{std::cosf(lat) * std::cosf(lon + kLonEvery),
				std::sinf(lat),
				std::cosf(lat) * std::sinf(lon + kLonEvery),
				1.0f
				},{
				float(lonIndex + 1) / float(kSubdivision),
				1.0f - float(latIndex) / float(kSubdivision)},{
					std::cosf(lat) * std::cosf(lon + kLonEvery),
				std::sinf(lat),
				std::cosf(lat) * std::sinf(lon + kLonEvery)

				}
			};

			VertexDate vertD = {
				{
					std::cosf(lat + kLatEvery) * std::cosf(lon + kLonEvery),
					std::sinf(lat + kLatEvery),
					std::cosf(lat + kLatEvery) * std::sinf(lon + kLonEvery),
					1.0f
				},{
				float(lonIndex + 1) / float(kSubdivision),
				1.0f - float(latIndex + 1) / float(kSubdivision)}
				,{
					std::cosf(lat + kLatEvery) * std::cosf(lon + kLonEvery),
					std::sinf(lat + kLatEvery),
					std::cosf(lat + kLatEvery) * std::sinf(lon + kLonEvery),

			}
			};

			vertexDateSphere[start + 0] = vertA;
			vertexDateSphere[start + 1] = vertB;
			vertexDateSphere[start + 2] = vertC;
			vertexDateSphere[start + 3] = vertC;
			vertexDateSphere[start + 4] = vertB;
			vertexDateSphere[start + 5] = vertD;
		}
	}




	//
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);
	//
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU = dxbasic->GetSRVCPUDescriptorHandle(1);
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU = dxbasic->GetSRVGPUDescriptorHandle(1);
	//
	textureSrvHandleCPU.ptr += dxbasic->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	textureSrvHandleGPU.ptr += dxbasic->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	//SRV
	dxbasic->GetDevice()->CreateShaderResourceView(textureResource.Get(), &srvDesc, textureSrvHandleCPU);



	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc2{};
	srvDesc2.Format = metadata2.format;
	srvDesc2.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc2.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc2.Texture2D.MipLevels = UINT(metadata2.mipLevels);
	//
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU2 = dxbasic->GetSRVCPUDescriptorHandle(2);
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU2 = dxbasic->GetSRVGPUDescriptorHandle(2);

	textureSrvHandleCPU2.ptr += dxbasic->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	textureSrvHandleGPU2.ptr += dxbasic->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	dxbasic->GetDevice()->CreateShaderResourceView(textureResource2.Get(), &srvDesc2, textureSrvHandleCPU2);




	 






	//Microsoft::WRL::ComPtr<ID3D12Resource> materialResource = dxbasic->CreateBufferResource(sizeof(Vector4));

	//Vector4* materialDate = nullptr;
	//materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialDate));




	//*materialDate = Vector4(1.0f, 0.0f, 0.0f, 1.0f);

	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource = dxbasic->CreateBufferResource(sizeof(TransformatioMatrix));

	TransformatioMatrix* wvpDate = nullptr;


	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpDate));


	wvpDate->WVP = MakeIdentity4x4();
	wvpDate->World = MakeIdentity4x4();



	//
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResourceSprite = dxbasic->CreateBufferResource(sizeof(Material));

	Material* materialDateSprite = nullptr;
	materialResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&materialDateSprite));
	materialDateSprite->color = { 1.0f,1.0f,1.0f,1.0f };
	materialDateSprite->enableLighting = true;
	materialDateSprite->uvTransform = MakeIdentity4x4();

	Microsoft::WRL::ComPtr<ID3D12Resource> windowResourceSprite = dxbasic->CreateBufferResource(sizeof(Material));

	Material* windowDateSprite = nullptr;
	windowResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&windowDateSprite));
	windowDateSprite->color = { 1.0f,1.0f,1.0f,1.0f };
	windowDateSprite->uvTransform = MakeIdentity4x4();
	windowDateSprite->enableLighting = false;

	//
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResourceSprite = dxbasic->CreateBufferResource(sizeof(TransformatioMatrix));

	TransformatioMatrix* transformationMatrixDateSprite = nullptr;
	transformationMatrixResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDateSprite));
	transformationMatrixDateSprite->WVP = MakeIdentity4x4();

	//
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource = dxbasic->CreateBufferResource(sizeof(DirectionaLight));

	DirectionaLight* directionalLightDate = nullptr;
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightDate));
	directionalLightDate->color = { 1.0f,1.0f,1.0f,1.0f };
	directionalLightDate->direction = { 0.0f,-1.0f,0.0f };
	directionalLightDate->intensity = 1.0f;

	Microsoft::WRL::ComPtr<ID3D12Resource> indexResourceSprite = dxbasic->CreateBufferResource(sizeof(uint32_t) * 6);

	D3D12_INDEX_BUFFER_VIEW indexBufferviewSprite{};

	indexBufferviewSprite.BufferLocation = indexResourceSprite->GetGPUVirtualAddress();

	indexBufferviewSprite.SizeInBytes = sizeof(uint32_t) * 6;

	indexBufferviewSprite.Format = DXGI_FORMAT_R32_UINT;

	uint32_t* indexDateSprite = nullptr;
	indexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&indexDateSprite));
	indexDateSprite[0] = 0; indexDateSprite[1] = 1; indexDateSprite[2] = 2;
	indexDateSprite[3] = 1; indexDateSprite[4] = 3; indexDateSprite[5] = 2;






	while (!winApp->ProcessMessage()) {

		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		//ImGui::ShowDemoWindow();
		ImGui::Begin("anju");


		ImGui::ColorEdit3("RGB", &materialDate->color.x);
		ImGui::DragFloat3("Scale", &transform.scale.x, 0.01f);
		ImGui::DragFloat3("Rotate", &transform.rotate.x, 0.01f);
		ImGui::DragFloat3("Translate", &transform.translate.x, 0.01f);

		ImGui::DragFloat4("Light color", &directionalLightDate->color.x, 0.01f);
		ImGui::DragFloat3("Light Direction", &directionalLightDate->direction.x, 0.01f);
		ImGui::DragFloat("Light Intensity", &directionalLightDate->intensity, 0.01f);

		ImGui::DragFloat2("UVTranslate", &uvTransformSprite.translate.x, 0.01f, -10.0f, 10.0f);
		ImGui::DragFloat2("UVScale", &uvTransformSprite.scale.x, 0.01f, -10.0f, 10.0f);
		ImGui::SliderAngle("UVRotate", &uvTransformSprite.rotate.z);


		ImGui::Checkbox("useMonsterBall", &useMonsterBall);

		ImGui::End();





		//transform.rotate.y += 0.03f;

		input->Update();






		 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
		 cameraMatrix = MakeAffineMatrix(cameratransform.scale, cameratransform.rotate, cameratransform.translate);
		 viewMatrix = Inverse(cameraMatrix);
		 projectionMatrix = MakePerspectiveFovMatrix(0.45f, (1280.0f / 720.0f), 0.1f, 100.0f);
		 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));

		wvpDate->WVP = worldViewProjectionMatrix;
		wvpDate->World = worldMatrix;


		 worldMatrixSprite = MakeAffineMatrix(transformSprite.scale, transformSprite.rotate, transformSprite.translate);
		 viewMatrixSprite = MakeIdentity4x4();
		 projectionMatrixSprite = MakeOrthograhicMatrix(0.0f, float(WinApp::kClientWidth), 0.0f, float(WinApp::kClientHeight), 0.0f, 100.0f);
		 worldViewProjectionMatrixSprate = Multiply(worldMatrixSprite, Multiply(viewMatrixSprite, projectionMatrixSprite));



		Matrix4x4 uvTransformMatrix = MakeScaleMatrix(uvTransformSprite.scale);
		uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(uvTransformSprite.rotate.z));
		uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(uvTransformSprite.translate));
		windowDateSprite->uvTransform = uvTransformMatrix;


		transformationMatrixDateSprite->WVP = worldViewProjectionMatrixSprate;
		transformationMatrixDateSprite->World = worldMatrixSprite;


		ImGui::Render();

		dxbasic->PreDraw();


		dxbasic->GetCommandList()->SetGraphicsRootSignature(rootSignature.Get());
		dxbasic->GetCommandList()->SetPipelineState(graphicsPipelinestate.Get());

		dxbasic->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);



		dxbasic->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
		dxbasic->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
		dxbasic->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());


		dxbasic->GetCommandList()->SetGraphicsRootDescriptorTable(2, useMonsterBall ? textureSrvHandleGPU2 : textureSrvHandleGPU);

		dxbasic->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());


		dxbasic->GetCommandList()->DrawInstanced(UINT(modelData.vertices.size()), 1, 0, 0);


		//sprite
		dxbasic->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResourceSprite->GetGPUVirtualAddress());

		dxbasic->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferViewSprite);

		dxbasic->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResourceSprite->GetGPUVirtualAddress());

		dxbasic->GetCommandList()->SetGraphicsRootConstantBufferView(0, windowResourceSprite->GetGPUVirtualAddress());
		dxbasic->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);

		dxbasic->GetCommandList()->DrawInstanced(6, 1, 0, 0);


		dxbasic->GetCommandList()->IASetIndexBuffer(&indexBufferviewSprite);

		dxbasic->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);

		dxbasic->PostDraw();
	}

	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	OutputDebugStringA("Hello,DiretX!\n");

	//CloseHandle(fenceEvent);

	delete input;
	winApp->Finalize();
	delete winApp;
	delete dxbasic;

	return 0;
}