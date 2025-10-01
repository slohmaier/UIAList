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
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif
#include "ControlInteraction.h"
#include "SettingsManager.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Microsoft::UI::Xaml::Input;
using namespace Windows::System;

namespace winrt::UIAList::implementation
{
    MainWindow::MainWindow()
    {
        InitializeComponent();

        // Get HWND for this window
        auto windowNative = this->try_as<::IWindowNative>();
        if (windowNative)
        {
            windowNative->get_WindowHandle(&m_hwnd);
        }

        // Handle window activation/deactivation to auto-close
        m_activatedToken = this->Activated([this](auto&&, auto&&)
        {
            // Window deactivated - hide it
            HideWindow();
        });

        // Handle Escape key globally
        this->KeyDown([this](auto&&, KeyRoutedEventArgs const& e)
        {
            if (e.Key() == VirtualKey::Escape)
            {
                HideWindow();
                e.Handled(true);
            }
        });

        // Initialize control enumerator
        m_enumerator = std::make_unique<ControlEnumerator>();
    }

    MainWindow::~MainWindow()
    {
        // Clean up
        if (m_enumerator)
        {
            m_enumerator->Cancel();
        }
    }

    void MainWindow::ShowForWindow(HWND foregroundWindow)
    {
        // Clear previous data
        m_allControls.clear();
        m_selectedControl = ControlInfo();
        ControlsListView().Items().Clear();

        // Show window
        this->Activate();

        // Show loading overlay
        ShowLoadingOverlay();

        // Start enumeration in background
        m_enumerator->EnumerateAsync(foregroundWindow,
            [this](const ControlInfo& info) { OnControlFound(info); },
            [this](const winrt::hstring& title) { OnEnumerationFinished(title); },
            [this]() { OnEnumerationCancelled(); }
        );
    }

    void MainWindow::HideWindow()
    {
        // Cancel any ongoing enumeration
        if (m_enumerator)
        {
            m_enumerator->Cancel();
        }

        // Hide the window
        this->Activate(false);  // Or use AppWindow to hide
    }

    void MainWindow::FilterTextBox_TextChanged(IInspectable const&, TextChangedEventArgs const&)
    {
        ApplyFilter();
    }

    void MainWindow::FilterTextBox_KeyDown(IInspectable const&, KeyRoutedEventArgs const& e)
    {
        // Special handling for arrow keys in filter box
        if (e.Key() == VirtualKey::Up)
        {
            SelectVisibleListItem(-1);
            e.Handled(true);
        }
        else if (e.Key() == VirtualKey::Down)
        {
            SelectVisibleListItem(1);
            e.Handled(true);
        }
        else if (e.Key() == VirtualKey::Enter)
        {
            EnsureItemSelected();
            ExecuteDefaultAction();
            HideWindow();
            e.Handled(true);
        }
    }

    void MainWindow::ControlsListView_SelectionChanged(IInspectable const&, SelectionChangedEventArgs const&)
    {
        int selectedIndex = ControlsListView().SelectedIndex();
        if (selectedIndex >= 0 && selectedIndex < static_cast<int>(m_allControls.size()))
        {
            // Find the actual control info based on visible index
            // This requires mapping from ListView to m_allControls
            // For now, store index in ItemsSource
            m_selectedControl = m_allControls[selectedIndex];
        }
        else
        {
            m_selectedControl = ControlInfo();
        }

        UpdateButtonStates();
    }

    void MainWindow::FilterOption_Changed(IInspectable const&, RoutedEventArgs const&)
    {
        PopulateListView();
    }

