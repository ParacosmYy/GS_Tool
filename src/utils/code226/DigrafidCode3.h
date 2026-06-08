/**
 * @file DigrafidCode3.h
 * @brief Digrafid密码(三图替换+双关键字分数化网格) — Digrafid Cipher with Trigraphic Substitution and Dual Keyword-Driven Fractionation Grid
 *
 * 功能: 实现Digrafid密码的三图替换(trigraphic substitution)加密/解密，
 *       通过双关键字驱动的分数化网格(fractionation grid)进行字符分合操作。
 *
 * 协作: ADFGVX3(ADFGVX密码) / Bifid5(Bifid密码) / Playfair7(Playfair密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <QPair>

/**
 * @brief Digrafid密码(三图替换+双关键字分数化)
 */
class DigrafidCode3 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numEncryptions = 0;
        int numDecryptions = 0;
        int inputLength = 0;
        int gridRows = 0;
        int gridCols = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DigrafidCode3(QObject *parent = nullptr);
    ~DigrafidCode3() override;

    /** @brief Set keywords and grid dimensions (rows=cols=period) */
    void setKeys(const QString& keyword1, const QString& keyword2,
                 int period = 7);

    /** @brief Encrypt plaintext using Digrafid cipher */
    QString encrypt(const QString& plaintext) const;

    /** @brief Decrypt ciphertext using Digrafid cipher */
    QString decrypt(const QString& ciphertext) const;

    /** @brief Build fractionation grid from keyword */
    QVector<QVector<QChar>> buildGrid(const QString& keyword) const;

    /** @brief Validate keyword (unique letters) */
    bool validateKeyword(const QString& keyword) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encryptionCompleted(int inputLen, int outputLen, double timeMs);
    void decryptionCompleted(int inputLen, int outputLen, double timeMs);

private:
    QString m_key1;
    QString m_key2;
    int m_period = 7;

    Stats m_stats;
    double m_timeSum = 0.0;

    // Pre-built grids
    QVector<QVector<QChar>> m_grid1;
    QVector<QVector<QChar>> m_grid2;
    QString m_alphabet;

    /** @brief Remove duplicate chars and build key-ordered alphabet */
    QString processKeyword(const QString& key) const;

    /** @brief Find character position in grid */
    QPair<int, int> findInGrid(const QVector<QVector<QChar>>& grid,
                                QChar ch) const;

    /** @brief Convert position to mixed-radix digit */
    int posToDigit(int row, int col, int gridSize) const;

    /** @brief Convert digit back to row/col */
    QPair<int, int> digitToPos(int digit, int gridSize) const;
};
