/*
 * UIAList - Accessibility Tool for Screen Reader Users
 * Copyright (C) 2025 Stefan Lohmaier
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include "pch.h"
#include "SettingsManager.h"

namespace winrt::UIAList::implementation
{
    SettingsManager& SettingsManager::GetInstance()
    {
        static SettingsManager instance;
        return instance;
    }

    int SettingsManager::GetDefaultAction()
    {
        return static_cast<int>(ReadDWORD(L"defaultAction", 0));  // 0 = Click by default
    }

    void SettingsManager::SetDefaultAction(int action)
    {
        WriteDWORD(L"defaultAction", static_cast<DWORD>(action));
    }

    bool SettingsManager::GetAutoStart()
    {
        HKEY hKey;
        LONG result = RegOpenKeyExW(HKEY_CURRENT_USER, REG_AUTOSTART_PATH, 0, KEY_QUERY_VALUE, &hKey);
        if (result != ERROR_SUCCESS) return false;

        DWORD dataSize = 0;
        result = RegQueryValueExW(hKey, REG_AUTOSTART_NAME, nullptr, nullptr, nullptr, &dataSize);
        RegCloseKey(hKey);

        return (result == ERROR_SUCCESS && dataSize > 0);
    }

    void SettingsManager::SetAutoStart(bool enabled)
    {
        HKEY hKey;
        LONG result = RegOpenKeyExW(HKEY_CURRENT_USER, REG_AUTOSTART_PATH, 0, KEY_SET_VALUE | KEY_QUERY_VALUE, &hKey);
        if (result != ERROR_SUCCESS) return;

        if (enabled)
        {
            // Get current executable path
            wchar_t exePath[MAX_PATH];
            GetModuleFileNameW(nullptr, exePath, MAX_PATH);

            result = RegSetValueExW(hKey, REG_AUTOSTART_NAME, 0, REG_SZ,
                                   (const BYTE*)exePath,
                                   (wcslen(exePath) + 1) * sizeof(wchar_t));
        }
        else
        {
            RegDeleteValueW(hKey, REG_AUTOSTART_NAME);
        }

        RegCloseKey(hKey);
    }

    winrt::hstring SettingsManager::GetHotkeyString()
    {
        return ReadString(L"shortcutKey", L"Ctrl+Alt+U");
    }

    void SettingsManager::SetHotkeyString(const winrt::hstring& hotkey)
    {
        WriteString(L"shortcutKey", hotkey.c_str());
    }

    bool SettingsManager::GetWelcomeShown()
    {
        return ReadDWORD(L"welcomeShown", 0) != 0;
    }

    void SettingsManager::SetWelcomeShown(bool shown)
    {
        WriteDWORD(L"welcomeShown", shown ? 1 : 0);
    }

    DWORD SettingsManager::ReadDWORD(const wchar_t* valueName, DWORD defaultValue)
    {
        HKEY hKey;
        LONG result = RegOpenKeyExW(HKEY_CURRENT_USER, REG_KEY_PATH, 0, KEY_QUERY_VALUE, &hKey);
        if (result != ERROR_SUCCESS) return defaultValue;

        DWORD value = defaultValue;
        DWORD dataSize = sizeof(DWORD);
        DWORD type;

        result = RegQueryValueExW(hKey, valueName, nullptr, &type, (LPBYTE)&value, &dataSize);
        RegCloseKey(hKey);

        if (result != ERROR_SUCCESS || type != REG_DWORD)
            return defaultValue;

        return value;
    }

    void SettingsManager::WriteDWORD(const wchar_t* valueName, DWORD value)
    {
        HKEY hKey;
        LONG result = RegCreateKeyExW(HKEY_CURRENT_USER, REG_KEY_PATH, 0, nullptr,
                                      REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &hKey, nullptr);
        if (result != ERROR_SUCCESS) return;

        RegSetValueExW(hKey, valueName, 0, REG_DWORD, (const BYTE*)&value, sizeof(DWORD));
        RegCloseKey(hKey);
    }

    winrt::hstring SettingsManager::ReadString(const wchar_t* valueName, const wchar_t* defaultValue)
    {
        HKEY hKey;
        LONG result = RegOpenKeyExW(HKEY_CURRENT_USER, REG_KEY_PATH, 0, KEY_QUERY_VALUE, &hKey);
        if (result != ERROR_SUCCESS) return defaultValue;

        wchar_t buffer[256] = {0};
        DWORD dataSize = sizeof(buffer);
        DWORD type;

        result = RegQueryValueExW(hKey, valueName, nullptr, &type, (LPBYTE)buffer, &dataSize);
        RegCloseKey(hKey);

        if (result != ERROR_SUCCESS || type != REG_SZ)
            return defaultValue;

        return buffer;
    }

    void SettingsManager::WriteString(const wchar_t* valueName, const wchar_t* value)
    {
        HKEY hKey;
        LONG result = RegCreateKeyExW(HKEY_CURRENT_USER, REG_KEY_PATH, 0, nullptr,
                                      REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &hKey, nullptr);
        if (result != ERROR_SUCCESS) return;

        RegSetValueExW(hKey, valueName, 0, REG_SZ, (const BYTE*)value, (wcslen(value) + 1) * sizeof(wchar_t));
        RegCloseKey(hKey);
    }
}
