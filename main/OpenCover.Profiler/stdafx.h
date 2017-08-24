//
// OpenCover - S Wilde
//
// This source code is released under the MIT License; see the accompanying license file.
//
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef STRICT
#define STRICT
#endif

#pragma warning (disable : 4710) // method not inlined
#pragma warning (disable : 4711) // method selected to be inlined
#pragma warning (disable : 4514) // unreferenced inline : minwinbase.h(434):/* Leave 4514 disabled.  It's an unneeded warning anyway. */

#pragma warning(push)
// system header files are very noisy
#pragma warning (disable : 4191 4263 4264 4355 4365 4458 4571 4623 4625 4626 4668 4755 4774 4820 4987 5026 5027 26461 26481 26493 26495)

#include "targetver.h"

#define _ATL_APARTMENT_THREADED

#define _ATL_NO_AUTOMATIC_NAMESPACE

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

#define ATL_NO_ASSERT_ON_DESTROY_NONEXISTENT_WINDOW

#include "resource.h"
#include <range/v3/all.hpp>

#include <gsl.h>
#pragma warning (suppress : 5031)// work around bug in gsl.h
#pragma warning(pop)             // work around bug in gsl.h -- span.h doesn't close its push

#include <atlbase.h>
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
#include <algorithm>

#pragma warning(pop)

#ifdef UNICODE
#define tstring std::wstring
#else
#define tstring std::string
#endif

#include "ReleaseTrace.h"