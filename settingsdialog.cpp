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

#include "settingsdialog.h"
#include <QApplication>
#include <QMessageBox>
#include <QDebug>
#include <QDir>
#include <QIcon>
#include <QKeyEvent>

#include <windows.h>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent), m_mainLayout(nullptr), m_autoStartCheckBox(nullptr),
      m_defaultActionLabel(nullptr), m_defaultActionComboBox(nullptr),
      m_shortcutLabel(nullptr), m_shortcutButton(nullptr), m_buttonBox(nullptr),
      m_settings(nullptr), m_currentShortcut(QKeySequence("Ctrl+Alt+U")), m_capturingShortcut(false)
{
    m_settings = new QSettings("UIAList", "Settings", this);
    setupUI();
    loadSettings();
}

SettingsDialog::~SettingsDialog()
{
}

void SettingsDialog::setupUI()
{
    setWindowTitle("UIAList Settings");
    setWindowIcon(QIcon(":/icons/uialist_icon.png"));
    setModal(true);
    resize(400, 200);

    m_mainLayout = new QVBoxLayout(this);

    // Auto-start checkbox
    m_autoStartCheckBox = new QCheckBox("Start automatically when Windows starts", this);
    m_autoStartCheckBox->setAccessibleName("Auto-start with Windows");
    m_autoStartCheckBox->setAccessibleDescription("When checked, UIAList will start automatically when Windows starts");
    m_mainLayout->addWidget(m_autoStartCheckBox);

    // Default action combobox
    m_defaultActionLabel = new QLabel("Default action when pressing Enter:", this);
    m_mainLayout->addWidget(m_defaultActionLabel);

    m_defaultActionComboBox = new QComboBox(this);
    m_defaultActionComboBox->addItem("Click", ActionClick);
    m_defaultActionComboBox->addItem("Double Click", ActionDoubleClick);
    m_defaultActionComboBox->addItem("Focus", ActionFocus);
    m_defaultActionComboBox->setAccessibleName("Default action");
    m_defaultActionComboBox->setAccessibleDescription("Choose what action to perform when pressing Enter on a UI element");
    m_defaultActionLabel->setBuddy(m_defaultActionComboBox);
    m_mainLayout->addWidget(m_defaultActionComboBox);

    // Shortcut key editor
    m_shortcutLabel = new QLabel("Global shortcut key:", this);
    m_mainLayout->addWidget(m_shortcutLabel);

    m_shortcutButton = new QPushButton(this);
    m_shortcutButton->setText("Ctrl+Alt+U");
    m_shortcutButton->setToolTip("Click to change shortcut, then press the desired key combination");
    connect(m_shortcutButton, &QPushButton::clicked, this, &SettingsDialog::onCaptureShortcut);
    m_mainLayout->addWidget(m_shortcutButton);

    // Button box
    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &SettingsDialog::reject);
    m_mainLayout->addWidget(m_buttonBox);

    setLayout(m_mainLayout);
}

void SettingsDialog::loadSettings()
{
    // Load auto-start setting from registry
    m_autoStartCheckBox->setChecked(getAutoStartRegistry());

    // Load default action
    int defaultAction = m_settings->value("defaultAction", ActionClick).toInt();
    m_defaultActionComboBox->setCurrentIndex(defaultAction);

    // Load shortcut key
    QString shortcutString = m_settings->value("shortcutKey", "Ctrl+Alt+U").toString();
    m_currentShortcut = QKeySequence::fromString(shortcutString);
    m_shortcutButton->setText(m_currentShortcut.toString());
}

void SettingsDialog::saveSettings()
{
    // Save auto-start setting to registry
    setAutoStartRegistry(m_autoStartCheckBox->isChecked());

    // Save default action
    m_settings->setValue("defaultAction", m_defaultActionComboBox->currentIndex());

    // Save shortcut key
    m_settings->setValue("shortcutKey", m_currentShortcut.toString());
    
    m_settings->sync();
}

void SettingsDialog::accept()
{
    saveSettings();
    QDialog::accept();
}

