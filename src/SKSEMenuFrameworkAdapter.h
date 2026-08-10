#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Windows.h>

namespace PTMI::MenuFramework
{
    using RenderFunction = void(__stdcall*)();
    using AddSectionItemFunction = void (*)(const char*, RenderFunction);
    using CheckboxFunction = bool (*)(const char*, bool*);

    [[nodiscard]] inline HMODULE GetModule() noexcept
    {
        static HMODULE module = nullptr;
        if (!module) {
            module = GetModuleHandleW(L"SKSEMenuFramework");
        }
        return module;
    }

    template <class T>
    [[nodiscard]] T Resolve(const char* a_name) noexcept
    {
        const auto module = GetModule();
        return module ? reinterpret_cast<T>(GetProcAddress(module, a_name)) : nullptr;
    }

    [[nodiscard]] inline bool IsLoaded() noexcept
    {
        return GetModule() != nullptr;
    }

    [[nodiscard]] inline bool AddSectionItem(const char* a_path, RenderFunction a_renderer) noexcept
    {
        static AddSectionItemFunction function = nullptr;
        if (!function) {
            function = Resolve<AddSectionItemFunction>("AddSectionItem");
        }
        if (!function) {
            return false;
        }

        function(a_path, a_renderer);
        return true;
    }

    [[nodiscard]] inline bool Checkbox(const char* a_label, bool* a_value) noexcept
    {
        static CheckboxFunction function = nullptr;
        if (!function) {
            function = Resolve<CheckboxFunction>("igCheckbox");
        }
        return function ? function(a_label, a_value) : false;
    }
}
