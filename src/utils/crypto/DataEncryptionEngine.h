/**
 * @file DataEncryptionEngine.h
 * @brief 数据加密引擎 — 提供串口通信中常用的加密/解密操作
 *
 * 支持 XOR / AES-128-CBC / AES-256-CBC / RC4 / Caesar / Vigenere 六种算法，
 * 提供 PBKDF2 密钥派生与随机密钥生成，适用于嵌入式调试场景的数据混淆与安全验证。
 * 统计模块记录加密解密次数、字节数、错误数及各算法调用分布。
 */
#ifndef DATA_ENCRYPTION_ENGINE_H
#define DATA_ENCRYPTION_ENGINE_H

#include <QObject>
#include <QByteArray>
#include <QCryptographicHash>

/**
 * @class DataEncryptionEngine
 * @brief 数据加密/解密引擎 — XOR/AES/RC4/Caesar/Vigenere 多算法统一接口
 *
 * 典型用法:
 * @code
 *   DataEncryptionEngine engine;
 *   QByteArray key = engine.generateKey(Algorithm::AES256, 32);
 *   QByteArray cipher = engine.encrypt(plain, Algorithm::AES256, key);
 *   QByteArray recovered = engine.decrypt(cipher, Algorithm::AES256, key);
 * @endcode
 */
class DataEncryptionEngine : public QObject {
    Q_OBJECT

public:
    /** @brief 支持的加密算法枚举 */
    enum class Algorithm {
        XOR = 0,     ///< XOR异或加密(单字节/多字节密钥)
        AES128,      ///< AES-128 CBC模式
        AES256,      ///< AES-256 CBC模式
        RC4,         ///< RC4流加密
        Caesar,      ///< Caesar字节旋转
        Vigenere,    ///< Vigenere字节密钥加密
        None         ///< 无加密(透传)
    };
    Q_ENUM(Algorithm)

    /** @brief 加密/解密操作统计结构 */
    struct Stats {
        quint64 totalEncrypts = 0;           ///< 加密操作总次数
        quint64 totalDecrypts = 0;           ///< 解密操作总次数
        quint64 totalBytesEncrypted = 0;     ///< 已加密字节总数
        quint64 totalBytesDecrypted = 0;     ///< 已解密字节总数
        quint64 encryptErrors = 0;           ///< 加密错误次数
        quint64 decryptErrors = 0;           ///< 解密错误次数
        quint64 operationsByAlgorithm[7] = {}; ///< 各算法操作次数[XOR..None]
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit DataEncryptionEngine(QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~DataEncryptionEngine() override;

    // ── 核心操作 ──

    /**
     * @brief 加密数据
     * @param data 明文数据
     * @param algorithm 加密算法
     * @param key 加密密钥
     * @return 密文数据; 失败返回空 QByteArray 并发射 error 信号
     */
    QByteArray encrypt(const QByteArray& data, Algorithm algorithm, const QByteArray& key);

    /**
     * @brief 解密数据
     * @param data 密文数据
     * @param algorithm 解密算法(须与加密时一致)
     * @param key 解密密钥
     * @return 明文数据; 失败返回空 QByteArray 并发射 error 信号
     */
    QByteArray decrypt(const QByteArray& data, Algorithm algorithm, const QByteArray& key);

    // ── 密钥管理 ──

    /**
     * @brief 生成指定算法的随机密钥
     * @param algorithm 目标算法(决定默认长度)
     * @param length 密钥字节数(0=使用算法默认长度)
     * @return 随机密钥字节序列
     */
    QByteArray generateKey(Algorithm algorithm, int length = 0);

    /**
     * @brief PBKDF2密钥派生函数
     * @param password 口令
     * @param salt 盐值
     * @param iterations 迭代次数(推荐 ≥ 10000)
     * @param length 派生密钥长度(字节)
     * @return 派生密钥
     */
    QByteArray keyDerivation(const QByteArray& password, const QByteArray& salt,
                             int iterations, int length);

    /**
     * @brief 检查指定算法是否可用
     * @param algorithm 待检查算法
     * @return true 表示该算法在当前平台可用
     */
    static bool isAlgorithmSupported(Algorithm algorithm);

    // ── 统计 ──

    /** @brief 获取当前统计数据快照 @return Stats结构体副本 */
    Stats stats() const;

    /** @brief 重置所有统计计数器归零 */
    void resetStatistics();

signals:
    /** @brief 加密完成信号 @param result 密文结果 @param algorithm 使用的算法 */
    void encryptComplete(const QByteArray& result, Algorithm algorithm);
    /** @brief 解密完成信号 @param result 明文结果 @param algorithm 使用的算法 */
    void decryptComplete(const QByteArray& result, Algorithm algorithm);
    /** @brief 错误信号 @param errorMessage 错误描述 @param algorithm 出错的算法 */
    void error(const QString& errorMessage, Algorithm algorithm);

private:
    // ── 内部算法实现 ──
    QByteArray xorCipher(const QByteArray& data, const QByteArray& key, bool encrypt);
    QByteArray aesCbcEncrypt(const QByteArray& data, const QByteArray& key);
    QByteArray aesCbcDecrypt(const QByteArray& data, const QByteArray& key);
    QByteArray rc4(const QByteArray& data, const QByteArray& key);
    QByteArray caesar(const QByteArray& data, const QByteArray& key, bool encrypt);
    QByteArray vigenere(const QByteArray& data, const QByteArray& key, bool encrypt);

    /** @brief PKCS#7 填充到指定块大小 */
    QByteArray pkcs7Pad(const QByteArray& data, int blockSize);
    /** @brief PKCS#7 去填充 */
    QByteArray pkcs7Unpad(const QByteArray& data);

    Stats m_stats; ///< 操作统计数据
};

#endif // DATA_ENCRYPTION_ENGINE_H