    void MainWindow::DoubleClickButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        EnsureItemSelected();
        DoubleClickSelectedControl();
        HideWindow();
    }

    void MainWindow::ClickButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        EnsureItemSelected();
        ClickSelectedControl();
        HideWindow();
    }

    void MainWindow::FocusButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        EnsureItemSelected();
        FocusSelectedControl();
        HideWindow();
    }

    void MainWindow::CancelButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        if (m_enumerator)
        {
            m_enumerator->Cancel();
        }
    }

    void MainWindow::PopulateListView()
    {
        ControlsListView().Items().Clear();

        bool hideEmptyTitles = HideEmptyTitlesCheckBox().IsChecked().GetBoolean();
        bool hideMenus = HideMenusCheckBox().IsChecked().GetBoolean();

        for (const auto& controlInfo : m_allControls)
        {
            // Always filter out Text and Window control types
            if (controlInfo.ControlType == UIA_TextControlTypeId ||
                controlInfo.ControlType == UIA_WindowControlTypeId)
            {
                continue;
            }

            // Skip controls with empty or no titles if checkbox is checked
            if (hideEmptyTitles)
            {
                std::wstring name(controlInfo.OriginalName);
                if (name.empty() || name == L"(no name)")
                {
                    continue;
                }
            }

            // Skip menus and menu items if checkbox is checked
            if (hideMenus)
            {
                if (controlInfo.ControlType == UIA_MenuControlTypeId ||
                    controlInfo.ControlType == UIA_MenuBarControlTypeId ||
                    controlInfo.ControlType == UIA_MenuItemControlTypeId)
                {
                    continue;
                }
            }

            // Add to list
            ControlsListView().Items().Append(box_value(controlInfo.DisplayText));
        }

        // Auto-select first item
        if (ControlsListView().Items().Size() > 0)
        {
            ControlsListView().SelectedIndex(0);
            AnnounceToScreenReader(m_allControls[0].DisplayText);
        }

        UpdateButtonStates();
    }

    void MainWindow::ApplyFilter()
    {
        winrt::hstring filterText = FilterTextBox().Text();
        std::wstring filter(filterText);

        // Convert to lowercase for case-insensitive search
        std::transform(filter.begin(), filter.end(), filter.begin(), ::towlower);

        // Split into words
        std::vector<std::wstring> filterWords;
        std::wstringstream ss(filter);
        std::wstring word;
        while (ss >> word)
        {
            filterWords.push_back(word);
        }

        // Filter items in ListView
        for (uint32_t i = 0; i < ControlsListView().Items().Size(); ++i)
        {
            auto item = ControlsListView().Items().GetAt(i);
            winrt::hstring itemText = unbox_value<winrt::hstring>(item);
            std::wstring itemStr(itemText);
            std::transform(itemStr.begin(), itemStr.end(), itemStr.begin(), ::towlower);

            bool visible = true;
            if (!filterWords.empty())
            {
                for (const auto& filterWord : filterWords)
                {
                    if (itemStr.find(filterWord) == std::wstring::npos)
                    {
                        visible = false;
                        break;
                    }
                }
            }

            // WinUI doesn't have direct item visibility - we need to use CollectionViewSource
            // For now, rebuild the list (simplified approach)
        }

        // Simplified: Rebuild list with filter
        // TODO: Optimize with CollectionViewSource
        PopulateListView();
    }

    void MainWindow::SelectVisibleListItem(int direction)
    {
        int currentIndex = ControlsListView().SelectedIndex();
        int itemCount = static_cast<int>(ControlsListView().Items().Size());

        if (itemCount == 0) return;

        int newIndex = currentIndex + direction;

        // Wrap around
        if (newIndex < 0)
        {
            newIndex = itemCount - 1;
        }
        else if (newIndex >= itemCount)
        {
            newIndex = 0;
        }

        ControlsListView().SelectedIndex(newIndex);

        // Announce to screen reader
        if (newIndex >= 0 && newIndex < itemCount)
        {
            auto item = ControlsListView().Items().GetAt(newIndex);
            winrt::hstring itemText = unbox_value<winrt::hstring>(item);
            AnnounceToScreenReader(itemText);
        }
    }

    void MainWindow::UpdateButtonStates()
    {
        bool hasItems = ControlsListView().Items().Size() > 0;
        DoubleClickButton().IsEnabled(hasItems);
        ClickButton().IsEnabled(hasItems);
        FocusButton().IsEnabled(hasItems);
    }

    void MainWindow::ShowLoadingOverlay()
    {
        LoadingOverlay().Visibility(Visibility::Visible);
        CancelButton().Focus(FocusState::Programmatic);
    }

    void MainWindow::HideLoadingOverlay()
    {
        LoadingOverlay().Visibility(Visibility::Collapsed);
        FilterTextBox().Focus(FocusState::Programmatic);
    }

    void MainWindow::AnnounceToScreenReader(const winrt::hstring& text)
    {
        // Use UIA to announce to screen readers
        // This requires creating an automation peer and raising events
        // For now, setting accessible name triggers announcement
        if (ControlsListView().SelectedIndex() >= 0)
        {
            auto peer = Automation::Peers::FrameworkElementAutomationPeer(ControlsListView());
            // Raise selection changed event
        }
    }

    void MainWindow::EnsureItemSelected()
    {
        if (ControlsListView().SelectedIndex() < 0 && ControlsListView().Items().Size() > 0)
        {
            ControlsListView().SelectedIndex(0);
        }
    }

    void MainWindow::ExecuteDefaultAction()
    {
        int defaultAction = SettingsManager::GetInstance().GetDefaultAction();

        switch (defaultAction)
        {
        case 0: // Click
            ClickSelectedControl();
            break;
        case 1: // Double Click
            DoubleClickSelectedControl();
            break;
        case 2: // Focus
            FocusSelectedControl();
            break;
        default:
            ClickSelectedControl();
            break;
        }
    }

    void MainWindow::ClickSelectedControl()
    {
        if (m_selectedControl.Element)
        {
            ControlInteraction::ClickControl(m_selectedControl.Element);
        }
    }

    void MainWindow::DoubleClickSelectedControl()
    {
        if (m_selectedControl.Element)
        {
            ControlInteraction::DoubleClickControl(m_selectedControl.Element);
        }
    }

    void MainWindow::FocusSelectedControl()
    {
        if (m_selectedControl.Element)
        {
            ControlInteraction::FocusControl(m_selectedControl.Element);
        }
    }

    void MainWindow::OnControlFound(const ControlInfo& controlInfo)
    {
        // Called from background thread - marshal to UI thread
        this->DispatcherQueue().TryEnqueue([this, controlInfo]()
        {
            m_allControls.push_back(controlInfo);
        });
    }

    void MainWindow::OnEnumerationFinished(const winrt::hstring& windowTitle)
    {
        // Marshal to UI thread
        this->DispatcherQueue().TryEnqueue([this, windowTitle]()
        {
            m_targetWindowTitle = windowTitle;
            WindowTitleLabel().Text(winrt::hstring(L"Controls for: ") + windowTitle);

            PopulateListView();
            HideLoadingOverlay();

            FilterTextBox().Text(L"");
            FilterTextBox().Focus(FocusState::Programmatic);

            AnnounceToScreenReader(winrt::hstring(L"Showing controls for ") + windowTitle);
        });
    }

    void MainWindow::OnEnumerationCancelled()
    {
        // Marshal to UI thread
        this->DispatcherQueue().TryEnqueue([this]()
        {
            HideLoadingOverlay();
            HideWindow();
        });
    }
}
