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
#include "AboutDialog.xaml.h"
#if __has_include("AboutDialog.g.cpp")
#include "AboutDialog.g.cpp"
#endif
#include "SettingsManager.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;

namespace winrt::UIAList::implementation
{
    AboutDialog::AboutDialog()
    {
        InitializeComponent();
    }

    void AboutDialog::OnResetSettingsClick(IInspectable const&, RoutedEventArgs const&)
    {
        // Show confirmation dialog
        ContentDialog confirmDialog;
        confirmDialog.Title(box_value(L"Reset Settings"));
        confirmDialog.Content(box_value(L"Are you sure you want to reset all settings to defaults? This cannot be undone."));
        confirmDialog.PrimaryButtonText(L"Reset");
        confirmDialog.CloseButtonText(L"Cancel");
        confirmDialog.DefaultButton(ContentDialogButton::Close);

        confirmDialog.PrimaryButtonClick([](ContentDialog const&, ContentDialogButtonClickEventArgs const&)
        {
            auto& settings = SettingsManager::GetInstance();

            // Reset all settings to defaults
            settings.SetDefaultAction(0);  // Click
            settings.SetAutoStart(false);
            settings.SetHotkeyString(L"Ctrl+Alt+U");
            settings.SetWelcomeShown(false);

            // Show confirmation
            ContentDialog successDialog;
            successDialog.Title(box_value(L"Settings Reset"));
            successDialog.Content(box_value(L"All settings have been reset to defaults. Please restart UIAList for changes to take effect."));
            successDialog.CloseButtonText(L"OK");
            successDialog.ShowAsync();
        });

        confirmDialog.ShowAsync();
    }
}
