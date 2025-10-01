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
#include "WelcomeDialog.xaml.h"
#if __has_include("WelcomeDialog.g.cpp")
#include "WelcomeDialog.g.cpp"
#endif
#include "SettingsManager.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;

namespace winrt::UIAList::implementation
{
    WelcomeDialog::WelcomeDialog()
    {
        InitializeComponent();
    }

    void WelcomeDialog::OnGetStartedClick(ContentDialog const&, ContentDialogButtonClickEventArgs const&)
    {
        // Save "don't show again" preference
        if (DontShowAgainCheckBox().IsChecked().GetBoolean())
        {
            SettingsManager::GetInstance().SetWelcomeShown(true);
        }
    }
}
