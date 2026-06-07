/**
 * @file BaudotCode.h
 * @brief 博多码/ITA2电传编码(5位编码+数字字母切换+MTBF检错) — Baudot/ITA2 Teleprinter Code with 5-bit Encoding, Figure/Letter Shift and MTBF Error Detection
 *
 * 功能: 实现Baudot/ITA2电传编码，支持5位编码、
 *       数字/字母切换和MTBF误码检测。
 *
 * 协作: Huffman8(霍夫曼编码) / ReedSolomon6(纠错码) / ManchesterCodec6(曼彻斯特编解码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief 博多码/ITA2电传编码(5位编码+MTBF检错)
 */
class BaudotCode : public QObject {
    Q_OBJECT

public:
    /** @brief Shift state */
    enum ShiftState {
        Letters = 0,
        Figures = 1
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalEncodes = 0;
        quint64 totalDecodes = 0;
        int inputChars = 0;
        int outputBits = 0;
        int errorsDetected = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BaudotCode(QObject *parent = nullptr);
    ~BaudotCode() override;

    void setMTBFThreshold(double threshold);
    void setErrorDetectionEnabled(bool enabled);

    /** @brief Encode string to Baudot 5-bit code sequence */
    QVector<quint8> encode(const QString& text);

    /** @brief Decode Baudot 5-bit sequence to string */
    QString decode(const QVector<quint8>& code);

    /** @brief Encode single character, returns 5-bit value + shift flag */
    QPair<quint8, ShiftState> encodeChar(QChar ch, ShiftState currentShift) const;

    /** @brief Decode single 5-bit value */
    QChar decodeChar(quint8 code5bit, ShiftState shift) const;

    /** @brief Calculate MTBF-based error probability for a sequence */
    double errorProbability(const QVector<quint8>& code) const;

    /** @brief Detect and mark errors in code sequence */
    QVector<int> detectErrors(const QVector<quint8>& code) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encodeCompleted(int chars, int bits, double timeMs);
    void errorDetected(int position, double probability);

private:
    double m_mtbfThreshold = 0.01;
    bool m_errorDetection = true;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Letter shift lookup table (5-bit -> char) */
    static const char s_letters[32];

    /** @brief Figure shift lookup table (5-bit -> char) */
    static const char s_figures[32];

    /** @brief Character to 5-bit code (letter mode) */
    static const QHash<QChar, quint8> s_charToLetter;

    /** @brief Character to 5-bit code (figure mode) */
    static const QHash<QChar, quint8> s_charToFigure;

    static constexpr quint8 LETTER_SHIFT = 0x1F;  // 11111
    static constexpr quint8 FIGURE_SHIFT = 0x1B;  // 11011

    /** @brief Initialize lookup tables */
    static QHash<QChar, quint8> buildLetterMap();
    static QHash<QChar, quint8> buildFigureMap();

    /** @brief Compute parity-based confidence for a 5-bit code */
    double bitConfidence(quint8 code) const;
};
