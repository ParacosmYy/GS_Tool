/**
 * @file BaudotCode3.h
 * @brief Baudot编码(Murray/ITA2变体+无歧义移位状态解码与错误恢复) — Baudot Code with Murray/ITA2 Variant and Unambiguous Shift-State Decoder with Error Recovery
 *
 * 功能: 实现Baudot/Murray/ITA2编码与解码，支持字母/数字移位状态机，
 *       无歧义状态解码，具备错误检测与自动恢复能力。
 *
 * 协作: HammingCode4(汉明码) / ConvolutionalCode2(卷积码) / HuffmanCode3(霍夫曼编码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief Baudot编码(Murray/ITA2变体+移位状态解码)
 */
class BaudotCode3 : public QObject {
    Q_OBJECT

public:
    /** @brief Shift state for decoder */
    enum ShiftState { Letters = 0, Figures = 1 };

    /** @brief Error type for recovery */
    enum ErrorType { None = 0, InvalidCode = 1, StateConflict = 2, ParityError = 3 };

    /** @brief Decode result */
    struct DecodeResult {
        QChar character;
        ShiftState state = Letters;
        ErrorType error = None;
        bool wasShifted = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int encodedChars = 0;
        int decodedChars = 0;
        int shiftTransitions = 0;
        int errorsRecovered = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BaudotCode3(QObject *parent = nullptr);
    ~BaudotCode3() override;

    /** @brief Set variant: 0=ITA2, 1=Murray */
    void setVariant(int variant);

    /** @brief Encode string to 5-bit Baudot codes */
    QVector<quint8> encode(const QString& text);

    /** @brief Decode 5-bit codes to string with error recovery */
    QString decode(const QVector<quint8>& codes);

    /** @brief Decode single 5-bit code with full state info */
    DecodeResult decodeSingle(quint8 code);

    /** @brief Get current shift state */
    ShiftState currentShiftState() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void decodingCompleted(int chars, int errors, double timeMs);

private:
    int m_variant = 0;  // 0=ITA2, 1=Murray
    ShiftState m_shiftState = Letters;

    // Character tables
    QVector<QChar> m_lettersTable;
    QVector<QChar> m_figuresTable;

    // Reverse lookup: char -> 5-bit code
    QVector<quint8> m_letterLookup;
    QVector<quint8> m_figureLookup;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build ITA2 character tables */
    void buildITA2Tables();

    /** @brief Build Murray character tables */
    void buildMurrayTables();

    /** @brief Attempt error recovery for invalid code */
    DecodeResult recoverError(quint8 code, ErrorType type);

    /** @brief Force shift to specified state (emitting shift code if needed) */
    quint8 forceShift(ShiftState target);
};
