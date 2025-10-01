/*
 * UIAList - Accessibility Tool for Screen Reader Users
 * Copyright (C) 2025 Stefan Lohmaier
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#pragma once

#include "pch.h"

namespace UIAList
{
    // Settings manager using Windows Registry
    // Replaces Qt's QSettings
    class SettingsManager
    {
    public:
        static SettingsManager& GetInstance();

        // Settings
        int GetDefaultAction();  // 0=Click, 1=DoubleClick, 2=Focus
        void SetDefaultAction(int action);

        bool GetAutoStart();
        void SetAutoStart(bool enabled);

        winrt::hstring GetHotkeyString();
        void SetHotkeyString(const winrt::hstring& hotkey);

        bool GetWelcomeShown();
        void SetWelcomeShown(bool shown);

    private:
        SettingsManager() = default;
        ~SettingsManager() = default;
        SettingsManager(const SettingsManager&) = delete;
        SettingsManager& operator=(const SettingsManager&) = delete;

        // Registry helpers
        DWORD ReadDWORD(const wchar_t* valueName, DWORD defaultValue);
        void WriteDWORD(const wchar_t* valueName, DWORD value);
        winrt::hstring ReadString(const wchar_t* valueName, const wchar_t* defaultValue);
        void WriteString(const wchar_t* valueName, const wchar_t* value);

        static constexpr const wchar_t* REG_KEY_PATH = L"Software\\UIAList\\Settings";
        static constexpr const wchar_t* REG_AUTOSTART_PATH = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
        static constexpr const wchar_t* REG_AUTOSTART_NAME = L"UIAList";
    };
}
