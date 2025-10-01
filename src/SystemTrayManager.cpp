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
#include "SystemTrayManager.h"
#include "MainWindow.xaml.h"
#include "SettingsManager.h"

#define WM_TRAYICON (WM_USER + 1)
#define HOTKEY_ID 1

namespace winrt::UIAList::implementation
{
    SystemTrayManager& SystemTrayManager::GetInstance()
    {
        static SystemTrayManager instance;
        return instance;
    }

    SystemTrayManager::~SystemTrayManager()
    {
        Cleanup();
    }

    void SystemTrayManager::Initialize(winrt::Microsoft::UI::Xaml::Window mainWindow)
    {
        m_mainWindow = mainWindow;
        m_initialized = false;
        m_hotkeyId = HOTKEY_ID;
        m_trayWindow = nullptr;

        // Create hidden window for tray icon messages
        CreateTrayIcon();

        // Register default hotkey (Ctrl+Alt+U)
        auto settings = SettingsManager::GetInstance();
        // TODO: Load hotkey from settings
        RegisterGlobalHotkey(MOD_CONTROL | MOD_ALT, 'U');

        m_initialized = true;
    }

    void SystemTrayManager::Cleanup()
    {
        if (!m_initialized) return;

        UnregisterGlobalHotkey();
        DestroyTrayIcon();
        m_initialized = false;
    }

    bool SystemTrayManager::CreateTrayIcon()
    {
        // Create a message-only window for tray icon
        WNDCLASSEXW wc = {0};
        wc.cbSize = sizeof(WNDCLASSEXW);
        wc.lpfnWndProc = TrayWndProc;
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.lpszClassName = L"UIAListTrayClass";
        RegisterClassExW(&wc);

        m_trayWindow = CreateWindowExW(
            0, L"UIAListTrayClass", L"UIAList Tray",
            0, 0, 0, 0, 0,
            HWND_MESSAGE, nullptr, GetModuleHandleW(nullptr), this
        );

        if (!m_trayWindow) return false;

        // Initialize NOTIFYICONDATA
        ZeroMemory(&m_nid, sizeof(m_nid));
        m_nid.cbSize = sizeof(NOTIFYICONDATAW);
        m_nid.hWnd = m_trayWindow;
        m_nid.uID = 1;
        m_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        m_nid.uCallbackMessage = WM_TRAYICON;

        // Load icon (use application icon)
        m_nid.hIcon = LoadIconW(GetModuleHandleW(nullptr), MAKEINTRESOURCEW(1));
        if (!m_nid.hIcon)
        {
            m_nid.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
        }

        wcscpy_s(m_nid.szTip, L"UIAList - Screen Reader Control Navigator");

        // Add the tray icon
        return Shell_NotifyIconW(NIM_ADD, &m_nid);
    }

    void SystemTrayManager::DestroyTrayIcon()
    {
        if (m_trayWindow)
        {
            Shell_NotifyIconW(NIM_DELETE, &m_nid);
            DestroyWindow(m_trayWindow);
            m_trayWindow = nullptr;
        }
    }

    bool SystemTrayManager::RegisterGlobalHotkey(UINT modifiers, UINT vk)
    {
        UnregisterGlobalHotkey();
        return ::RegisterHotKey(m_trayWindow, m_hotkeyId, modifiers, vk);
    }

    void SystemTrayManager::UnregisterGlobalHotkey()
    {
        if (m_trayWindow)
        {
            ::UnregisterHotKey(m_trayWindow, m_hotkeyId);
        }
    }

    LRESULT CALLBACK SystemTrayManager::TrayWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        if (message == WM_CREATE)
        {
            CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
            return 0;
        }

        SystemTrayManager* pThis = reinterpret_cast<SystemTrayManager*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        if (!pThis) return DefWindowProcW(hwnd, message, wParam, lParam);

        if (message == WM_TRAYICON)
        {
            pThis->HandleTrayMessage(LOWORD(lParam), lParam);
            return 0;
        }
        else if (message == WM_HOTKEY && wParam == HOTKEY_ID)
        {
            pThis->HandleHotkeyMessage();
            return 0;
        }

        return DefWindowProcW(hwnd, message, wParam, lParam);
    }

    void SystemTrayManager::HandleTrayMessage(UINT message, LPARAM lParam)
    {
        switch (message)
        {
        case WM_LBUTTONDBLCLK:
            HandleHotkeyMessage();  // Double-click shows window
            break;

        case WM_RBUTTONUP:
            ShowContextMenu();
            break;
        }
    }

    void SystemTrayManager::HandleHotkeyMessage()
    {
        // Get foreground window before activating our window
        HWND foregroundWindow = GetForegroundWindowBeforeActivation();

        // Show main window for the foreground window
        if (m_mainWindow)
        {
            auto mainWindowImpl = m_mainWindow.try_as<MainWindow>();
            if (mainWindowImpl)
            {
                mainWindowImpl->ShowForWindow(foregroundWindow);
            }
        }
    }

    void SystemTrayManager::ShowContextMenu()
    {
        HMENU hMenu = CreatePopupMenu();
        if (!hMenu) return;

        AppendMenuW(hMenu, MF_STRING, 1, L"Settings");
        AppendMenuW(hMenu, MF_STRING, 2, L"About");
        AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
        AppendMenuW(hMenu, MF_STRING, 3, L"Exit");

        POINT pt;
        GetCursorPos(&pt);

        // Required to make menu disappear when clicking outside
        SetForegroundWindow(m_trayWindow);

        int cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_NONOTIFY, pt.x, pt.y, 0, m_trayWindow, nullptr);

        DestroyMenu(hMenu);

        switch (cmd)
        {
        case 1:  // Settings
            // TODO: Show settings dialog
            break;

        case 2:  // About
            // TODO: Show about dialog
            break;

        case 3:  // Exit
            // TODO: Exit application
            break;
        }
    }

    HWND SystemTrayManager::GetForegroundWindowBeforeActivation()
    {
        return GetForegroundWindow();
    }
}
