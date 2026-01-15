#pragma once

#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QProgressBar>
#include <QPushButton>
#include <QTreeWidget>
#include <QWidget>
#include "api_client.h"

namespace sap::client {

    class DriveScreen : public QWidget {
        Q_OBJECT

    public:
        explicit DriveScreen(ApiClient* api, QWidget* parent = nullptr);
        void refresh();

    private slots:
        void on_upload();
        void on_delete();
        void on_download();
        void on_rename();
        void on_info();
        void on_selection_changed();
        void on_search(const QString& text);
        void on_item_double_clicked(QTreeWidgetItem* item, int column);
        void on_context_menu(const QPoint& pos);

    private:
        void setup_ui();
        void load_files();
        QString format_size(qint64 bytes);
        QString format_time(qint64 ms);
        QString get_file_icon(const QString& path);
        void show_file_info_dialog(const FileInfo& file);

        ApiClient* m_Api;

        // Header
        QLabel* m_Title;
        QLineEdit* m_Search;

        // Content
        QTreeWidget* m_Tree;

        // Toolbar buttons
        QPushButton* m_UploadBtn;
        QPushButton* m_DownloadBtn;
        QPushButton* m_DeleteBtn;
        QPushButton* m_InfoBtn;

        // Status
        QLabel* m_Status;
        QProgressBar* m_Progress;

        // Data
        QVector<FileInfo> m_Files;
    };

} // namespace sap::client
