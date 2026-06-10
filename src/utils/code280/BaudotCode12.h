/**
 * @file BaudotCode12.h
 * @brief 博多码(Murray码变体与字母/数字切换管理的ITA2兼容电报通信) — Baudot Code with Murray Code Variant and Figure/Letter Shift Management for ITA2-compatible Teleprinter Communication
 *
 * 功能: 实现博多码(Baudot code)，采用Murray码变体(Murray code variant)
 *       与字母/数字切换管理(figure/letter shift management)实现ITA2兼容电报通信(ITA2-compatible teleprinter communication)。
 *
 * 协作: ManchesterCode10(曼彻斯特码) / NRZEncoder9(NRZ编码) / CRC16(循环冗余校验)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QChar>

/**
 * @brief 博多码(Murray码变体与字母/数字切换管理)
 */
class BaudotCode12 : public QObject {
    Q_OBJECT

public:
    /** @brief Shift mode */
    enum ShiftMode { Letters = 0, Figures = 1 };

    /** @brief Encoding result */
    struct EncodeResult {
        QVector<quint8> fiveBitCodes;
        QVector<bool> shiftSequence;
        int numShifts = 0;
    };

    /** @brief Decoding result */
    struct DecodeResult {
        QString text;
        int numErrors = 0;
        int numShifts = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int charsEncoded = 0;
        int charsDecoded = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BaudotCode12(QObject *parent = nullptr);
    ~BaudotCode12() override;

    /** @brief Encode text to 5-bit Baudot codes with shift management */
    EncodeResult encode(const QString& text);

    /** @brief Decode 5-bit Baudot codes to text */
    DecodeResult decode(const QVector<quint8>& codes);

    /** @brief Set ITA2 variant (true = Murray/ITA2, false = original Baudot) */
    void setMurrayMode(bool murray);

    /** @brief Get letter table entry */
    QChar letterFromCode(quint8 code) const;

    /** @brief Get figure table entry */
    QChar figureFromCode(quint8 code) const;

    /** @brief Convert 5-bit code to wire state (mark/space) */
    QVector<bool> codeToWire(quint8 code) const;

    /** @brief Convert wire state back to 5-bit code */
    quint8 wireToCode(const QVector<bool>& wire) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encodeDone(int chars, int codes, double timeMs);
    void decodeDone(int codes, int chars, int errors, double timeMs);
    void shiftChanged(ShiftMode mode);

private:
    bool m_murray = true;
    ShiftMode m_currentShift = Letters;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief ITA2 letter shift table (32 entries) */
    QVector<QChar> m_letterTable;

    /** @brief ITA2 figure shift table (32 entries) */
    QVector<QChar> m_figureTable;

    /** @brief Reverse lookup: character -> 5-bit code for letters */
    QMap<QChar, quint8> m_letterReverse;

    /** @brief Reverse lookup: character -> 5-bit code for figures */
    QMap<QChar, quint8> m_figureReverse;

    /** @brief Build ITA2 lookup tables */
    void buildTables();

    /** @brief LTRS shift code (11111) */
    static constexpr quint8 LTRS_CODE = 0x1F;

    /** @brief FIGS shift code (11011) */
    static constexpr quint8 FIGS_CODE = 0x1B;
};
