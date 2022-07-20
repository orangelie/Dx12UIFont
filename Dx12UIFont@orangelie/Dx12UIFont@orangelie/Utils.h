#pragma once
#pragma comment(linker, "/SUBSYSTEM:WINDOWS")
#pragma comment(lib, "d3d12")
#pragma comment(lib, "dxgi")
#pragma comment(lib, "dxguid")
#pragma comment(lib, "d3dcompiler")

#include <fstream>
#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <cassert>

#include <Windows.h>
#include <windowsx.h>
#include <wrl.h>

#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi1_4.h>

#define CLASSIFICATION_H(n) n(); \
n(const n&) = delete; \
n operator=(const n) = delete; \
virtual ~n();

#define CLASSIFICATION_C1(n) n::n() {} \
n::~n() {}

#define CLASSIFICATION_C2(n, e) n::n() e \
n::~n() {}

#define HR(n) { \
	if(FAILED(n)) \
	{ \
		std::string err = std::string(__FILE__) + std::string(__FUNCTION__) + std::to_string(__LINE__); \
		throw std::runtime_error(err); \
	} \
}

template <class _Tp>
_Tp& unmove(_Tp&& __value)
{
	return __value;
}