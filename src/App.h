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

#include <winrt/Microsoft.UI.Xaml.h>

namespace UIAList
{
    class App
    {
    public:
        App();
        ~App();

        void Run();
        void Exit();

    private:
        winrt::Microsoft::UI::Xaml::Window m_window{ nullptr };
        bool m_running{ false };
    };
}
