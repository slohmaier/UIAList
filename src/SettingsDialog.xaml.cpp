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
#include "SettingsDialog.xaml.h"
#if __has_include("SettingsDialog.g.cpp")
#include "SettingsDialog.g.cpp"
#endif
#include "SettingsManager.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Microsoft::UI::Xaml::Input;
using namespace Windows::System;

namespace winrt::UIAList::implementation
{
    SettingsDialog::SettingsDialog()
        : m_capturingShortcut(false)
    {
        InitializeComponent();
        LoadSettings();
    }

    void SettingsDialog::LoadSettings()
    {
        auto& settings = SettingsManager::GetInstance();

        // Load auto-start
        AutoStartCheckBox().IsChecked(settings.GetAutoStart());

        // Load default action
        int defaultAction = settings.GetDefaultAction();
        DefaultActionComboBox().SelectedIndex(defaultAction);

        // Load shortcut
        m_currentShortcut = settings.GetHotkeyString();
        ShortcutButton().Content(box_value(m_currentShortcut));
    }

    void SettingsDialog::SaveSettings()
    {
        auto& settings = SettingsManager::GetInstance();

        // Save auto-start
        settings.SetAutoStart(AutoStartCheckBox().IsChecked().GetBoolean());

        // Save default action
        settings.SetDefaultAction(DefaultActionComboBox().SelectedIndex());

        // Save shortcut
        settings.SetHotkeyString(m_currentShortcut);

        // TODO: Update global hotkey registration in SystemTrayManager
    }

    void SettingsDialog::OnPrimaryButtonClick(ContentDialog const&, ContentDialogButtonClickEventArgs const&)
    {
        SaveSettings();
    }

    void SettingsDialog::OnCloseButtonClick(ContentDialog const&, ContentDialogButtonClickEventArgs const&)
    {
        // Cancel - don't save
    }

    void SettingsDialog::OnShortcutButtonClick(IInspectable const&, RoutedEventArgs const&)
    {
        if (m_capturingShortcut)
        {
            StopShortcutCapture();
        }
        else
        {
            StartShortcutCapture();
        }
    }

    void SettingsDialog::StartShortcutCapture()
    {
        m_capturingShortcut = true;
        ShortcutButton().Content(box_value(L"Press key combination..."));
        ShortcutButton().Background(Microsoft::UI::Xaml::Media::SolidColorBrush(Microsoft::UI::Colors::LightPink()));
        ShortcutHintText().Visibility(Visibility::Visible);

        // Listen for key down events
        m_keyDownToken = this->KeyDown({ this, &SettingsDialog::OnKeyDown });
    }

    void SettingsDialog::StopShortcutCapture()
    {
        m_capturingShortcut = false;
        ShortcutButton().Content(box_value(m_currentShortcut));
        ShortcutButton().ClearValue(Button::BackgroundProperty());
        ShortcutHintText().Visibility(Visibility::Collapsed);

        // Remove key down listener
        this->KeyDown(m_keyDownToken);
    }

    void SettingsDialog::OnKeyDown(IInspectable const&, KeyRoutedEventArgs const& e)
    {
        if (!m_capturingShortcut) return;

        auto key = e.Key();

        // Ignore lone modifier keys
        if (key == VirtualKey::Control || key == VirtualKey::Shift ||
            key == VirtualKey::Menu || key == VirtualKey::LeftWindows || key == VirtualKey::RightWindows)
        {
            return;
        }

        // Get current modifiers
        auto window = Microsoft::UI::Xaml::Window::Current();
        auto coreWindow = window.CoreWindow();

        bool ctrl = (coreWindow.GetKeyState(VirtualKey::Control) & Windows::UI::Core::CoreVirtualKeyStates::Down) == Windows::UI::Core::CoreVirtualKeyStates::Down;
        bool shift = (coreWindow.GetKeyState(VirtualKey::Shift) & Windows::UI::Core::CoreVirtualKeyStates::Down) == Windows::UI::Core::CoreVirtualKeyStates::Down;
        bool alt = (coreWindow.GetKeyState(VirtualKey::Menu) & Windows::UI::Core::CoreVirtualKeyStates::Down) == Windows::UI::Core::CoreVirtualKeyStates::Down;

        // Must have at least one modifier
        if (!ctrl && !shift && !alt) return;

        // Build shortcut string
        std::wstring shortcut;
        if (ctrl) shortcut += L"Ctrl+";
        if (shift) shortcut += L"Shift+";
        if (alt) shortcut += L"Alt+";

        // Add key name
        wchar_t keyName = static_cast<wchar_t>(key);
        if (keyName >= 'A' && keyName <= 'Z')
        {
            shortcut += keyName;
        }
        else if (key >= VirtualKey::F1 && key <= VirtualKey::F24)
        {
            int fNum = static_cast<int>(key) - static_cast<int>(VirtualKey::F1) + 1;
            shortcut += L"F" + std::to_wstring(fNum);
        }
        else
        {
            // For other keys, use the key value
            shortcut += std::to_wstring(static_cast<int>(key));
        }

        m_currentShortcut = shortcut;
        StopShortcutCapture();

        e.Handled(true);
    }
}
