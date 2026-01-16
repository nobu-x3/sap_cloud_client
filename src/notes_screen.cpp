#include "sap_cloud_client/notes_screen.h"
#include <QDateTime>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QMessageBox>
#include <QRegularExpression>
#include <QScrollBar>
#include <QShortcut>
#include <QVBoxLayout>
#include "sap_cloud_client/theme.h"

namespace sap::client {

    NotesScreen::NotesScreen(ApiClient* api, QWidget* parent) : QWidget(parent), m_Api(api) {
        setup_ui();

        // Auto-save timer (saves 2 seconds after last edit)
        m_AutoSaveTimer = new QTimer(this);
        m_AutoSaveTimer->setSingleShot(true);
        connect(m_AutoSaveTimer, &QTimer::timeout, this, &NotesScreen::on_auto_save);

        // Keyboard shortcuts
        auto* save_shortcut = new QShortcut(QKeySequence::Save, this);
        connect(save_shortcut, &QShortcut::activated, this, &NotesScreen::on_save_note);

        auto* new_shortcut = new QShortcut(QKeySequence::New, this);
        connect(new_shortcut, &QShortcut::activated, this, &NotesScreen::on_new_note);

        auto* preview_shortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_P), this);
        connect(preview_shortcut, &QShortcut::activated, this, &NotesScreen::on_toggle_preview);
    }

    void NotesScreen::setup_ui() {
        auto* main_layout = new QHBoxLayout(this);
        main_layout->setContentsMargins(0, 0, 0, 0);
        main_layout->setSpacing(0);

        // ===== Sidebar =====
        m_Sidebar = new QWidget(this);
        m_Sidebar->setFixedWidth(300);
        m_Sidebar->setStyleSheet("background-color: #16162a;");

        auto* sidebar_layout = new QVBoxLayout(m_Sidebar);
        sidebar_layout->setContentsMargins(16, 24, 16, 16);
        sidebar_layout->setSpacing(16);

        // Sidebar header
        auto* sidebar_header = new QHBoxLayout();

        auto* notes_title = new QLabel("Notes", this);
        notes_title->setStyleSheet("font-size: 20px; font-weight: 700; color: #ffffff;");
        sidebar_header->addWidget(notes_title);

        sidebar_header->addStretch();

        m_NewBtn = new QPushButton("+", this);
        m_NewBtn->setFixedSize(32, 32);
        m_NewBtn->setCursor(Qt::PointingHandCursor);
        m_NewBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #4a4ae8;
            border: none;
            border-radius: 8px;
            font-size: 18px;
            font-weight: 600;
            color: white;
        }
        QPushButton:hover {
            background-color: #5a5af8;
        }
    )");
        connect(m_NewBtn, &QPushButton::clicked, this, &NotesScreen::on_new_note);
        sidebar_header->addWidget(m_NewBtn);

        sidebar_layout->addLayout(sidebar_header);

        // Search
        m_Search = new QLineEdit(this);
        m_Search->setObjectName("search_input");
        m_Search->setPlaceholderText("Search notes...");
        connect(m_Search, &QLineEdit::textChanged, this, &NotesScreen::on_search);
        sidebar_layout->addWidget(m_Search);

        // Notes list
        m_List = new QListWidget(this);
        m_List->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        m_List->verticalScrollBar()->setSingleStep(15);
        connect(m_List, &QListWidget::itemClicked, this, &NotesScreen::on_note_clicked);
        sidebar_layout->addWidget(m_List, 1);

        // Collapse button
        m_CollapseBtn = new QPushButton(this);
        m_CollapseBtn->setIcon(QIcon(":/icons/chevron_left.svg"));
        m_CollapseBtn->setFixedSize(24, 48);
        m_CollapseBtn->setCursor(Qt::PointingHandCursor);
        m_CollapseBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #22223a;
            border: none;
            border-radius: 4px;
        }
        QPushButton:hover {
            background-color: #2a2a4a;
        }
    )");
        connect(m_CollapseBtn, &QPushButton::clicked, this, &NotesScreen::on_toggle_sidebar);

        main_layout->addWidget(m_Sidebar);
        main_layout->addWidget(m_CollapseBtn);

        // ===== Editor Container =====
        m_EditorContainer = new QWidget(this);
        auto* editor_layout = new QVBoxLayout(m_EditorContainer);
        editor_layout->setContentsMargins(32, 24, 32, 24);
        editor_layout->setSpacing(16);

        // ===== Empty State =====
        m_EmptyState = new QWidget(this);
        auto* empty_layout = new QVBoxLayout(m_EmptyState);
        empty_layout->setAlignment(Qt::AlignCenter);

        auto* empty_icon = new QLabel("📝", m_EmptyState);
        empty_icon->setStyleSheet("font-size: 64px;");
        empty_icon->setAlignment(Qt::AlignCenter);
        empty_layout->addWidget(empty_icon);

        auto* empty_title = new QLabel("No note selected", m_EmptyState);
        empty_title->setStyleSheet("font-size: 24px; font-weight: 600; color: #8888aa; margin-top: 16px;");
        empty_title->setAlignment(Qt::AlignCenter);
        empty_layout->addWidget(empty_title);

        auto* empty_subtitle = new QLabel("Select a note from the sidebar or create a new one", m_EmptyState);
        empty_subtitle->setStyleSheet("font-size: 14px; color: #6a6a8a;");
        empty_subtitle->setAlignment(Qt::AlignCenter);
        empty_layout->addWidget(empty_subtitle);

        auto* empty_btn = new QPushButton("+ Create New Note", m_EmptyState);
        empty_btn->setCursor(Qt::PointingHandCursor);
        empty_btn->setStyleSheet("margin-top: 24px; padding: 12px 24px;");
        connect(empty_btn, &QPushButton::clicked, this, &NotesScreen::on_new_note);
        empty_layout->addWidget(empty_btn, 0, Qt::AlignCenter);

        // Editor header
        auto* editor_header = new QHBoxLayout();

        m_Title = new QLineEdit(this);
        m_Title->setObjectName("title_input");
        m_Title->setPlaceholderText("Note title...");
        connect(m_Title, &QLineEdit::textChanged, this, [this]() {
            if (!m_Modified) {
                m_Modified = true;
                m_SaveBtn->setEnabled(true);
                m_AutoSaveTimer->start(2000);
            }
        });
        editor_header->addWidget(m_Title, 1);

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
            QPushButton:checked {
                background-color: #4a4ae8;
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

        // Editor toolbar
        m_PreviewBtn = new QPushButton(this);
        m_PreviewBtn->setIcon(QIcon(":/icons/preview.svg"));
        m_PreviewBtn->setIconSize(QSize(20, 20));
        m_PreviewBtn->setFixedSize(36, 36);
        m_PreviewBtn->setCursor(Qt::PointingHandCursor);
        m_PreviewBtn->setToolTip("Preview");
        m_PreviewBtn->setCheckable(true);
        m_PreviewBtn->setStyleSheet(icon_button_style);
        connect(m_PreviewBtn, &QPushButton::clicked, this, &NotesScreen::on_toggle_preview);
        editor_header->addWidget(m_PreviewBtn);

        m_SaveBtn = new QPushButton(this);
        m_SaveBtn->setIcon(QIcon(":/icons/save.svg"));
        m_SaveBtn->setIconSize(QSize(20, 20));
        m_SaveBtn->setFixedSize(36, 36);
        m_SaveBtn->setCursor(Qt::PointingHandCursor);
        m_SaveBtn->setToolTip("Save");
        m_SaveBtn->setStyleSheet(icon_button_style);
        m_SaveBtn->setEnabled(false);
        connect(m_SaveBtn, &QPushButton::clicked, this, &NotesScreen::on_save_note);
        editor_header->addWidget(m_SaveBtn);

        m_DeleteBtn = new QPushButton(this);
        m_DeleteBtn->setIcon(QIcon(":/icons/delete.svg"));
        m_DeleteBtn->setIconSize(QSize(20, 20));
        m_DeleteBtn->setFixedSize(36, 36);
        m_DeleteBtn->setCursor(Qt::PointingHandCursor);
        m_DeleteBtn->setToolTip("Delete");
        m_DeleteBtn->setStyleSheet(danger_button_style);
        m_DeleteBtn->setEnabled(false);
        connect(m_DeleteBtn, &QPushButton::clicked, this, &NotesScreen::on_delete_note);
        editor_header->addWidget(m_DeleteBtn);

        editor_layout->addLayout(editor_header);

        // Editor stack (editor/preview)
        m_EditorStack = new QStackedWidget(this);

        m_Editor = new SmartTextEdit(this);
        m_Editor->setPlaceholderText("Start writing...\n\nSupported Markdown:\n• # Headings\n• **bold** and *italic*\n• `code` and ```code "
                                     "blocks```\n• - Lists\n• > Blockquotes\n• [links](url)\n• --- horizontal rules");
        m_Editor->setAcceptRichText(false);
        m_Editor->setTabStopDistance(40);
        connect(m_Editor, &QTextEdit::textChanged, this, &NotesScreen::on_content_changed);

        m_Preview = new QTextBrowser(this);
        m_Preview->setOpenExternalLinks(true);
        m_Preview->document()->setDefaultStyleSheet(get_markdown_preview_style());

        m_EditorStack->addWidget(m_Editor);
        m_EditorStack->addWidget(m_Preview);

        editor_layout->addWidget(m_EditorStack, 1);

        // Status bar
        auto* status_bar = new QHBoxLayout();

        m_Status = new QLabel("Ready", this);
        m_Status->setObjectName("status_label");
        status_bar->addWidget(m_Status);

        status_bar->addStretch();

        m_WordCount = new QLabel("", this);
        m_WordCount->setObjectName("status_label");
        status_bar->addWidget(m_WordCount);

        editor_layout->addLayout(status_bar);

        // Stack the empty state and editor
        auto* content_stack = new QStackedWidget(this);
        content_stack->addWidget(m_EmptyState);
        content_stack->addWidget(m_EditorContainer);

        main_layout->addWidget(content_stack, 1);

        // Store reference to switch between states
        m_EditorContainer = content_stack;

        show_empty_state();
    }

    void NotesScreen::show_empty_state() { static_cast<QStackedWidget*>(m_EditorContainer)->setCurrentIndex(0); }

    void NotesScreen::hide_empty_state() { static_cast<QStackedWidget*>(m_EditorContainer)->setCurrentIndex(1); }

    void NotesScreen::refresh() { load_notes(); }

    void NotesScreen::load_notes() {
        m_Status->setText("Loading...");
        m_Api->list_notes([this](bool ok, QVector<NoteItem> notes) {
            if (!ok) {
                m_Status->setText("Failed to load notes");
                return;
            }

            m_Notes = notes;
            m_List->clear();

            // Sort by updated_at descending
            std::sort(m_Notes.begin(), m_Notes.end(), [](const NoteItem& a, const NoteItem& b) { return a.updated_at > b.updated_at; });

            for (const auto& n : m_Notes) {
                auto* item = new QListWidgetItem();

                QString title = n.title.isEmpty() ? "Untitled" : n.title;
                QString preview = n.preview.left(80).replace('\n', ' ');
                if (preview.length() >= 80)
                    preview += "...";

                // Format date
                QString date;
                QDateTime dt = QDateTime::fromMSecsSinceEpoch(n.updated_at);
                QDateTime now = QDateTime::currentDateTime();
                if (dt.date() == now.date()) {
                    date = dt.toString("h:mm AP");
                } else if (dt.daysTo(now) < 7) {
                    date = dt.toString("ddd");
                } else {
                    date = dt.toString("MMM d");
                }

                item->setText(title);
                item->setData(Qt::UserRole, n.id);
                item->setData(Qt::UserRole + 1, preview);
                item->setData(Qt::UserRole + 2, date);
                item->setToolTip(preview);

                m_List->addItem(item);
            }

            m_Status->setText(QString("%1 note%2").arg(notes.size()).arg(notes.size() != 1 ? "s" : ""));
        });
    }

    void NotesScreen::load_note(const QString& id) {
        m_Status->setText("Loading...");
        m_Api->get_note(id, [this, id](bool ok, Note note) {
            if (!ok) {
                m_Status->setText("Failed to load note");
                return;
            }

            hide_empty_state();

            m_CurrentId = id;
            m_Title->setText(note.title);
            m_Editor->setPlainText(note.content);
            m_Modified = false;
            m_SaveBtn->setEnabled(false);
            m_DeleteBtn->setEnabled(true);

            // Update preview if in preview mode
            if (m_PreviewMode) {
                m_Preview->setMarkdown(note.content);
            }

            update_word_count();
            m_Status->setText("Loaded");
        });
    }

    void NotesScreen::on_new_note() {
        if (m_Modified) {
            QMessageBox msg(this);
            msg.setWindowTitle("Save Changes");
            msg.setText("Save changes to current note?");
            msg.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
            msg.setDefaultButton(QMessageBox::Yes);
            msg.setStyleSheet(get_dark_stylesheet());

            int reply = msg.exec();
            if (reply == QMessageBox::Cancel)
                return;
            if (reply == QMessageBox::Yes)
                save_current_note();
        }

        hide_empty_state();
        clear_editor();
        m_CurrentId.clear();
        m_Title->setFocus();
        m_DeleteBtn->setEnabled(false);
        m_Status->setText("New note");
    }

    void NotesScreen::on_delete_note() {
        if (m_CurrentId.isEmpty())
            return;

        QMessageBox msg(this);
        msg.setWindowTitle("Delete Note");
        msg.setText("Are you sure you want to delete this note?");
        msg.setInformativeText("This action cannot be undone.");
        msg.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msg.setDefaultButton(QMessageBox::No);
        msg.setStyleSheet(get_dark_stylesheet());

        if (msg.exec() != QMessageBox::Yes)
            return;

        m_Status->setText("Deleting...");
        m_Api->delete_note(m_CurrentId, [this](bool ok) {
            if (ok) {
                clear_editor();
                m_CurrentId.clear();
                show_empty_state();
                load_notes();
                m_Status->setText("Deleted");
            } else {
                m_Status->setText("Delete failed");
            }
        });
    }

    void NotesScreen::on_save_note() { save_current_note(); }

    void NotesScreen::on_auto_save() {
        if (m_Modified && !m_CurrentId.isEmpty()) {
            save_current_note();
        }
    }

    void NotesScreen::save_current_note() {
        QString title = m_Title->text().trimmed();
        if (title.isEmpty()) {
            // Use first line of content as title
            QString content = m_Editor->toPlainText().trimmed();
            int newline = content.indexOf('\n');
            title = newline > 0 ? content.left(newline).trimmed() : content.left(50).trimmed();
            if (title.isEmpty())
                title = "Untitled";
            m_Title->setText(title);
        }

        Note note;
        note.title = title;
        note.content = m_Editor->toPlainText();

        m_Status->setText("Saving...");

        if (m_CurrentId.isEmpty()) {
            m_Api->create_note(note, [this](bool ok, Note created) {
                if (ok) {
                    m_CurrentId = created.id;
                    m_Modified = false;
                    m_SaveBtn->setEnabled(false);
                    m_DeleteBtn->setEnabled(true);
                    load_notes();
                    m_Status->setText("Created");
                } else {
                    m_Status->setText("Save failed");
                }
            });
        } else {
            m_Api->update_note(m_CurrentId, note, [this](bool ok, Note) {
                if (ok) {
                    m_Modified = false;
                    m_SaveBtn->setEnabled(false);
                    load_notes();
                    m_Status->setText("Saved");
                } else {
                    m_Status->setText("Save failed");
                }
            });
        }
    }

    void NotesScreen::on_note_clicked(QListWidgetItem* item) {
        if (!item)
            return;

        if (m_Modified) {
            QMessageBox msg(this);
            msg.setWindowTitle("Save Changes");
            msg.setText("Save changes to current note?");
            msg.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
            msg.setDefaultButton(QMessageBox::Yes);
            msg.setStyleSheet(get_dark_stylesheet());

            int reply = msg.exec();
            if (reply == QMessageBox::Cancel) {
                // Re-select the previous item
                for (int i = 0; i < m_List->count(); i++) {
                    if (m_List->item(i)->data(Qt::UserRole).toString() == m_CurrentId) {
                        m_List->setCurrentRow(i);
                        break;
                    }
                }
                return;
            }
            if (reply == QMessageBox::Yes)
                save_current_note();
        }

        QString id = item->data(Qt::UserRole).toString();
        load_note(id);
    }

    void NotesScreen::on_toggle_preview() {
        m_PreviewMode = !m_PreviewMode;

        if (m_PreviewMode) {
            // Use Qt's native markdown support
            m_Preview->setMarkdown(m_Editor->toPlainText());
            m_EditorStack->setCurrentWidget(m_Preview);
            m_PreviewBtn->setIcon(QIcon(":/icons/edit.svg"));
            m_PreviewBtn->setToolTip("Edit");
            m_PreviewBtn->setChecked(true);
        } else {
            m_EditorStack->setCurrentWidget(m_Editor);
            m_PreviewBtn->setIcon(QIcon(":/icons/preview.svg"));
            m_PreviewBtn->setToolTip("Preview");
            m_PreviewBtn->setChecked(false);
        }
    }

    void NotesScreen::on_toggle_sidebar() {
        m_SidebarCollapsed = !m_SidebarCollapsed;

        if (m_SidebarCollapsed) {
            m_Sidebar->setFixedWidth(0);
            m_Sidebar->hide();
            m_CollapseBtn->setIcon(QIcon(":/icons/chevron_right.svg"));
        } else {
            m_Sidebar->setFixedWidth(300);
            m_Sidebar->show();
            m_CollapseBtn->setIcon(QIcon(":/icons/chevron_left.svg"));
        }
    }

    void NotesScreen::on_search(const QString& text) {
        for (int i = 0; i < m_List->count(); ++i) {
            auto* item = m_List->item(i);
            QString title = item->text();
            QString preview = item->data(Qt::UserRole + 1).toString();

            bool match = text.isEmpty() || title.contains(text, Qt::CaseInsensitive) || preview.contains(text, Qt::CaseInsensitive);
            item->setHidden(!match);
        }
    }

    void NotesScreen::on_content_changed() {
        if (!m_Modified) {
            m_Modified = true;
            m_SaveBtn->setEnabled(true);
        }

        // Reset auto-save timer
        m_AutoSaveTimer->start(2000);

        // Update word count
        update_word_count();

        // Update preview in real-time if in preview mode
        if (m_PreviewMode) {
            m_Preview->setMarkdown(m_Editor->toPlainText());
        }
    }

    void NotesScreen::update_word_count() {
        QString text = m_Editor->toPlainText();
        int chars = text.length();
        int words = text.isEmpty() ? 0 : text.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts).count();

        m_WordCount->setText(QString("%1 words, %2 characters").arg(words).arg(chars));
    }

    void NotesScreen::clear_editor() {
        m_Title->clear();
        m_Editor->clear();
        m_Modified = false;
        m_SaveBtn->setEnabled(false);
        m_DeleteBtn->setEnabled(false);
        m_List->clearSelection();
        m_WordCount->clear();

        if (m_PreviewMode) {
            on_toggle_preview();
        }
    }

} // namespace sap::client
