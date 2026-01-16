#include "sap_cloud_client/drive_screen.h"
#include <QApplication>
#include <QClipboard>
#include <QDateTime>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QMimeDatabase>
#include <QVBoxLayout>
#include "sap_cloud_client/theme.h"

namespace sap::client {

    DriveScreen::DriveScreen(ApiClient* api, QWidget* parent) : QWidget(parent), m_Api(api) { setup_ui(); }

    QString DriveScreen::get_file_icon(const QString& path) {
        // Return empty - icons will be handled separately
        return "";
    }

    void DriveScreen::setup_ui() {
        auto* layout = new QVBoxLayout(this);
        layout->setContentsMargins(32, 24, 32, 24);
        layout->setSpacing(20);

        // Header row
        auto* header = new QHBoxLayout();
        header->setSpacing(16);

        m_Title = new QLabel("Drive", this);
        m_Title->setObjectName("page_title");
        header->addWidget(m_Title);

        header->addStretch();

        // Search box with icon
        auto* search_container = new QWidget(this);
        search_container->setFixedWidth(300);
        auto* search_layout = new QHBoxLayout(search_container);
        search_layout->setContentsMargins(0, 0, 0, 0);

        m_Search = new QLineEdit(this);
        m_Search->setObjectName("search_input");
        m_Search->setPlaceholderText("Search files...");
        connect(m_Search, &QLineEdit::textChanged, this, &DriveScreen::on_search);
        search_layout->addWidget(m_Search);

        header->addWidget(search_container);
        layout->addLayout(header);

        // Toolbar
        auto* toolbar = new QHBoxLayout();
        toolbar->setSpacing(12);

        auto icon_button_style = R"(
            QPushButton {
                background-color: #2a2a4a;
                border: none;
                border-radius: 8px;
                padding: 8px;
            }
            QPushButton:hover {
                background-color: #3a3a5a;
            }
            QPushButton:pressed {
                background-color: #4a4a6a;
            }
            QPushButton:disabled {
                background-color: #1a1a2a;
                opacity: 0.5;
            }
        )";

