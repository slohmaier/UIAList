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
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include "ControlEnumerator.h"

namespace UIAList
{
    class MainWindow
    {
    public:
        MainWindow();
        ~MainWindow();

        winrt::Microsoft::UI::Xaml::Window GetWindow() { return m_window; }
        void Show();
        void Hide();

    private:
        void CreateUI();
        void OnFilterTextChanged(winrt::Windows::Foundation::IInspectable const& sender,
                                winrt::Microsoft::UI::Xaml::Controls::TextChangedEventArgs const& args);
        void OnFilterKeyDown(winrt::Windows::Foundation::IInspectable const& sender,
                            winrt::Microsoft::UI::Xaml::Input::KeyRoutedEventArgs const& args);
        void OnListSelectionChanged(winrt::Windows::Foundation::IInspectable const& sender,
                                   winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& args);
        void OnClickButtonClick(winrt::Windows::Foundation::IInspectable const& sender,
                              winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);
        void OnFocusButtonClick(winrt::Windows::Foundation::IInspectable const& sender,
                              winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);
        void OnDoubleClickButtonClick(winrt::Windows::Foundation::IInspectable const& sender,
                                    winrt::Microsoft::UI::Xaml::RoutedEventArgs const& args);
        void OnClosed(winrt::Windows::Foundation::IInspectable const& sender,
                     winrt::Microsoft::UI::Xaml::WindowEventArgs const& args);

        void StartEnumeration();
        void OnControlFound(const ControlInfo& control);
        void OnEnumerationFinished();

        winrt::Microsoft::UI::Xaml::Window m_window{ nullptr };
        winrt::Microsoft::UI::Xaml::Controls::TextBox m_filterBox{ nullptr };
        winrt::Microsoft::UI::Xaml::Controls::ListView m_listView{ nullptr };
        winrt::Microsoft::UI::Xaml::Controls::Button m_clickButton{ nullptr };
        winrt::Microsoft::UI::Xaml::Controls::Button m_focusButton{ nullptr };
        winrt::Microsoft::UI::Xaml::Controls::Button m_doubleClickButton{ nullptr };

        std::unique_ptr<ControlEnumerator> m_enumerator;
        std::vector<ControlInfo> m_allControls;
        int m_selectedIndex{ -1 };
    };
}
