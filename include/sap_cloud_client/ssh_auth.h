#pragma once

#include <QByteArray>
#include <QString>

typedef struct evp_pkey_st EVP_PKEY;

namespace sap::client {

    class SshAuth {
    public:
        SshAuth();
        ~SshAuth();

        // Load SSH key pair from files
        bool load_private_key(const QString& path, const QString& passphrase = "");
        bool load_public_key(const QString& path);

        // Generate new Ed25519 key pair
        bool generate_key_pair();

        // Save keys to files
        bool save_private_key(const QString& path);
        bool save_public_key(const QString& path);

        // Get public key in SSH format (ssh-ed25519 ... for sending to server)
        QString get_public_key_string() const;

        // Sign a challenge with the private key
        QByteArray sign_challenge(const QString& challenge) const;

        // Check if keys are loaded
        bool has_private_key() const { return m_PrivateKey != nullptr; }
        bool has_public_key() const { return !m_PublicKeyData.isEmpty(); }

        QString last_error() const { return m_LastError; }

    private:
        EVP_PKEY* m_PrivateKey = nullptr;
        QByteArray m_PublicKeyData; // 32-byte Ed25519 public key
        mutable QString m_LastError;

        void set_error(const QString& error) const { m_LastError = error; }
        void clear_error() const { m_LastError.clear(); }
    };

} // namespace sap::client
