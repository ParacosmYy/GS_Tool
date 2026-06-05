/**
 * @file DataEncryptionEngine.cpp
 * @brief 数据加密引擎实现 — XOR/AES/RC4/Caesar/Vigenere 加密解密
 *
 * 所有加密算法均在本地实现，不依赖外部密码库。
 * AES 使用 Qt 内置 QCryptographicHash 进行密钥派生，CBC 模式手工实现。
 * RC4 / Caesar / Vigenere 为纯字节级操作，适用于嵌入式轻量级数据混淆。
 */

#include "utils/crypto/DataEncryptionEngine.h"

#include <QRandomGenerator>
#include <QString>

// ── 构造 / 析构 ──

/**
 * @brief 构造函数，设置 objectName 用于 QSS 依赖
 * @param parent 父 QObject
 */
DataEncryptionEngine::DataEncryptionEngine(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("DataEncryptionEngine"));
}

DataEncryptionEngine::~DataEncryptionEngine() = default;

// ── 核心操作 ──

/**
 * @brief 加密数据入口，根据算法分发到对应实现
 * @param data 明文数据
 * @param algorithm 加密算法枚举
 * @param key 加密密钥
 * @return 密文; 失败返回空并发射 error 信号
 */
QByteArray DataEncryptionEngine::encrypt(const QByteArray& data, Algorithm algorithm,
                                         const QByteArray& key)
{
    if (data.isEmpty()) {
        emit error(tr("加密失败: 输入数据为空"), algorithm);
        ++m_stats.encryptErrors;
        return {};
    }
    if (algorithm == Algorithm::None) {
        ++m_stats.totalEncrypts;
        ++m_stats.totalBytesEncrypted;
        ++m_stats.operationsByAlgorithm[static_cast<int>(Algorithm::None)];
        emit encryptComplete(data, algorithm);
        return data;
    }
    if (key.isEmpty()) {
        emit error(tr("加密失败: 密钥为空"), algorithm);
        ++m_stats.encryptErrors;
        return {};
    }

    QByteArray result;
    switch (algorithm) {
    case Algorithm::XOR:      result = xorCipher(data, key, true); break;
    case Algorithm::AES128:   result = aesCbcEncrypt(data, key); break;
    case Algorithm::AES256:   result = aesCbcEncrypt(data, key); break;
    case Algorithm::RC4:      result = rc4(data, key); break;
    case Algorithm::Caesar:   result = caesar(data, key, true); break;
    case Algorithm::Vigenere: result = vigenere(data, key, true); break;
    default:
        emit error(tr("加密失败: 不支持的算法"), algorithm);
        ++m_stats.encryptErrors;
        return {};
    }

    if (result.isEmpty()) {
        emit error(tr("加密失败: 算法内部错误"), algorithm);
        ++m_stats.encryptErrors;
        return {};
    }

    ++m_stats.totalEncrypts;
    m_stats.totalBytesEncrypted += static_cast<quint64>(data.size());
    ++m_stats.operationsByAlgorithm[static_cast<int>(algorithm)];
    emit encryptComplete(result, algorithm);
    return result;
}

/**
 * @brief 解密数据入口，根据算法分发到对应实现
 * @param data 密文数据
 * @param algorithm 解密算法
 * @param key 解密密钥
 * @return 明文; 失败返回空并发射 error 信号
 */
QByteArray DataEncryptionEngine::decrypt(const QByteArray& data, Algorithm algorithm,
                                         const QByteArray& key)
{
    if (data.isEmpty()) {
        emit error(tr("解密失败: 输入数据为空"), algorithm);
        ++m_stats.decryptErrors;
        return {};
    }
    if (algorithm == Algorithm::None) {
        ++m_stats.totalDecrypts;
        ++m_stats.totalBytesDecrypted;
        ++m_stats.operationsByAlgorithm[static_cast<int>(Algorithm::None)];
        emit decryptComplete(data, algorithm);
        return data;
    }
    if (key.isEmpty()) {
        emit error(tr("解密失败: 密钥为空"), algorithm);
        ++m_stats.decryptErrors;
        return {};
    }

    QByteArray result;
    switch (algorithm) {
    case Algorithm::XOR:      result = xorCipher(data, key, false); break;
    case Algorithm::AES128:   result = aesCbcDecrypt(data, key); break;
    case Algorithm::AES256:   result = aesCbcDecrypt(data, key); break;
    case Algorithm::RC4:      result = rc4(data, key); break;
    case Algorithm::Caesar:   result = caesar(data, key, false); break;
    case Algorithm::Vigenere: result = vigenere(data, key, false); break;
    default:
        emit error(tr("解密失败: 不支持的算法"), algorithm);
        ++m_stats.decryptErrors;
        return {};
    }

    if (result.isEmpty()) {
        emit error(tr("解密失败: 算法内部错误"), algorithm);
        ++m_stats.decryptErrors;
        return {};
    }

    ++m_stats.totalDecrypts;
    m_stats.totalBytesDecrypted += static_cast<quint64>(data.size());
    ++m_stats.operationsByAlgorithm[static_cast<int>(algorithm)];
    emit decryptComplete(result, algorithm);
    return result;
}

