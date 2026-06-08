/**
 * @file BaudotCode5.h
 * @brief 博多码(ITA2国际/US TTY双模式自动检测纠错) — Baudot Code with ITA2 International and US TTY Variant featuring Automatic Mode Detection and Error Correction
 *
 * 功能: 实现博多码(Baudot Code)编解码器，支持ITA2国际标准和US TTY变体，
 *       具备自动模式检测(letters/figures shift)和单比特纠错能力。
 *
 * 协作: HammingCode4(汉明码) / ManchesterEncoder3(曼彻斯特) / HuffmanEncoder5(哈夫曼)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 博多码(ITA2国际/US TTY双模式自动检测纠错)
 */
class BaudotCode5 : public QObject {
    Q_OBJECT

public:
    /** @brief Code variant selection */
    enum Variant {
        ITA2International = 0,  // International Telegraph Alphabet No.2
        UsTty = 1               // US TTY (Teletype) variant
    };
    Q_ENUM(Variant)

    /** @brief Shift state */
    enum ShiftState {
        Letters = 0,
        Figures = 1
    };
    Q_ENUM(ShiftState)

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numEncoded = 0;
        int numDecoded = 0;
        int numErrorsCorrected = 0;
        int numUncorrectable = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BaudotCode5(QObject *parent = nullptr);
    ~BaudotCode5() override;

    /** @brief Set code variant (ITA2 or US TTY) */
    void setVariant(Variant variant);

    /** @brief Encode text string to Baudot 5-bit code words */
    QVector<quint8> encode(const QString& text) const;

    /** @brief Decode Baudot 5-bit code words to text string */
    QString decode(const QVector<quint8>& codeWords);

    /** @brief Auto-detect variant from a sample of code words */
    Variant detectVariant(const QVector<quint8>& sample) const;

    /** @brief Correct single-bit errors using parity and context */
    QVector<quint8> correctErrors(const QVector<quint8>& codeWords);

    /** @brief Get current shift state */
    ShiftState shiftState() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void shiftChanged(ShiftState newState);
    void errorCorrected(int position, quint8 original, quint8 corrected);
    void uncorrectableError(int position, quint8 codeWord);

private:
    Variant m_variant = ITA2International;
    ShiftState m_shiftState = Letters;

    // Letter and figure lookup tables (32 entries each)
    QVector<QChar> m_lettersTable;
    QVector<QChar> m_figuresTable;

    // Reverse lookup: character -> code word
    QVector<int> m_letterEncode;
    QVector<int> m_figureEncode;

    static constexpr quint8 LETTERS_SHIFT = 0x1F;  // 11111
    static constexpr quint8 FIGURES_SHIFT = 0x1B;  // 11011

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Initialize ITA2 international lookup tables */
    void initITA2Tables();

    /** @brief Initialize US TTY variant lookup tables */
    void initUsTtyTables();

    /** @brief Build reverse lookup maps for encoding */
    void buildReverseLookup();

    /** @brief Validate parity for a 5-bit code word with parity bit */
    bool validateParity(quint8 codeWord) const;

    /** @brief Find nearest valid code word (Hamming distance 1) */
    quint8 findNearestValid(quint8 codeWord) const;

    /** @brief Decode a single code word given current shift state */
    QChar decodeCodeWord(quint8 codeWord);
};