void SettingsDialog::onCaptureShortcut()
{
    if (m_capturingShortcut) {
        // Stop capturing
        m_capturingShortcut = false;
        m_shortcutButton->setText(m_currentShortcut.toString());
        m_shortcutButton->setStyleSheet("");
        removeEventFilter(this);
    } else {
        // Start capturing
        m_capturingShortcut = true;
        m_shortcutButton->setText("Press key combination...");
        m_shortcutButton->setStyleSheet("QPushButton { background-color: #ffcccc; }");
        installEventFilter(this);
        setFocus();
    }
}

bool SettingsDialog::autoStartEnabled() const
{
    return m_autoStartCheckBox->isChecked();
}

SettingsDialog::DefaultAction SettingsDialog::defaultAction() const
{
    return static_cast<DefaultAction>(m_defaultActionComboBox->currentIndex());
}

QKeySequence SettingsDialog::shortcutKey() const
{
    return m_currentShortcut;
}

void SettingsDialog::setAutoStartRegistry(bool enabled)
{
    HKEY hKey;
    LONG result = RegOpenKeyEx(HKEY_CURRENT_USER,
                              L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
                              0, KEY_SET_VALUE | KEY_QUERY_VALUE, &hKey);
    
    if (result != ERROR_SUCCESS) {
        qDebug() << "Failed to open registry key for auto-start";
        return;
    }

    if (enabled) {
        // Get the current executable path
        QString appPath = QApplication::applicationFilePath();
        appPath = QDir::toNativeSeparators(appPath);
        
        std::wstring widePath = appPath.toStdWString();
        
        result = RegSetValueEx(hKey, L"UIAList", 0, REG_SZ,
                              (const BYTE*)widePath.c_str(),
                              (widePath.length() + 1) * sizeof(wchar_t));
        
        if (result != ERROR_SUCCESS) {
            qDebug() << "Failed to set auto-start registry value";
        } else {
            qDebug() << "Auto-start enabled in registry";
        }
    } else {
        result = RegDeleteValue(hKey, L"UIAList");
        if (result == ERROR_SUCCESS) {
            qDebug() << "Auto-start disabled in registry";
        } else if (result != ERROR_FILE_NOT_FOUND) {
            qDebug() << "Failed to delete auto-start registry value";
        }
    }

    RegCloseKey(hKey);
}

bool SettingsDialog::getAutoStartRegistry() const
{
    HKEY hKey;
    LONG result = RegOpenKeyEx(HKEY_CURRENT_USER,
                              L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
                              0, KEY_QUERY_VALUE, &hKey);
    
    if (result != ERROR_SUCCESS) {
        return false;
    }

    DWORD dataSize = 0;
    result = RegQueryValueEx(hKey, L"UIAList", nullptr, nullptr, nullptr, &dataSize);
    
    RegCloseKey(hKey);
    
    return (result == ERROR_SUCCESS && dataSize > 0);
}

bool SettingsDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (m_capturingShortcut && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        
        // Get modifier keys
        Qt::KeyboardModifiers modifiers = keyEvent->modifiers();
        int key = keyEvent->key();
        
        // Ignore lone modifier keys
        if (key == Qt::Key_Control || key == Qt::Key_Alt || key == Qt::Key_Shift || 
            key == Qt::Key_Meta || key == Qt::Key_AltGr) {
            return true;
        }
        
        // Create key sequence
        QKeySequence newSequence(modifiers | key);
        
        // Validate the sequence (must have at least one modifier)
        if (modifiers != Qt::NoModifier && !newSequence.isEmpty()) {
            m_currentShortcut = newSequence;
            m_shortcutButton->setText(m_currentShortcut.toString());
            m_shortcutButton->setStyleSheet("");
            m_capturingShortcut = false;
            removeEventFilter(this);
            
            qDebug() << "New shortcut captured:" << m_currentShortcut.toString();
        }
        
        return true; // Consume the event
    }
    
    return QDialog::eventFilter(obj, event);
}