/**
 * @file BaudotCode10.h
 * @brief 博多码(FIGS/LETRS换档管理与CRC-5检错的5位电传编码) — Baudot Code with FIGS/LETRS Shift Management and CRC-5 Error Detection for 5-Bit Teleprinter Encoding
 *
 * 功能: 实现博多码(Baudot code)，采用FIGS/LETRS换档管理(shift management)
 *       和CRC-5检错(CRC-5 error detection)实现5位电传编码(5-bit teleprinter encoding)。
 *
 * 协作: ManchesterCode8(曼彻斯特码) / NRZI7(NRZI编码) / DifferentialDecoder9(差分解码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QByteArray>

/**
 * @brief 博多码(FIGS/LETRS换档管理与CRC-5检错的5位电传编码)
 */
class BaudotCode10 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int encodedChars = 0;
        int decodedChars = 0;
        int crcErrors = 0;
        int shiftCount = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BaudotCode10(QObject *parent = nullptr);
    ~BaudotCode10() override;

    /** @brief Encode text to 5-bit Baudot code words */
    QVector<quint8> encode(const QString& text);

    /** @brief Decode 5-bit Baudot code words to text */
    QString decode(const QVector<quint8>& codeWords);

    /** @brief Compute CRC-5 over code words */
    quint8 computeCRC5(const QVector<quint8>& data) const;

    /** @brief Verify CRC-5 appended to code words */
    bool verifyCRC5(const QVector<quint8>& dataWithCRC) const;

    /** @brief Convert code words to byte stream (packed 5-bit) */
    QByteArray packBits(const QVector<quint8>& codeWords) const;

    /** @brief Unpack byte stream to 5-bit code words */
    QVector<quint8> unpackBits(const QByteArray& bytes, int numCodeWords) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encodingUpdated(int numChars, int shifts, double timeMs);

private:
    /** @brief Shift state: false=LETRS, true=FIGS */
    bool m_inFigShift = false;

    Stats m_stats;
    double m_timeSum = 0.0;

    // ITA2 code tables
    static const QVector<QChar> s_lettersTable;
    static const QVector<QChar> s_figuresTable;
    static const quint8 FIGS_CODE;
    static const quint8 LETRS_CODE;

    /** @brief Lookup character in letters table */
    int findLetter(QChar ch) const;

    /** @brief Lookup character in figures table */
    int findFigure(QChar ch) const;
};
