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

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QSettings>

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    enum DefaultAction {
        ActionClick = 0,
        ActionDoubleClick = 1,
        ActionFocus = 2
    };

    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

    bool autoStartEnabled() const;
    DefaultAction defaultAction() const;
    QKeySequence shortcutKey() const;

public slots:
    void accept() override;

private slots:
    void onCaptureShortcut();

private:
    void setupUI();
    void loadSettings();
    void saveSettings();
    void setAutoStartRegistry(bool enabled);
    bool getAutoStartRegistry() const;
    bool eventFilter(QObject *obj, QEvent *event) override;

    QVBoxLayout *m_mainLayout;
    QCheckBox *m_autoStartCheckBox;
    QLabel *m_defaultActionLabel;
    QComboBox *m_defaultActionComboBox;
    QLabel *m_shortcutLabel;
    QPushButton *m_shortcutButton;
    QDialogButtonBox *m_buttonBox;

    QSettings *m_settings;
    QKeySequence m_currentShortcut;
    bool m_capturingShortcut;
};

#endif // SETTINGSDIALOG_H