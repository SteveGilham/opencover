// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#pragma warning (disable : 4710) // method not inlined
#pragma warning (disable : 4711) // method selected to be inlined
#pragma warning (disable : 4514) // unreferenced inline : minwinbase.h(434):/* Leave 4514 disabled.  It's an unneeded warning anyway. */

#pragma warning(push)
// system header files are very noisy
#pragma warning (disable : 4191 4263 4264 4355 4365 4458 4571 4623 4625 4626 4628 4668 4755 4774 4820 4826 4987 5026 5027 5038 )

#include "targetver.h"

#define _CRT_SECURE_NO_WARNINGS

#include <gsl.h>
#pragma warning (suppress : 5031)// work around bug in gsl.h
#pragma warning(pop)             // work around bug in gsl.h -- span.h doesn't close its push

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include <stdio.h>
#include <tchar.h>

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

#include <atlbase.h>
#include <atlstr.h>
#include <atlcom.h>
#include <atlctl.h>

#pragma pack(push)
#pragma pack(4)

#include <cor.h>
#include <corsym.h>
#include <corprof.h>
#include <corhlpr.h>

#pragma pack(pop)

#include <string>
#include <vector>
#include <unordered_map>

// This line ensures that gmock.h can be compiled on its own, even
// when it's fused.
#include <gmock/gmock.h>

// This line ensures that gtest.h can be compiled on its own, even
// when it's fused.
#include <gtest/gtest.h>

#pragma warning(pop)
/*
#define _TEST_METHOD_EX_EXPANDER(_testMethod)\
_testMethod { try

// Adds support for seeing std::exception in test output. Requires TEST_METHOD_EX_END after test.
// Example:
// TEST_METHOD_EX_BEGIN(MyFailingTest){ throw std::exception("What happened"); } TEST_METHOD_EX_END;
#define TEST_METHOD_EX_BEGIN(_methodName) _TEST_METHOD_EX_EXPANDER(TEST_METHOD(_methodName))

// Use following test declared with TEST_METHOD_EX_BEGIN
#define TEST_METHOD_EX_END\
catch (::std::exception& ex) \
{ \
::std::wstringstream ws; ws << "Unhandled Exception:" << ::std::endl << ex.what(); \
::Microsoft::VisualStudio::CppUnitTestFramework::Assert::Fail(ws.str().c_str());\
} \
}

#define _TEST_METHOD_CLEANUP_EX_EXPANDER(_testCleanUp)\
_testCleanUp { try

// Adds support for seeing std::exception in test output. Requires TEST_METHOD_CLEANUP_EX_END after CleanUp.
// Example:
// TEST_METHOD_CLEANUP_EX_BEGIN(CleanUp){ throw std::exception("What happened"); } TEST_METHOD_CLEANUP_EX_END;
#define TEST_METHOD_CLEANUP_EX_BEGIN(_methodName) _TEST_METHOD_CLEANUP_EX_EXPANDER(TEST_METHOD_CLEANUP(_methodName))

// Use following test declared with TEST_METHOD_CLEANUP_EX_BEGIN
#define TEST_METHOD_CLEANUP_EX_END\
catch (::std::exception& ex) \
{ \
::std::wstringstream ws; ws << "Unhandled Exception:" << ::std::endl << ex.what(); \
::Microsoft::VisualStudio::CppUnitTestFramework::Assert::Fail(ws.str().c_str());\
} \
}

*/
