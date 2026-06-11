/**
 * @file BaudotCode13.h
 * @brief 博多码(移位状态自动化与字符集切换实现ITA2兼容电传打字机编码) — Baudot Code with Shift-state Automation and Character Set Switching for ITA2-compatible Teletypewriter Encoding with Figure/Letter Modes
 *
 * 功能: 实现博多码(Baudot code)，采用移位状态自动化(shift-state automation)
 *       与字符集切换(character set switching)实现ITA2兼容电传打字机编码(ITA2-compatible teletypewriter encoding)，
 *       支持数字/字母模式(figure/letter modes)。
 *
 * 协作: ManchesterEncoder9(曼彻斯特编码) / HdlcFramer10(HDLC帧) / CrcProcessor7(CRC处理器)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QChar>

class BaudotCode13 : public QObject {
    Q_OBJECT

public:
    /** @brief Shift mode: Letter or Figure */
    enum class ShiftMode {
        Letter = 0,
        Figure = 1
    };

    /** @brief Encoding result for a single character */
    struct EncodedBit {
        quint8 code5 = 0;          // 5-bit Baudot code
        bool shiftNeeded = false;
        ShiftMode newMode = ShiftMode::Letter;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalEncodes = 0;
        quint64 totalDecodes = 0;
        quint64 shiftTransitions = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BaudotCode13(QObject *parent = nullptr);
    ~BaudotCode13() override;

    /** @brief Encode a string to Baudot 5-bit codes with automatic shift */
    QVector<EncodedBit> encode(const QString& text);

    /** @brief Decode Baudot 5-bit codes back to string */
    QString decode(const QVector<quint8>& codes);

    /** @brief Encode single character with shift handling */
    EncodedBit encodeChar(QChar ch, ShiftMode currentMode) const;

    /** @brief Decode single 5-bit code */
    QChar decodeCode(quint8 code5, ShiftMode mode) const;

    /** @brief Get current shift mode */
    ShiftMode currentMode() const { return m_currentMode; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encodeDone(int inputLen, int outputCodes, int shifts, double timeMs);

private:
    ShiftMode m_currentMode = ShiftMode::Letter;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief ITA2 letter set lookup */
    QVector<QChar> m_letterSet;

    /** @brief ITA2 figure set lookup */
    QVector<QChar> m_figureSet;

    /** @brief Reverse lookup: letter char -> 5-bit code */
    QMap<QChar, quint8> m_letterReverse;

    /** @brief Reverse lookup: figure char -> 5-bit code */
    QMap<QChar, quint8> m_figureReverse;

    static constexpr quint8 SHIFT_LETTER = 0x1F;  // 11111
    static constexpr quint8 SHIFT_FIGURE = 0x1B;  // 11011

    /** @brief Initialize ITA2 code tables */
    void initTables();
};
