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

namespace winrt::UIAList::implementation
{
    // Static helper class for control interaction
    // Ports the exact logic from Qt version (uialist.cpp:715-979)
    class ControlInteraction
    {
    public:
        // Click control (uses mouse simulation + fallbacks)
        static bool ClickControl(IUIAutomationElement* element);

        // Double-click control
        static bool DoubleClickControl(IUIAutomationElement* element);

        // Focus control
        static bool FocusControl(IUIAutomationElement* element);

    private:
        // Helper methods
        static bool ClickViaMouse(IUIAutomationElement* element);
        static bool ClickViaInvokePattern(IUIAutomationElement* element);
        static bool ClickViaLegacyPattern(IUIAutomationElement* element);
        static bool ClickViaSelectionPattern(IUIAutomationElement* element);
    };
}
