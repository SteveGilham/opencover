#include "stdafx.h"
#include "CodeCoverage.h"

#include "dllmain.h"

#include "Method.h"
#include "ProfilerInfo.h"

#define TESTPLATFORM_UTILITIES_ASSEMBLY L"Microsoft.VisualStudio.TestPlatform.Utilities"
#define DEFAULTTESTEXECUTOR_LAUNCHPROCESS L"Microsoft.VisualStudio.TestPlatform.Utilities.DefaultTestExecutorLauncher::LaunchProcess"
#define DEFAULTTESTEXECUTOR_CTOR L"Microsoft.VisualStudio.TestPlatform.Utilities.DefaultTestExecutorLauncher::.ctor"

#define TESTPLATFORM_TESTEXECUTOR_CORE_ASSEMBLY L"Microsoft.VisualStudio.TestPlatform.TestExecutor.Core"
#define TESTEXECUTORMAIN_RUN L"Microsoft.VisualStudio.TestPlatform.TestExecutor.TestExecutorMain::Run"
#define TESTEXECUTORMAIN_CTOR L"Microsoft.VisualStudio.TestPlatform.TestExecutor.TestExecutorMain::.ctor"

#define TESTTOOLS_UITESTING_ASSEMBLY L"Microsoft.VisualStudio.TestTools.UITesting"
#define APPLICATIONUNDERTEST_START L"Microsoft.VisualStudio.TestTools.UITesting.ApplicationUnderTest::Start"
#define APPLICATIONUNDERTEST_CCTOR L"Microsoft.VisualStudio.TestTools.UITesting.ApplicationUnderTest::.cctor"

#pragma warning (push)
#pragma warning (disable : 4471 4820)
#import <mscorlib.tlb> raw_interfaces_only, rename("ReportEvent", "ReportEvent2")
#pragma warning (pop)
using namespace mscorlib;

extern COpenCoverProfilerModule _AtlModule;

namespace {
	struct __declspec(uuid("2180EC45-CF11-456E-9A76-389A4521A4BE"))
	IDomainHelper : IUnknown
	{
		virtual HRESULT __stdcall AddResolveEventHandler() = 0;
	};
}

using namespace Instrumentation;

LPSAFEARRAY GetInjectedDllAsSafeArray()
{
	HINSTANCE hInst = _AtlModule.m_hModule;
	HRSRC hClrHookDllRes = FindResource(hInst, MAKEINTRESOURCE(IDR_SUPPORT), L"ASSEMBLY");
	ATLASSERT(hClrHookDllRes != NULL);

#pragma warning (suppress : 6387) // that's what the Assert() is all about
	HGLOBAL hClrHookDllHGlb = LoadResource(hInst, hClrHookDllRes);
	ATLASSERT(hClrHookDllHGlb != NULL);

#pragma warning (suppress : 6387) // that's what the Assert() is all about
	DWORD dllMemorySize = SizeofResource(hInst, hClrHookDllRes);

#pragma warning (suppress : 6387) // that's what the Assert() is all about
	const BYTE * const lpDllData = static_cast<LPBYTE>(LockResource(hClrHookDllHGlb));
	ATLASSERT(lpDllData != NULL);

	SAFEARRAYBOUND bound = { 0 };
	bound.cElements = dllMemorySize;
	bound.lLbound = 0;

	LPBYTE lpArrayData{ nullptr };
	LPSAFEARRAY lpAsmblyData = SafeArrayCreate(VT_UI1, 1, &bound);
	ATLASSERT(lpAsmblyData != NULL);

#pragma warning (suppress : 26490) // cast is simplest and safe
	SafeArrayAccessData(lpAsmblyData, reinterpret_cast<void**>(&lpArrayData));
	memcpy(lpArrayData, lpDllData, dllMemorySize);
	SafeArrayUnaccessData(lpAsmblyData);

	return lpAsmblyData;
}

