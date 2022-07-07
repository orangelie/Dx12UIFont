#pragma once
#pragma comment(linker, "/SUBSYSTEM:WINDOWS")

#include <fstream>
#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>

#include <Windows.h>
#include <windowsx.h>

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