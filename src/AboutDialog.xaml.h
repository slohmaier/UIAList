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
#include "AboutDialog.g.h"

namespace winrt::UIAList::implementation
{
    struct AboutDialog : AboutDialogT<AboutDialog>
    {
        AboutDialog();

        void OnResetSettingsClick(winrt::Windows::Foundation::IInspectable const& sender,
                                 winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
    };
}

namespace winrt::UIAList::factory_implementation
{
    struct AboutDialog : AboutDialogT<AboutDialog, implementation::AboutDialog>
    {
    };
}
