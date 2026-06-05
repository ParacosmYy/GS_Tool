/**
 * @file SimonSpeckCipher.h
 * @brief Simon/Speck轻量级分组密码 — IoT与嵌入式安全通信
 *
 * 功能: 实现Simon(硬件优化)和Speck(软件优化)两种轻量级
 *       分组密码算法，适用于资源受限的嵌入式设备安全通信。
 *
 * 协作: DataEncryptionEngine(加密引擎) / CrcStreamVerifier(完整性校验)
 */
#pragma once

#include <QObject>
#include <QByteArray>
#include <QElapsedTimer>

/**
 * @brief Simon/Speck轻量级分组密码
 */
class SimonSpeckCipher : public QObject {
    Q_OBJECT

public:
    /** @brief 密码算法类型 */
    enum class Algorithm {
        Simon,  ///< Simon — 硬件优化变体
        Speck   ///< Speck — 软件优化变体
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalEncrypted = 0;        ///< 累计加密次数
        quint64 totalDecrypted = 0;        ///< 累计解密次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit SimonSpeckCipher(QObject* parent = nullptr);

    /** @brief 设置密钥 @param key 64位密钥 */
    void setKey(quint64 key);

    /** @brief 选择算法 @param algo Simon或Speck */
    void setAlgorithm(Algorithm algo);

    /**
     * @brief 加密数据
     * @param data 明文数据
     * @return 密文(PKCS7填充到8字节对齐)
     */
    QByteArray encrypt(const QByteArray& data);

    /**
     * @brief 解密数据
     * @param data 密文数据
     * @return 明文(去除PKCS7填充)
     */
    QByteArray decrypt(const QByteArray& data);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 加/解密完成 @param bytes 处理字节数 */
    void encryptionCompleted(int bytes);

private:
    /** @brief Speck单轮加密 @param x 左半 @param y 右半 @param k 轮密钥 */
    void speckRound(quint32& x, quint32& y, quint32 k) const;

    /** @brief Speck单轮解密(逆) @param x 左半 @param y 右半 @param k 轮密钥 */
    void speckRoundInv(quint32& x, quint32& y, quint32 k) const;

    /** @brief Simon单轮加密 @param x 左半 @param y 右半 @param k 轮密钥 */
    void simonRound(quint32& x, quint32& y, quint32 k) const;

    /** @brief Simon单轮解密(逆) @param x 左半 @param y 右半 @param k 轮密钥 */
    void simonRoundInv(quint32& x, quint32& y, quint32 k) const;

    /** @brief 生成Speck轮密钥 */
    void expandSpeckKey();

    /** @brief 生成Simon轮密钥 */
    void expandSimonKey();

    /** @brief 加密单个8字节块 */
    void encryptBlock(quint32& left, quint32& right) const;

    /** @brief 解密单个8字节块 */
    void decryptBlock(quint32& left, quint32& right) const;

    static constexpr int ROUNDS = 32;  ///< Speck64/96轮数
    static constexpr quint32 MASK32 = 0xFFFFFFFFU;

    quint64     m_key = 0;                ///< 64位密钥
    Algorithm   m_algo = Algorithm::Speck; ///< 当前算法
    quint32     m_subkeys[32];            ///< 轮密钥
    bool        m_keyExpanded = false;     ///< 密钥是否已扩展

    mutable Stats        m_stats;
    mutable double       m_totalTimeMs = 0.0;
    mutable QElapsedTimer m_timer;
};
