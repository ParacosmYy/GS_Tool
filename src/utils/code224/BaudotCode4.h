/**
 * @file BaudotCode4.h
 * @brief 博多码(CCITT-2国际变体+移位状态有限自动机同步恢复) — Baudot Code with CCITT-2 International Variant and Shift-State Finite Automaton with Synchronization Recovery
 *
 * 功能: 实现CCITT-2博多码编解码，集成移位状态有限自动机(FIGS/LETTERS)，
 *       支持同步恢复和位填充/去填充。
 *
 * 协作: ManchesterCodec3(曼彻斯特编解码) / HDB3Coder2(HDB3编码) / NRZICodec5(NRZI)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QByteArray>

/**
 * @brief 博多码(CCITT-2+移位状态自动机)
 */
class BaudotCode4 : public QObject {
    Q_OBJECT

public:
    /** @brief Shift state */
    enum ShiftState {
        Letters = 0,
        Figures = 1
    };
    Q_ENUM(ShiftState)

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int charsEncoded = 0;
        int charsDecoded = 0;
        int shiftTransitions = 0;
        int syncRecoveries = 0;
        int framingErrors = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BaudotCode4(QObject *parent = nullptr);
    ~BaudotCode4() override;

    /** @brief Encode text string to Baudot bit stream */
    QVector<quint8> encode(const QString& text);

    /** @brief Decode Baudot bit stream to text string */
    QString decode(const QVector<quint8>& codeWords);

    /** @brief Get current shift state */
    ShiftState shiftState() const;

    /** @brief Force shift state (for sync recovery) */
    void forceShiftState(ShiftState state);

    /** @brief Add start/stop bits for transmission framing */
    QByteArray addFraming(const QVector<quint8>& codeWords) const;

    /** @brief Remove start/stop bits and recover sync */
    QVector<quint8> removeFraming(const QByteArray& framed);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encodingCompleted(int chars, int codeWords, double timeMs);
    void shiftChanged(ShiftState newState);
    void syncRecovered(int position);

private:
    ShiftState m_shiftState = Letters;

    // CCITT-2 letter and figure tables (32 entries each)
    QVector<QChar> m_letterTable;
    QVector<QChar> m_figureTable;

    // Reverse lookup: character -> code
    QMap<QChar, quint8> m_letterLookup;
    QMap<QChar, quint8> m_figureLookup;

    static constexpr quint8 LETTERS_SHIFT = 0x1F;
    static constexpr quint8 FIGURES_SHIFT = 0x1B;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Initialize CCITT-2 code tables */
    void initTables();

    /** @brief Encode single character with state machine */
    QVector<quint8> encodeChar(QChar ch);

    /** @brief Decode single codeword with state machine */
    QChar decodeCodeWord(quint8 code);
};
