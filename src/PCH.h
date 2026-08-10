#pragma once

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Windows.h>

#include <spdlog/sinks/basic_file_sink.h>

#include <cstring>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>

using namespace std::literals;
