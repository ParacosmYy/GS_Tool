/**
 * @file BaudotCode7.h
 * @brief Baudot码(扩展FIGS转移字符+奇偶校验位无歧义模式检测) — Baudot Code with Extended FIGS Shift Characters and Unambiguous Mode Detection via Parity Bit Validation
 *
 * 功能: 实现Baudot码(Baudot Code)编解码，支持扩展FIGS转移字符(extended FIGS
 *       shift characters)和通过奇偶校验位(parity bit)验证实现无歧义模式检测
 *       (unambiguous mode detection)。
 *
 * 协作: HuffmanEncoder9(哈夫曼编码) / ShannonFano7(香农-法诺) / CRC16(校验)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <QByteArray>

/**
 * @brief Baudot码(扩展FIGS转移字符+奇偶校验位无歧义模式检测)
 */
class BaudotCode7 : public QObject {
    Q_OBJECT

public:
    /** @brief Shift mode */
    enum Mode {
        LettersMode = 0,   // LETTERS shift
        FiguresMode = 1    // FIGURES shift
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int charsEncoded = 0;
        int charsDecoded = 0;
        int parityErrors = 0;
        int modeSwitches = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BaudotCode7(QObject *parent = nullptr);
    ~BaudotCode7() override;

    /** @brief Enable parity checking for unambiguous mode detection */
    void setParityCheck(bool enabled);

    /** @brief Set parity type: true=even, false=odd */
    void setEvenParity(bool even);

    /** @brief Encode a string to Baudot 5-bit codes */
    QVector<quint8> encode(const QString& text);

    /** @brief Decode Baudot 5-bit codes to string */
    QString decode(const QVector<quint8>& codes);

    /** @brief Detect mode from stream using parity analysis */
    Mode detectMode(const QVector<quint8>& codes) const;

    /** @brief Get current shift mode */
    Mode currentMode() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encodingCompleted(int charCount, int modeSwitches, double timeMs);
    void parityErrorDetected(int position, quint8 code);

private:
    bool m_parityCheck = true;
    bool m_evenParity = true;
    Mode m_mode = LettersMode;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief LETTERS shift character table (ITA2 standard) */
    static const char s_letters[32];

    /** @brief FIGURES shift character table (extended) */
    static const char s_figures[32];

    /** @brief LETTERS shift code (11111) */
    static constexpr quint8 LTRS_SHIFT = 0x1F;

    /** @brief FIGURES shift code (11011) */
    static constexpr quint8 FIGS_SHIFT = 0x1B;

    /** @brief Build reverse lookup table */
    quint8 charToCode(QChar ch, Mode mode) const;

    /** @brief Validate parity bit of a 5-bit code */
    bool validateParity(quint8 code) const;

    /** @brief Add parity bit to 5-bit code */
    quint8 addParity(quint8 code5) const;

    /** @brief Strip parity bit, returning 5-bit data */
    quint8 stripParity(quint8 code6) const;

    /** @brief Analyze parity distribution to determine mode */
    Mode analyzeParity(const QVector<quint8>& codes) const;
};
