/*
 * UIAList - Accessibility Tool for Screen Reader Users
 * Copyright (C) 2025 Stefan Lohmaier
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "welcomedialog.h"
#include "settingsdialog.h"
#include <QApplication>
#include <QIcon>
#include <QSettings>

WelcomeDialog::WelcomeDialog(QWidget *parent)
    : QDialog(parent), m_mainLayout(nullptr), m_iconLayout(nullptr), 
      m_iconLabel(nullptr), m_titleLabel(nullptr), m_versionLabel(nullptr),
      m_welcomeText(nullptr), m_settingsButton(nullptr), m_closeButton(nullptr), m_buttonLayout(nullptr)
{
    setupUI();
    setWindowTitle("Welcome to UIAList");
    setFixedSize(550, 450);
    setModal(true);
    
    // Mark that welcome screen has been shown
    QSettings settings("UIAList", "Settings");
    settings.setValue("welcomeShown", true);
    settings.sync();
}

void WelcomeDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(15);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);
    
    // Icon and title section
    m_iconLayout = new QHBoxLayout();
    
    // Load and display the icon
    m_iconLabel = new QLabel();
    QIcon appIcon(":/icons/uialist_icon.png");
    if (!appIcon.isNull()) {
        QPixmap iconPixmap = appIcon.pixmap(64, 64);
        m_iconLabel->setPixmap(iconPixmap);
    } else {
        m_iconLabel->setText("[Icon]");
        m_iconLabel->setStyleSheet("border: 1px solid gray; padding: 20px;");
    }
    m_iconLabel->setAlignment(Qt::AlignCenter);
    m_iconLabel->setFixedSize(80, 80);
    
    // Title and version
    QVBoxLayout *titleLayout = new QVBoxLayout();
    
    m_titleLabel = new QLabel("Welcome to UIAList!");
    m_titleLabel->setStyleSheet("QLabel { font-size: 24px; font-weight: bold; color: palette(text); }");
    
    m_versionLabel = new QLabel("Accessibility Tool for Screen Reader Users");
    m_versionLabel->setStyleSheet("QLabel { font-size: 14px; color: palette(disabled-text); font-style: italic; }");
    
    titleLayout->addWidget(m_titleLabel);
    titleLayout->addWidget(m_versionLabel);
    titleLayout->addStretch();
    
    m_iconLayout->addWidget(m_iconLabel);
    m_iconLayout->addSpacing(15);
    m_iconLayout->addLayout(titleLayout);
    m_iconLayout->addStretch();
    
    // Welcome text
    m_welcomeText = new QTextBrowser();
    m_welcomeText->setFrameStyle(QFrame::NoFrame);
    m_welcomeText->setStyleSheet("QTextBrowser { background: transparent; border: none; }");
    m_welcomeText->setOpenExternalLinks(false);
    m_welcomeText->setMaximumHeight(200);
    
    QString welcomeContent = 
        "<h3>Thank you for using UIAList!</h3>"
        "<p><b>UIAList</b> is designed to make Windows applications more accessible for screen reader users.</p>"
        "<p><b>Getting Started:</b></p>"
        "<ul>"
        "<li><b>Global Shortcut:</b> Press <b>Ctrl+Alt+U</b> to activate UIAList on any window</li>"
        "<li><b>Search Controls:</b> Use the filter box to quickly find controls by name</li>"
        "<li><b>Interact:</b> Press Enter to perform the default action, or use the action buttons</li>"
        "<li><b>Settings:</b> Configure your preferences including the global shortcut key</li>"
        "</ul>"
        "<p><b>Tips:</b></p>"
        "<ul>"
        "<li>UIAList runs in the system tray - look for the icon near your clock</li>"
        "<li>You can customize the default action (Click, Double Click, or Focus) in Settings</li>"
        "<li>The app can start automatically with Windows if desired</li>"
        "</ul>";
    
    m_welcomeText->setHtml(welcomeContent);
    
    // Buttons
    m_buttonLayout = new QHBoxLayout();
    
    m_settingsButton = new QPushButton("Open Settings");
    m_settingsButton->setStyleSheet("QPushButton { padding: 8px 16px; border-radius: 4px; font-weight: bold; }");
    connect(m_settingsButton, &QPushButton::clicked, this, &WelcomeDialog::openSettings);
    
    m_closeButton = new QPushButton("Get Started");
    m_closeButton->setStyleSheet("QPushButton { padding: 8px 16px; border-radius: 4px; font-weight: bold; }");
    m_closeButton->setDefault(true);
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::accept);
    
    m_buttonLayout->addWidget(m_settingsButton);
    m_buttonLayout->addStretch();
    m_buttonLayout->addWidget(m_closeButton);
    
    // Add all components to main layout
    m_mainLayout->addLayout(m_iconLayout);
    m_mainLayout->addWidget(m_welcomeText);
    m_mainLayout->addStretch();
    m_mainLayout->addLayout(m_buttonLayout);
}

void WelcomeDialog::openSettings()
{
    SettingsDialog settings(this);
    settings.exec();
}