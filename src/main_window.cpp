#include "sap_cloud_client/main_window.h"
#include <QDebug>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QFont>
#include <QFormLayout>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QMessageBox>
#include <QSettings>
#include <QStatusBar>
#include <QVBoxLayout>
#include "sap_cloud_client/drive_screen.h"
#include "sap_cloud_client/notes_screen.h"
#include "sap_cloud_client/theme.h"

namespace sap::client {

    MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
        m_Api = new ApiClient(this);
        m_SshAuth = new SshAuth();

        QSettings settings("SapCloud", "Client");
        m_Api->set_server_url(settings.value("serverUrl", "http://localhost:8080").toString());

        connect(m_Api, &ApiClient::authenticated, this, &MainWindow::on_authenticated);
        connect(m_Api, &ApiClient::error, this, &MainWindow::on_auth_error);

        apply_theme();
        setup_ui();
        setWindowTitle("SapCloud");
        resize(1400, 900);
        setMinimumSize(1000, 600);

        // Attempt authentication on startup
        authenticate();

        m_PostAuthenticationQueue.push_back([this]() { show_drive(); });
    }

    void MainWindow::apply_theme() { setStyleSheet(get_dark_stylesheet()); }

    QToolButton* MainWindow::create_nav_button(const QString& text, const QString& icon_path) {
        auto* btn = new QToolButton(this);
        btn->setObjectName("nav_button");
        btn->setCheckable(true);
        btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        btn->setText(text);
        btn->setIcon(QIcon(icon_path));
        btn->setIconSize(QSize(24, 24));
        btn->setFixedSize(72, 64);
        btn->setCursor(Qt::PointingHandCursor);

        return btn;
    }

    void MainWindow::setup_ui() {
        auto* central = new QWidget(this);
        auto* main_layout = new QHBoxLayout(central);
        main_layout->setContentsMargins(0, 0, 0, 0);
        main_layout->setSpacing(0);

        // Sidebar container
        m_Sidebar = new QFrame(this);
        m_Sidebar->setObjectName("sidebar");
        m_Sidebar->setFixedWidth(m_SidebarExpandedWidth + 24); // +24 for collapse button

        auto* sidebar_outer_layout = new QHBoxLayout(m_Sidebar);
        sidebar_outer_layout->setContentsMargins(0, 0, 0, 0);
        sidebar_outer_layout->setSpacing(0);

        // Sidebar content
        m_SidebarContent = new QWidget(m_Sidebar);
        m_SidebarContent->setFixedWidth(m_SidebarExpandedWidth);

        auto* sidebar_layout = new QVBoxLayout(m_SidebarContent);
        sidebar_layout->setContentsMargins(8, 16, 8, 16);
        sidebar_layout->setSpacing(8);

        // App logo/title
        auto* logo_label = new QLabel("Sap", this);
        logo_label->setAlignment(Qt::AlignCenter);
        logo_label->setStyleSheet(R"(
        QLabel {
            font-size: 24px;
            font-weight: 800;
            color: #4a4ae8;
            padding: 16px 0;
        }
    )");
        sidebar_layout->addWidget(logo_label);

        sidebar_layout->addSpacing(16);

        // Navigation buttons with icons
        m_DriveBtn = create_nav_button("", ":/icons/drive.svg");
        connect(m_DriveBtn, &QToolButton::clicked, this, &MainWindow::show_drive);

        m_NotesBtn = create_nav_button("", ":/icons/notes.svg");
        connect(m_NotesBtn, &QToolButton::clicked, this, &MainWindow::show_notes);

        sidebar_layout->addWidget(m_DriveBtn);
        sidebar_layout->addWidget(m_NotesBtn);
        sidebar_layout->addStretch();

        // Settings at bottom
        m_SettingsBtn = create_nav_button("", ":/icons/settings.svg");
        connect(m_SettingsBtn, &QToolButton::clicked, this, &MainWindow::show_settings);
        sidebar_layout->addWidget(m_SettingsBtn);

        sidebar_outer_layout->addWidget(m_SidebarContent);

        // Collapse button - outside sidebar content so it remains visible when collapsed
        m_CollapseBtn = new QToolButton(this);
        m_CollapseBtn->setIcon(QIcon(":/icons/chevron_left.svg"));
        m_CollapseBtn->setIconSize(QSize(20, 20));
        m_CollapseBtn->setFixedSize(24, 48);
        m_CollapseBtn->setCursor(Qt::PointingHandCursor);
        m_CollapseBtn->setToolTip("Collapse sidebar");
        m_CollapseBtn->setStyleSheet(R"(
        QToolButton {
            background-color: #22223a;
            border: none;
            border-radius: 4px;
            color: #8888aa;
        }
        QToolButton:hover {
            background-color: #2a2a4a;
        }
    )");
        connect(m_CollapseBtn, &QToolButton::clicked, this, &MainWindow::toggle_sidebar);
        sidebar_outer_layout->addWidget(m_CollapseBtn);

        // Main content area
        auto* content_container = new QWidget(this);
        auto* content_layout = new QVBoxLayout(content_container);
        content_layout->setContentsMargins(0, 0, 0, 0);
        content_layout->setSpacing(0);

        m_Stack = new QStackedWidget(this);
        m_Drive = new DriveScreen(m_Api, this);
        m_Notes = new NotesScreen(m_Api, this);

        m_Stack->addWidget(m_Drive);
        m_Stack->addWidget(m_Notes);

        content_layout->addWidget(m_Stack);

        main_layout->addWidget(m_Sidebar);
        main_layout->addWidget(content_container, 1);

        setCentralWidget(central);
    }

    void MainWindow::toggle_sidebar() {
        m_SidebarCollapsed = !m_SidebarCollapsed;

        if (m_SidebarCollapsed) {
            m_SidebarContent->hide();
            m_Sidebar->setFixedWidth(m_SidebarCollapsedWidth + 24); // Just the collapse button width
            m_CollapseBtn->setIcon(QIcon(":/icons/chevron_right.svg"));
            m_CollapseBtn->setToolTip("Expand sidebar");
        } else {
            m_Sidebar->setFixedWidth(m_SidebarExpandedWidth + 24);
            m_SidebarContent->show();
            m_CollapseBtn->setIcon(QIcon(":/icons/chevron_left.svg"));
            m_CollapseBtn->setToolTip("Collapse sidebar");
        }
    }

    void MainWindow::update_nav_state() {
        m_DriveBtn->setChecked(m_CurrentIndex == 0);
        m_NotesBtn->setChecked(m_CurrentIndex == 1);
        m_SettingsBtn->setChecked(false);
    }

    void MainWindow::show_drive() {
        m_CurrentIndex = 0;
        m_Stack->setCurrentWidget(m_Drive);
        update_nav_state();
        m_Drive->refresh();
    }

    void MainWindow::show_notes() {
        m_CurrentIndex = 1;
        m_Stack->setCurrentWidget(m_Notes);
        update_nav_state();
        m_Notes->refresh();
    }

    void MainWindow::show_settings() {
        QSettings settings("SapCloud", "Client");

        // Create custom settings dialog
        QDialog dialog(this);
        dialog.setWindowTitle("Settings");
        dialog.setStyleSheet(get_dark_stylesheet());

        // Responsive sizing - fit to screen on mobile
        QSize screen_size = this->size();
        int dialog_width = qMin(500, screen_size.width() - 32);
        int dialog_height = qMin(350, screen_size.height() - 64);
        dialog.setMinimumSize(280, 250);
        dialog.resize(dialog_width, dialog_height);

        auto* layout = new QVBoxLayout(&dialog);
        layout->setContentsMargins(16, 16, 16, 16);
        layout->setSpacing(16);

        auto* title = new QLabel("Settings", &dialog);
        title->setStyleSheet("font-size: 18px; font-weight: 700; color: #ffffff;");
        layout->addWidget(title);

        // Use vertical layout for narrow screens
        auto* form = new QVBoxLayout();
        form->setSpacing(12);

        // Server URL
        auto* server_label = new QLabel("Server URL", &dialog);
        server_label->setStyleSheet("color: #8888aa; font-size: 12px;");
        form->addWidget(server_label);

        auto* server_input = new QLineEdit(&dialog);
        server_input->setText(settings.value("serverUrl", "http://localhost:8080").toString());
        server_input->setPlaceholderText("https://example.com:8080");
        form->addWidget(server_input);

        // SSH Key selection
        auto* ssh_label = new QLabel("SSH Key", &dialog);
        ssh_label->setStyleSheet("color: #8888aa; font-size: 12px;");
        form->addWidget(ssh_label);

        auto* ssh_input = new QLineEdit(&dialog);
        ssh_input->setText(settings.value("sshKeyPath", "").toString());
        ssh_input->setPlaceholderText("~/.ssh/id_rsa");
        ssh_input->setReadOnly(true);
        form->addWidget(ssh_input);

        auto* browse_btn = new QPushButton("Browse...", &dialog);
        browse_btn->setObjectName("secondary_button");
        browse_btn->setCursor(Qt::PointingHandCursor);
        connect(browse_btn, &QPushButton::clicked, [&]() {
            QString home = QDir::homePath();
            QString path = QFileDialog::getOpenFileName(&dialog, "Select SSH Key", home + "/.ssh", "All Files (*)");
            if (!path.isEmpty()) {
                ssh_input->setText(path);
            }
        });
        form->addWidget(browse_btn);

        layout->addLayout(form);
        layout->addStretch();

        // Buttons - horizontal layout
        auto* button_layout = new QHBoxLayout();
        button_layout->setSpacing(8);

        auto* cancel_btn = new QPushButton("Cancel", &dialog);
        cancel_btn->setObjectName("secondary_button");
        cancel_btn->setCursor(Qt::PointingHandCursor);
        connect(cancel_btn, &QPushButton::clicked, &dialog, &QDialog::reject);
        button_layout->addWidget(cancel_btn);

        auto* save_btn = new QPushButton("Save", &dialog);
        save_btn->setCursor(Qt::PointingHandCursor);
        connect(save_btn, &QPushButton::clicked, &dialog, &QDialog::accept);
        button_layout->addWidget(save_btn);

        layout->addLayout(button_layout);

        if (dialog.exec() == QDialog::Accepted) {
            QString url = server_input->text().trimmed();
            QString ssh_path = ssh_input->text().trimmed();

            if (!url.isEmpty()) {
                settings.setValue("serverUrl", url);
                m_Api->set_server_url(url);
            }

            settings.setValue("sshKeyPath", ssh_path);

            // Use statusBar instead of QMessageBox to avoid Android OpenGL deadlock
            statusBar()->showMessage("Settings saved successfully", 3000);
        }

        update_nav_state();
    }

    void MainWindow::authenticate() {
        QSettings settings("SapCloud", "Client");
        QString ssh_private_key_path = settings.value("sshKeyPath").toString();
        if (ssh_private_key_path.isEmpty()) {
            ssh_private_key_path = QDir::homePath() + "/.sapcloud/id_rsa";
        }
        // Try to load private key, generate if it doesn't exist
        if (!m_SshAuth->load_private_key(ssh_private_key_path)) {
            if (!m_SshAuth->generate_key_pair()) {
                QMessageBox::critical(this, "Authentication Error", "Failed to generate SSH key pair: " + m_SshAuth->last_error());
                return;
            }
            // Save the generated keys
            QString key_dir = QDir::homePath() + "/.sapcloud";
            QDir().mkpath(key_dir);
            m_SshAuth->save_private_key(key_dir + "/id_rsa");
            m_SshAuth->save_public_key(key_dir + "/id_rsa.pub");
            statusBar()->showMessage("Generated new SSH key pair in " + key_dir, 5000);
        }
        if (!m_SshAuth->load_public_key(ssh_private_key_path + ".pub")) {
            QMessageBox::critical(this, "Authentication Error", "Could not load public key: " + m_SshAuth->last_error());
            return;
        }
        // Get public key
        QString public_key = m_SshAuth->get_public_key_string();
        if (public_key.isEmpty()) {
            QMessageBox::critical(this, "Authentication Error", "Failed to read public key: " + m_SshAuth->last_error());
            return;
        }
        // Request challenge from server
        m_Api->request_challenge(public_key, [this, public_key](bool success, AuthChallenge challenge) {
            if (!success) {
                return; // Error already emitted via signal
            }
            // Sign the challenge
            QByteArray signature = m_SshAuth->sign_challenge(challenge.challenge);
            if (signature.isEmpty()) {
                QMessageBox::critical(this, "Authentication Error", "Failed to sign challenge: " + m_SshAuth->last_error());
                return;
            }
            // Verify signature with server
            m_Api->verify_challenge(challenge.challenge, public_key, QString::fromUtf8(signature),
                                    [this](bool success, AuthToken token) {});
        });
    }

    void MainWindow::on_authenticated() {
        statusBar()->showMessage("Authenticated successfully", 3000);
        for (auto it = m_PostAuthenticationQueue.rbegin(); it != m_PostAuthenticationQueue.rend(); ++it) {
            (*it)();
        }
        m_PostAuthenticationQueue.clear();
    }

    void MainWindow::on_auth_error(const QString& msg) {
        qDebug() << "Authentication error: " + msg;
        statusBar()->showMessage("Authentication error: " + msg, 5000);
    }

} // namespace sap::client
