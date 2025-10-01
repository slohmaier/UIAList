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
#include "App.xaml.h"
#include "MainWindow.xaml.h"
#include "SystemTrayManager.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;

namespace winrt::UIAList::implementation
{
    App::App()
    {
        InitializeComponent();

#if defined _DEBUG && !defined DISABLE_XAML_GENERATED_BREAK_ON_UNHANDLED_EXCEPTION
        UnhandledException([this](IInspectable const&, UnhandledExceptionEventArgs const& e)
        {
            if (IsDebuggerPresent())
            {
                auto errorMessage = e.Message();
                __debugbreak();
            }
        });
#endif
    }

    App::~App()
    {
    }

    void App::OnLaunched(LaunchActivatedEventArgs const&)
    {
        // Create main window but don't show it - we'll work from system tray
        m_window = make<MainWindow>();

        // Initialize system tray instead of showing window
        // The system tray will manage window visibility
        SystemTrayManager::GetInstance().Initialize(m_window);
    }
}
