/**
 * @file BaudotCode2.h
 * @brief 博多码(ITA2字符集+数字/字母移位状态机解码器) — Baudot Code with ITA2 Character Set and Figure/Letter Shift State Machine Decoder
 *
 * 功能: 实现博多码编解码，支持ITA2国际字符集、
 *       数字移位/字母移位状态机和5位编码。
 *
 * 协作: ManchesterCodec2(曼彻斯特) / CRC32(校验) / FrameParser(帧解析)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QByteArray>
#include <QString>

/**
 * @brief 博多码(ITA2字符集+数字/字母移位状态机解码器)
 */
class BaudotCode2 : public QObject {
    Q_OBJECT

public:
    /** @brief Shift state */
    enum ShiftState {
        LetterShift = 0,  // Letters mode
        FigureShift = 1   // Figures (digits/symbols) mode
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int charsEncoded = 0;
        int charsDecoded = 0;
        int shiftTransitions = 0;
        int decodeErrors = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BaudotCode2(QObject *parent = nullptr);
    ~BaudotCode2() override;

    /** @brief Encode text string to Baudot 5-bit codes */
    QVector<quint8> encode(const QString& text);

    /** @brief Decode 5-bit Baudot codes to text */
    QString decode(const QVector<quint8>& codes);

    /** @brief Decode raw byte stream (MSB first, packed 5-bit) */
    QString decodePacked(const QByteArray& packed);

    /** @brief Encode to packed byte stream */
    QByteArray encodePacked(const QString& text);

    /** @brief Get current shift state */
    ShiftState shiftState() const;

    /** @brief Reset shift state machine */
    void resetState();

    /** @brief Look up letter character for 5-bit code */
    static QChar letterChar(quint8 code);

    /** @brief Look up figure character for 5-bit code */
    static QChar figureChar(quint8 code);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void decodeCompleted(int chars, int errors, double timeMs);
    void shiftChanged(ShiftState newState);

private:
    ShiftState m_state = LetterShift;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief ITA2 letter set table (32 entries) */
    static const char* s_letterTable;

    /** @brief ITA2 figure set table (32 entries) */
    static const char* s_figureTable;

    /** @brief Find code for a character in given table */
    static int findCode(char ch, const char* table);

    /** @brief Reverse bit order of 5-bit code */
    static quint8 reverseBits5(quint8 code);
};
