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
#include "ControlEnumerator.h"
#include "MainWindow.xaml.h"

namespace winrt::UIAList::implementation
{
    ControlEnumerator::ControlEnumerator()
        : m_uiAutomation(nullptr)
        , m_controlViewWalker(nullptr)
        , m_cancelled(false)
    {
        InitializeUIAutomation();
    }

    ControlEnumerator::~ControlEnumerator()
    {
        Cancel();
        if (m_workerThread && m_workerThread->joinable())
        {
            m_workerThread->join();
        }
        CleanupUIAutomation();
    }

    bool ControlEnumerator::InitializeUIAutomation()
    {
        HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        if (FAILED(hr) && hr != RPC_E_CHANGED_MODE)
        {
            return false;
        }

        hr = CoCreateInstance(__uuidof(CUIAutomation), nullptr, CLSCTX_INPROC_SERVER,
                             __uuidof(IUIAutomation), (void**)&m_uiAutomation);
        if (FAILED(hr))
        {
            return false;
        }

        hr = m_uiAutomation->get_ControlViewWalker(&m_controlViewWalker);
        if (FAILED(hr))
        {
            return false;
        }

        return true;
    }

    void ControlEnumerator::CleanupUIAutomation()
    {
        if (m_controlViewWalker)
        {
            m_controlViewWalker->Release();
            m_controlViewWalker = nullptr;
        }

        if (m_uiAutomation)
        {
            m_uiAutomation->Release();
            m_uiAutomation = nullptr;
        }

        CoUninitialize();
    }

    void ControlEnumerator::EnumerateAsync(HWND targetWindow,
                                          ControlFoundCallback onControlFound,
                                          EnumerationFinishedCallback onFinished,
                                          EnumerationCancelledCallback onCancelled)
    {
        // Cancel any previous enumeration
        Cancel();
        if (m_workerThread && m_workerThread->joinable())
        {
            m_workerThread->join();
        }

        // Store callbacks
        m_onControlFound = onControlFound;
        m_onFinished = onFinished;
        m_onCancelled = onCancelled;

        // Reset cancelled flag
        m_cancelled = false;

        // Start worker thread
        m_workerThread = std::make_unique<std::thread>(&ControlEnumerator::EnumerateWorker, this, targetWindow);
    }

    void ControlEnumerator::Cancel()
    {
        m_cancelled = true;
    }

    void ControlEnumerator::EnumerateWorker(HWND targetWindow)
    {
        // Initialize COM for this thread
        HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        bool comInitialized = SUCCEEDED(hr);

        if (!targetWindow)
        {
            if (m_onCancelled) m_onCancelled();
            if (comInitialized) CoUninitialize();
            return;
        }

        // Get window title
        wchar_t windowTitle[256] = {0};
        GetWindowTextW(targetWindow, windowTitle, 256);
        winrt::hstring windowTitleStr = windowTitle;
        if (windowTitleStr.empty())
        {
            windowTitleStr = L"Untitled Window";
        }

        // Validate UI Automation objects
        if (!m_uiAutomation || !m_controlViewWalker)
        {
            if (m_onCancelled) m_onCancelled();
            if (comInitialized) CoUninitialize();
            return;
        }

        // Get UI Automation element for the target window
        IUIAutomationElement* rootElement = nullptr;
        hr = m_uiAutomation->ElementFromHandle(targetWindow, &rootElement);
        if (FAILED(hr) || !rootElement)
        {
            if (m_onCancelled) m_onCancelled();
            if (comInitialized) CoUninitialize();
            return;
        }

        // Walk through all controls
        WalkControls(rootElement, m_controlViewWalker);

        rootElement->Release();

        // Check if cancelled before calling finished callback
        if (!m_cancelled && m_onFinished)
        {
            m_onFinished(windowTitleStr);
        }
        else if (m_cancelled && m_onCancelled)
        {
            m_onCancelled();
        }

        if (comInitialized) CoUninitialize();
    }

