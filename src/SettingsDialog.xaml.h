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
#include "SettingsDialog.g.h"

namespace winrt::UIAList::implementation
{
    struct SettingsDialog : SettingsDialogT<SettingsDialog>
    {
        SettingsDialog();

        // Event handlers
        void OnPrimaryButtonClick(winrt::Microsoft::UI::Xaml::Controls::ContentDialog const& sender,
                                 winrt::Microsoft::UI::Xaml::Controls::ContentDialogButtonClickEventArgs const& args);
        void OnCloseButtonClick(winrt::Microsoft::UI::Xaml::Controls::ContentDialog const& sender,
                               winrt::Microsoft::UI::Xaml::Controls::ContentDialogButtonClickEventArgs const& args);
        void OnShortcutButtonClick(winrt::Windows::Foundation::IInspectable const& sender,
                                  winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);

    private:
        void LoadSettings();
        void SaveSettings();
        void StartShortcutCapture();
        void StopShortcutCapture();
        void OnKeyDown(winrt::Windows::Foundation::IInspectable const& sender,
                      winrt::Microsoft::UI::Xaml::Input::KeyRoutedEventArgs const& e);

        bool m_capturingShortcut;
        winrt::hstring m_currentShortcut;
        winrt::event_token m_keyDownToken;
    };
}

namespace winrt::UIAList::factory_implementation
{
    struct SettingsDialog : SettingsDialogT<SettingsDialog, implementation::SettingsDialog>
    {
    };
}