EXTERN_C HRESULT STDAPICALLTYPE LoadOpenCoverSupportAssembly(IUnknown *pUnk)
{
	ATLTRACE(_T("****LoadInjectorAssembly - Start****"));

	CComPtr<_AppDomain> pAppDomain;
#pragma warning (suppress : 26490) // cast is simplest and safe
	HRESULT hr = pUnk->QueryInterface(__uuidof(_AppDomain), reinterpret_cast<void**>(&pAppDomain));
	ATLASSERT(hr == S_OK);
	LPSAFEARRAY lpAsmblyData = GetInjectedDllAsSafeArray();
	ATLASSERT(lpAsmblyData != NULL);

	CComPtr<_Assembly> pAssembly;
	hr = pAppDomain->Load_3(lpAsmblyData, &pAssembly);
	ATLASSERT(hr == S_OK);

	SafeArrayDestroy(lpAsmblyData);

	CComVariant variant;
	hr = pAssembly->CreateInstance(W2BSTR(L"OpenCover.Support.DomainHelper"), &variant);
	ATLASSERT(hr == S_OK);

	CComPtr<IDomainHelper> pDomainHelper;
#pragma warning (suppress : 26490) // cast is simplest and safe
	hr = variant.punkVal->QueryInterface(__uuidof(IDomainHelper), reinterpret_cast<void**>(&pDomainHelper));
	ATLASSERT(hr == S_OK);

    hr = pDomainHelper->AddResolveEventHandler();
	ATLASSERT(hr == S_OK);
	ATLTRACE(_T("****LoadInjectorAssembly - End****"));

	return S_OK;
}

HRESULT CCodeCoverage::OpenCoverSupportInitialize(
    /* [in] */ IUnknown *pICorProfilerInfoUnk) {
    TCHAR ext[1024] = { 0 };
    ::GetEnvironmentVariable(_T("CHAIN_EXTERNAL_PROFILER"), &ext[0], 1024);
    if (ext[0] != 0) {
        ATLTRACE(_T("::OpenCoverSupportInitialize"));

        ATLTRACE(_T("    ::OpenCoverSupportInitialize(...) => ext = %s"), &ext[0]);

        TCHAR loc[1024] = { 0 };
        ::GetEnvironmentVariable(_T("CHAIN_EXTERNAL_PROFILER_LOCATION"), &loc[0], 1024);
        ATLTRACE(_T("    ::OpenCoverSupportInitialize(...) => loc = %s"), &loc[0]);

        if (PathFileExists(&loc[0])) {
            CLSID clsid;
            HRESULT hr = CLSIDFromString(T2OLE(&ext[0]), &clsid);
            ATLASSERT(hr == S_OK);

            chained_module_ = LoadLibrary(&loc[0]);
            ATLASSERT(chained_module_ != NULL);

#pragma warning (suppress : 4191 26496 26493) // unsafe conversion from 'FARPROC' to 'HRESULT (__cdecl *)(const IID &,const IID &,LPVOID)'
            HRESULT(WINAPI*DllGetClassObject)(REFCLSID, REFIID, LPVOID) =
                (HRESULT(WINAPI*)(REFCLSID, REFIID, LPVOID))GetProcAddress(chained_module_, "DllGetClassObject");
            ATLASSERT(DllGetClassObject != NULL);

            CComPtr<IClassFactory> pClassFactory;
            hr = DllGetClassObject(clsid, IID_IClassFactory, &pClassFactory);
            ATLASSERT(hr == S_OK);

			CComPtr<ICorProfilerCallback> chainedProfiler;
#pragma warning (suppress : 26490) // cast is simplest and safe
			hr = pClassFactory->CreateInstance(nullptr, __uuidof(ICorProfilerCallback), reinterpret_cast<void**>(&chainedProfiler));
            ATLASSERT(hr == S_OK);

            /*HRESULT hr2 =*/ (void) CComObject<CProfilerInfo>::CreateInstance(&m_infoHook);
            /*ULONG count =*/ (void) m_infoHook->AddRef();

            m_infoHook->m_pProfilerHook = this;

            m_infoHook->SetProfilerInfo(pICorProfilerInfoUnk);

            hr = chainedProfiler->Initialize(m_infoHook);

			HookChainedProfiler(chainedProfiler);

            ATLTRACE(_T("    ::OpenCoverSupportInitialize => fakes = 0x%X"), hr);
        }
        else {
            RELTRACE(_T("    ::OpenCoverSupportInitialize => Failed to locate external profiler at '%s'"), &loc[0]);
        }
    }

    return S_OK;
}