    void ControlEnumerator::WalkControls(IUIAutomationElement* element, IUIAutomationTreeWalker* walker)
    {
        if (!element || !walker) return;

        // Check if cancelled
        if (m_cancelled) return;

        // Get control type
        CONTROLTYPEID controlType;
        HRESULT hr = element->get_CurrentControlType(&controlType);
        if (FAILED(hr)) return;

        // Get control name
        BSTR name = nullptr;
        element->get_CurrentName(&name);
        winrt::hstring controlName = name ? winrt::hstring(name) : L"(no name)";
        if (name) SysFreeString(name);

        // Get control type string
        winrt::hstring controlTypeStr = GetControlTypeString(controlType);

        // Create display text
        winrt::hstring displayText = controlTypeStr + L": " + controlName;

        // Create ControlInfo and emit callback
        if (m_onControlFound)
        {
            ControlInfo info(displayText, controlName, element, controlType);
            m_onControlFound(info);
        }

        // Walk child elements
        IUIAutomationElement* child = nullptr;
        hr = walker->GetFirstChildElement(element, &child);
        while (SUCCEEDED(hr) && child)
        {
            // Check if cancelled before processing child
            if (m_cancelled)
            {
                child->Release();
                return;
            }

            WalkControls(child, walker);

            IUIAutomationElement* nextChild = nullptr;
            hr = walker->GetNextSiblingElement(child, &nextChild);
            child->Release();
            child = nextChild;
        }
    }

    winrt::hstring ControlEnumerator::GetControlTypeString(CONTROLTYPEID controlType)
    {
        switch (controlType)
        {
            case UIA_ButtonControlTypeId: return L"Button";
            case UIA_CheckBoxControlTypeId: return L"CheckBox";
            case UIA_ComboBoxControlTypeId: return L"ComboBox";
            case UIA_EditControlTypeId: return L"Edit";
            case UIA_HyperlinkControlTypeId: return L"Hyperlink";
            case UIA_ImageControlTypeId: return L"Image";
            case UIA_ListItemControlTypeId: return L"ListItem";
            case UIA_ListControlTypeId: return L"List";
            case UIA_MenuControlTypeId: return L"Menu";
            case UIA_MenuBarControlTypeId: return L"MenuBar";
            case UIA_MenuItemControlTypeId: return L"MenuItem";
            case UIA_ProgressBarControlTypeId: return L"ProgressBar";
            case UIA_RadioButtonControlTypeId: return L"RadioButton";
            case UIA_ScrollBarControlTypeId: return L"ScrollBar";
            case UIA_SliderControlTypeId: return L"Slider";
            case UIA_SpinnerControlTypeId: return L"Spinner";
            case UIA_StatusBarControlTypeId: return L"StatusBar";
            case UIA_TabControlTypeId: return L"Tab";
            case UIA_TabItemControlTypeId: return L"TabItem";
            case UIA_TextControlTypeId: return L"Text";
            case UIA_ToolBarControlTypeId: return L"ToolBar";
            case UIA_ToolTipControlTypeId: return L"ToolTip";
            case UIA_TreeControlTypeId: return L"Tree";
            case UIA_TreeItemControlTypeId: return L"TreeItem";
            case UIA_CustomControlTypeId: return L"Custom";
            case UIA_GroupControlTypeId: return L"Group";
            case UIA_ThumbControlTypeId: return L"Thumb";
            case UIA_DataGridControlTypeId: return L"DataGrid";
            case UIA_DataItemControlTypeId: return L"DataItem";
            case UIA_DocumentControlTypeId: return L"Document";
            case UIA_SplitButtonControlTypeId: return L"SplitButton";
            case UIA_WindowControlTypeId: return L"Window";
            case UIA_PaneControlTypeId: return L"Pane";
            case UIA_HeaderControlTypeId: return L"Header";
            case UIA_HeaderItemControlTypeId: return L"HeaderItem";
            case UIA_TableControlTypeId: return L"Table";
            case UIA_TitleBarControlTypeId: return L"TitleBar";
            case UIA_SeparatorControlTypeId: return L"Separator";
            default:
            {
                wchar_t buffer[64];
                swprintf(buffer, 64, L"Unknown(%d)", controlType);
                return buffer;
            }
        }
    }
}
