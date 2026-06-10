/**
 * @file StraddlingCheckerboard6.h
 * @brief 跨越棋盘密码(Polybius坐标嵌入行列移位周期转位置换) — Straddling Checkerboard with Polybius Coordinate Embedding and Row/Column Shift for Periodic Transposition Layering
 *
 * 功能: 实现跨越棋盘密码(straddling checkerboard cipher)，采用Polybius坐标嵌入
 *       (Polybius coordinate embedding)和行列移位(row/column shift)进行周期
 *       转位置换(periodic transposition layering)。
 *
 * 协作: HillCipher6(希尔密码) / PlayfairCipher6(Playfair密码) / VigenereCipher6(维吉尼亚)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QChar>

/**
 * @brief 跨越棋盘密码(Polybius坐标嵌入行列移位周期转位置换)
 */
class StraddlingCheckerboard6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int inputLength = 0;
        int outputLength = 0;
        int numPeriods = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit StraddlingCheckerboard6(QObject *parent = nullptr);
    ~StraddlingCheckerboard6() override;

    /** @brief Set alphabet key (26 letters) and blank row indices */
    void setKey(const QString& alphabetKey, const QVector<int>& blankRows);

    /** @brief Set transposition period length */
    void setPeriod(int period);

    /** @brief Encode plaintext to numeric ciphertext */
    QString encode(const QString& plaintext);

    /** @brief Decode numeric ciphertext to plaintext */
    QString decode(const QString& ciphertext);

    /** @brief Get current checkerboard layout */
    QVector<QVector<QChar>> checkerboard() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encodingCompleted(int inputLen, int outputLen, double timeMs);
    void decodingCompleted(int inputLen, int outputLen, double timeMs);

private:
    QString m_alphabet;             // 26-letter permuted alphabet
    QVector<int> m_blankRows;       // indices of blank rows (typically 2)
    int m_period = 7;               // transposition period

    // Checkerboard: 3 rows x 10 cols, some cells blank
    QVector<QVector<QChar>> m_board;
    QVector<int> m_rowMap;          // char -> row mapping
    QVector<int> m_colMap;          // char -> col mapping

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build the checkerboard from alphabet and blank rows */
    void buildCheckerboard();

    /** @brief Encode single character to digit string */
    QString encodeChar(QChar ch) const;

    /** @brief Apply row/column shift for periodic transposition */
    QVector<int> transpose(const QVector<int>& digits, bool encrypt) const;

    /** @brief Extract digit values from ciphertext string */
    QVector<int> parseDigits(const QString& text) const;
};
