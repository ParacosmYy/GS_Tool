/**
 * @file ChaChaCipher.h
 * @brief ChaCha20流密码(ChaCha20 Stream Cipher)
 */

#pragma once

#include <QObject>
#include <QByteArray>

/**
 * @class ChaChaCipher
 * @brief ChaCha20流密码 — 高速安全的流加密
 *
 * 支持ChaCha20加密/解密、Poly1305消息认证(简化)。
 * 适用于嵌入式通信加密、数据安全传输等场景。
 */
class ChaChaCipher : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalEncrypted = 0;  /**< 总加密次数 */
        int totalDecrypted = 0;  /**< 总解密次数 */
        long long totalBytes = 0; /**< 总字节数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 构造函数 */
    explicit ChaChaCipher(QObject* parent = nullptr);

    /**
     * @brief 设置256位密钥
     * @param key 32字节密钥
     */
    void setKey(const QByteArray& key);

    /**
     * @brief 设置96位nonce
     * @param nonce 12字节nonce
     */
    void setNonce(const QByteArray& nonce);

    /**
     * @brief 加密/解密(ChaCha20是对称的)
     * @param data 输入数据
     @param counter 初始计数器(默认0)
     * @return 加密/解密后数据
     */
    QByteArray process(const QByteArray& data, quint32 counter = 0);

    /**
     * @brief 加密
     * @param plaintext 明文
     * @param counter 初始计数器
     * @return 密文
     */
    QByteArray encrypt(const QByteArray& plaintext, quint32 counter = 0);

    /**
     * @brief 解密
     * @param ciphertext 密文
     * @param counter 初始计数器
     * @return 明文
     */
    QByteArray decrypt(const QByteArray& ciphertext, quint32 counter = 0);

    /**
     * @brief 生成密钥流块(64字节)
     * @param counter 块计数器
     * @return 64字节密钥流
     */
    QByteArray generateBlock(quint32 counter);

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 加密完成信号 */
    void encryptionCompleted(int bytes);

private:
    void quarterRound(quint32& a, quint32& b, quint32& c, quint32& d);
    void innerBlock(quint32 state[16]);
    quint32 rotl32(quint32 x, int n);

    QByteArray m_key;
    QByteArray m_nonce;
    Stats m_stats;
    double m_timeSum;
};
