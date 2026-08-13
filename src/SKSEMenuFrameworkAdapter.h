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
    using TextDisabledFunction = void (*)(const char*, ...);
    using SeparatorTextFunction = void (*)(const char*);
    using SeparatorFunction = void (*)();

    [[nodiscard]] inline HMODULE GetModule() noexcept
    {
        static HMODULE module = nullptr;
        if (!module) {
            module = GetModuleHandleW(L"SKSEMenuFramework.dll");
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

    inline void Description(const char* a_text) noexcept
    {
        static TextDisabledFunction function = nullptr;
        if (!function) {
            function = Resolve<TextDisabledFunction>("igTextDisabled");
        }

        if (function) {
            function("%s", a_text);
        }
    }

    inline void SectionSeparator(const char* a_label) noexcept
    {
        static SeparatorTextFunction separatorText = nullptr;
        static SeparatorFunction separator = nullptr;
        if (!separatorText) {
            separatorText = Resolve<SeparatorTextFunction>("igSeparatorText");
        }
        if (separatorText) {
            separatorText(a_label);
            return;
        }

        if (!separator) {
            separator = Resolve<SeparatorFunction>("igSeparator");
        }
        if (separator) {
            separator();
        }
    }

    [[nodiscard]] inline bool IsCheckboxAvailable() noexcept
    {
        return Resolve<CheckboxFunction>("igCheckbox") != nullptr;
    }

    [[nodiscard]] inline bool IsDescriptionAvailable() noexcept
    {
        return Resolve<TextDisabledFunction>("igTextDisabled") != nullptr;
    }

    [[nodiscard]] inline bool IsSeparatorAvailable() noexcept
    {
        return Resolve<SeparatorTextFunction>("igSeparatorText") != nullptr ||
               Resolve<SeparatorFunction>("igSeparator") != nullptr;
    }
}
