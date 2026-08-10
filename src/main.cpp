#include "PCH.h"
#include "SKSEMenuFrameworkAdapter.h"

namespace
{
    constexpr std::string_view kPluginName = "PrismaUITeleportMenuIntegration.esp";
    constexpr std::string_view kModEventName = "PTMI_OpenTeleportMenu";

    constexpr RE::FormID kCarriagesGlobalID = 0x809;
    constexpr RE::FormID kFerriesGlobalID = 0x80A;
    constexpr RE::FormID kInnkeepersGlobalID = 0x80B;
    constexpr RE::FormID kCourtWizardsGlobalID = 0x80C;

    using TeleportOpen = bool (*)();
    TeleportOpen g_openTeleportMenu = nullptr;

    RE::TESGlobal* g_enableCarriages = nullptr;
    RE::TESGlobal* g_enableFerries = nullptr;
    RE::TESGlobal* g_enableInnkeepers = nullptr;
    RE::TESGlobal* g_enableCourtWizards = nullptr;

    void InitializeLog()
    {
        auto logDirectory = SKSE::log::log_directory();
        if (!logDirectory) {
            return;
        }

        *logDirectory /= "PrismaUITeleportMenuIntegration.log";
        auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logDirectory->string(), true);
        auto logger = std::make_shared<spdlog::logger>("global log", std::move(sink));

