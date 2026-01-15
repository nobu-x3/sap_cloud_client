#pragma once

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <functional>
#include "types.h"

namespace sap::client {

    class ApiClient : public QObject {
        Q_OBJECT

    public:
        explicit ApiClient(QObject* parent = nullptr);

        void set_server_url(const QString& url) { m_BaseUrl = url; }
        void set_token(const QString& token) { m_Token = token; }
        QString token() const { return m_Token; }
        bool is_authenticated() const { return !m_Token.isEmpty(); }

        // Authentication
        void request_challenge(const QString& public_key, std::function<void(bool, AuthChallenge)> cb);
        void verify_challenge(const QString& challenge, const QString& public_key, const QString& signature,
                              std::function<void(bool, AuthToken)> cb);

        // Files
        void list_files(std::function<void(bool, QVector<FileInfo>)> cb);
        void get_file(const QString& path, std::function<void(bool, QByteArray)> cb);
        void upload_file(const QString& path, const QByteArray& data, std::function<void(bool)> cb);
        void delete_file(const QString& path, std::function<void(bool)> cb);

        // Sync
        void get_sync_state(std::function<void(bool, SyncState)> cb, std::optional<Timestamp> since = std::nullopt);

        // Notes
        void list_notes(std::function<void(bool, QVector<NoteItem>)> cb);
        void get_note(const QString& id, std::function<void(bool, Note)> cb);
        void create_note(const Note& note, std::function<void(bool, Note)> cb);
        void update_note(const QString& id, const Note& note, std::function<void(bool, Note)> cb);
        void delete_note(const QString& id, std::function<void(bool)> cb);
        void get_tags(std::function<void(bool, QVector<QString>)> cb);
        void search_notes(const QString& query, std::function<void(bool, QVector<NoteItem>)> cb);

    signals:
        void error(const QString& msg);
        void authenticated();

    private:
        QNetworkRequest make_request(const QString& endpoint);

        QNetworkAccessManager* m_Net;
        QString m_BaseUrl;
        QString m_Token;
    };

} // namespace sap::client
