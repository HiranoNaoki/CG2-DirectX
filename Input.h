#pragma once
#include <Windows.h>
#include <wrl.h>

#define DIRECTINPUT_VERSION  0x0800
#include <dinput.h>

class Input
{
public: 

	template <class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

	void Initialize(HINSTANCE wc, HWND hwnd);

	void Update();

	bool PushKey(BYTE KeyNumber);

	bool TriggerKey(BYTE keyNumber);

private:
		ComPtr<IDirectInputDevice8> keyboard;

		ComPtr<IDirectInput8> directInput;

		BYTE key[256] = {};

		BYTE keyPre[256] = {};
};