        spdlog::set_default_logger(std::move(logger));
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
        spdlog::set_level(spdlog::level::warn);
        spdlog::flush_on(spdlog::level::warn);
    }

    [[nodiscard]] const std::wstring& GetSettingsPath()
    {
        static const std::wstring path = [] {
            wchar_t executablePath[MAX_PATH]{};
            const auto length = GetModuleFileNameW(nullptr, executablePath, MAX_PATH);
            if (length == 0 || length == MAX_PATH) {
                return std::wstring{ L"Data\\SKSE\\Plugins\\PrismaUITeleportMenuIntegration.ini" };
            }

            auto result = std::filesystem::path(executablePath).parent_path();
            result /= L"Data";
            result /= L"SKSE";
            result /= L"Plugins";
            result /= L"PrismaUITeleportMenuIntegration.ini";
            return result.wstring();
        }();

        return path;
    }

    [[nodiscard]] bool ReadSetting(const wchar_t* a_key, bool a_default)
    {
        return GetPrivateProfileIntW(
                   L"DialogueOptions",
                   a_key,
                   a_default ? 1 : 0,
                   GetSettingsPath().c_str()) != 0;
    }

    void WriteSetting(const wchar_t* a_key, bool a_enabled)
    {
        if (!WritePrivateProfileStringW(
                L"DialogueOptions",
                a_key,
                a_enabled ? L"1" : L"0",
                GetSettingsPath().c_str())) {
            spdlog::warn("INI write failed");
        }
    }

    void SetGlobal(RE::TESGlobal* a_global, bool a_enabled)
    {
        if (a_global) {
            a_global->value = a_enabled ? 1.0F : 0.0F;
        }
    }

    void ApplyDialogueSettings()
    {
        SetGlobal(g_enableCarriages, ReadSetting(L"Carriages", true));
        SetGlobal(g_enableFerries, ReadSetting(L"Ferries", true));
        SetGlobal(g_enableInnkeepers, ReadSetting(L"Innkeepers", false));
        SetGlobal(g_enableCourtWizards, ReadSetting(L"CourtWizards", false));
    }

    void ResolveDialogueGlobals()
    {
        auto* dataHandler = RE::TESDataHandler::GetSingleton();
        if (!dataHandler) {
            spdlog::error("TESDataHandler unavailable");
            return;
        }

        g_enableCarriages = dataHandler->LookupForm<RE::TESGlobal>(kCarriagesGlobalID, kPluginName);
        g_enableFerries = dataHandler->LookupForm<RE::TESGlobal>(kFerriesGlobalID, kPluginName);
        g_enableInnkeepers = dataHandler->LookupForm<RE::TESGlobal>(kInnkeepersGlobalID, kPluginName);
        g_enableCourtWizards = dataHandler->LookupForm<RE::TESGlobal>(kCourtWizardsGlobalID, kPluginName);

        ApplyDialogueSettings();
    }

    void RenderSetting(const char* a_label, const wchar_t* a_iniKey, RE::TESGlobal* a_global)
    {
        if (!a_global) {
            return;
        }

        bool enabled = a_global->value >= 0.5F;
        if (PTMI::MenuFramework::Checkbox(a_label, std::addressof(enabled))) {
            SetGlobal(a_global, enabled);
            WriteSetting(a_iniKey, enabled);
        }
    }

    void __stdcall RenderDialogueSettings()
    {
        RenderSetting("Carriages", L"Carriages", g_enableCarriages);
        RenderSetting("Ferries", L"Ferries", g_enableFerries);
        RenderSetting("Innkeepers", L"Innkeepers", g_enableInnkeepers);
        RenderSetting("Court Wizards", L"CourtWizards", g_enableCourtWizards);
    }

    void RegisterMenuFrameworkSettings()
    {
        if (PTMI::MenuFramework::IsLoaded()) {
            (void)PTMI::MenuFramework::AddSectionItem(
                "PrismaUI Teleport Menu Integration/Dialogue Options",
                RenderDialogueSettings);
        }
    }

    void ResolveTeleportOpen()
    {
        const auto module = GetModuleHandleW(L"PrismaUITeleportMenu.dll");
        if (module) {
            g_openTeleportMenu = reinterpret_cast<TeleportOpen>(GetProcAddress(module, "Teleport_Open"));
        }

        if (!g_openTeleportMenu) {
            spdlog::warn("Teleport API unavailable");
        }
    }

    class ModCallbackSink final : public RE::BSTEventSink<SKSE::ModCallbackEvent>
    {
    public:
        RE::BSEventNotifyControl ProcessEvent(
            const SKSE::ModCallbackEvent* a_event,
            RE::BSTEventSource<SKSE::ModCallbackEvent>*) override
        {
            if (!a_event || !a_event->eventName.c_str()) {
                return RE::BSEventNotifyControl::kContinue;
            }

            if (std::strcmp(a_event->eventName.c_str(), kModEventName.data()) == 0 && g_openTeleportMenu) {
                g_openTeleportMenu();
            }

            return RE::BSEventNotifyControl::kContinue;
        }
    };

    ModCallbackSink g_modCallbackSink;

    void OnDataLoaded()
    {
        ResolveDialogueGlobals();
        ResolveTeleportOpen();

        auto* eventSource = SKSE::GetModCallbackEventSource();
        if (!eventSource) {
            spdlog::error("Mod callback source unavailable");
            return;
        }

        eventSource->AddEventSink(std::addressof(g_modCallbackSink));
    }

    void MessageHandler(SKSE::MessagingInterface::Message* a_message)
    {
        if (!a_message) {
            return;
        }

        switch (a_message->type) {
        case SKSE::MessagingInterface::kPostLoad:
            RegisterMenuFrameworkSettings();
            break;
        case SKSE::MessagingInterface::kDataLoaded:
            OnDataLoaded();
            break;
        case SKSE::MessagingInterface::kPostLoadGame:
        case SKSE::MessagingInterface::kNewGame:
            ApplyDialogueSettings();
            break;
        default:
            break;
        }
    }
}

SKSEPluginLoad(const SKSE::LoadInterface* a_skse)
{
    SKSE::Init(a_skse);
    InitializeLog();

    auto* messaging = SKSE::GetMessagingInterface();
    if (!messaging || !messaging->RegisterListener(MessageHandler)) {
        spdlog::critical("Messaging registration failed");
        return false;
    }

    return true;
}