mdMethodDef CCodeCoverage::CreatePInvokeHook(ModuleID moduleId){

	mdTypeDef	tkInjClass{};

    CComPtr<IMetaDataEmit> metaDataEmit;
#pragma warning (suppress : 26490) // cast is simplest and safe
	HRESULT hr = m_profilerInfo->GetModuleMetaData(moduleId,
        ofRead | ofWrite, IID_IMetaDataEmit, reinterpret_cast<IUnknown**>(&metaDataEmit));
    ATLASSERT(hr == S_OK);

    mdModuleRef mscorlibRef;
    hr = GetModuleRef(moduleId, MSCORLIB_NAME, mscorlibRef);
    COM_FAIL_MSG_RETURN_ERROR(mdMethodDef, hr, _T("    ::CreatePInvokeHook(...) => GetModuleRef => 0x%X"));

    mdTypeRef objectTypeRef;
    hr = metaDataEmit->DefineTypeRefByName(mscorlibRef,
        L"System.Object", &objectTypeRef);
    COM_FAIL_MSG_RETURN_ERROR(mdMethodDef, hr, _T("    ::CreatePInvokeHook(...) => DefineTypeRefByName => 0x%X"));

    hr = metaDataEmit->DefineTypeDef(L"__OpenCoverSupportInjection__", tdAbstract | tdSealed, objectTypeRef, NULL, &tkInjClass);
    ATLASSERT(hr == S_OK);

    static BYTE sg_sigPLoadInjectorAssembly[] = {
        0, // IMAGE_CEE_CS_CALLCONV_DEFAULT
        1, // argument count
        ELEMENT_TYPE_VOID, // ret = ELEMENT_TYPE_VOID
        ELEMENT_TYPE_OBJECT
    };

    mdModuleRef	tkRefClrProbe;
    hr = metaDataEmit->DefineModuleRef(L"OPENCOVER.PROFILER.DLL", &tkRefClrProbe);
    ATLASSERT(hr == S_OK);

    mdMethodDef	tkAttachDomain;
    metaDataEmit->DefineMethod(tkInjClass, L"LoadOpenCoverSupportAssembly",
        mdStatic | mdPinvokeImpl,
        &sg_sigPLoadInjectorAssembly[0], sizeof(sg_sigPLoadInjectorAssembly),
        0, 0, &tkAttachDomain
        );
    ATLASSERT(hr == S_OK);

    BYTE tiunk = NATIVE_TYPE_IUNKNOWN;
	mdParamDef paramDef{};
    hr = metaDataEmit->DefinePinvokeMap(tkAttachDomain, 0, L"LoadOpenCoverSupportAssembly", tkRefClrProbe);
    ATLASSERT(hr == S_OK);

    hr = metaDataEmit->DefineParam(tkAttachDomain, 1, L"appDomain",
        pdIn | pdHasFieldMarshal, 0, NULL, 0, &paramDef);
    ATLASSERT(hr == S_OK);

    hr = metaDataEmit->SetFieldMarshal(paramDef, &tiunk, 1);
    ATLASSERT(hr == S_OK);

    return tkAttachDomain;
}

mdMethodDef CCodeCoverage::Get_CurrentDomainMethod(ModuleID moduleID)
{
    CComPtr<IMetaDataEmit> metaDataEmit;
    COM_FAIL_MSG_RETURN_OTHER(m_profilerInfo->GetModuleMetaData(moduleID, ofWrite, IID_IMetaDataEmit, (IUnknown**)&metaDataEmit), 0,
        _T("    ::Get_CurrentDomainMethod(ModuleID) => GetModuleMetaData => 0x%X"));

    mdModuleRef mscorlibRef;
    HRESULT hr = GetModuleRef(moduleID, MSCORLIB_NAME, mscorlibRef);
    ATLASSERT(hr == S_OK);

	mdMethodDef getCurrentDomain{};
    mdTypeDef tkAppDomain;
    hr = metaDataEmit->DefineTypeRefByName(mscorlibRef, L"System.AppDomain", &tkAppDomain);
    ATLASSERT(hr == S_OK);

    BYTE importSig[] = { IMAGE_CEE_CS_CALLCONV_DEFAULT, 0 /*<no args*/, 0x12 /*< ret class*/, 0, 0, 0, 0, 0 };
    ULONG l = CorSigCompressToken(tkAppDomain, &importSig[3]);	//< Add the System.AppDomain token ref
    hr = metaDataEmit->DefineMemberRef(tkAppDomain, L"get_CurrentDomain", &importSig[0], 3 + l, &getCurrentDomain);
    ATLASSERT(hr == S_OK);
    return getCurrentDomain;
}

