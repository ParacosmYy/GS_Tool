/**
 * @file CaesarCode.h
 * @brief 凯撒/ROT密码编解码+频率分析破解 — Caesar/ROT Cipher Encoder/Decoder with Brute-Force Frequency Analysis Cracking
 *
 * 功能: 实现Caesar/ROT密码的编码解码，支持自定义偏移量、ROT13/47特殊模式、
 *       基于英文字母频率分析的暴力破解。
 *
 * 协作: HexConverter(十六进制转换) / Base64Coder(Base64编解码) / AesEncryptor(AES加密)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <QMap>

/**
 * @brief Caesar/ROT密码编解码器
 */
class CaesarCode : public QObject {
    Q_OBJECT

public:
    /** @brief 预设模式 */
    enum PresetMode {
        CustomShift = 0,  ///< 自定义偏移
        ROT13 = 13,       ///< ROT13(字母)
        ROT47 = 47        ///< ROT47(ASCII可打印字符)
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalEncodes = 0;         ///< 累计编码次数
        quint64 totalDecodes = 0;         ///< 累计解码次数
        quint64 totalCracks = 0;          ///< 累计破解次数
        double avgProcessingTimeMs = 0.0; ///< 平均耗时(ms)
    };

    /** @brief 破解结果 */
    struct CrackResult {
        int shift = 0;           ///< 推测的偏移量
        double score = 0.0;      ///< 频率匹配得分
        QString plaintext;       ///< 解密后的明文
    };

    explicit CaesarCode(QObject *parent = nullptr);
    ~CaesarCode() override;

    void setShift(int shift);
    void setMode(PresetMode mode);

    /** @brief 编码(加密) */
    QString encode(const QString& plaintext) const;

    /** @brief 解码(解密) */
    QString decode(const QString& ciphertext) const;

    /** @brief 使用已知偏移量解密 */
    QString decryptWithShift(const QString& ciphertext, int shift) const;

    /** @brief 暴力破解: 生成所有26种可能 */
    QVector<CrackResult> bruteForce(const QString& ciphertext) const;

    /** @brief 频率分析破解: 返回最佳匹配 */
    CrackResult crackFrequency(const QString& ciphertext) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encodeCompleted(int length);
    void decodeCompleted(int length);
    void crackCompleted(int bestShift, double score);

private:
    /** @brief 单字符Caesar移位 */
    QChar shiftChar(QChar c, int offset) const;

    /** @brief ROT47移位 */
    QChar shiftRot47(QChar c, int offset) const;

    /** @brief 计算文本的字母频率分布 */
    QMap<QChar, double> frequencyDistribution(const QString& text) const;

    /** @brief 计算频率匹配得分(卡方检验) */
    double chiSquaredScore(const QMap<QChar, double>& observed) const;

    int m_shift = 3;
    PresetMode m_mode = CustomShift;

    /** @brief 标准英文字母频率 */
    static const double s_englishFreq[26];

    Stats m_stats;
    double m_timeSum = 0.0;
};
