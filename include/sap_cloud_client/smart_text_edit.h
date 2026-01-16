#pragma once

#include <QKeyEvent>
#include <QTextEdit>

namespace sap::client {

    // QTextEdit with smart markdown editing behaviors:
    // - Auto-continue lists on Enter (*, -, +, 1., 2., etc.)
    // - Remove list marker on Enter if line is empty (double-Enter exits list)
    // - Auto-indent for nested lists
    class SmartTextEdit : public QTextEdit {
        Q_OBJECT

    public:
        explicit SmartTextEdit(QWidget* parent = nullptr);

    protected:
        void keyPressEvent(QKeyEvent* event) override;

    private:
        // Returns the list prefix for continuation, or empty string if not a list
        QString get_list_continuation(const QString& line);
        // Increments numbered list (e.g., "1. " -> "2. ")
        QString increment_number(const QString& prefix);
    };

} // namespace sap::client
