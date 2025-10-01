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
#include "MainWindow.g.h"
#include "ControlEnumerator.h"

namespace winrt::UIAList::implementation
{
    struct ControlInfo
    {
        winrt::hstring DisplayText;
        winrt::hstring OriginalName;
        IUIAutomationElement* Element;
        CONTROLTYPEID ControlType;

        ControlInfo() : Element(nullptr), ControlType(0) {}
        ControlInfo(const winrt::hstring& displayText, const winrt::hstring& originalName,
                    IUIAutomationElement* element, CONTROLTYPEID controlType)
            : DisplayText(displayText), OriginalName(originalName), Element(element), ControlType(controlType)
        {
            if (Element) Element->AddRef();
        }

        ~ControlInfo()
        {
            if (Element) Element->Release();
        }

        ControlInfo(const ControlInfo& other)
            : DisplayText(other.DisplayText), OriginalName(other.OriginalName),
              Element(other.Element), ControlType(other.ControlType)
        {
            if (Element) Element->AddRef();
        }

        ControlInfo& operator=(const ControlInfo& other)
        {
            if (this != &other)
            {
                if (Element) Element->Release();
                DisplayText = other.DisplayText;
                OriginalName = other.OriginalName;
                Element = other.Element;
                ControlType = other.ControlType;
                if (Element) Element->AddRef();
            }
            return *this;
        }
    };

    struct MainWindow : MainWindowT<MainWindow>
    {
        MainWindow();
        ~MainWindow();

        // Event handlers
        void FilterTextBox_TextChanged(winrt::Windows::Foundation::IInspectable const& sender,
                                      winrt::Microsoft::UI::Xaml::Controls::TextChangedEventArgs const& e);
        void FilterTextBox_KeyDown(winrt::Windows::Foundation::IInspectable const& sender,
                                   winrt::Microsoft::UI::Xaml::Input::KeyRoutedEventArgs const& e);
        void ControlsListView_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender,
                                              winrt::Microsoft::UI::Xaml::Controls::SelectionChangedEventArgs const& e);
        void FilterOption_Changed(winrt::Windows::Foundation::IInspectable const& sender,
                                 winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void DoubleClickButton_Click(winrt::Windows::Foundation::IInspectable const& sender,
                                    winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void ClickButton_Click(winrt::Windows::Foundation::IInspectable const& sender,
                              winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void FocusButton_Click(winrt::Windows::Foundation::IInspectable const& sender,
                              winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void CancelButton_Click(winrt::Windows::Foundation::IInspectable const& sender,
                               winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);

        // Public methods
        void ShowForWindow(HWND foregroundWindow);
        void HideWindow();

    private:
        // UI update methods
        void PopulateListView();
        void ApplyFilter();
        void SelectVisibleListItem(int direction);
        void UpdateButtonStates();
        void ShowLoadingOverlay();
        void HideLoadingOverlay();
        void AnnounceToScreenReader(const winrt::hstring& text);
        void EnsureItemSelected();
        void ExecuteDefaultAction();

        // Control interaction
        void ClickSelectedControl();
        void DoubleClickSelectedControl();
        void FocusSelectedControl();

        // Enumeration callbacks
        void OnControlFound(const ControlInfo& controlInfo);
        void OnEnumerationFinished(const winrt::hstring& windowTitle);
        void OnEnumerationCancelled();

        // Data members
        std::vector<ControlInfo> m_allControls;
        std::unique_ptr<ControlEnumerator> m_enumerator;
        ControlInfo m_selectedControl;
        winrt::hstring m_targetWindowTitle;
        HWND m_hwnd{ nullptr };

        // Event tokens
        winrt::event_token m_activatedToken;
    };
}

namespace winrt::UIAList::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}
