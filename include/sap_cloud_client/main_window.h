#pragma once

#include <QFrame>
#include <QLabel>
#include <QMainWindow>
#include <QPropertyAnimation>
#include <QStackedWidget>
#include <QToolButton>
#include "api_client.h"
#include "ssh_auth.h"

namespace sap::client {

    class DriveScreen;
    class NotesScreen;

    class MainWindow : public QMainWindow {
        Q_OBJECT

    public:
        explicit MainWindow(QWidget* parent = nullptr);

    private slots:
        void show_drive();
        void show_notes();
        void show_settings();
        void toggle_sidebar();
        void on_authenticated();
        void on_auth_error(const QString& msg);

    private:
        void setup_ui();
        void apply_theme();
        QToolButton* create_nav_button(const QString& text, const QString& icon_path);
        void update_nav_state();
        void authenticate();

        QVector<std::function<void()>> m_PostAuthenticationQueue;
        ApiClient* m_Api;
        SshAuth* m_SshAuth;
        QStackedWidget* m_Stack;
        DriveScreen* m_Drive;
        NotesScreen* m_Notes;

        // Sidebar
        QFrame* m_Sidebar;
        QWidget* m_SidebarContent;
        QToolButton* m_CollapseBtn;
        bool m_SidebarCollapsed = false;
        int m_SidebarExpandedWidth = 88;
        int m_SidebarCollapsedWidth = 0;

        // Navigation buttons
        QToolButton* m_DriveBtn;
        QToolButton* m_NotesBtn;
        QToolButton* m_SettingsBtn;

        int m_CurrentIndex = 0;
    };

} // namespace sap::client
