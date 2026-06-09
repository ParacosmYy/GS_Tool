/**
 * @file TapirCode4.h
 * @brief Tapir编码(扩展多表替换+关键字周期密钥调度) — Tapir Code with Extended Polyalphabetic Substitution and Keyword-Derived Periodic Key Scheduling
 *
 * 功能: 实现Tapir编码(Tapir code)，采用扩展多表替换(extended polyalphabetic substitution)
 *       通过关键字派生周期密钥调度(keyword-derived periodic key scheduling)生成多表密钥序列，
 *       结合Vigenere风格加密与位置敏感变换实现文本编解码。
 *
 * 协作: HuffmanCodec10(哈夫曼) / LZWCoder7(LZW) / AriCodec13(算术编码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief Tapir编码(扩展多表替换+关键字周期密钥调度)
 */
class TapirCode4 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int inputLength = 0;
        int outputLength = 0;
        int numEncode = 0;
        int numDecode = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit TapirCode4(QObject *parent = nullptr);
    ~TapirCode4() override;

    /** @brief Set keyword for key scheduling */
    void setKeyword(const QString& keyword);

    /** @brief Encode plaintext to Tapir ciphertext */
    QString encode(const QString& plaintext);

    /** @brief Decode Tapir ciphertext to plaintext */
    QString decode(const QString& ciphertext);

    /** @brief Encode raw bytes to hex Tapir string */
    QByteArray encodeBytes(const QByteArray& data);

    /** @brief Decode hex Tapir string to raw bytes */
    QByteArray decodeBytes(const QByteArray& encoded);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encodeCompleted(int inputLen, int outputLen, double timeMs);
    void decodeCompleted(int inputLen, int outputLen, double timeMs);

private:
    QString m_keyword;
    QVector<int> m_keySchedule;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build periodic key schedule from keyword */
    void buildKeySchedule();

    /** @brief Forward substitution at position i */
    int substitute(int charVal, int pos) const;

    /** @brief Inverse substitution at position i */
    int invSubstitute(int cipherVal, int pos) const;

    /** @brief Extended alphabet size (supports printable ASCII range) */
    static constexpr int ALPHABET_SIZE = 95;
    static constexpr int ASCII_OFFSET = 32;
};
