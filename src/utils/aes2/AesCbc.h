/**
 * @file AesCbc.h
 * @brief AES-CBC加密/解密 — 支持128/192/256位密钥
 *
 * 功能: 提供AES-CBC模式的加密与解密，支持PKCS7填充，
 *       128/192/256位密钥长度，适用于数据安全传输场景。
 *
 * 协作: DataEncryptionEngine(加密引擎) / CrcStreamVerifier(完整性校验)
 */
#pragma once

#include <QObject>
#include <QByteArray>

#include <vector>

/**
 * @brief AES-CBC加密/解密 — 128/192/256位密钥
 */
class AesCbc : public QObject {
    Q_OBJECT

public:
    /** @brief 密钥长度枚举 */
    enum KeySize : int {
        AES128 = 16,   ///< 128位密钥
        AES192 = 24,   ///< 192位密钥
        AES256 = 32    ///< 256位密钥
    };
    Q_ENUM(KeySize)

    /** @brief 统计信息 */
    struct Stats {
        quint64 totalEncryptions     = 0;   ///< 累计加密次数
        quint64 totalDecryptions     = 0;   ///< 累计解密次数
        quint64 totalBytesEncrypted  = 0;   ///< 累计加密字节数
        quint64 totalBytesDecrypted  = 0;   ///< 累计解密字节数
        double  avgProcessingTimeMs  = 0.0; ///< 平均处理时间(ms)
    };

    /**
     * @brief 构造函数
     * @param keySize 密钥长度
     * @param parent 父对象
     */
    explicit AesCbc(KeySize keySize = AES256, QObject* parent = nullptr);

    /**
     * @brief 设置密钥和IV
     * @param key 密钥(长度必须匹配KeySize)
     * @param iv 初始化向量(16字节)
     * @return true=成功设置
     */
    bool setKeyAndIv(const QByteArray& key, const QByteArray& iv);

    /**
     * @brief CBC模式加密(PKCS7填充)
     * @param plaintext 明文
     * @return 密文(失败返回空)
     */
    QByteArray encrypt(const QByteArray& plaintext);

    /**
     * @brief CBC模式解密(PKCS7去填充)
     * @param ciphertext 密文(必须是16字节的倍数)
     * @return 明文(失败返回空)
     */
    QByteArray decrypt(const QByteArray& ciphertext);

    /**
     * @brief 生成随机密钥
     * @return 随机密钥
     */
    QByteArray generateKey() const;

    /**
     * @brief 生成随机IV
     * @return 16字节随机IV
     */
    QByteArray generateIv() const;

    /** @brief 当前密钥长度 @return 字节数 */
    int keySize() const { return m_keySize; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 加密完成 @param cipherSize 密文大小 */
    void encryptionCompleted(int cipherSize);

    /** @brief 解密完成 @param plainSize 明文大小 */
    void decryptionCompleted(int plainSize);

private:
    /**
     * @brief AES轮密钥扩展
     * @param key 原始密钥
     * @return 扩展后的轮密钥
     */
    std::vector<quint32> keyExpansion(const QByteArray& key) const;

    /**
     * @brief AES单块加密(10/12/14轮)
     * @param state 16字节状态
     * @param roundKeys 轮密钥
     * @return 加密后的16字节
     */
    void encryptBlock(quint8* state, const std::vector<quint32>& roundKeys) const;

    /**
     * @brief AES单块解密
     * @param state 16字节状态
     * @param roundKeys 轮密钥
     */
    void decryptBlock(quint8* state, const std::vector<quint32>& roundKeys) const;

    /** @brief SubBytes变换 @param state 状态 */
    void subBytes(quint8* state) const;
    /** @brief 逆SubBytes @param state 状态 */
    void invSubBytes(quint8* state) const;
    /** @brief ShiftRows变换 @param state 状态 */
    void shiftRows(quint8* state) const;
    /** @brief 逆ShiftRows @param state 状态 */
    void invShiftRows(quint8* state) const;
    /** @brief MixColumns变换 @param state 状态 */
    void mixColumns(quint8* state) const;
    /** @brief 逆MixColumns @param state 状态 */
    void invMixColumns(quint8* state) const;
    /** @brief AddRoundKey @param state 状态 @param roundKeys 轮密钥 @param round 轮次 */
    void addRoundKey(quint8* state, const std::vector<quint32>& roundKeys, int round) const;

    /** @brief PKCS7填充 @param data 原始数据 @return 填充后数据 */
    QByteArray pkcs7Pad(const QByteArray& data) const;

    /** @brief PKCS7去填充 @param data 填充数据 @return 原始数据 */
    QByteArray pkcs7Unpad(const QByteArray& data) const;

    int m_keySize;                          ///< 密钥字节长度
    int m_numRounds;                        ///< AES轮数
    QByteArray m_key;                       ///< 密钥
    QByteArray m_iv;                        ///< 初始化向量
    std::vector<quint32> m_roundKeys;       ///< 扩展轮密钥
    bool m_keySet = false;                  ///< 密钥是否已设置

    mutable Stats  m_stats;
    mutable double m_timeSumMs = 0.0;

    static const quint8 SBOX[256];      ///< AES S-Box
    static const quint8 INV_SBOX[256];  ///< AES 逆S-Box
    static const quint8 RCON[11];       ///< 轮常量
};
