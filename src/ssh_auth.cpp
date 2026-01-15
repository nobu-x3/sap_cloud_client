#include "sap_cloud_client/ssh_auth.h"
#include <QDir>
#include <QFile>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/core_names.h>
#include <openssl/evp.h>
#include <openssl/pem.h>

namespace {

    // Encode a string in SSH wire format (length-prefixed)
    void write_ssh_string(QByteArray& out, const QByteArray& str) {
        uint32_t len = str.size();
        out.append(static_cast<char>((len >> 24) & 0xFF));
        out.append(static_cast<char>((len >> 16) & 0xFF));
        out.append(static_cast<char>((len >> 8) & 0xFF));
        out.append(static_cast<char>(len & 0xFF));
        out.append(str);
    }

    // Read a length-prefixed string from SSH wire format
    QByteArray read_ssh_string(const QByteArray& data, int& offset) {
        if (offset + 4 > data.size())
            return {};
        uint32_t len = (static_cast<unsigned char>(data[offset]) << 24) | (static_cast<unsigned char>(data[offset + 1]) << 16) |
            (static_cast<unsigned char>(data[offset + 2]) << 8) | static_cast<unsigned char>(data[offset + 3]);
        offset += 4;
        if (offset + static_cast<int>(len) > data.size())
            return {};
        QByteArray result = data.mid(offset, len);
        offset += len;
        return result;
    }

} // anonymous namespace

namespace sap::client {

    SshAuth::SshAuth() {}

    SshAuth::~SshAuth() {
        if (m_PrivateKey) {
            EVP_PKEY_free(m_PrivateKey);
        }
    }

    bool SshAuth::load_private_key(const QString& path, const QString& passphrase) {
        clear_error();

        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) {
            set_error("Cannot open private key file: " + path);
            return false;
        }

        QByteArray key_data = file.readAll();
        file.close();

        BIO* bio = BIO_new_mem_buf(key_data.data(), key_data.size());
        if (!bio) {
            set_error("Failed to create BIO");
            return false;
        }

        EVP_PKEY* pkey = nullptr;
        if (!passphrase.isEmpty()) {
            pkey = PEM_read_bio_PrivateKey(bio, nullptr, nullptr, const_cast<char*>(passphrase.toUtf8().constData()));
        } else {
            pkey = PEM_read_bio_PrivateKey(bio, nullptr, nullptr, nullptr);
        }
        BIO_free(bio);

        if (!pkey) {
            set_error("Failed to load private key - invalid format or wrong passphrase");
            return false;
        }

        // Check if it's Ed25519
        if (EVP_PKEY_id(pkey) != EVP_PKEY_ED25519) {
            EVP_PKEY_free(pkey);
            set_error("Only Ed25519 keys are supported");
            return false;
        }

        if (m_PrivateKey) {
            EVP_PKEY_free(m_PrivateKey);
        }
        m_PrivateKey = pkey;

        // Extract public key bytes
        size_t pub_len = 32;
        m_PublicKeyData.resize(pub_len);
        if (EVP_PKEY_get_raw_public_key(pkey, reinterpret_cast<unsigned char*>(m_PublicKeyData.data()), &pub_len) <= 0) {
            set_error("Failed to extract public key");
            return false;
        }

