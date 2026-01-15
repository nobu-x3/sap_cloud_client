#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QVector>
#include <optional>

namespace sap::client {

    using Timestamp = qint64;

    struct AuthChallenge {
        QString challenge;
        QString public_key;
        Timestamp expires_at = 0;

        static AuthChallenge from_json(const QJsonObject& obj) {
            AuthChallenge c;
            c.challenge = obj["challenge"].toString();
            c.public_key = obj["public_key"].toString();
            c.expires_at = obj["expires_at"].toInteger();
            return c;
        }
    };

    struct AuthToken {
        QString token;
        Timestamp expires_at = 0;

        static AuthToken from_json(const QJsonObject& obj) {
            AuthToken t;
            t.token = obj["token"].toString();
            t.expires_at = obj["expires_at"].toInteger();
            return t;
        }
    };

    struct FileInfo {
        QString path;
        QString hash;
        qint64 size = 0;
        Timestamp mtime = 0;
        Timestamp created_at = 0;
        Timestamp updated_at = 0;
        bool is_deleted = false;

        static FileInfo from_json(const QJsonObject& obj) {
            FileInfo f;
            f.path = obj["path"].toString();
            f.hash = obj["hash"].toString();
            f.size = obj["size"].toInteger();
            f.mtime = obj["mtime"].toInteger();
            f.created_at = obj["created_at"].toInteger();
            f.updated_at = obj["updated_at"].toInteger();
            f.is_deleted = obj["is_deleted"].toBool();
            return f;
        }
    };

    struct SyncState {
        Timestamp server_time = 0;
        QVector<FileInfo> files;

        static SyncState from_json(const QJsonObject& obj) {
            SyncState s;
            s.server_time = obj["server_time"].toInteger();
            for (const auto& v : obj["files"].toArray()) {
                s.files.append(FileInfo::from_json(v.toObject()));
            }
            return s;
        }
    };

    struct NoteItem {
        QString id;
        QString title;
        QVector<QString> tags;
        Timestamp updated_at = 0;
        QString preview;

        static NoteItem from_json(const QJsonObject& obj) {
            NoteItem n;
            n.id = obj["id"].toString();
            n.title = obj["title"].toString();
            n.updated_at = obj["updated_at"].toInteger();
            n.preview = obj["preview"].toString();
            for (const auto& t : obj["tags"].toArray()) {
                n.tags.append(t.toString());
            }
            return n;
        }
    };

    struct Note {
        QString id;
        QString title;
        QString content;
        QVector<QString> tags;
        Timestamp created_at = 0;
        Timestamp updated_at = 0;

        static Note from_json(const QJsonObject& obj) {
            Note n;
            n.id = obj["id"].toString();
            n.title = obj["title"].toString();
            n.content = obj["content"].toString();
            n.created_at = obj["created_at"].toInteger();
            n.updated_at = obj["updated_at"].toInteger();
            for (const auto& t : obj["tags"].toArray()) {
                n.tags.append(t.toString());
            }
            return n;
        }

        QJsonObject to_json() const {
            QJsonObject obj;
            obj["title"] = title;
            obj["content"] = content;
            QJsonArray tags_arr;
            for (const auto& t : tags)
                tags_arr.append(t);
            obj["tags"] = tags_arr;
            return obj;
        }
    };

} // namespace sap::client
