#pragma once

#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QSplitter>
#include <QStackedWidget>
#include <QTextBrowser>
#include <QTimer>
#include <QWidget>
#include "api_client.h"
#include "smart_text_edit.h"

namespace sap::client {

    class NotesScreen : public QWidget {
        Q_OBJECT

    public:
        explicit NotesScreen(ApiClient* api, QWidget* parent = nullptr);
        void refresh();

    private slots:
        void on_new_note();
        void on_delete_note();
        void on_save_note();
        void on_note_clicked(QListWidgetItem* item);
        void on_toggle_preview();
        void on_toggle_sidebar();
        void on_search(const QString& text);
        void on_content_changed();
        void on_auto_save();

    private:
        void setup_ui();
        void load_notes();
        void load_note(const QString& id);
        void clear_editor();
        void save_current_note();
        void update_word_count();
        void show_empty_state();
        void hide_empty_state();

        ApiClient* m_Api;

        // Sidebar
        QWidget* m_Sidebar;
        QListWidget* m_List;
        QLineEdit* m_Search;
        QPushButton* m_NewBtn;
        QPushButton* m_CollapseBtn;
        bool m_SidebarCollapsed = false;

        // Editor area
        QWidget* m_EditorContainer;
        QWidget* m_EmptyState;
        QLineEdit* m_Title;
        SmartTextEdit* m_Editor;
        QTextBrowser* m_Preview;
        QStackedWidget* m_EditorStack;
        QPushButton* m_SaveBtn;
        QPushButton* m_DeleteBtn;
        QPushButton* m_PreviewBtn;
        QLabel* m_Status;
        QLabel* m_WordCount;

        // Auto-save timer
        QTimer* m_AutoSaveTimer;

        // Data
        QVector<NoteItem> m_Notes;
        QString m_CurrentId;
        bool m_Modified = false;
        bool m_PreviewMode = false;
    };

} // namespace sap::client
