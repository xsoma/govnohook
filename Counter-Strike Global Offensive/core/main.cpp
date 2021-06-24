#include "source.hpp"
#include <icons/visitor.hpp>
#include <icons/hooge.hpp>
#include <icons/icons.hpp>
#include "sdk.hpp"
#define _ITERATOR_DEBUG_LEVEL 0

#pragma warning( once : 2386 )

DWORD installed;

class init_font
{
public:
	init_font(void* font, uint32_t length)
	{
		if (handle = AddFontMemResourceEx(font, length, nullptr, &installed); handle == nullptr)
			return;

		VirtualProtect(font, length, PAGE_READWRITE, 0);
		memset(font, 0, length);
	}

private:
	HANDLE handle;
};

FILE* fpstdin = stdin, * fpstdout = stdout, * fpstderr = stderr;

void Entry(HMODULE hModule)
{
	VIRTUALIZER_START;
	init_font(static_cast<void*>(visitor), sizeof(visitor));
	init_font(static_cast<void*>(hooge), sizeof(hooge));
	init_font(static_cast<void*>(icons_font), sizeof(icons_font));
	init_font(static_cast<void*>(skeet_visuals), sizeof(skeet_visuals));

	while (!GetModuleHandleA(sxor("serverbrowser.dll")))
		Sleep(200);

	if (Source::Create())
	{
		//while (!ctx.force_unload) {
		while (!GetAsyncKeyState(VK_F11) && !ctx.force_unload) {
			Sleep(200);
		}
		Source::Destroy();


		Sleep(1000);
	}

	FreeLibraryAndExitThread(hModule, EXIT_SUCCESS);

	VIRTUALIZER_END;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		CreateThread(nullptr, 0u, (LPTHREAD_START_ROUTINE)Entry, hModule, 0u, nullptr);

		DisableThreadLibraryCalls(hModule);
	}
	return TRUE;
}