/**
 * @file BazeleriesCode.h
 * @brief Bazeleries密码(不规则Polybius矩形+列置换混合) — Bazeleries Cipher with Irregular Polybius Rectangle and Columnar Transposition Hybrid
 *
 * 功能: 实现Bazeleries密码算法，支持不规则Polybius矩形映射、
 *       列置换(Columnar Transposition)混合加密、自定义密钥和中文支持。
 *
 * 协作: VigenereCipher2(维吉尼亚) / PlayfairCipher3(Playfair) / ADFGVX3(ADFGVX)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <QPair>

/**
 * @brief Bazeleries密码器(不规则Polybius+列置换)
 */
class BazeleriesCode : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalOperations = 0;
        int inputLength = 0;
        int outputLength = 0;
        int gridSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BazeleriesCode(QObject *parent = nullptr);
    ~BazeleriesCode() override;

    void setKeyword(const QString& key);
    void setTranspositionKey(const QString& key);
    void setGridRows(int rows);
    void setGridCols(int cols);

    /** @brief Encrypt plaintext via Bazeleries cipher */
    QString encrypt(const QString& plaintext);

    /** @brief Decrypt ciphertext via Bazeleries cipher */
    QString decrypt(const QString& ciphertext);

    /** @brief Get current Polybius grid (for inspection) */
    QVector<QVector<QChar>> polybiusGrid() const { return m_grid; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(const QString& op, int inLen, int outLen, double timeMs);

private:
    QString m_keyword;
    QString m_transKey;
    int m_gridRows = 5;
    int m_gridCols = 6;

    QVector<QVector<QChar>> m_grid;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build irregular Polybius rectangle from keyword */
    void buildGrid();

    /** @brief Map character to (row, col) in Polybius grid */
    QPair<int, int> findInGrid(QChar ch) const;

    /** @brief Columnar transposition encrypt */
    QString columnarEncrypt(const QString& text);

    /** @brief Columnar transposition decrypt */
    QString columnarDecrypt(const QString& text);

    /** @brief Read grid cell at position */
    QChar gridAt(int row, int col) const;
};