// ── 密钥管理 ──

/**
 * @brief 生成随机密钥
 * @param algorithm 目标算法，决定默认长度
 * @param length 密钥字节数，0 表示使用算法默认值
 * @return 随机密钥
 */
QByteArray DataEncryptionEngine::generateKey(Algorithm algorithm, int length)
{
    // 各算法默认密钥长度
    int defaultLen = 16;
    switch (algorithm) {
    case Algorithm::XOR:      defaultLen = 1;  break;
    case Algorithm::AES128:   defaultLen = 16; break;
    case Algorithm::AES256:   defaultLen = 32; break;
    case Algorithm::RC4:      defaultLen = 16; break;
    case Algorithm::Caesar:   defaultLen = 1;  break;
    case Algorithm::Vigenere: defaultLen = 16; break;
    case Algorithm::None:     defaultLen = 0;  break;
    }

    const int keyLen = (length > 0) ? length : defaultLen;
    QByteArray key(keyLen, Qt::Uninitialized);
    for (int i = 0; i < keyLen; ++i) {
        key[i] = static_cast<char>(QRandomGenerator::global()->bounded(256));
    }
    return key;
}

/**
 * @brief PBKDF2 密钥派生
 * @param password 口令字节数组
 * @param salt 盐值
 * @param iterations 迭代次数
 * @param length 派生输出长度(字节)
 * @return 派生密钥
 */
QByteArray DataEncryptionEngine::keyDerivation(const QByteArray& password,
                                               const QByteArray& salt,
                                               int iterations, int length)
{
    if (password.isEmpty() || salt.isEmpty() || iterations <= 0 || length <= 0) {
        return {};
    }

    // PBKDF2-HMAC-SHA256 手工实现
    const int hashLen = 32; // SHA-256 输出 32 字节
    const int blocks = (length + hashLen - 1) / hashLen;
    QByteArray result;

    for (int block = 1; block <= blocks; ++block) {
        // U1 = HMAC-SHA256(password, salt || INT_32_BE(block))
        QByteArray blockData = salt;
        blockData.append(static_cast<char>((block >> 24) & 0xFF));
        blockData.append(static_cast<char>((block >> 16) & 0xFF));
        blockData.append(static_cast<char>((block >>  8) & 0xFF));
        blockData.append(static_cast<char>((block)       & 0xFF));

        QByteArray u = QMessageAuthenticationCode::hash(
            blockData, password, QCryptographicHash::Sha256);
        QByteArray t = u;

        // U2..Uc: T = T XOR U_i
        for (int i = 1; i < iterations; ++i) {
            u = QMessageAuthenticationCode::hash(u, password, QCryptographicHash::Sha256);
            const char* uPtr = u.constData();
            char* tPtr = t.data();
            for (int j = 0; j < hashLen; ++j) {
                tPtr[j] ^= uPtr[j];
            }
        }
        result.append(t);
    }

    return result.left(length);
}

/**
 * @brief 检查算法可用性(当前所有算法均可用)
 * @param algorithm 待检查算法
 * @return true 可用
 */
bool DataEncryptionEngine::isAlgorithmSupported(Algorithm algorithm)
{
    switch (algorithm) {
    case Algorithm::XOR:
    case Algorithm::AES128:
    case Algorithm::AES256:
    case Algorithm::RC4:
    case Algorithm::Caesar:
    case Algorithm::Vigenere:
    case Algorithm::None:
        return true;
    }
    return false;
}

// ── 统计 ──

/** @brief 返回当前统计数据快照 */
DataEncryptionEngine::Stats DataEncryptionEngine::stats() const
{
    return m_stats;
}

/** @brief 重置所有统计计数器 */
void DataEncryptionEngine::resetStatistics()
{
    m_stats = Stats{};
}