HRESULT CCodeCoverage::GetOpenCoverSupportRef(ModuleID moduleId, mdModuleRef &supportRef)
{
	// get interfaces
	CComPtr<IMetaDataEmit2> metaDataEmit;
#pragma warning (suppress : 26490) // cast is simplest and safe
	HRESULT hr = m_profilerInfo->GetModuleMetaData(moduleId,
		ofRead | ofWrite, IID_IMetaDataEmit2, reinterpret_cast<IUnknown**>(&metaDataEmit));
	ATLASSERT(hr == S_OK);

	CComPtr<IMetaDataAssemblyEmit> metaDataAssemblyEmit;
#pragma warning (suppress : 26490) // cast is simplest and safe
	hr = metaDataEmit->QueryInterface(
		IID_IMetaDataAssemblyEmit, reinterpret_cast<void**>(&metaDataAssemblyEmit));
	ATLASSERT(hr == S_OK);

	// find injected
	ASSEMBLYMETADATA assembly;
	ZeroMemory(&assembly, sizeof(assembly));
	assembly.usMajorVersion = 1;
	assembly.usMinorVersion = 0;
	assembly.usBuildNumber = 0;
	assembly.usRevisionNumber = 0;
	const BYTE pubToken[] = { 0xe1, 0x91, 0x8c, 0xac, 0x69, 0xeb, 0x73, 0xf4 };
	hr = metaDataAssemblyEmit->DefineAssemblyRef(&pubToken[0],
		sizeof(pubToken), L"OpenCover.Support", &assembly, NULL, 0, 0,
		&supportRef);
	ATLASSERT(hr == S_OK);
	return hr;
}

HRESULT CCodeCoverage::OpenCoverSupportCompilation(FunctionID functionId, mdToken functionToken, ModuleID moduleId, AssemblyID assemblyId, std::wstring & /*modulePath*/)
{
    InstrumentTestPlatformUtilities(functionId, functionToken, moduleId, assemblyId);
    InstrumentTestPlatformTestExecutor(functionId, functionToken, moduleId, assemblyId);
    InstrumentTestToolsUITesting(functionId, functionToken, moduleId, assemblyId);

    return S_OK;
}

bool CCodeCoverage::OpenCoverSupportRequired(AssemblyID assemblyId, FunctionID functionId)
{
    std::wstring assemblyName = GetAssemblyName(assemblyId);
    if ((TESTPLATFORM_UTILITIES_ASSEMBLY != assemblyName) &&
        (TESTPLATFORM_TESTEXECUTOR_CORE_ASSEMBLY != assemblyName) &&
        (TESTTOOLS_UITESTING_ASSEMBLY != assemblyName))
        return false;

    std::wstring typeMethodName = GetTypeAndMethodName(functionId);
    if ((TESTPLATFORM_UTILITIES_ASSEMBLY == assemblyName) &&
        (DEFAULTTESTEXECUTOR_CTOR != typeMethodName) &&
        (DEFAULTTESTEXECUTOR_LAUNCHPROCESS != typeMethodName))
        return false;

    if ((TESTPLATFORM_TESTEXECUTOR_CORE_ASSEMBLY == assemblyName) &&
        (TESTEXECUTORMAIN_CTOR != typeMethodName) &&
        (TESTEXECUTORMAIN_RUN != typeMethodName))
        return false;

    if ((TESTTOOLS_UITESTING_ASSEMBLY == assemblyName) &&
        (APPLICATIONUNDERTEST_CCTOR != typeMethodName) &&
        (APPLICATIONUNDERTEST_START != typeMethodName))
        return false;

    return true;
}

