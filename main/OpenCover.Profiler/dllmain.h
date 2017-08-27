//
// OpenCover - S Wilde
//
// This source code is released under the MIT License; see the accompanying license file.
//
// dllmain.h : Declaration of module class.

class COpenCoverProfilerModule : public ATL::CAtlDllModuleT< COpenCoverProfilerModule >
{
public :
	DECLARE_LIBID(LIBID_OpenCoverProfilerLib)
#pragma warning (suppress : 26485 26497) // postential constexpr in macro
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_OPENCOVERPROFILER, "{0A09C7B0-D778-49CF-8EE7-5C7145885ABF}")
	HINSTANCE m_hModule { nullptr };
};

extern class COpenCoverProfilerModule _AtlModule;
