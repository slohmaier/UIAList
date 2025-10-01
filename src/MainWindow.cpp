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
#include "MainWindow.h"
#include "ControlInteraction.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;

namespace UIAList
{
    MainWindow::MainWindow()
    {
        // Create the WinUI3 window
        m_window = Window();
        m_window.Title(L"UIAList");

        // Create UI programmatically
        CreateUI();

        // Register for close event
        m_window.Closed({ this, &MainWindow::OnClosed });
    }

    MainWindow::~MainWindow()
    {
        if (m_window)
        {
            m_window.Close();
        }
    }

    void MainWindow::CreateUI()
    {
        // Create root panel
        StackPanel root;
        root.Padding({ 10, 10, 10, 10 });
        root.Spacing(10);

        // Create filter text box
        m_filterBox = TextBox();
        m_filterBox.PlaceholderText(L"Filter controls...");
        m_filterBox.TextChanged({ this, &MainWindow::OnFilterTextChanged });
        m_filterBox.KeyDown({ this, &MainWindow::OnFilterKeyDown });
        root.Children().Append(m_filterBox);

        // Create list view
        m_listView = ListView();
        m_listView.Height(400);
        m_listView.SelectionChanged({ this, &MainWindow::OnListSelectionChanged });
        root.Children().Append(m_listView);

        // Create button panel
        StackPanel buttonPanel;
        buttonPanel.Orientation(Orientation::Horizontal);
        buttonPanel.Spacing(10);

        m_clickButton = Button();
        m_clickButton.Content(box_value(L"Click"));
        m_clickButton.Click({ this, &MainWindow::OnClickButtonClick });
        buttonPanel.Children().Append(m_clickButton);

        m_focusButton = Button();
        m_focusButton.Content(box_value(L"Focus"));
        m_focusButton.Click({ this, &MainWindow::OnFocusButtonClick });
        buttonPanel.Children().Append(m_focusButton);

        m_doubleClickButton = Button();
        m_doubleClickButton.Content(box_value(L"Double Click"));
        m_doubleClickButton.Click({ this, &MainWindow::OnDoubleClickButtonClick });
        buttonPanel.Children().Append(m_doubleClickButton);

        root.Children().Append(buttonPanel);

        m_window.Content(root);
    }

    void MainWindow::Show()
    {
        m_window.Activate();
        StartEnumeration();
    }

    void MainWindow::Hide()
    {
        // Minimize or close window
    }

    void MainWindow::StartEnumeration()
    {
        m_allControls.clear();
        m_listView.Items().Clear();

        // Get foreground window
        HWND targetWindow = GetForegroundWindow();

        m_enumerator = std::make_unique<ControlEnumerator>();
        m_enumerator->EnumerateAsync(
            targetWindow,
            [this](const ControlInfo& control) { OnControlFound(control); },
            [this](const winrt::hstring& message) { OnEnumerationFinished(); },
            []() { /* cancelled */ }
        );
    }

    void MainWindow::OnControlFound(const ControlInfo& control)
    {
        m_allControls.push_back(control);
        // Update UI on dispatcher thread
        m_window.DispatcherQueue().TryEnqueue([this, control]() {
            m_listView.Items().Append(box_value(control.Name));
        });
    }

    void MainWindow::OnEnumerationFinished()
    {
        // Enumeration complete
    }

    void MainWindow::OnFilterTextChanged(winrt::Windows::Foundation::IInspectable const&,
                                        TextChangedEventArgs const&)
    {
        // Filter the list based on text
    }

    void MainWindow::OnFilterKeyDown(winrt::Windows::Foundation::IInspectable const&,
                                    winrt::Microsoft::UI::Xaml::Input::KeyRoutedEventArgs const& args)
    {
        // Handle arrow keys for list navigation
        using namespace winrt::Windows::System;

        if (args.Key() == VirtualKey::Down)
        {
            // Select next item
            if (m_selectedIndex < static_cast<int>(m_listView.Items().Size()) - 1)
            {
                m_selectedIndex++;
                m_listView.SelectedIndex(m_selectedIndex);
            }
            args.Handled(true);
        }
        else if (args.Key() == VirtualKey::Up)
        {
            // Select previous item
            if (m_selectedIndex > 0)
            {
                m_selectedIndex--;
                m_listView.SelectedIndex(m_selectedIndex);
            }
            args.Handled(true);
        }
        else if (args.Key() == VirtualKey::Enter)
        {
            // Execute default action (Click)
            OnClickButtonClick(nullptr, nullptr);
            args.Handled(true);
        }
    }

    void MainWindow::OnListSelectionChanged(winrt::Windows::Foundation::IInspectable const&,
                                          SelectionChangedEventArgs const&)
    {
        m_selectedIndex = m_listView.SelectedIndex();
    }

    void MainWindow::OnClickButtonClick(winrt::Windows::Foundation::IInspectable const&,
                                      RoutedEventArgs const&)
    {
        if (m_selectedIndex >= 0 && m_selectedIndex < static_cast<int>(m_allControls.size()))
        {
            const auto& control = m_allControls[m_selectedIndex];
            ControlInteraction::ClickControl(control.Element.get());
            Hide();
        }
    }

    void MainWindow::OnFocusButtonClick(winrt::Windows::Foundation::IInspectable const&,
                                      RoutedEventArgs const&)
    {
        if (m_selectedIndex >= 0 && m_selectedIndex < static_cast<int>(m_allControls.size()))
        {
            const auto& control = m_allControls[m_selectedIndex];
            ControlInteraction::FocusControl(control.Element.get());
            Hide();
        }
    }

    void MainWindow::OnDoubleClickButtonClick(winrt::Windows::Foundation::IInspectable const&,
                                            RoutedEventArgs const&)
    {
        if (m_selectedIndex >= 0 && m_selectedIndex < static_cast<int>(m_allControls.size()))
        {
            const auto& control = m_allControls[m_selectedIndex];
            ControlInteraction::DoubleClickControl(control.Element.get());
            Hide();
        }
    }

    void MainWindow::OnClosed(winrt::Windows::Foundation::IInspectable const&,
                             WindowEventArgs const&)
    {
        m_window = nullptr;
    }
}
