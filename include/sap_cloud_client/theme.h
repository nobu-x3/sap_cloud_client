#pragma once

#include <QString>

namespace sap::client {

    inline QString get_dark_stylesheet() {
        return R"(
        /* Global */
        QWidget {
            background-color: #1a1a2e;
            color: #eaeaea;
            font-family: 'Segoe UI', 'SF Pro Display', -apple-system, sans-serif;
            font-size: 13px;
        }

        /* Main Window */
        QMainWindow {
            background-color: #1a1a2e;
        }

        /* Sidebar */
        QFrame#sidebar {
            background-color: #16162a;
            border-right: 1px solid #2a2a4a;
        }

        QToolButton#nav_button {
            background-color: transparent;
            border: none;
            border-radius: 8px;
            padding: 12px;
            margin: 4px 8px;
            color: #8888aa;
        }

        QToolButton#nav_button:hover {
            background-color: #2a2a4a;
            color: #eaeaea;
        }

        QToolButton#nav_button:checked {
            background-color: #4a4ae8;
            color: #ffffff;
        }

        /* Scrollbars */
        QScrollBar:vertical {
            background-color: #1a1a2e;
            width: 10px;
            border-radius: 5px;
            margin: 0;
        }

        QScrollBar::handle:vertical {
            background-color: #3a3a5a;
            border-radius: 5px;
            min-height: 30px;
        }

        QScrollBar::handle:vertical:hover {
            background-color: #4a4a7a;
        }

        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0;
        }

        QScrollBar:horizontal {
            background-color: #1a1a2e;
            height: 10px;
            border-radius: 5px;
        }

        QScrollBar::handle:horizontal {
            background-color: #3a3a5a;
            border-radius: 5px;
            min-width: 30px;
        }

        QScrollBar::handle:horizontal:hover {
            background-color: #4a4a7a;
        }

        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
            width: 0;
        }

        /* Push Buttons */
        QPushButton {
            background-color: #4a4ae8;
            color: #ffffff;
            border: none;
            border-radius: 6px;
            padding: 8px 16px;
            font-weight: 500;
        }

        QPushButton:hover {
            background-color: #5a5af8;
        }

        QPushButton:pressed {
            background-color: #3a3ad8;
        }

        QPushButton:disabled {
            background-color: #2a2a4a;
            color: #6a6a8a;
        }

        QPushButton#danger_button {
            background-color: #e84a4a;
        }

        QPushButton#danger_button:hover {
            background-color: #f85a5a;
        }

        QPushButton#secondary_button {
            background-color: #2a2a4a;
            color: #eaeaea;
        }

        QPushButton#secondary_button:hover {
            background-color: #3a3a5a;
        }

        QPushButton#icon_button {
            background-color: transparent;
            border-radius: 6px;
            padding: 8px;
            min-width: 36px;
            max-width: 36px;
            min-height: 36px;
            max-height: 36px;
        }

        QPushButton#icon_button:hover {
            background-color: #2a2a4a;
        }

        /* Line Edits */
        QLineEdit {
            background-color: #2a2a4a;
            border: 1px solid #3a3a5a;
            border-radius: 8px;
            padding: 10px 14px;
            color: #eaeaea;
            selection-background-color: #4a4ae8;
        }

        QLineEdit:focus {
            border: 1px solid #4a4ae8;
        }

        QLineEdit::placeholder {
            color: #6a6a8a;
        }

        QLineEdit#search_input {
            background-color: #22223a;
            border: none;
            border-radius: 20px;
            padding: 10px 16px 10px 40px;
        }

        QLineEdit#title_input {
            background-color: transparent;
            border: none;
            font-size: 24px;
            font-weight: 600;
            padding: 8px 0;
        }

        QLineEdit#title_input:focus {
            border: none;
        }

        /* Text Edit */
        QTextEdit {
            background-color: #1e1e36;
            border: none;
            border-radius: 8px;
            padding: 16px;
            color: #eaeaea;
            selection-background-color: #4a4ae8;
            font-family: 'JetBrains Mono', 'Fira Code', 'SF Mono', Consolas, monospace;
            font-size: 14px;
            line-height: 1.6;
        }

        QTextEdit:focus {
            background-color: #22223a;
        }

        /* Text Browser (Preview) */
        QTextBrowser {
            background-color: #1e1e36;
            border: none;
            border-radius: 8px;
            padding: 20px;
            color: #eaeaea;
        }

        /* Tree Widget */
        QTreeWidget {
            background-color: #1e1e36;
            border: none;
            border-radius: 12px;
            padding: 8px;
            outline: none;
        }

        QTreeWidget::item {
            padding: 12px 8px;
            border-radius: 8px;
            margin: 2px 4px;
        }

        QTreeWidget::item:hover {
            background-color: #2a2a4a;
        }

        QTreeWidget::item:selected {
            background-color: #4a4ae8;
        }

        QHeaderView::section {
            background-color: #1e1e36;
            color: #8888aa;
            border: none;
            padding: 12px 8px;
            font-weight: 600;
            text-transform: uppercase;
            font-size: 11px;
            letter-spacing: 0.5px;
        }

        /* List Widget */
        QListWidget {
            background-color: transparent;
            border: none;
            outline: none;
        }

        QListWidget::item {
            background-color: #22223a;
            border-radius: 10px;
            padding: 14px 16px;
            margin: 4px 0;
        }

        QListWidget::item:hover {
            background-color: #2a2a4a;
        }

        QListWidget::item:selected {
            background-color: #4a4ae8;
        }

        /* Labels */
        QLabel {
            color: #eaeaea;
        }

        QLabel#section_title {
            font-size: 12px;
            font-weight: 600;
            color: #8888aa;
            text-transform: uppercase;
            letter-spacing: 1px;
            padding: 8px 0;
        }

        QLabel#page_title {
            font-size: 28px;
            font-weight: 700;
            color: #ffffff;
        }

        QLabel#status_label {
            color: #6a6a8a;
            font-size: 12px;
        }

        /* Splitter */
        QSplitter::handle {
            background-color: #2a2a4a;
            width: 1px;
        }

        QSplitter::handle:hover {
            background-color: #4a4ae8;
        }

        /* Stacked Widget */
        QStackedWidget {
            background-color: transparent;
        }

        /* Message Box */
        QMessageBox {
            background-color: #1a1a2e;
        }

        QMessageBox QLabel {
            color: #eaeaea;
        }

        /* Input Dialog */
        QInputDialog {
            background-color: #1a1a2e;
        }

        /* File Dialog */
        QFileDialog {
            background-color: #1a1a2e;
        }

        /* Menu */
        QMenu {
            background-color: #22223a;
            border: 1px solid #3a3a5a;
            border-radius: 8px;
            padding: 8px;
        }

        QMenu::item {
            padding: 10px 20px;
            border-radius: 6px;
        }

        QMenu::item:selected {
            background-color: #4a4ae8;
        }

        /* Tooltips */
        QToolTip {
            background-color: #22223a;
            color: #eaeaea;
            border: 1px solid #3a3a5a;
            border-radius: 6px;
            padding: 8px 12px;
        }

        /* Tab Widget (for future use) */
        QTabWidget::pane {
            border: none;
            background-color: #1e1e36;
            border-radius: 8px;
        }

        QTabBar::tab {
            background-color: transparent;
            color: #8888aa;
            padding: 12px 20px;
            border: none;
            margin-right: 4px;
        }

        QTabBar::tab:hover {
            color: #eaeaea;
        }

        QTabBar::tab:selected {
            color: #4a4ae8;
            border-bottom: 2px solid #4a4ae8;
        }

        /* Context Menu Button */
        QPushButton#context_menu_btn {
            background-color: transparent;
            border: none;
            color: #8888aa;
            font-size: 18px;
            padding: 4px 8px;
        }

        QPushButton#context_menu_btn:hover {
            color: #eaeaea;
            background-color: #2a2a4a;
            border-radius: 4px;
        }
    )";
    }

    inline QString get_markdown_preview_style() {
        return R"(
        <style>
            body {
                font-family: 'Segoe UI', 'SF Pro Display', -apple-system, sans-serif;
                line-height: 1.8;
                color: #eaeaea;
                background-color: #1e1e36;
                padding: 0;
                margin: 0;
            }
            h1 {
                font-size: 2em;
                font-weight: 700;
                color: #ffffff;
                margin: 1.5em 0 0.5em 0;
                padding-bottom: 0.3em;
                border-bottom: 1px solid #3a3a5a;
            }
            h2 {
                font-size: 1.5em;
                font-weight: 600;
                color: #ffffff;
                margin: 1.3em 0 0.4em 0;
                padding-bottom: 0.2em;
                border-bottom: 1px solid #3a3a5a;
            }
            h3 {
                font-size: 1.25em;
                font-weight: 600;
                color: #ffffff;
                margin: 1.2em 0 0.3em 0;
            }
            h4, h5, h6 {
                font-size: 1em;
                font-weight: 600;
                color: #cccccc;
                margin: 1em 0 0.2em 0;
            }
            p {
                margin: 0.8em 0;
            }
            a {
                color: #6a9fff;
                text-decoration: none;
            }
            a:hover {
                text-decoration: underline;
            }
            code {
                font-family: 'JetBrains Mono', 'Fira Code', 'SF Mono', Consolas, monospace;
                background-color: #2a2a4a;
                padding: 2px 6px;
                border-radius: 4px;
                font-size: 0.9em;
                color: #f8a5c2;
            }
            pre {
                background-color: #16162a;
                border-radius: 8px;
                padding: 16px;
                overflow-x: auto;
                margin: 1em 0;
                border: 1px solid #2a2a4a;
            }
            pre code {
                background: none;
                padding: 0;
                color: #eaeaea;
                font-size: 13px;
                line-height: 1.6;
            }
            blockquote {
                border-left: 4px solid #4a4ae8;
                margin: 1em 0;
                padding: 0.5em 0 0.5em 1em;
                color: #aaaacc;
                background-color: #22223a;
                border-radius: 0 8px 8px 0;
            }
            ul, ol {
                margin: 0.8em 0;
                padding-left: 1.5em;
            }
            li {
                margin: 0.4em 0;
            }
            li::marker {
                color: #4a4ae8;
            }
            hr {
                border: none;
                border-top: 1px solid #3a3a5a;
                margin: 2em 0;
            }
            table {
                border-collapse: collapse;
                width: 100%;
                margin: 1em 0;
            }
            th, td {
                border: 1px solid #3a3a5a;
                padding: 10px 14px;
                text-align: left;
            }
            th {
                background-color: #22223a;
                font-weight: 600;
            }
            tr:nth-child(even) {
                background-color: #1a1a2e;
            }
            img {
                max-width: 100%;
                border-radius: 8px;
            }
            .task-list-item {
                list-style: none;
                margin-left: -1.5em;
            }
            .task-list-item input {
                margin-right: 0.5em;
            }
            mark {
                background-color: #4a4ae855;
                color: #ffffff;
                padding: 2px 4px;
                border-radius: 3px;
            }
        </style>
    )";
    }

} // namespace sap::client
