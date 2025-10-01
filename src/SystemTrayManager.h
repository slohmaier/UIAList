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
    // System tray icon and global hotkey manager
    // Replaces Qt's QSystemTrayIcon and global hotkey handling
    class SystemTrayManager
    {
    public:
        static SystemTrayManager& GetInstance();

        void Initialize(winrt::Microsoft::UI::Xaml::Window mainWindow);
        void Cleanup();

        // Register/unregister global hotkey
        bool RegisterGlobalHotkey(UINT modifiers, UINT vk);
        void UnregisterGlobalHotkey();

    private:
        SystemTrayManager() = default;
        ~SystemTrayManager();
        SystemTrayManager(const SystemTrayManager&) = delete;
        SystemTrayManager& operator=(const SystemTrayManager&) = delete;

        // System tray management
        bool CreateTrayIcon();
        void DestroyTrayIcon();

        // Message handling
        static LRESULT CALLBACK TrayWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
        void HandleTrayMessage(UINT message, LPARAM lParam);
        void HandleHotkeyMessage();

        // Context menu
        void ShowContextMenu();

        // Window management
        HWND GetForegroundWindowBeforeActivation();

        // Members
        HWND m_trayWindow;
        NOTIFYICONDATAW m_nid;
        winrt::Microsoft::UI::Xaml::Window m_mainWindow{ nullptr };
        int m_hotkeyId;
        bool m_initialized;
    };
}
