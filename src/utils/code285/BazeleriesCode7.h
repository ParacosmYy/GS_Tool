/**
 * @file BazeleriesCode7.h
 * @brief Bazeleries密码(嵌套对角列置换与关键字派生矩形网格多层排列) — Bazeleries Cipher with Nested Diagonal-columnar Transposition and Keyword-derived Rectangular Grid for Multi-layer Permutation
 *
 * 功能: 实现Bazeleries密码(Bazeleries cipher)，采用嵌套对角列置换(nested diagonal-columnar transposition)
 *       与关键字派生矩形网格(keyword-derived rectangular grid)实现多层排列(multi-layer permutation)。
 *
 * 协作: ADFGVXCode5(ADFGVX密码) / PlayfairCode4(Playfair密码) / VigenereCode3(Vigenere密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief Bazeleries密码(嵌套对角列置换与关键字派生矩形网格多层排列)
 */
class BazeleriesCode7 : public QObject {
    Q_OBJECT

public:
    /** @brief Cipher configuration */
    struct Config {
        QString keyword;
        int gridRows = 0;       // 0 = auto-compute from text
        int gridCols = 0;       // 0 = auto-compute from keyword
    };

    /** @brief Cipher result */
    struct CipherResult {
        QString text;
        int gridRows = 0;
        int gridCols = 0;
        QVector<int> columnOrder;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numEncrypts = 0;
        int numDecrypts = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BazeleriesCode7(QObject *parent = nullptr);
    ~BazeleriesCode7() override;

    void setKeyword(const QString& keyword);

    /** @brief Encrypt plaintext using Bazeleries cipher */
    CipherResult encrypt(const QString& plaintext);

    /** @brief Decrypt ciphertext using Bazeleries cipher */
    CipherResult decrypt(const QString& ciphertext);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encryptDone(int length, double timeMs);
    void decryptDone(int length, double timeMs);

private:
    QString m_keyword;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Derive column permutation order from keyword */
    QVector<int> deriveColumnOrder(const QString& keyword) const;

    /** @brief Build rectangular grid from text */
    QVector<QVector<QChar>> buildGrid(const QString& text, int cols) const;

    /** @brief Read grid in diagonal pattern for Bazeleries */
    QString readDiagonal(const QVector<QVector<QChar>>& grid, int cols) const;

    /** @brief Fill grid from diagonal-ordered text */
    QVector<QVector<QChar>> fillDiagonal(const QString& text, int rows, int cols) const;

    /** @brief Columnar transposition */
    QString columnarTranspose(const QString& text, const QVector<int>& order, int cols) const;

    /** @brief Reverse columnar transposition */
    QString columnarReverse(const QString& text, const QVector<int>& order, int rows, int cols) const;

    /** @brief Auto-compute grid dimensions */
    QPair<int, int> computeDimensions(int textLen) const;
};
