/**
 * @file BaudotCode6.h
 * @brief 博多码(ITA2字符集+移位状态管理+字母/数字模式切换) — Baudot Code with ITA2 Character Set and Shift-State Management Including Letters/Figures Mode Switching
 *
 * 功能: 实现博多码(Baudot code)编解码，采用ITA2字符集(ITA2 character set)定义32个字符编码，
 *       利用移位状态管理(shift-state management)在字母模式(letters mode)与数字模式(figures mode)
 *       之间自动切换，实现5位编码的高效通信。
 *
 * 协作: ManchesterCodec5(曼彻斯特编解码) / HdlcFramer5(HDLC帧) / CrcCalculator12(CRC校验)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QByteArray>

/**
 * @brief 博多码(ITA2字符集+移位状态管理+字母/数字模式切换)
 */
class BaudotCode6 : public QObject {
    Q_OBJECT

public:
    /** @brief Shift mode */
    enum Mode { Letters = 0, Figures = 1 };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numEncoded = 0;
        int numDecoded = 0;
        int numShiftLetters = 0;
        int numShiftFigures = 0;
        int numErrors = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BaudotCode6(QObject *parent = nullptr);
    ~BaudotCode6() override;

    /** @brief Encode ASCII text to Baudot 5-bit codes */
    QVector<quint8> encode(const QString& text);

    /** @brief Decode Baudot 5-bit codes to ASCII text */
    QString decode(const QVector<quint8>& codes);

    /** @brief Get current shift mode */
    Mode currentMode() const;

    /** @brief Reset shift state to Letters mode */
    void resetMode();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void modeChanged(Mode newMode);
    void encodeCompleted(int numChars, int numCodes);
    void decodeCompleted(int numCodes, const QString& text);

private:
    Mode m_mode = Letters;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief ITA2 letters table: 5-bit code -> character */
    QVector<QChar> m_lettersTable;

    /** @brief ITA2 figures table: 5-bit code -> character */
    QVector<QChar> m_figuresTable;

    /** @brief Reverse lookup: char -> 5-bit code (letters) */
    QVector<int> m_lettersReverse;

    /** @brief Reverse lookup: char -> 5-bit code (figures) */
    QVector<int> m_figuresReverse;

    static const int SHIFT_LETTERS = 0x1F;  // 11111
    static const int SHIFT_FIGURES = 0x1B;  // 11011
    static const int BLANK = 0x00;

    /** @brief Initialize ITA2 tables */
    void initTables();

    /** @brief Encode single character, inserting shift codes as needed */
    QVector<quint8> encodeChar(QChar ch);
};
