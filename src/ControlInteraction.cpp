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
#include "ControlInteraction.h"

namespace winrt::UIAList::implementation
{
    // Exact port of Qt implementation (uialist.cpp:715-803)
    bool ControlInteraction::ClickControl(IUIAutomationElement* element)
    {
        if (!element) return false;

        // Method 1: Try to get bounding rectangle and simulate mouse click
        if (ClickViaMouse(element)) return true;

        // Method 2: Try invoke pattern
        if (ClickViaInvokePattern(element)) return true;

        // Method 3: Try legacy action
        if (ClickViaLegacyPattern(element)) return true;

        // Method 4: Try selection pattern
        if (ClickViaSelectionPattern(element)) return true;

        return false;
    }

    // Exact port of Qt implementation (uialist.cpp:879-979)
    bool ControlInteraction::DoubleClickControl(IUIAutomationElement* element)
    {
        if (!element) return false;

        // Method 1: Try to get bounding rectangle and simulate mouse double-click
        RECT rect;
        HRESULT hr = element->get_CurrentBoundingRectangle(&rect);
        if (SUCCEEDED(hr))
        {
            int x = rect.left + (rect.right - rect.left) / 2;
            int y = rect.top + (rect.bottom - rect.top) / 2;

            SetCursorPos(x, y);
            Sleep(50);
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
            Sleep(50);
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);

            return true;
        }

        // Method 2: Try toggle pattern
        IUIAutomationTogglePattern* togglePattern = nullptr;
        hr = element->GetCurrentPatternAs(UIA_TogglePatternId, __uuidof(IUIAutomationTogglePattern), (void**)&togglePattern);
        if (SUCCEEDED(hr) && togglePattern)
        {
            hr = togglePattern->Toggle();
            togglePattern->Release();
            if (SUCCEEDED(hr)) return true;
        }

        // Method 3: Try double click via legacy pattern
        IUIAutomationLegacyIAccessiblePattern* legacyPattern = nullptr;
        hr = element->GetCurrentPatternAs(UIA_LegacyIAccessiblePatternId, __uuidof(IUIAutomationLegacyIAccessiblePattern), (void**)&legacyPattern);
        if (SUCCEEDED(hr) && legacyPattern)
        {
            hr = legacyPattern->DoDefaultAction();
            if (SUCCEEDED(hr))
            {
                Sleep(50);
                legacyPattern->DoDefaultAction();
            }
            legacyPattern->Release();
            if (SUCCEEDED(hr)) return true;
        }

        // Method 4: Try double invoke
        IUIAutomationInvokePattern* invokePattern = nullptr;
        hr = element->GetCurrentPatternAs(UIA_InvokePatternId, __uuidof(IUIAutomationInvokePattern), (void**)&invokePattern);
        if (SUCCEEDED(hr) && invokePattern)
        {
            hr = invokePattern->Invoke();
            if (SUCCEEDED(hr))
            {
                Sleep(50);
                invokePattern->Invoke();
            }
            invokePattern->Release();
            if (SUCCEEDED(hr)) return true;
        }

        return false;
    }

    // Exact port of Qt implementation (uialist.cpp:805-877)
    bool ControlInteraction::FocusControl(IUIAutomationElement* element)
    {
        if (!element) return false;

        // Method 1: Use UI Automation SetFocus
        HRESULT hr = element->SetFocus();
        if (SUCCEEDED(hr)) return true;

        // Method 2: Try to get bounding rectangle and click to focus
        RECT rect;
        hr = element->get_CurrentBoundingRectangle(&rect);
        if (SUCCEEDED(hr))
        {
            int x = rect.left + (rect.right - rect.left) / 2;
            int y = rect.top + (rect.bottom - rect.top) / 2;

            SetCursorPos(x, y);
            Sleep(50);
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);

            return true;
        }

        // Method 3: Try keyboard Tab navigation
        keybd_event(VK_TAB, 0, 0, 0);
        keybd_event(VK_TAB, 0, KEYEVENTF_KEYUP, 0);
        Sleep(100);

        return false;
    }

    bool ControlInteraction::ClickViaMouse(IUIAutomationElement* element)
    {
        RECT rect;
        HRESULT hr = element->get_CurrentBoundingRectangle(&rect);
        if (SUCCEEDED(hr))
        {
            int x = rect.left + (rect.right - rect.left) / 2;
            int y = rect.top + (rect.bottom - rect.top) / 2;

            SetCursorPos(x, y);
            Sleep(50);
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);

            return true;
        }
        return false;
    }

    bool ControlInteraction::ClickViaInvokePattern(IUIAutomationElement* element)
    {
        IUIAutomationInvokePattern* invokePattern = nullptr;
        HRESULT hr = element->GetCurrentPatternAs(UIA_InvokePatternId, __uuidof(IUIAutomationInvokePattern), (void**)&invokePattern);

        if (SUCCEEDED(hr) && invokePattern)
        {
            hr = invokePattern->Invoke();
            invokePattern->Release();
            return SUCCEEDED(hr);
        }
        return false;
    }

    bool ControlInteraction::ClickViaLegacyPattern(IUIAutomationElement* element)
    {
        IUIAutomationLegacyIAccessiblePattern* legacyPattern = nullptr;
        HRESULT hr = element->GetCurrentPatternAs(UIA_LegacyIAccessiblePatternId, __uuidof(IUIAutomationLegacyIAccessiblePattern), (void**)&legacyPattern);

        if (SUCCEEDED(hr) && legacyPattern)
        {
            hr = legacyPattern->DoDefaultAction();
            legacyPattern->Release();
            return SUCCEEDED(hr);
        }
        return false;
    }

    bool ControlInteraction::ClickViaSelectionPattern(IUIAutomationElement* element)
    {
        IUIAutomationSelectionItemPattern* selectionPattern = nullptr;
        HRESULT hr = element->GetCurrentPatternAs(UIA_SelectionItemPatternId, __uuidof(IUIAutomationSelectionItemPattern), (void**)&selectionPattern);

        if (SUCCEEDED(hr) && selectionPattern)
        {
            hr = selectionPattern->Select();
            selectionPattern->Release();
            return SUCCEEDED(hr);
        }
        return false;
    }
}
