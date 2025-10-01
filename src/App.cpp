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
#include "App.h"
#include "MainWindow.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;

namespace UIAList
{
    App::App()
    {
        // Initialize WinUI3
    }

    App::~App()
    {
    }

    void App::Run()
    {
        m_running = true;

        // Create main window
        MainWindow mainWin;
        m_window = mainWin.GetWindow();

        // Don't activate window yet - wait for hotkey
        // m_window.Activate();
    }

    void App::Exit()
    {
        m_running = false;
        if (m_window)
        {
            m_window.Close();
        }
    }
}