mdMethodDef CCodeCoverage::GetFakesHelperMethodRef(TCHAR* methodName, ModuleID moduleId) {
	return GetHelperMethodRef(L"OpenCover.Support.Fakes.FakesHelper", methodName, moduleId);
}

mdMethodDef CCodeCoverage::GetHelperMethodRef(const wchar_t * helperSource, TCHAR* methodName, ModuleID moduleId){
    // get reference to injected
    mdModuleRef injectedRef;
    HRESULT hr = GetOpenCoverSupportRef(moduleId, injectedRef);
    ATLASSERT(hr == S_OK);

    // get interfaces
    CComPtr<IMetaDataEmit> metaDataEmit;
#pragma warning (suppress : 26490) // cast is simplest and safe
	hr = m_profilerInfo->GetModuleMetaData(moduleId,
        ofRead | ofWrite, IID_IMetaDataEmit, reinterpret_cast<IUnknown**>(&metaDataEmit));
    ATLASSERT(hr == S_OK);

    static COR_SIGNATURE methodCallSignature[] =
    {
        IMAGE_CEE_CS_CALLCONV_DEFAULT,
        0x01,
        ELEMENT_TYPE_VOID,
        ELEMENT_TYPE_OBJECT
    };

    // get method to call
    mdTypeRef classTypeRef;
    hr = metaDataEmit->DefineTypeRefByName(injectedRef,
		helperSource, &classTypeRef);
    ATLASSERT(hr == S_OK);

    // L"LoadOpenCoverProfilerInstead"
    mdMemberRef memberRef;
    hr = metaDataEmit->DefineMemberRef(classTypeRef,
        T2W(methodName), &methodCallSignature[0],
        sizeof(methodCallSignature), &memberRef);
    ATLASSERT(hr == S_OK);

    return memberRef;
}

mdMethodDef CCodeCoverage::GetUITestingHelperMethodRef(TCHAR* methodName, ModuleID moduleId){
	return GetHelperMethodRef(L"OpenCover.Support.UITesting.UITestingHelper", methodName, moduleId);
}

void CCodeCoverage::InstrumentTestToolsUITesting(FunctionID functionId, mdToken functionToken, ModuleID moduleId, AssemblyID assemblyId)
{
    if (TESTTOOLS_UITESTING_ASSEMBLY == GetAssemblyName(assemblyId))
    {
        std::wstring typeMethodName = GetTypeAndMethodName(functionId);

        if (APPLICATIONUNDERTEST_CCTOR == typeMethodName)
        {
            ATLTRACE(_T("::InstrumentTestToolsUITesting(%X, ...) => %d, %X => %s"), functionId, functionToken, moduleId, W2CT(typeMethodName.c_str()));

            mdMethodDef invokeAttach = CreatePInvokeHook(moduleId);
            InstructionList instructions;

            mdMethodDef getCurrentDomain = Get_CurrentDomainMethod(moduleId);
#pragma warning (disable : 26409) // new being used in a controlled fashion
			instructions.push_back(new Instruction(CEE_CALL, getCurrentDomain));
            instructions.push_back(new Instruction(CEE_CALL, invokeAttach));
#pragma warning (default : 26409)

            InstrumentMethodWith(moduleId, functionToken, instructions);
        }

        if (APPLICATIONUNDERTEST_START == typeMethodName)
        {
            ATLTRACE(_T("::InstrumentTestToolsUITesting(%X, ...) => %d, %X => %s"), functionId, functionToken, moduleId, W2CT(typeMethodName.c_str()));

            mdMemberRef memberRef = GetUITestingHelperMethodRef(_T("PropagateRequiredEnvironmentVariables"), moduleId);
            InstructionList instructions;

#pragma warning (disable : 26409) // new being used in a controlled fashion
			instructions.push_back(new Instruction(CEE_LDARG, 1));
            instructions.push_back(new Instruction(CEE_CALL, memberRef));
#pragma warning (default : 26409)

            InstrumentMethodWith(moduleId, functionToken, instructions);
        }
    }
}

