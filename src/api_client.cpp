#include "sap_cloud_client/api_client.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>

namespace sap::client {

    ApiClient::ApiClient(QObject* parent) : QObject(parent), m_Net(new QNetworkAccessManager(this)), m_BaseUrl("http://localhost:8080") {}

    QNetworkRequest ApiClient::make_request(const QString& endpoint) {
        QNetworkRequest req(QUrl(m_BaseUrl + endpoint));
        req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        if (!m_Token.isEmpty()) {
            req.setRawHeader("Authorization", ("Bearer " + m_Token).toUtf8());
        }
        return req;
    }

    void ApiClient::request_challenge(const QString& public_key, std::function<void(bool, AuthChallenge)> cb) {
        QJsonObject obj;
        obj["public_key"] = public_key;
        auto* reply = m_Net->post(make_request("/api/v1/auth/challenge"), QJsonDocument(obj).toJson());
        connect(reply, &QNetworkReply::finished, this, [this, reply, cb]() {
            reply->deleteLater();
            if (reply->error() != QNetworkReply::NoError) {
                emit error(reply->errorString());
                cb(false, {});
                return;
            }
            auto doc = QJsonDocument::fromJson(reply->readAll());
            cb(true, AuthChallenge::from_json(doc.object()));
        });
    }

    void ApiClient::verify_challenge(const QString& challenge, const QString& public_key, const QString& signature,
                                     std::function<void(bool, AuthToken)> cb) {
        QJsonObject obj;
        obj["challenge"] = challenge;
        obj["public_key"] = public_key;
        obj["signature"] = signature;
        auto* reply = m_Net->post(make_request("/api/v1/auth/verify"), QJsonDocument(obj).toJson());
        connect(reply, &QNetworkReply::finished, this, [this, reply, cb]() {
            reply->deleteLater();
            if (reply->error() != QNetworkReply::NoError) {
                emit error(reply->errorString());
                cb(false, {});
                return;
            }
            auto doc = QJsonDocument::fromJson(reply->readAll());
            auto token = AuthToken::from_json(doc.object());
            m_Token = token.token;
            emit authenticated();
            cb(true, token);
        });
    }

    void ApiClient::get_sync_state(std::function<void(bool, SyncState)> cb, std::optional<Timestamp> since) {
        QString endpoint = "/api/v1/sync/state";
        if (since.has_value()) {
            endpoint += "?since=" + QString::number(since.value());
        }
        auto* reply = m_Net->get(make_request(endpoint));
        connect(reply, &QNetworkReply::finished, this, [this, reply, cb]() {
            reply->deleteLater();
            if (reply->error() != QNetworkReply::NoError) {
                emit error(reply->errorString());
                cb(false, {});
                return;
            }
            auto doc = QJsonDocument::fromJson(reply->readAll());
            cb(true, SyncState::from_json(doc.object()));
        });
    }

    void ApiClient::list_files(std::function<void(bool, QVector<FileInfo>)> cb) {
        auto* reply = m_Net->get(make_request("/api/v1/files/"));
        connect(reply, &QNetworkReply::finished, this, [this, reply, cb]() {
            reply->deleteLater();
            if (reply->error() != QNetworkReply::NoError) {
                emit error(reply->errorString());
                cb(false, {});
                return;
            }
            auto doc = QJsonDocument::fromJson(reply->readAll());
            QVector<FileInfo> files;
            for (const auto& v : doc.array()) {
                files.append(FileInfo::from_json(v.toObject()));
            }
            cb(true, files);
        });
    }

    void ApiClient::get_file(const QString& path, std::function<void(bool, QByteArray)> cb) {
        auto* reply = m_Net->get(make_request("/api/v1/files/" + path));
        connect(reply, &QNetworkReply::finished, this, [this, reply, cb]() {
            reply->deleteLater();
            if (reply->error() != QNetworkReply::NoError) {
                emit error(reply->errorString());
                cb(false, {});
                return;
            }
            cb(true, reply->readAll());
        });
    }

    void ApiClient::upload_file(const QString& path, const QByteArray& data, std::function<void(bool)> cb) {
        QNetworkRequest req(QUrl(m_BaseUrl + "/api/v1/files/" + path));
        req.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");
        if (!m_Token.isEmpty()) {
            req.setRawHeader("Authorization", ("Bearer " + m_Token).toUtf8());
        }
        auto* reply = m_Net->put(req, data);
        connect(reply, &QNetworkReply::finished, this, [this, reply, cb]() {
            reply->deleteLater();
            if (reply->error() != QNetworkReply::NoError) {
                emit error(reply->errorString());
                cb(false);
                return;
            }
            cb(true);
        });
    }

