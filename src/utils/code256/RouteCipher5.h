/**
 * @file RouteCipher5.h
 * @brief 路线密码(行列转置+可配置读取模式空字符填充) — Route Cipher with Column/Row Transposition and Configurable Reading Pattern with Null Character Padding
 *
 * 功能: 实现路线密码(Route Cipher)，支持行列转置(column/row transposition)
 *       和可配置读取模式(configurable reading pattern)，含空字符填充
 *       (null character padding)机制，用于数据编解码。
 *
 * 协作: SubstitutionCipher3(替换密码) / VigenereCipher4(维吉尼亚密码) / XORCipher2(XOR密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QChar>

/**
 * @brief 路线密码(行列转置+可配置读取模式)
 */
class RouteCipher5 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numRows = 0;
        int numCols = 0;
        int inputLength = 0;
        int paddedLength = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Reading pattern for route traversal */
    enum class Pattern {
        SpiralIn,       // Clockwise spiral inward
        SpiralOut,      // Counter-clockwise spiral outward
        ZigzagRow,      // Zigzag across rows
        ZigzagCol,      // Zigzag across columns
        Diagonal        // Diagonal snake pattern
    };

    /** @brief Route direction */
    enum class Direction { RowFirst, ColFirst };

    explicit RouteCipher5(QObject *parent = nullptr);
    ~RouteCipher5() override;

    /** @brief Set grid rows */
    void setRows(int rows);

    /** @brief Set grid columns */
    void setCols(int cols);

    /** @brief Set reading pattern */
    void setPattern(Pattern pattern);

    /** @brief Set fill direction */
    void setDirection(Direction dir);

    /** @brief Set null padding character */
    void setPaddingChar(QChar ch);

    /** @brief Encrypt plaintext via route cipher */
    QString encrypt(const QString& plaintext);

    /** @brief Decrypt ciphertext via route cipher */
    QString decrypt(const QString& ciphertext);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void cipherCompleted(int inputLen, int outputLen, double timeMs);

private:
    int m_rows = 4;
    int m_cols = 5;
    Pattern m_pattern = Pattern::SpiralIn;
    Direction m_direction = Direction::RowFirst;
    QChar m_paddingChar = QChar('X');

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build grid from text */
    QVector<QVector<QChar>> buildGrid(const QString& text, int& paddedLen) const;

    /** @brief Read grid in spiral-in pattern */
    QString readSpiralIn(const QVector<QVector<QChar>>& grid) const;

    /** @brief Read grid in spiral-out pattern */
    QString readSpiralOut(const QVector<QVector<QChar>>& grid) const;

    /** @brief Read grid in zigzag row pattern */
    QString readZigzagRow(const QVector<QVector<QChar>>& grid) const;

    /** @brief Read grid in zigzag col pattern */
    QString readZigzagCol(const QVector<QVector<QChar>>& grid) const;

    /** @brief Read grid in diagonal pattern */
    QString readDiagonal(const QVector<QVector<QChar>>& grid) const;

    /** @brief Fill grid from spiral-in sequence */
    void fillSpiralIn(QVector<QVector<QChar>>& grid, const QString& text);

    /** @brief Fill grid from zigzag row sequence */
    void fillZigzagRow(QVector<QVector<QChar>>& grid, const QString& text);

    /** @brief Read grid using current pattern */
    QString readGrid(const QVector<QVector<QChar>>& grid) const;

    /** @brief Fill grid using current pattern */
    void fillGrid(QVector<QVector<QChar>>& grid, const QString& text);
};