        return true;
    }

    bool SshAuth::load_public_key(const QString& path) {
        clear_error();

        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) {
            set_error("Cannot open public key file: " + path);
            return false;
        }

        QByteArray key_data = file.readAll().trimmed();
        file.close();

        // Check if it's SSH format (starts with "ssh-ed25519")
        if (key_data.startsWith("ssh-ed25519 ")) {
            QList<QByteArray> parts = key_data.split(' ');
            if (parts.size() < 2) {
                set_error("Invalid SSH public key format");
                return false;
            }

            QByteArray blob = QByteArray::fromBase64(parts[1]);
            int offset = 0;

            QByteArray key_type = read_ssh_string(blob, offset);
            if (key_type != "ssh-ed25519") {
                set_error("Unsupported key type: " + QString::fromUtf8(key_type));
                return false;
            }

            m_PublicKeyData = read_ssh_string(blob, offset);
            if (m_PublicKeyData.size() != 32) {
                set_error("Invalid Ed25519 public key size");
                return false;
            }

            return true;
        }

        set_error("Unsupported public key format (expected ssh-ed25519)");
        return false;
    }

    bool SshAuth::generate_key_pair() {
        clear_error();

        EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_ED25519, nullptr);
        if (!ctx) {
            set_error("Failed to create key generation context");
            return false;
        }

        if (EVP_PKEY_keygen_init(ctx) <= 0) {
            EVP_PKEY_CTX_free(ctx);
            set_error("Failed to initialize key generation");
            return false;
        }

        EVP_PKEY* pkey = nullptr;
        if (EVP_PKEY_keygen(ctx, &pkey) <= 0) {
            EVP_PKEY_CTX_free(ctx);
            set_error("Failed to generate Ed25519 key pair");
            return false;
        }

        EVP_PKEY_CTX_free(ctx);

        if (m_PrivateKey) {
            EVP_PKEY_free(m_PrivateKey);
        }
        m_PrivateKey = pkey;

        // Extract public key bytes
        size_t pub_len = 32;
        m_PublicKeyData.resize(pub_len);
        if (EVP_PKEY_get_raw_public_key(pkey, reinterpret_cast<unsigned char*>(m_PublicKeyData.data()), &pub_len) <= 0) {
            set_error("Failed to extract public key");
            return false;
        }

        return true;
    }

    bool SshAuth::save_private_key(const QString& path) {
        clear_error();

        if (!m_PrivateKey) {
            set_error("No private key loaded");
            return false;
        }

        BIO* bio = BIO_new(BIO_s_mem());
        if (!bio) {
            set_error("Failed to create BIO");
            return false;
        }

        if (PEM_write_bio_PrivateKey(bio, m_PrivateKey, nullptr, nullptr, 0, nullptr, nullptr) <= 0) {
            BIO_free(bio);
            set_error("Failed to write private key");
            return false;
        }

        BUF_MEM* mem = nullptr;
        BIO_get_mem_ptr(bio, &mem);

        QFile file(path);
        if (!file.open(QIODevice::WriteOnly)) {
            BIO_free(bio);
            set_error("Cannot open file for writing: " + path);
            return false;
        }

        file.write(mem->data, mem->length);
        file.close();
        file.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);
        BIO_free(bio);

        return true;
    }

    bool SshAuth::save_public_key(const QString& path) {
        clear_error();

        QString ssh_key = get_public_key_string();
        if (ssh_key.isEmpty()) {
            return false;
        }

        QFile file(path);
        if (!file.open(QIODevice::WriteOnly)) {
            set_error("Cannot open file for writing: " + path);
            return false;
        }

        file.write(ssh_key.toUtf8());
        file.write("\n");
        file.close();

        return true;
    }

    QString SshAuth::get_public_key_string() const {
        clear_error();

        if (m_PublicKeyData.isEmpty()) {
            set_error("No public key loaded");
            return QString();
        }

        // Build SSH wire format: string "ssh-ed25519" + string <32-byte key>
        QByteArray ssh_blob;
        write_ssh_string(ssh_blob, "ssh-ed25519");
        write_ssh_string(ssh_blob, m_PublicKeyData);

        return QString("ssh-ed25519 %1 sapcloud@client").arg(QString::fromLatin1(ssh_blob.toBase64()));
    }

    QByteArray SshAuth::sign_challenge(const QString& challenge) const {
        clear_error();

        if (!m_PrivateKey) {
            set_error("No private key loaded");
            return QByteArray();
        }

        // Decode the base64 challenge
        QByteArray challenge_bytes = QByteArray::fromBase64(challenge.toUtf8());
        if (challenge_bytes.isEmpty()) {
            set_error("Invalid challenge encoding");
            return QByteArray();
        }

        EVP_MD_CTX* md_ctx = EVP_MD_CTX_new();
        if (!md_ctx) {
            set_error("Failed to create message digest context");
            return QByteArray();
        }

        // Ed25519 uses nullptr for digest type
        if (EVP_DigestSignInit(md_ctx, nullptr, nullptr, nullptr, m_PrivateKey) <= 0) {
            EVP_MD_CTX_free(md_ctx);
            set_error("Failed to initialize signing");
            return QByteArray();
        }

        size_t sig_len = 64; // Ed25519 signatures are always 64 bytes
        QByteArray signature(sig_len, 0);

        if (EVP_DigestSign(md_ctx, reinterpret_cast<unsigned char*>(signature.data()), &sig_len,
                           reinterpret_cast<const unsigned char*>(challenge_bytes.data()), challenge_bytes.size()) <= 0) {
            EVP_MD_CTX_free(md_ctx);
            set_error("Failed to sign challenge");
            return QByteArray();
        }

        EVP_MD_CTX_free(md_ctx);
        signature.resize(sig_len);

        return signature.toBase64();
    }

} // namespace sap::client
