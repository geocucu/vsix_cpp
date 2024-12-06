#pragma warning(push)
#pragma warning(disable:28251) // Inconsistent annotations 

// Bake GUIDs
#define INITGUID
#include <guiddef.h>
#include "Guids.h"
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
#include "resource.h"


struct __declspec(novtable) pkg_t : 
  // Non-thread safe COM object, partial IUnknown implementation.
  CComObjectRootEx<CComSingleThreadModel>, CComCoClass<pkg_t, &CLSID_pkg>,
  // Make this COM object into a VS package.
  VSL::IVsPackageImpl<pkg_t, &CLSID_pkg>, VSL::IOleCommandTargetImpl<pkg_t>,
  // Consumers of this object can determine which interfaces support extended error information.
  ATL::ISupportErrorInfoImpl<&__uuidof(IVsPackage)> 
{
public:
	pkg_t() = default;
	~pkg_t() = default;

	HRESULT _InternalQueryInterface(const IID &iid, void **obj) throw() {
		return this->InternalQueryInterface(this, _GetEntries(), iid, obj);
	} 

	const static ATL::_ATL_INTMAP_ENTRY *__stdcall _GetEntries() throw() {
		static const ATL::_ATL_INTMAP_ENTRY _entries[] = { 
			{ 0, (DWORD_PTR)L"pkg_t", (ATL::_ATL_CREATORARGFUNC *)0 },
		  { &__uuidof(IVsPackage), ((DWORD_PTR)(static_cast<IVsPackage *>((pkg_t *)8)) - 8), ((ATL::_ATL_CREATORARGFUNC *)1) },
			{ &__uuidof(IOleCommandTarget), ((DWORD_PTR)(static_cast<IOleCommandTarget *>((pkg_t *)8)) - 8), ((ATL::_ATL_CREATORARGFUNC *)1) },
			{ &__uuidof(ISupportErrorInfo), ((DWORD_PTR)(static_cast<ISupportErrorInfo *>((pkg_t *)8)) - 8), ((ATL::_ATL_CREATORARGFUNC *)1) },
			__if_exists(_GetAttrEntries) { { 0, (DWORD_PTR)_GetAttrEntries, _ChainAttr }, } { 0, 0, 0 } 
		}; 

		return &_entries[1];
	} 

	// Get error info if the UI DLL can't be loaded. 
	static const LoadUILibrary::ExtendedErrorInfo &GetLoadUILibraryErrorInfo() {
		static LoadUILibrary::ExtendedErrorInfo info(L"The product is not installed properly. Please reinstall.");
		return info;
	}

	// DLL is registered with VS via a pkgdef file. Don't do anything if asked to self-register.
	static HRESULT UpdateRegistry(BOOL bRegister) {
		return S_OK;
	}

	// NOTE - the arguments passed to these macros can not have names longer then 30 characters
	// Definition of the commands handled by this package
	static CommandHandler *GetCommand(const VSL::CommandId &rId) {
		UINT iNumberOfBins = 17;
		__if_exists(CAtlMapNumberOfBins) {
			iNumberOfBins = CAtlMapNumberOfBins;
		}
		typedef CAtlMap<const VSL::CommandId, CommandHandler *> CommandMap;
		static CommandMap commands;
		static bool bInitialized = false;
		if (!bInitialized) {
			commands.InitHashTable(iNumberOfBins, false);
			static CommandHandler cmdset_cmdidtest_handler(
				CLSID_cmdset,  // cmdset GUID 
				cmdidtest,     // cmd      ID 
				static_cast<CommandHandler::QueryStatusHandler>(0),
				static_cast<CommandHandler::ExecHandler>(CommandHandler::ExecHandler(&OnMyCommand)));
			commands[cmdset_cmdidtest_handler.GetId()] = &cmdset_cmdidtest_handler;
			bInitialized = true;
		};
		CommandMap::CPair *pair = commands.Lookup(rId);
		if (0 == pair) {
			return 0;
		}
		return pair->m_value;
	}

	// Command handler called when the user selects the "My Command" command.
	void OnMyCommand(CommandHandler * /*pSender*/, DWORD /*flags*/, VARIANT * /*pIn*/, VARIANT * /*pOut*/) {
		// Get the string for the title of the message box from the resource dll.
		CComBSTR bstrTitle;
		VSL::FailOnError<VSL::BOOLGetLastErrorTraits >::Invoke(bstrTitle.LoadStringW(_AtlBaseModule.GetResourceInstance(), 100));
		// Get a pointer to the UI Shell service to show the message box.
		CComPtr<IVsUIShell> vs_ui_shell = this->GetVsSiteCache().GetCachedService<IVsUIShell, IID_IVsUIShell>();
		LONG lResult;
		HRESULT hr = vs_ui_shell->ShowMessageBox(
			0,
			GUID_NULL,
			bstrTitle,
			W2OLE(L"Hallo"),
			0,
			0,
			OLEMSGBUTTON_OK,
			OLEMSGDEFBUTTON_FIRST,
			OLEMSGICON_INFO,
			0,
			&lResult);
		VSL::FailOnError<VSL::HRESULTTraits>::Invoke(hr);
	}
};

// This exposes pkg_t for instantiation via DllGetClassObject; however, an instance
// can not be created by CoCreateInstance, as pkg_t is specifically registered with
// VS, not the the system in general.
__declspec(selectany) ATL::_ATL_OBJMAP_CACHE __objCache__pkg_t = { 0, 0 };
const ATL::_ATL_OBJMAP_ENTRY_EX __objMap_pkg_t = {
	&CLSID_pkg, 
	pkg_t::UpdateRegistry,
	pkg_t::_ClassFactoryCreatorClass::CreateInstance,
	pkg_t::_CreatorClass::CreateInstance,
	&__objCache__pkg_t,
	pkg_t::GetObjectDescription,
	pkg_t::GetCategoryMap,
	pkg_t::ObjectMain };
extern "C" __declspec(allocate("ATL$__m")) __declspec(selectany) 
const ATL::_ATL_OBJMAP_ENTRY_EX *const __pobjMap_pkg_t = &__objMap_pkg_t;
__pragma(comment(linker, "/include:__pobjMap_" "pkg_t"));

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