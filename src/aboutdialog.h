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

#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPixmap>
#include <QTextBrowser>
#include <QSettings>

class UIAList;

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = nullptr);

private slots:
    void openGitHubPage();
    void resetSettings();

private:
    void setupUI();
    void removeFromAutoStart();
    
    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_iconLayout;
    QLabel *m_iconLabel;
    QLabel *m_titleLabel;
    QLabel *m_versionLabel;
    QTextBrowser *m_descriptionText;
    QLabel *m_copyrightLabel;
    QPushButton *m_githubButton;
    QPushButton *m_resetButton;
    QPushButton *m_closeButton;
    QHBoxLayout *m_buttonLayout;
};

#endif // ABOUTDIALOG_H