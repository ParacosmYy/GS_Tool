/**
 * @file BaudotCode9.h
 * @brief 博多码(仅图型转义编码+5位奇偶校验纠错启发式) — Baudot Code with Figure-shift-only Encoding and Error Detection via 5-bit Parity Check with Correction Heuristic
 *
 * 功能: 实现博多码(Baudot Code)编解码器，采用仅图型转义
 *       (figure-shift-only)编码策略，通过5位奇偶校验实现错误
 *       检测与启发式纠正(heuristic correction)。
 *
 * 协作: HuffmanCodec5(哈夫曼编解码) / ShannonFano7(香农-范诺编码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QChar>

/**
 * @brief 博多码(仅图型转义编码+5位奇偶校验纠错启发式)
 */
class BaudotCode9 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numEncoded = 0;
        int numDecoded = 0;
        int numCorrected = 0;
        int numUncorrectable = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Decode result with parity status */
    struct DecodeResult {
        QChar character;
        bool parityOk = true;
        bool wasCorrected = false;
        quint8 rawCode = 0;
    };

    explicit BaudotCode9(QObject *parent = nullptr);
    ~BaudotCode9() override;

    /** @brief Encode a text string to Baudot 5-bit codes */
    QVector<quint8> encode(const QString& text);

    /** @brief Decode 5-bit Baudot codes to text */
    QString decode(const QVector<quint8>& codes);

    /** @brief Decode single code with error detection info */
    DecodeResult decodeSingle(quint8 code) const;

    /** @brief Compute parity bit for 5-bit data */
    quint8 computeParity(quint8 data5bit) const;

    /** @brief Check parity of 6-bit code (5 data + 1 parity) */
    bool checkParity(quint8 code6bit) const;

    /** @brief Attempt single-bit error correction via heuristic */
    quint8 correctError(quint8 code6bit) const;

    /** @brief Get the current shift state (letters/figures) */
    bool isInFigureShift() const;

    /** @brief Reset shift state */
    void reset();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encodeCompleted(int numChars, int numCodes, double timeMs);
    void decodeCompleted(int numChars, int numCorrected, double timeMs);

private:
    bool m_figureShift = false;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief ITA2 letters-shift lookup table */
    static const char s_lettersTable[32];

    /** @brief ITA2 figures-shift lookup table */
    static const char s_figuresTable[32];

    /** @brief Reverse lookup: character to 5-bit code (letters) */
    QMap<QChar, quint8> m_lettersReverse;

    /** @brief Reverse lookup: character to 5-bit code (figures) */
    QMap<QChar, quint8> m_figuresReverse;

    /** @brief Build reverse lookup tables */
    void buildReverseTables();

    /** @brief Encode single character to 5-bit code */
    quint8 encodeChar(QChar ch, bool& needShift);
};
