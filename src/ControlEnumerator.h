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

namespace UIAList
{
    struct ControlInfo
    {
        winrt::hstring Name;
        winrt::hstring Type;
        winrt::hstring AutomationId;
        winrt::com_ptr<IUIAutomationElement> Element;
    };

    class ControlEnumerator
    {
    public:
        ControlEnumerator();
        ~ControlEnumerator();

        // Callback types
        using ControlFoundCallback = std::function<void(const ControlInfo&)>;
        using EnumerationFinishedCallback = std::function<void(const winrt::hstring&)>;
        using EnumerationCancelledCallback = std::function<void()>;

        // Start enumeration asynchronously
        void EnumerateAsync(HWND targetWindow,
                           ControlFoundCallback onControlFound,
                           EnumerationFinishedCallback onFinished,
                           EnumerationCancelledCallback onCancelled);

        // Cancel ongoing enumeration
        void Cancel();

    private:
        // UI Automation initialization
        bool InitializeUIAutomation();
        void CleanupUIAutomation();

        // Enumeration worker (runs on background thread)
        void EnumerateWorker(HWND targetWindow);

        // Recursive tree walking
        void WalkControls(IUIAutomationElement* element, IUIAutomationTreeWalker* walker);

        // Helper to get control type string
        winrt::hstring GetControlTypeString(CONTROLTYPEID controlType);

        // UI Automation objects
        IUIAutomation* m_uiAutomation;
        IUIAutomationTreeWalker* m_controlViewWalker;

        // Threading
        std::unique_ptr<std::thread> m_workerThread;
        std::atomic<bool> m_cancelled;
        std::mutex m_mutex;

        // Callbacks
        ControlFoundCallback m_onControlFound;
        EnumerationFinishedCallback m_onFinished;
        EnumerationCancelledCallback m_onCancelled;
    };
}