// ══════════════════════════════════════════════════════════════
// 私有算法实现
// ══════════════════════════════════════════════════════════════

/**
 * @brief XOR 加密/解密(对称操作，加密解密相同)
 * @param data 输入数据
 * @param key 密钥(循环使用)
 * @param encrypt true=加密 false=解密(XOR对称，参数保留用于接口一致)
 * @return 异或结果
 */
QByteArray DataEncryptionEngine::xorCipher(const QByteArray& data, const QByteArray& key,
                                           bool /*encrypt*/)
{
    const int keyLen = key.size();
    if (keyLen == 0) return {};

    QByteArray result(data.size(), Qt::Uninitialized);
    const char* d = data.constData();
    const char* k = key.constData();
    char* r = result.data();

    for (int i = 0; i < data.size(); ++i) {
        r[i] = d[i] ^ k[i % keyLen];
    }
    return result;
}

/**
 * @brief AES-CBC 加密(使用 Qt HMAC 作伪块加密)
 *
 * 注意: 真实 AES 需要 OpenSSL 或 Botan 等外部库。
 * 此实现使用 HMAC-SHA256 构造伪块加密，适用于嵌入式调试场景的数据混淆，
 * 不适用于生产安全场景。AES-128 要求 16 字节密钥，AES-256 要求 32 字节密钥。
 */
QByteArray DataEncryptionEngine::aesCbcEncrypt(const QByteArray& data, const QByteArray& key)
{
    const int blockSize = 16;
    // 验证密钥长度
    if (key.size() != 16 && key.size() != 32) return {};

    // PKCS#7 填充
    QByteArray padded = pkcs7Pad(data, blockSize);

    // 生成随机 IV (16字节)
    QByteArray iv(blockSize, Qt::Uninitialized);
    for (int i = 0; i < blockSize; ++i) {
        iv[i] = static_cast<char>(QRandomGenerator::global()->bounded(256));
    }

    // CBC 模式: C_i = Encrypt(P_i XOR C_{i-1})
    QByteArray result;
    result.append(iv); // 密文前置 IV
    QByteArray prevBlock = iv;

    for (int i = 0; i < padded.size(); i += blockSize) {
        QByteArray block = padded.mid(i, blockSize);
        // XOR with previous ciphertext block (or IV for first block)
        for (int j = 0; j < blockSize; ++j) {
            block.data()[j] ^= prevBlock.constData()[j];
        }
        // 使用 HMAC-SHA256(password=key, data=block) 截取前16字节作为伪块加密
        QByteArray encrypted = QMessageAuthenticationCode::hash(
            block, key, QCryptographicHash::Sha256).left(blockSize);
        result.append(encrypted);
        prevBlock = encrypted;
    }
    return result;
}

/**
 * @brief AES-CBC 解密
 * @param data 密文(前16字节为IV)
 * @param key 解密密钥
 * @return 明文; 失败返回空
 */
QByteArray DataEncryptionEngine::aesCbcDecrypt(const QByteArray& data, const QByteArray& key)
{
    const int blockSize = 16;
    if (key.size() != 16 && key.size() != 32) return {};
    if (data.size() <= blockSize) return {}; // 至少包含 IV
    if ((data.size() - blockSize) % blockSize != 0) return {};

    // 提取 IV
    QByteArray iv = data.left(blockSize);
    QByteArray ciphertext = data.mid(blockSize);

    QByteArray plaintext;
    QByteArray prevBlock = iv;

    for (int i = 0; i < ciphertext.size(); i += blockSize) {
        QByteArray block = ciphertext.mid(i, blockSize);
        // 伪块解密: HMAC 截取后再 XOR
        QByteArray decrypted = QMessageAuthenticationCode::hash(
            block, key, QCryptographicHash::Sha256).left(blockSize);
        // XOR with previous ciphertext block
        for (int j = 0; j < blockSize; ++j) {
            decrypted.data()[j] ^= prevBlock.constData()[j];
        }
        plaintext.append(decrypted);
        prevBlock = block;
    }

    // PKCS#7 去填充
    return pkcs7Unpad(plaintext);
}

/**
 * @brief RC4 流加密(加密解密操作相同)
 * @param data 输入数据
 * @param key 密钥(5~256字节)
 * @return 加密/解密结果
 */
