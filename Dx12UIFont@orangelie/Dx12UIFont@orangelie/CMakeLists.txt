cmake_minimum_required (VERSION 3.8)


set(DEPENDENCIES "Dependencies/d3dx12.h" "Dependencies/DDSTextureLoader.cpp" "Dependencies/DDSTextureLoader.h")
set(UTILITIES "Utils.cpp" "Utils.h")
set(DXGI_RENDERER "Renderer.cpp" "Renderer.h")
set(GAMETIMER "GameTimer.h" "GameTimer.cpp")

add_executable (ZekrosEngine "main.cpp" "Dx12UIFont.h" ${DEPENDENCIES} ${DXGI_RENDERER} ${UTILITIES} ${GAMETIMER})

include_directories(ZekrosEngine "./Dependencies")