void CCodeCoverage::InstrumentTestPlatformUtilities(FunctionID functionId, mdToken functionToken, ModuleID moduleId, AssemblyID assemblyId)
{
    if (TESTPLATFORM_UTILITIES_ASSEMBLY == GetAssemblyName(assemblyId))
    {
        std::wstring typeMethodName = GetTypeAndMethodName(functionId);

        if (DEFAULTTESTEXECUTOR_CTOR == typeMethodName)
        {
            ATLTRACE(_T("::InstrumentTestPlatformUtilities(%X, ...) => %d, %X => %s"), functionId, functionToken, moduleId, W2CT(typeMethodName.c_str()));

            mdMethodDef invokeAttach = CreatePInvokeHook(moduleId);
            InstructionList instructions;

            mdMethodDef getCurrentDomain = Get_CurrentDomainMethod(moduleId);
#pragma warning (disable : 26409) // new being used in a controlled fashion
			instructions.push_back(new Instruction(CEE_CALL, getCurrentDomain));
            instructions.push_back(new Instruction(CEE_CALL, invokeAttach));
#pragma warning (default : 26409)

            InstrumentMethodWith(moduleId, functionToken, instructions);
        }

        if (DEFAULTTESTEXECUTOR_LAUNCHPROCESS == typeMethodName)
        {
            ATLTRACE(_T("::InstrumentTestPlatformUtilities(%X, ...) => %d, %X => %s"), functionId, functionToken, moduleId, W2CT(typeMethodName.c_str()));

            mdMemberRef memberRef = GetFakesHelperMethodRef(_T("LoadOpenCoverProfilerInstead"), moduleId);
            InstructionList instructions;

#pragma warning (disable : 26409) // new being used in a controlled fashion
			instructions.push_back(new Instruction(CEE_LDARG_S, 4));
            instructions.push_back(new Instruction(CEE_CALL, memberRef));
#pragma warning (default : 26409)

            InstrumentMethodWith(moduleId, functionToken, instructions);
        }
    }
}

void CCodeCoverage::InstrumentTestPlatformTestExecutor(FunctionID functionId, mdToken functionToken, ModuleID moduleId, AssemblyID assemblyId)
{
    if (TESTPLATFORM_TESTEXECUTOR_CORE_ASSEMBLY == GetAssemblyName(assemblyId))
    {
        std::wstring typeMethodName = GetTypeAndMethodName(functionId);

        if (TESTEXECUTORMAIN_CTOR == typeMethodName)
        {
            ATLTRACE(_T("::InstrumentTestPlatformTestExecutor(%X, ...) => %d, %X => %s"), functionId, functionToken, moduleId, W2CT(typeMethodName.c_str()));

            mdMethodDef invokeAttach = CreatePInvokeHook(moduleId);

            InstructionList instructions;

            mdMethodDef getCurrentDomain = Get_CurrentDomainMethod(moduleId);
#pragma warning (disable : 26409) // new being used in a controlled fashion
			instructions.push_back(new Instruction(CEE_CALL, getCurrentDomain));
            instructions.push_back(new Instruction(CEE_CALL, invokeAttach));
#pragma warning (default : 26409)

            InstrumentMethodWith(moduleId, functionToken, instructions);
        }

        if (TESTEXECUTORMAIN_RUN == typeMethodName)
        {
            ATLTRACE(_T("::InstrumentTestPlatformTestExecutor(%X, ...) => %d, %X => %s"), functionId, functionToken, moduleId, W2CT(typeMethodName.c_str()));

            mdMemberRef memberRef = GetFakesHelperMethodRef(_T("PretendWeLoadedFakesProfiler"), moduleId);
            InstructionList instructions;

#pragma warning (disable : 26409) // new being used in a controlled fashion
			instructions.push_back(new Instruction(CEE_LDARG, 0));
            instructions.push_back(new Instruction(CEE_CALL, memberRef));
#pragma warning (default : 26409)

            InstrumentMethodWith(moduleId, functionToken, instructions);
        }
    }
}