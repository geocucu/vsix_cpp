#pragma warning(push)
#pragma warning(disable:28251) // Inconsistent annotations 

// Bake GUIDs
#define INITGUID
#include <guiddef.h>
#include "..\res\guids.h"
#undef INITGUID
#include <VSShellInterfaces.h>

// ======== MIDL ======== 
#define _MIDL_USE_GUIDDEF_ 
#include "vsix_cpp.c" 
#include "vsix_cpp.h" 

// ======== ATL ======== 
#define _ATL_APARTMENT_THREADED
#define _ATL_REGISTER_PER_USER
#include <atlbase.h>
#include <atlcom.h>
#include <atlwin.h>
#include <atlstr.h>
#include <atlfile.h>
#include <atlsafe.h>

// ======== Visual Studio ======== 
#include <dte.h> // for extensibility
#include <objext.h> // for ILocalRegistry
#include <vshelp.h> // for Help
#include <uilocale.h> // for IUIHostLocale2
#include <IVsQueryEditQuerySave2.h> // for IVsQueryEditQuerySave2
#include <vbapkg.h> // for IVsMacroRecorder
#include <fpstfmt.h> // for IPersistFileFormat
#include <VSRegKeyNames.h>
#include <stdidcmd.h>
#include <stdiduie.h> // For status bar consts.
#include <textfind.h>
#include <textmgr.h>

// ======== VSL ======== 
#define VSLASSERT _ASSERTE
#define VSLASSERTEX(exp, szMsg) _ASSERT_BASE(exp, szMsg)
#define VSLTRACE ATLTRACE
#include <VSLPackage.h>
#include <VSLCommandTarget.h>
#include <VSLWindows.h>
#include <VSLFile.h>
#include <VSLContainers.h>
#include <VSLComparison.h>
#include <VSLAutomation.h>
#include <VSLFindAndReplace.h>
#include <VSLShortNameDefines.h>

// ======== GUIDs/IDs ======== 
#include "..\res\resource.h"


typedef unsigned long long u64;
typedef long long i64;

struct __declspec(novtable) pkg_t :
	// Non-thread safe COM object; partial implementation for IUnknown (the COM map below provides the rest).
	CComObjectRootEx<CComSingleThreadModel>,
	CComCoClass<pkg_t, &CLSID_pkg>,
	// Implement IVsPackage => make this COM object into a VS Package.
	VSL::IVsPackageImpl<pkg_t, &CLSID_pkg>,
	VSL::IOleCommandTargetImpl<pkg_t>,
	// Provides consumers of this object with the ability to determine which interfaces support extended error information.
	ATL::ISupportErrorInfoImpl<&IID_IVsPackage>
{
	// ==== Start of COM map ==== 
	// Provides a portion of the implementation of IUnknown, in particular the list of interfaces the pkg_t object will support via QueryInterface
	
	//BEGIN_COM_MAP(pkg_t)
	typedef pkg_t _ComMapClass; 
	static HRESULT __stdcall _Cache(void *pv, const IID &iid, void **ppvObject, u64 dw) {
		pkg_t *p = (pkg_t *)pv;
		p->Lock(); 
		HRESULT hr = E_FAIL; 
		__try {
			hr = ATL::CComObjectRootBase::_Cache(pv, iid, ppvObject, dw);
		}
		__finally {
			p->Unlock();
		} 
		return hr;
	} 
	
	IUnknown *_GetRawUnknown() {
		return (IUnknown *)((i64)this + _GetEntries()->dw);
	} 
	
	IUnknown *GetUnknown() {
		return (IUnknown *)((i64)this + _GetEntries()->dw);
	} 
	
	HRESULT _InternalQueryInterface(const IID &iid, void **ppvObject) {
		return this->InternalQueryInterface(this, _GetEntries(), iid, ppvObject);
	} 
	
	static const ATL::_ATL_INTMAP_ENTRY* _GetEntries() {
		static pkg_t *addr_8 = (pkg_t *)8; // Fake non-null addr for adjusting pointers by base interfaces (within the derived pkg_t)
		static u64 dw_IVsPackage        = u64((IVsPackage *)       addr_8) - 8; // Offset of vfptr for IVsPackage
		static u64 dw_IOleCommandTarget = u64((IOleCommandTarget *)addr_8) - 8; // Offset of vfptr for IOleCommandTarget
		static u64 dw_ISupportErrorInfo = u64((ISupportErrorInfo *)addr_8) - 8; // Offset of vfptr for ISupportErrorInfo
		static _ATL_CREATORARGFUNC *func = (ATL::_ATL_CREATORARGFUNC *)1; // Sentinel?

		static const ATL::_ATL_INTMAP_ENTRY _entries[] = {
			//COM_INTERFACE_ENTRY(IVsPackage)
			{ &IID_IVsPackage, dw_IVsPackage, func },
		
			//COM_INTERFACE_ENTRY(IOleCommandTarget)
			{ &IID_IOleCommandTarget, dw_IOleCommandTarget, func },
		
			//COM_INTERFACE_ENTRY(ISupportErrorInfo)
			{ &IID_ISupportErrorInfo, dw_ISupportErrorInfo, func },

			//END_COM_MAP()
__if_exists(_GetAttrEntries) { 
			{ 0, (u64)_GetAttrEntries, _ChainAttr }, } 

			{ 0, 0, 0 } 
		}; 
		
		return _entries;
	} 
	
	virtual ULONG __stdcall AddRef() = 0; 
	virtual ULONG __stdcall Release() = 0; 
	virtual __declspec(nothrow) HRESULT __stdcall QueryInterface(const IID &, void **) = 0;

	// ==== End of COM map === 

	// Provide error information if it is not possible to load the UI dll. 
	static const LoadUILibrary::ExtendedErrorInfo &GetLoadUILibraryErrorInfo() {
		static LoadUILibrary::ExtendedErrorInfo errorInfo(L"The product is not installed properly. Please reinstall.");
		return errorInfo;
	}

	// DLL is registered with VS via a pkgdef file. Don't do anything if asked to self-register.
	static HRESULT UpdateRegistry(BOOL bRegister) { return S_OK; }

	// ================================ Interesting non-boilerplate START ================================

	static CommandHandler *GetCommand(const VSL::CommandId &target_id) {
		// Command map specific to this package
		// Combined command id: cmdset GUID & ID within cmdset
		static VSL::CommandId id_test_btn(CLSID_cmdset, CMDID_test_btn);
		static CommandHandler test_btn_handler(id_test_btn, 0, &on_test_btn);

		if (target_id == id_test_btn) {
			return &test_btn_handler;
		}

		return 0;
	}

	void on_test_btn(CommandHandler *sender, DWORD flags, VARIANT *in, VARIANT *out) {
		CComPtr<IVsUIShell> spUiShell = this->GetVsSiteCache().GetCachedService<IVsUIShell, IID_IVsUIShell>();
		LONG lResult;
		HRESULT hr = spUiShell->ShowMessageBox(
			0,
			GUID_NULL,
			L"vsix_cpp",
			L"Heelo 3",
			0,
			0,
			OLEMSGBUTTON_OK,
			OLEMSGDEFBUTTON_FIRST,
			OLEMSGICON_INFO,
			0,
			&lResult);

		// VSL::FailOnError<VSL::HRESULTTraits>::Invoke(hr);
	}

	// ================================ Interesting non-boilerplate END ================================
};

