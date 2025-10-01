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
#include "WelcomeDialog.g.h"

namespace winrt::UIAList::implementation
{
    struct WelcomeDialog : WelcomeDialogT<WelcomeDialog>
    {
        WelcomeDialog();

        void OnGetStartedClick(winrt::Microsoft::UI::Xaml::Controls::ContentDialog const& sender,
                              winrt::Microsoft::UI::Xaml::Controls::ContentDialogButtonClickEventArgs const& args);
    };
}

namespace winrt::UIAList::factory_implementation
{
    struct WelcomeDialog : WelcomeDialogT<WelcomeDialog, implementation::WelcomeDialog>
    {
    };
}