    void ApiClient::delete_file(const QString& path, std::function<void(bool)> cb) {
        auto* reply = m_Net->deleteResource(make_request("/api/v1/files/" + path));
        connect(reply, &QNetworkReply::finished, this, [this, reply, cb]() {
            reply->deleteLater();
            cb(reply->error() == QNetworkReply::NoError);
        });
    }

    void ApiClient::list_notes(std::function<void(bool, QVector<NoteItem>)> cb) {
        auto* reply = m_Net->get(make_request("/api/v1/notes"));
        connect(reply, &QNetworkReply::finished, this, [this, reply, cb]() {
            reply->deleteLater();
            if (reply->error() != QNetworkReply::NoError) {
                emit error(reply->errorString());
                cb(false, {});
                return;
            }
            auto doc = QJsonDocument::fromJson(reply->readAll());
            QVector<NoteItem> notes;
            for (const auto& v : doc.object()["notes"].toArray()) {
                notes.append(NoteItem::from_json(v.toObject()));
            }
            cb(true, notes);
        });
    }

    void ApiClient::get_note(const QString& id, std::function<void(bool, Note)> cb) {
        auto* reply = m_Net->get(make_request("/api/v1/notes/" + id));
        connect(reply, &QNetworkReply::finished, this, [this, reply, cb]() {
            reply->deleteLater();
            if (reply->error() != QNetworkReply::NoError) {
                emit error(reply->errorString());
                cb(false, {});
                return;
            }
            auto doc = QJsonDocument::fromJson(reply->readAll());
            cb(true, Note::from_json(doc.object()));
        });
    }

    void ApiClient::create_note(const Note& note, std::function<void(bool, Note)> cb) {
        auto* reply = m_Net->post(make_request("/api/v1/notes"), QJsonDocument(note.to_json()).toJson());
        connect(reply, &QNetworkReply::finished, this, [this, reply, cb]() {
            reply->deleteLater();
            if (reply->error() != QNetworkReply::NoError) {
                emit error(reply->errorString());
                cb(false, {});
                return;
            }
            auto doc = QJsonDocument::fromJson(reply->readAll());
            cb(true, Note::from_json(doc.object()));
        });
    }

    void ApiClient::update_note(const QString& id, const Note& note, std::function<void(bool, Note)> cb) {
        auto* reply = m_Net->put(make_request("/api/v1/notes/" + id), QJsonDocument(note.to_json()).toJson());
        connect(reply, &QNetworkReply::finished, this, [this, reply, cb]() {
            reply->deleteLater();
            if (reply->error() != QNetworkReply::NoError) {
                emit error(reply->errorString());
                cb(false, {});
                return;
            }
            auto doc = QJsonDocument::fromJson(reply->readAll());
            cb(true, Note::from_json(doc.object()));
        });
    }

    void ApiClient::delete_note(const QString& id, std::function<void(bool)> cb) {
        auto* reply = m_Net->deleteResource(make_request("/api/v1/notes/" + id));
        connect(reply, &QNetworkReply::finished, this, [this, reply, cb]() {
            reply->deleteLater();
            cb(reply->error() == QNetworkReply::NoError);
        });
    }

    void ApiClient::get_tags(std::function<void(bool, QVector<QString>)> cb) {
        auto* reply = m_Net->get(make_request("/api/v1/notes/tags"));
        connect(reply, &QNetworkReply::finished, this, [this, reply, cb]() {
            reply->deleteLater();
            if (reply->error() != QNetworkReply::NoError) {
                emit error(reply->errorString());
                cb(false, {});
                return;
            }
            auto doc = QJsonDocument::fromJson(reply->readAll());
            QVector<QString> tags;
            for (const auto& t : doc.object()["tags"].toArray()) {
                tags.append(t.toString());
            }
            cb(true, tags);
        });
    }

    void ApiClient::search_notes(const QString& query, std::function<void(bool, QVector<NoteItem>)> cb) {
        QUrl url(m_BaseUrl + "/api/v1/notes/search");
        QUrlQuery q;
        q.addQueryItem("q", query);
        url.setQuery(q);
        QNetworkRequest req(url);
        req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        if (!m_Token.isEmpty()) {
            req.setRawHeader("Authorization", ("Bearer " + m_Token).toUtf8());
        }
        auto* reply = m_Net->get(req);
        connect(reply, &QNetworkReply::finished, this, [this, reply, cb]() {
            reply->deleteLater();
            if (reply->error() != QNetworkReply::NoError) {
                emit error(reply->errorString());
                cb(false, {});
                return;
            }
            auto doc = QJsonDocument::fromJson(reply->readAll());
            QVector<NoteItem> notes;
            for (const auto& v : doc.object()["notes"].toArray()) {
                notes.append(NoteItem::from_json(v.toObject()));
            }
            cb(true, notes);
        });
    }

} // namespace sap::client