// This exposes pkg_t for instantiation via DllGetClassObject; however, an instance
// can not be created by CoCreateInstance, as pkg_t is specifically registered with
// VS, not the the system in general.
OBJECT_ENTRY_AUTO(CLSID_pkg, pkg_t)

// Should be instantiated before the point where DllGetClassObject(factory...) goes through 
struct dll_module_t : CAtlDllModuleT<dll_module_t> {} dll_module;

extern "C" BOOL DllMain(HINSTANCE, DWORD reason, void *) {
	if (reason == DLL_THREAD_ATTACH && CAtlBaseModule::m_bInitFailed) {
		__debugbreak();
		return 0;
	}

	return 1;
}

extern "C" HRESULT DllCanUnloadNow() {
	if (dll_module.m_nLockCnt == 0) {
		return S_OK;
	}

	return S_FALSE;
}

// Returns a class factory to create an object of the requested type
extern "C" HRESULT DllGetClassObject(const IID &rclsid, const IID &riid, void **ppv) {
	if (!ppv) {
		return E_POINTER;
	}

	*ppv = 0;

	HRESULT hr = S_OK;

	for (_ATL_OBJMAP_ENTRY_EX **ppEntry = _AtlComModule.m_ppAutoObjMapFirst; ppEntry < _AtlComModule.m_ppAutoObjMapLast; ppEntry++) {
		if (*ppEntry != NULL) {
			const _ATL_OBJMAP_ENTRY_EX *pEntry = *ppEntry;

			if ((pEntry->pfnGetClassObject != NULL) && InlineIsEqualGUID(rclsid, *pEntry->pclsid)) {
				_ATL_OBJMAP_CACHE *pCache = pEntry->pCache;

				if (pCache->pCF == NULL) {
					CComCritSecLock<CComCriticalSection> lock(_AtlComModule.m_csObjMap, false);
					hr = lock.Lock();
					if (FAILED(hr)) {
						ATLTRACE(atlTraceCOM, 0, _T("ERROR : Unable to lock critical section in AtlComModuleGetClassObject\n"));
						_ASSERTE(FALSE);
						break;
					}

					if (pCache->pCF == NULL) {
						IUnknown *factory = NULL;
						hr = pEntry->pfnGetClassObject(pEntry->pfnCreateInstance, __uuidof(IUnknown), reinterpret_cast<void **>(&factory));
						if (SUCCEEDED(hr))
						{
							pCache->pCF = reinterpret_cast<IUnknown *>(::EncodePointer(factory));
						}
					}
				}

				if (pCache->pCF != NULL) {
					// Decode factory pointer
					IUnknown *factory = reinterpret_cast<IUnknown *>(::DecodePointer(pCache->pCF));
					hr = factory->QueryInterface(riid, ppv);
				}
				break;
			}
		}
	}

	if (*ppv == NULL && hr == S_OK) {
		hr = CLASS_E_CLASSNOTAVAILABLE;
	}

	return hr;

	//HRESULT hr = 0;
	//try { return _AtlModule.GetClassObject(rclsid, riid, ppv); }
	//catch (const VSL::ExceptionBase &e) { hr = vs_report_err(e); }
	//catch (const std::exception &e) { hr = vs_report_err(e); }
	//catch (const ATL::CAtlException &e) { hr = vs_report_err(e); }
	//catch (...) { hr = E_UNEXPECTED; };
	//return hr;
}

#pragma warning(pop)