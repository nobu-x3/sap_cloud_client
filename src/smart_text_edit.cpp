#include "sap_cloud_client/smart_text_edit.h"
#include <QRegularExpression>
#include <QTextCursor>

namespace sap::client {

    SmartTextEdit::SmartTextEdit(QWidget* parent) : QTextEdit(parent) {}

    void SmartTextEdit::keyPressEvent(QKeyEvent* event) {
        if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
            QTextCursor cursor = textCursor();
            cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
            QString current_line = cursor.selectedText();

            QString continuation = get_list_continuation(current_line);

            if (!continuation.isEmpty()) {
                // Check if the line is just the list marker (empty item)
                QString trimmed = current_line.trimmed();
                static QRegularExpression empty_bullet_re("^[\\*\\-\\+]$");
                static QRegularExpression empty_number_re("^\\d+\\.$");
                static QRegularExpression empty_task_re("^[\\*\\-\\+] \\[(x| )\\]$", QRegularExpression::CaseInsensitiveOption);

                if (empty_bullet_re.match(trimmed).hasMatch() || empty_number_re.match(trimmed).hasMatch() ||
                    empty_task_re.match(trimmed).hasMatch()) {
                    // Empty list item - remove the marker and don't continue
                    cursor.movePosition(QTextCursor::StartOfBlock);
                    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
                    cursor.removeSelectedText();
                    cursor.insertText("\n");
                    setTextCursor(cursor);
                    return;
                }

                // Continue the list
                QTextEdit::keyPressEvent(event);
                cursor = textCursor();
                cursor.insertText(continuation);
                setTextCursor(cursor);
                return;
            }
        }

        // Handle Tab for list indentation
        if (event->key() == Qt::Key_Tab) {
            QTextCursor cursor = textCursor();
            cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
            QString current_line = cursor.selectedText();

            if (!get_list_continuation(current_line).isEmpty()) {
                // In a list - add indentation
                cursor.movePosition(QTextCursor::StartOfBlock);
                cursor.insertText("    ");
                return;
            }
        }

        // Handle Shift+Tab for list outdentation
        if (event->key() == Qt::Key_Backtab) {
            QTextCursor cursor = textCursor();
            cursor.movePosition(QTextCursor::StartOfBlock);
            cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 4);
            QString prefix = cursor.selectedText();

            if (prefix == "    ") {
                cursor.removeSelectedText();
                return;
            }
        }

        QTextEdit::keyPressEvent(event);
    }

    QString SmartTextEdit::get_list_continuation(const QString& line) {
        // Match indentation + bullet lists: *, -, +
        static QRegularExpression bullet_re("^(\\s*)([\\*\\-\\+]) (.*)$");
        QRegularExpressionMatch m = bullet_re.match(line);
        if (m.hasMatch()) {
            QString indent = m.captured(1);
            QString marker = m.captured(2);
            QString content = m.captured(3);

            // Check for task list
            static QRegularExpression task_re("^\\[(x| )\\] ");
            if (task_re.match(content).hasMatch()) {
                return indent + marker + " [ ] ";
            }
            return indent + marker + " ";
        }

        // Match numbered lists: 1., 2., etc.
        static QRegularExpression number_re("^(\\s*)(\\d+)\\. (.*)$");
        m = number_re.match(line);
        if (m.hasMatch()) {
            QString indent = m.captured(1);
            QString number = m.captured(2);
            return indent + increment_number(number) + ". ";
        }

        return QString();
    }

    QString SmartTextEdit::increment_number(const QString& number) {
        bool ok;
        int n = number.toInt(&ok);
        if (ok) {
            return QString::number(n + 1);
        }
        return "1";
    }

} // namespace sap::client
