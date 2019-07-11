#include <dwmapi.h>
#include <node.h>
#include <node_buffer.h>

bool IsWindows10() {
	OSVERSIONINFOA info;
	ZeroMemory(&info, sizeof(OSVERSIONINFOA));
	info.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);

	GetVersionEx(&info);

	return info.dwMajorVersion == 10;
}

struct ACCENTPOLICY {
	int nAccentState;
	int nFlags;
	int nColor;
	int nAnimationId;
};
struct WINCOMPATTRDATA {
	int nAttribute;
	PVOID pData;
	ULONG ulDataSize;
};

enum AccentTypes {
	ACCENT_DISABLE = 0,
	ACCENT_ENABLE_GRADIENT = 1,
	ACCENT_ENABLE_TRANSPARENTGRADIENT = 2,
	ACCENT_ENABLE_BLURBEHIND = 3
};

void blurbehind(const v8::FunctionCallbackInfo<v8::Value> &args)
{
	const HWND handle = *reinterpret_cast<HWND *>(node::Buffer::Data(args[0].As<v8::Object>()));
	const bool state = args[1].As<v8::Boolean>()->Value();

	if (IsWindows10) {
		const HINSTANCE hModule = LoadLibrary(TEXT("user32.dll"));
		if (hModule) {
			typedef BOOL(WINAPI*pSetWindowCompositionAttribute)(HWND,
				WINCOMPATTRDATA*);
			const pSetWindowCompositionAttribute
				SetWindowCompositionAttribute =
				(pSetWindowCompositionAttribute)GetProcAddress(
					hModule,
					"SetWindowCompositionAttribute");

			// Only works on Win10
			if (SetWindowCompositionAttribute) {
				ACCENTPOLICY policy =
				{ state ? ACCENT_ENABLE_BLURBEHIND
					: ACCENT_DISABLE , 0, 0, 0 };
				WINCOMPATTRDATA data = { 19, &policy, sizeof(ACCENTPOLICY) };
				args.GetReturnValue().Set(SetWindowCompositionAttribute(handle, &data));
			}
			FreeLibrary(hModule);
		}
	}
	else
	{
		const DWM_BLURBEHIND bb = {
		DWM_BB_ENABLE,
		state,
		NULL,
		FALSE
		};

		const HRESULT returnValue = DwmEnableBlurBehindWindow(handle, &bb);

		args.GetReturnValue().Set(SUCCEEDED(returnValue));
	}
}

void init(v8::Local<v8::Object> exports)
{
	NODE_SET_METHOD(exports, "blurbehind", blurbehind);
}

NODE_MODULE(NODE_GYP_MODULE_NAME, init)
