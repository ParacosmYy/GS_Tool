/**
 * @file BaudotCode8.h
 * @brief 博多码(ITA2国际字母表+Unicode映射+自动Shift检测启发式) — Baudot Code with ITA2 International Alphabet and Unicode Mapping Table with Auto-Shift Detection Heuristic
 *
 * 功能: 实现博多码(Baudot Code)编解码，支持ITA2国际字母表(ITA2
 *       International Alphabet)5位编码，内置Unicode映射表(Unicode
 *       mapping table)，自动Shift检测启发式(auto-shift detection
 *       heuristic)智能判断字母/数字模式切换。
 *
 * 协作: HuffmanCodec6(哈夫曼编码) / ArithmeticCodec7(算术编码) / RSACrypto5(RSA加密)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QChar>

/**
 * @brief 博多码(ITA2+Unicode映射+自动Shift检测)
 */
class BaudotCode8 : public QObject {
    Q_OBJECT

public:
    /** @brief Shift state */
    enum ShiftState { Letters = 0, Figures = 1 };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numEncoded = 0;
        int numDecoded = 0;
        int numShifts = 0;
        int numErrors = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BaudotCode8(QObject *parent = nullptr);
    ~BaudotCode8() override;

    /** @brief Encode Unicode string to Baudot 5-bit codes */
    QVector<quint8> encode(const QString& text);

    /** @brief Decode Baudot 5-bit codes to Unicode string */
    QString decode(const QVector<quint8>& codes);

    /** @brief Set initial shift state */
    void setInitialShift(ShiftState state);

    /** @brief Enable/disable auto-shift detection heuristic */
    void setAutoShiftDetection(bool enabled);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encodeCompleted(int numChars, int numShifts, double timeMs);
    void decodeCompleted(int numCodes, int numChars, double timeMs);

private:
    ShiftState m_shiftState = Letters;
    bool m_autoShift = true;

    // ITA2 letter mode table: 5-bit code -> QChar
    QVector<QChar> m_letterTable;
    // ITA2 figure mode table: 5-bit code -> QChar
    QVector<QChar> m_figureTable;
    // Reverse mapping: QChar -> 5-bit code (letters)
    // Reverse mapping: QChar -> 5-bit code (figures)

    Stats m_stats;
    double m_timeSum = 0.0;

    static constexpr quint8 SHIFT_TO_FIGURES = 0x1B; // 27 = 11011
    static constexpr quint8 SHIFT_TO_LETTERS = 0x1F; // 31 = 11111

    /** @brief Build ITA2 mapping tables */
    void buildTables();

    /** @brief Look up character in current shift table */
    int lookupChar(QChar ch, ShiftState state) const;

    /** @brief Look up 5-bit code in current shift table */
    QChar lookupCode(quint8 code, ShiftState state) const;

    /** @brief Auto-detect best shift state for next character */
    ShiftState detectShift(QChar ch) const;
};
