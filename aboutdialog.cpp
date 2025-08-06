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

#include "aboutdialog.h"
#include <QDesktopServices>
#include <QUrl>
#include <QApplication>
#include <QIcon>
#include <QMessageBox>
#include <QSettings>
#include <QProcess>

#include <windows.h>

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent), m_mainLayout(nullptr), m_iconLayout(nullptr), 
      m_iconLabel(nullptr), m_titleLabel(nullptr), m_versionLabel(nullptr),
      m_descriptionText(nullptr), m_copyrightLabel(nullptr), 
      m_githubButton(nullptr), m_resetButton(nullptr), m_closeButton(nullptr), m_buttonLayout(nullptr)
{
    setupUI();
    setWindowTitle("About UIAList");
    setFixedSize(500, 400);
    setModal(true);
}

void AboutDialog::setupUI()
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
    
    m_titleLabel = new QLabel("UIAList");
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
    
    // Description
    m_descriptionText = new QTextBrowser();
    m_descriptionText->setFrameStyle(QFrame::NoFrame);
    m_descriptionText->setStyleSheet("QTextBrowser { background: transparent; border: none; }");
    m_descriptionText->setOpenExternalLinks(false);
    m_descriptionText->setMaximumHeight(150);
    
    QString description = 
        "<p><b>UIAList</b> is an accessibility tool designed for blind and visually impaired users who rely on screen readers like JAWS, NVDA, or Windows Narrator.</p>"
        "<p>This application lists all controls by name and type for the current application with search functionality to quickly find and interact with controls. It saves significant time by eliminating the need for traditional tabbing navigation that is typical with screen readers.</p>"
        "<p><b>Key Benefits:</b></p>"
        "<ul>"
        "<li>Lists all controls instantly - no more tabbing through forms</li>"
        "<li>Search controls by name for rapid access</li>"
        "<li>Direct interaction (click, focus, double-click)</li>"
        "<li>Optimized for screen reader users</li>"
        "</ul>";
    
    m_descriptionText->setHtml(description);
    
    // Copyright and license
    m_copyrightLabel = new QLabel();
    m_copyrightLabel->setWordWrap(true);
    m_copyrightLabel->setStyleSheet("QLabel { font-size: 12px; padding: 10px; border-radius: 5px; background-color: palette(alternate-base); color: palette(text); }");
    
    QString copyrightText = 
        "Copyright © 2025 Stefan Lohmaier. All rights reserved.\n\n"
        "This program is free software: you can redistribute it and/or modify "
        "it under the terms of the GNU General Public License as published by "
        "the Free Software Foundation, either version 3 of the License, or "
        "(at your option) any later version.\n\n"
        "This program is distributed in the hope that it will be useful, "
        "but WITHOUT ANY WARRANTY; without even the implied warranty of "
        "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the "
        "GNU General Public License for more details.\n\n"
        "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED.";
    
    m_copyrightLabel->setText(copyrightText);
    
    // Buttons
    m_buttonLayout = new QHBoxLayout();
    
    m_githubButton = new QPushButton("Visit GitHub Repository");
    m_githubButton->setStyleSheet("QPushButton { padding: 8px 16px; border-radius: 4px; font-weight: bold; }");
    connect(m_githubButton, &QPushButton::clicked, this, &AboutDialog::openGitHubPage);
    
    m_resetButton = new QPushButton("Reset All Settings");
    m_resetButton->setStyleSheet("QPushButton { padding: 8px 16px; border-radius: 4px; font-weight: bold; background-color: #ff6b6b; color: white; }");
    m_resetButton->setToolTip("Reset all settings to default values and remove from autostart");
    connect(m_resetButton, &QPushButton::clicked, this, &AboutDialog::resetSettings);
    
    m_closeButton = new QPushButton("Close");
    m_closeButton->setStyleSheet("QPushButton { padding: 8px 16px; border-radius: 4px; font-weight: bold; }");
    connect(m_closeButton, &QPushButton::clicked, this, &QDialog::accept);
    
    m_buttonLayout->addWidget(m_githubButton);
    m_buttonLayout->addWidget(m_resetButton);
    m_buttonLayout->addStretch();
    m_buttonLayout->addWidget(m_closeButton);
    
    // Add all components to main layout
    m_mainLayout->addLayout(m_iconLayout);
    m_mainLayout->addWidget(m_descriptionText);
    m_mainLayout->addWidget(m_copyrightLabel);
    m_mainLayout->addStretch();
    m_mainLayout->addLayout(m_buttonLayout);
}

void AboutDialog::openGitHubPage()
{
    QDesktopServices::openUrl(QUrl("https://github.com/slohmaier/UIAList"));
}

void AboutDialog::resetSettings()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, 
        "Reset All Settings", 
        "This will:\n\n"
        "• Reset all application settings to default values\n"
        "• Remove UIAList from Windows startup\n"
        "• Reset global shortcut to Ctrl+Alt+U\n"
        "• Reset default action to Click\n\n"
        "Are you sure you want to continue?",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        // Remove from Windows startup registry
        removeFromAutoStart();
        
        // Clear all application settings
        QSettings settings("UIAList", "Settings");
        settings.clear();
        settings.sync();
        
        QMessageBox::information(this, 
            "Settings Reset", 
            "All settings have been reset to default values and UIAList has been removed from Windows startup.\n\n"
            "The application will restart with default settings.");
        
        // Close the about dialog
        accept();
        
        // Request application restart
        QApplication::quit();
        QProcess::startDetached(QApplication::applicationFilePath(), QStringList());
    }
}

void AboutDialog::removeFromAutoStart()
{
    HKEY hKey;
    LONG result = RegOpenKeyEx(HKEY_CURRENT_USER,
                              L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
                              0, KEY_SET_VALUE, &hKey);
    
    if (result == ERROR_SUCCESS) {
        RegDeleteValue(hKey, L"UIAList");
        RegCloseKey(hKey);
    }
}