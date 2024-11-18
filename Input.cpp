#include "Input.h"
#include <cassert>



#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

//using namespace Microsoft::WRL;

void Input::Initialize(HINSTANCE hInstance, HWND hwnd)
{
	HRESULT hr;

	
	hr = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8,
		(void**)&directInput, nullptr);
	assert(SUCCEEDED(hr));




	hr = directInput->CreateDevice(GUID_SysKeyboard, &keyboard, NULL);
	assert(SUCCEEDED(hr));

	hr = keyboard->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(hr));

	hr = keyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(hr));
}

void Input::Update()
{
	keyboard->Acquire();
	
	/*if (input->Pushkey(DIK_0)) {
				OutputDebugStringA("Hit 0\n");
	}*/


	keyboard->GetDeviceState(sizeof(key), key);

	memcpy(keyPre, key, sizeof(key));

}

bool Input::PushKey(BYTE KeyNUmber) {
	if (key[KeyNUmber])
	{
		return true;
	}
	else
	{
		return false;
	}
	
	
}

bool Input::TriggerKey(BYTE keyNumber) {
	if (key[keyNumber] && !keyPre[keyNumber])
	{
		return true;

	}
	else {
			return false;
	}

}