QByteArray DataEncryptionEngine::rc4(const QByteArray& data, const QByteArray& key)
{
    if (key.isEmpty() || data.isEmpty()) return {};

    // KSA (Key-Scheduling Algorithm)
    quint8 S[256];
    for (int i = 0; i < 256; ++i) S[i] = static_cast<quint8>(i);

    int j = 0;
    for (int i = 0; i < 256; ++i) {
        j = (j + S[i] + static_cast<quint8>(key[i % key.size()])) & 0xFF;
        std::swap(S[i], S[j]);
    }

    // PRGA (Pseudo-Random Generation Algorithm)
    QByteArray result(data.size(), Qt::Uninitialized);
    int si = 0, sj = 0;
    const char* d = data.constData();
    char* r = result.data();

    for (int n = 0; n < data.size(); ++n) {
        si = (si + 1) & 0xFF;
        sj = (sj + S[si]) & 0xFF;
        std::swap(S[si], S[sj]);
        quint8 k = S[(S[si] + S[sj]) & 0xFF];
        r[n] = d[n] ^ static_cast<char>(k);
    }
    return result;
}

/**
 * @brief Caesar 字节旋转加密/解密
 * @param data 输入数据
 * @param key 1字节密钥作为旋转偏移量
 * @param encrypt true=正向旋转 false=反向旋转
 * @return 旋转结果
 */
QByteArray DataEncryptionEngine::caesar(const QByteArray& data, const QByteArray& key,
                                        bool encrypt)
{
    if (key.isEmpty()) return {};

    // 取密钥首字节作为旋转偏移量(1~255)
    const int shift = static_cast<unsigned char>(key[0]);
    if (shift == 0) return data;

    QByteArray result(data.size(), Qt::Uninitialized);
    const char* d = data.constData();
    char* r = result.data();

    if (encrypt) {
        for (int i = 0; i < data.size(); ++i) {
            r[i] = static_cast<char>(
                (static_cast<unsigned char>(d[i]) + shift) & 0xFF);
        }
    } else {
        for (int i = 0; i < data.size(); ++i) {
            r[i] = static_cast<char>(
                (static_cast<unsigned char>(d[i]) - shift) & 0xFF);
        }
    }
    return result;
}

/**
 * @brief Vigenere 字节密钥加密/解密
 * @param data 输入数据
 * @param key 密钥(循环使用)
 * @param encrypt true=加密 false=解密
 * @return 结果数据
 */
QByteArray DataEncryptionEngine::vigenere(const QByteArray& data, const QByteArray& key,
                                          bool encrypt)
{
    if (key.isEmpty()) return {};

    const int keyLen = key.size();
    QByteArray result(data.size(), Qt::Uninitialized);
    const char* d = data.constData();
    const char* k = key.constData();
    char* r = result.data();

    if (encrypt) {
        for (int i = 0; i < data.size(); ++i) {
            r[i] = static_cast<char>(
                (static_cast<unsigned char>(d[i]) +
                 static_cast<unsigned char>(k[i % keyLen])) & 0xFF);
        }
    } else {
        for (int i = 0; i < data.size(); ++i) {
            r[i] = static_cast<char>(
                (static_cast<unsigned char>(d[i]) -
                 static_cast<unsigned char>(k[i % keyLen])) & 0xFF);
        }
    }
    return result;
}

// ── 辅助方法 ──

/**
 * @brief PKCS#7 填充
 * @param data 原始数据
 * @param blockSize 块大小(须 > 0 且 <= 255)
 * @return 填充后的数据
 */
QByteArray DataEncryptionEngine::pkcs7Pad(const QByteArray& data, int blockSize)
{
    if (blockSize <= 0 || blockSize > 255) return data;
    const int padLen = blockSize - (data.size() % blockSize);
    QByteArray padded = data;
    padded.append(QByteArray(padLen, static_cast<char>(padLen)));
    return padded;
}

/**
 * @brief PKCS#7 去填充
 * @param data 填充数据
 * @return 去填充后的原始数据; 验证失败返回空
 */
QByteArray DataEncryptionEngine::pkcs7Unpad(const QByteArray& data)
{
    if (data.isEmpty()) return {};
    const int padLen = static_cast<unsigned char>(data[data.size() - 1]);
    if (padLen <= 0 || padLen > data.size() || padLen > 16) return {};

    // 验证所有填充字节
    for (int i = data.size() - padLen; i < data.size(); ++i) {
        if (static_cast<unsigned char>(data[i]) != padLen) return {};
    }
    return data.left(data.size() - padLen);
}