        auto danger_button_style = R"(
            QPushButton {
                background-color: #2a2a4a;
                border: none;
                border-radius: 8px;
                padding: 8px;
            }
            QPushButton:hover {
                background-color: #4a2a2a;
            }
            QPushButton:pressed {
                background-color: #6a3a3a;
            }
            QPushButton:disabled {
                background-color: #1a1a2a;
                opacity: 0.5;
            }
        )";

        m_UploadBtn = new QPushButton(this);
        m_UploadBtn->setIcon(QIcon(":/icons/upload.svg"));
        m_UploadBtn->setIconSize(QSize(20, 20));
        m_UploadBtn->setFixedSize(36, 36);
        m_UploadBtn->setCursor(Qt::PointingHandCursor);
        m_UploadBtn->setToolTip("Upload");
        m_UploadBtn->setStyleSheet(icon_button_style);
        connect(m_UploadBtn, &QPushButton::clicked, this, &DriveScreen::on_upload);

        m_DownloadBtn = new QPushButton(this);
        m_DownloadBtn->setIcon(QIcon(":/icons/download.svg"));
        m_DownloadBtn->setIconSize(QSize(20, 20));
        m_DownloadBtn->setFixedSize(36, 36);
        m_DownloadBtn->setCursor(Qt::PointingHandCursor);
        m_DownloadBtn->setToolTip("Download");
        m_DownloadBtn->setStyleSheet(icon_button_style);
        m_DownloadBtn->setEnabled(false);
        connect(m_DownloadBtn, &QPushButton::clicked, this, &DriveScreen::on_download);

        m_DeleteBtn = new QPushButton(this);
        m_DeleteBtn->setIcon(QIcon(":/icons/delete.svg"));
        m_DeleteBtn->setIconSize(QSize(20, 20));
        m_DeleteBtn->setFixedSize(36, 36);
        m_DeleteBtn->setCursor(Qt::PointingHandCursor);
        m_DeleteBtn->setToolTip("Delete");
        m_DeleteBtn->setStyleSheet(danger_button_style);
        m_DeleteBtn->setEnabled(false);
        connect(m_DeleteBtn, &QPushButton::clicked, this, &DriveScreen::on_delete);

        m_InfoBtn = new QPushButton(this);
        m_InfoBtn->setIcon(QIcon(":/icons/info.svg"));
        m_InfoBtn->setIconSize(QSize(20, 20));
        m_InfoBtn->setFixedSize(36, 36);
        m_InfoBtn->setCursor(Qt::PointingHandCursor);
        m_InfoBtn->setToolTip("Info");
        m_InfoBtn->setStyleSheet(icon_button_style);
        m_InfoBtn->setEnabled(false);
        connect(m_InfoBtn, &QPushButton::clicked, this, &DriveScreen::on_info);

        toolbar->addWidget(m_UploadBtn);
        toolbar->addWidget(m_DownloadBtn);
        toolbar->addWidget(m_DeleteBtn);
        toolbar->addWidget(m_InfoBtn);
        toolbar->addStretch();

        m_Status = new QLabel("Ready", this);
        m_Status->setObjectName("status_label");
        toolbar->addWidget(m_Status);

        layout->addLayout(toolbar);

        // File tree
        m_Tree = new QTreeWidget(this);
        m_Tree->setHeaderLabels({"Name", "Size", "Modified", "Type"});
        m_Tree->setRootIsDecorated(false);
        m_Tree->setAlternatingRowColors(false);
        m_Tree->setSelectionMode(QAbstractItemView::ExtendedSelection);
        m_Tree->setContextMenuPolicy(Qt::CustomContextMenu);
        m_Tree->setSortingEnabled(true);

        // Column sizing
        m_Tree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
        m_Tree->header()->setSectionResizeMode(1, QHeaderView::Fixed);
        m_Tree->header()->setSectionResizeMode(2, QHeaderView::Fixed);
        m_Tree->header()->setSectionResizeMode(3, QHeaderView::Fixed);
        m_Tree->header()->resizeSection(1, 100);
        m_Tree->header()->resizeSection(2, 160);
        m_Tree->header()->resizeSection(3, 120);

        connect(m_Tree, &QTreeWidget::itemSelectionChanged, this, &DriveScreen::on_selection_changed);
        connect(m_Tree, &QTreeWidget::itemDoubleClicked, this, &DriveScreen::on_item_double_clicked);
        connect(m_Tree, &QTreeWidget::customContextMenuRequested, this, &DriveScreen::on_context_menu);

        layout->addWidget(m_Tree, 1);

        // Progress bar (hidden by default)
        m_Progress = new QProgressBar(this);
        m_Progress->setVisible(false);
        m_Progress->setTextVisible(false);
        m_Progress->setFixedHeight(4);
        m_Progress->setStyleSheet(R"(
        QProgressBar {
            background-color: #2a2a4a;
            border: none;
            border-radius: 2px;
        }
        QProgressBar::chunk {
            background-color: #4a4ae8;
            border-radius: 2px;
        }
    )");
        layout->addWidget(m_Progress);
    }

    void DriveScreen::refresh() { load_files(); }

    void DriveScreen::load_files() {
        m_Status->setText("Loading...");
        m_Progress->setVisible(true);
        m_Progress->setRange(0, 0); // Indeterminate

        m_Api->list_files([this](bool ok, QVector<FileInfo> files) {
            m_Progress->setVisible(false);

            if (!ok) {
                m_Status->setText("Failed to load files");
                return;
            }

            m_Files = files;
            m_Tree->clear();

            int file_count = 0;
            for (const auto& f : files) {
                if (f.is_deleted)
                    continue;
                file_count++;

                auto* item = new QTreeWidgetItem(m_Tree);
                item->setText(0, f.path);
                item->setText(1, format_size(f.size));
                item->setText(2, format_time(f.mtime));

                // Get file type from extension
                QString ext = f.path.section('.', -1).toUpper();
                item->setText(3, ext.isEmpty() ? "File" : ext + " File");

                item->setData(0, Qt::UserRole, f.path);
            }

            m_Status->setText(QString("%1 file%2").arg(file_count).arg(file_count != 1 ? "s" : ""));
        });
    }

    void DriveScreen::on_upload() {
        QStringList paths = QFileDialog::getOpenFileNames(this, "Select Files to Upload");
        if (paths.isEmpty())
            return;

        m_Progress->setVisible(true);
        m_Progress->setRange(0, paths.size());
        m_Progress->setValue(0);

        int completed = 0;
        for (const QString& path : paths) {
            QFile file(path);
            if (!file.open(QIODevice::ReadOnly)) {
                QMessageBox msg(this);
                msg.setWindowTitle("Error");
                msg.setText(QString("Cannot open file: %1").arg(path));
                msg.setIcon(QMessageBox::Warning);
                msg.setStyleSheet(get_dark_stylesheet());
                msg.exec();
                continue;
            }

            QByteArray data = file.readAll();
            QString name = QFileInfo(path).fileName();

            m_Status->setText(QString("Uploading %1...").arg(name));

            m_Api->upload_file(name, data, [this, &completed, total = paths.size()](bool ok) {
                completed++;
                m_Progress->setValue(completed);

                if (completed >= total) {
                    m_Progress->setVisible(false);
                    if (ok) {
                        m_Status->setText("Upload complete");
                    } else {
                        m_Status->setText("Some uploads failed");
                    }
                    load_files();
                }
            });
        }
    }

    void DriveScreen::on_delete() {
        auto selected = m_Tree->selectedItems();
        if (selected.isEmpty())
            return;

        QString message = selected.size() == 1 ? QString("Delete '%1'?").arg(selected[0]->data(0, Qt::UserRole).toString())
                                               : QString("Delete %1 files?").arg(selected.size());

        QMessageBox msg(this);
        msg.setWindowTitle("Confirm Delete");
        msg.setText(message);
        msg.setInformativeText("This action cannot be undone.");
        msg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msg.setDefaultButton(QMessageBox::No);
        msg.setStyleSheet(get_dark_stylesheet());

        if (msg.exec() != QMessageBox::Yes)
            return;

        m_Progress->setVisible(true);
        m_Progress->setRange(0, selected.size());
        m_Progress->setValue(0);

        int completed = 0;
        for (auto* item : selected) {
            QString path = item->data(0, Qt::UserRole).toString();
            m_Status->setText(QString("Deleting %1...").arg(path));

            m_Api->delete_file(path, [this, &completed, total = selected.size()](bool ok) {
                completed++;
                m_Progress->setValue(completed);

                if (completed >= total) {
                    m_Progress->setVisible(false);
                    m_Status->setText("Deleted");
                    load_files();
                }
            });
        }
    }

    void DriveScreen::on_download() {
        auto selected = m_Tree->selectedItems();
        if (selected.isEmpty())
            return;

        for (auto* item : selected) {
            QString path = item->data(0, Qt::UserRole).toString();
            QString save_path = QFileDialog::getSaveFileName(this, "Save As", path);
            if (save_path.isEmpty())
                continue;

            m_Status->setText(QString("Downloading %1...").arg(path));
            m_Progress->setVisible(true);
            m_Progress->setRange(0, 0);

            m_Api->get_file(path, [this, save_path](bool ok, QByteArray data) {
                m_Progress->setVisible(false);

                if (!ok) {
                    m_Status->setText("Download failed");
                    return;
                }

                QFile file(save_path);
                if (file.open(QIODevice::WriteOnly)) {
                    file.write(data);
                    m_Status->setText("Downloaded successfully");
                } else {
                    m_Status->setText("Failed to save file");
                }
            });
        }
    }

    void DriveScreen::on_rename() {
        auto* item = m_Tree->currentItem();
        if (!item)
            return;

        QString old_path = item->data(0, Qt::UserRole).toString();

        QDialog dialog(this);
        dialog.setWindowTitle("Rename File");
        dialog.setStyleSheet(get_dark_stylesheet());
        dialog.setFixedWidth(400);

        auto* layout = new QVBoxLayout(&dialog);
        layout->setContentsMargins(24, 24, 24, 24);
        layout->setSpacing(16);

        auto* label = new QLabel("New name:", &dialog);
        layout->addWidget(label);

        auto* input = new QLineEdit(&dialog);
        input->setText(old_path);
        input->selectAll();
        layout->addWidget(input);

        auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
        connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
        layout->addWidget(buttons);

        if (dialog.exec() == QDialog::Accepted) {
            QString new_path = input->text().trimmed();
            if (!new_path.isEmpty() && new_path != old_path) {
                // Download, delete old, upload with new name
                m_Status->setText("Renaming...");
                m_Api->get_file(old_path, [this, old_path, new_path](bool ok, QByteArray data) {
                    if (!ok) {
                        m_Status->setText("Rename failed");
                        return;
                    }

                    m_Api->upload_file(new_path, data, [this, old_path](bool ok) {
                        if (!ok) {
                            m_Status->setText("Rename failed");
                            return;
                        }

                        m_Api->delete_file(old_path, [this](bool ok) {
                            if (ok) {
                                m_Status->setText("Renamed");
                                load_files();
                            } else {
                                m_Status->setText("Rename partially failed");
                            }
                        });
                    });
                });
            }
        }
    }

    void DriveScreen::on_info() {
        auto* item = m_Tree->currentItem();
        if (!item)
            return;

        QString path = item->data(0, Qt::UserRole).toString();
        for (const auto& f : m_Files) {
            if (f.path == path) {
                show_file_info_dialog(f);
                break;
            }
        }
    }

    void DriveScreen::show_file_info_dialog(const FileInfo& file) {
        QDialog dialog(this);
        dialog.setWindowTitle("File Information");
        dialog.setStyleSheet(get_dark_stylesheet());
        dialog.setFixedWidth(450);

        auto* layout = new QVBoxLayout(&dialog);
        layout->setContentsMargins(24, 24, 24, 24);
        layout->setSpacing(16);

        // File icon and name
        auto* header = new QLabel(file.path, &dialog);
        header->setStyleSheet("font-size: 18px; font-weight: 600; color: #ffffff;");
        header->setWordWrap(true);
        layout->addWidget(header);

        // Separator
        auto* sep = new QFrame(&dialog);
        sep->setFrameShape(QFrame::HLine);
        sep->setStyleSheet("background-color: #3a3a5a;");
        sep->setFixedHeight(1);
        layout->addWidget(sep);

        // Info rows
        auto* form = new QFormLayout();
        form->setSpacing(12);
        form->setLabelAlignment(Qt::AlignRight);

        auto create_label = [&](const QString& text) {
            auto* lbl = new QLabel(text, &dialog);
            lbl->setTextInteractionFlags(Qt::TextSelectableByMouse);
            return lbl;
        };

        form->addRow("Size:", create_label(format_size(file.size)));
        form->addRow("Modified:", create_label(format_time(file.mtime)));
        form->addRow("Created:", create_label(format_time(file.created_at)));
        form->addRow("Hash:", create_label(file.hash.left(16) + "..."));

        layout->addLayout(form);

        // Copy hash button
        auto* copy_btn = new QPushButton("Copy Full Hash", &dialog);
        copy_btn->setObjectName("secondary_button");
        connect(copy_btn, &QPushButton::clicked, [&]() {
            QApplication::clipboard()->setText(file.hash);
            copy_btn->setText("Copied!");
        });
        layout->addWidget(copy_btn);

        // Close button
        auto* close_btn = new QPushButton("Close", &dialog);
        connect(close_btn, &QPushButton::clicked, &dialog, &QDialog::accept);
        layout->addWidget(close_btn);

        dialog.exec();
    }

    void DriveScreen::on_selection_changed() {
        bool has_selection = !m_Tree->selectedItems().isEmpty();
        m_DownloadBtn->setEnabled(has_selection);
        m_DeleteBtn->setEnabled(has_selection);
        m_InfoBtn->setEnabled(m_Tree->selectedItems().size() == 1);
    }

    void DriveScreen::on_search(const QString& text) {
        for (int i = 0; i < m_Tree->topLevelItemCount(); ++i) {
            auto* item = m_Tree->topLevelItem(i);
            bool match = text.isEmpty() || item->text(0).contains(text, Qt::CaseInsensitive);
            item->setHidden(!match);
        }
    }

    void DriveScreen::on_item_double_clicked(QTreeWidgetItem* item, int) {
        if (!item)
            return;
        on_download();
    }

    void DriveScreen::on_context_menu(const QPoint& pos) {
        auto* item = m_Tree->itemAt(pos);
        if (!item)
            return;

        m_Tree->setCurrentItem(item);

        QMenu menu(this);
        menu.setStyleSheet(get_dark_stylesheet());

        auto* download_action = menu.addAction("Download");
        download_action->setIcon(QIcon(":/icons/download.svg"));
        connect(download_action, &QAction::triggered, this, &DriveScreen::on_download);

        auto* rename_action = menu.addAction("Rename");
        rename_action->setIcon(QIcon(":/icons/edit.svg"));
        connect(rename_action, &QAction::triggered, this, &DriveScreen::on_rename);

        auto* info_action = menu.addAction("Info");
        info_action->setIcon(QIcon(":/icons/info.svg"));
        connect(info_action, &QAction::triggered, this, &DriveScreen::on_info);

        menu.addSeparator();

        auto* delete_action = menu.addAction("Delete");
        delete_action->setIcon(QIcon(":/icons/delete.svg"));
        connect(delete_action, &QAction::triggered, this, &DriveScreen::on_delete);

        menu.exec(m_Tree->viewport()->mapToGlobal(pos));
    }

    QString DriveScreen::format_size(qint64 bytes) {
        if (bytes < 1024)
            return QString::number(bytes) + " B";
        if (bytes < 1024 * 1024)
            return QString::number(bytes / 1024.0, 'f', 1) + " KB";
        if (bytes < 1024LL * 1024 * 1024)
            return QString::number(bytes / (1024.0 * 1024), 'f', 1) + " MB";
        return QString::number(bytes / (1024.0 * 1024 * 1024), 'f', 2) + " GB";
    }

    QString DriveScreen::format_time(qint64 ms) {
        if (ms == 0)
            return "-";
        QDateTime dt = QDateTime::fromMSecsSinceEpoch(ms);
        QDateTime now = QDateTime::currentDateTime();

        if (dt.date() == now.date()) {
            return "Today " + dt.toString("HH:mm");
        } else if (dt.date() == now.date().addDays(-1)) {
            return "Yesterday " + dt.toString("HH:mm");
        } else if (dt.daysTo(now) < 7) {
            return dt.toString("dddd HH:mm");
        } else {
            return dt.toString("MMM d, yyyy");
        }
    }

} // namespace sap::client
