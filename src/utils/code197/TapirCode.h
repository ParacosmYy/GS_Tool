/**
 * @file TapirCode.h
 * @brief Tapir密码(Polybius分层+水平/垂直转位置换) — Tapir Code with Polybius-Based Fractionation and Horizontal/Vertical Readout Transposition
 *
 * 功能: 实现Tapir密码，支持Polybius方阵分层、
 *       水平/垂直读出转位和文本编解码。
 *
 * 协作: HillCipher3(Hill密码) / PlayfairCipher2(Playfair密码) / ADFGVPolybius4(ADFGVX密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <QPair>

/**
 * @brief Tapir密码(Polybius分层+转位)
 */
class TapirCode : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalOperations = 0;
        int inputLength = 0;
        int outputLength = 0;
        int gridSize = 5;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Transposition mode */
    enum TranspositionMode {
        HorizontalRead = 0,
        VerticalRead = 1,
        DiagonalRead = 2,
        SpiralRead = 3
    };

    explicit TapirCode(QObject *parent = nullptr);
    ~TapirCode() override;

    void setGridSize(int size);
    void setKeyword(const QString& keyword);
    void setTranspositionMode(TranspositionMode mode);
    void setColumnKey(const QString& key);

    /** @brief Encode plaintext using Tapir cipher */
    QString encode(const QString& plaintext);

    /** @brief Decode ciphertext using Tapir cipher */
    QString decode(const QString& ciphertext);

    /** @brief Build Polybius square from keyword */
    QVector<QVector<QChar>> buildPolybiusGrid() const;

    /** @brief Get grid coordinates for a character */
    QPair<int, int> findInGrid(QChar ch) const;

    /** @brief Transpose fractionated pairs via readout mode */
    QVector<int> transpose(const QVector<int>& indices, int width) const;

    /** @brief Reverse transpose to recover original order */
    QVector<int> reverseTranspose(const QVector<int>& indices, int width) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(const QString& op, int inLen, int outLen, double timeMs);

private:
    int m_gridSize = 5;
    QString m_keyword;
    TranspositionMode m_mode = HorizontalRead;
    QString m_columnKey;

    QVector<QVector<QChar>> m_grid;
    QVector<QChar> m_alphaSet;  // alphabet used in grid

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Populate the alphabet set from keyword */
    void buildAlphaSet();

    /** @brief Sort column key indices */
    QVector<int> columnOrder() const;
};
