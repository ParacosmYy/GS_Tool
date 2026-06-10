/**
 * @file BaudotCode11.h
 * @brief 博多码(CCITT国际电报字母表2与空格回退标准5比特电报) — Baudot Code with CCITT International Telegraph Alphabet 2 and Unshift-on-Space for Standard 5-bit Telegraphy
 *
 * 功能: 实现博多码(Baudot code)，采用CCITT国际电报字母表2(CCITT ITA2)
 *       与空格回退(unshift-on-space)实现标准5比特电报编码(standard 5-bit telegraphy)。
 *
 * 协作: HuffmanCodec8(哈夫曼编码) / ShannonFano12(香农-范诺编码) / GrayCode10(格雷码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief 博多码(CCITT国际电报字母表2与空格回退标准5比特电报)
 */
class BaudotCode11 : public QObject {
    Q_OBJECT

public:
    /** @brief Shift state for letters vs figures */
    enum ShiftState { Letters = 0, Figures = 1 };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numEncoded = 0;
        int numDecoded = 0;
        int numShifts = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BaudotCode11(QObject *parent = nullptr);
    ~BaudotCode11() override;

    /** @brief Enable/disable unshift-on-space behavior */
    void setUnshiftOnSpace(bool enabled);

    /** @brief Encode text string to 5-bit Baudot codewords */
    QVector<quint8> encode(const QString& text);

    /** @brief Decode 5-bit Baudot codewords to text string */
    QString decode(const QVector<quint8>& codewords);

    /** @brief Get current shift state */
    ShiftState shiftState() const;

    /** @brief Reset shift state to Letters */
    void resetShift();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encodingDone(int numCodewords, int numShifts, double timeMs);
    void decodingDone(int numChars, int numShifts, double timeMs);

private:
    bool m_unshiftOnSpace = true;
    ShiftState m_shift = Letters;

    // ITA2 lookup tables
    QVector<QChar> m_lettersTable;   // 32 entries for letters mode
    QVector<QChar> m_figuresTable;   // 32 entries for figures mode
    QVector<quint8> m_letterToCode;  // letter char -> 5-bit code
    QVector<quint8> m_figureToCode;  // figure char -> 5-bit code

    Stats m_stats;
    double m_timeSum = 0.0;

    static constexpr quint8 SHIFT_LETTERS = 0x1F;  // ITA2 letters shift
    static constexpr quint8 SHIFT_FIGURES = 0x1B;  // ITA2 figures shift

    /** @brief Build ITA2 lookup tables */
    void buildTables();

    /** @brief Encode a single character to 5-bit code(s) */
    QVector<quint8> encodeChar(QChar ch);
};
