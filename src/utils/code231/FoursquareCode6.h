/**
 * @file FoursquareCode6.h
 * @brief 四方密码(12x12扩展方阵+双列置换嵌套关键词) — Foursquare Cipher with 12x12 Extended Grid and Double-Columnar Transposition with Nested Keywords
 *
 * 功能: 实现四方密码(Foursquare cipher)，使用12x12扩展字符方阵(extended character grid)，
 *       结合双列置换(double-columnar transposition)与嵌套关键词(nested keyword)增强安全性。
 *
 * 协作: HillCipher3(希尔密码) / PlayfairCipher2(Playfair密码) / VigenereCipher4(Vigenere密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief 四方密码(12x12扩展方阵+双列置换嵌套关键词)
 */
class FoursquareCode6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int gridSize = 12;
        int inputLength = 0;
        int outputLength = 0;
        int numTranspositions = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FoursquareCode6(QObject *parent = nullptr);
    ~FoursquareCode6() override;

    /** @brief Set primary and secondary keywords for the foursquare grids */
    void setKeywords(const QString& keyword1, const QString& keyword2,
                     const QString& transKey1, const QString& transKey2);

    /** @brief Encrypt plaintext */
    QString encrypt(const QString& plaintext);

    /** @brief Decrypt ciphertext */
    QString decrypt(const QString& ciphertext);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encryptCompleted(int length, double timeMs);
    void decryptCompleted(int length, double timeMs);

private:
    QString m_keyword1;
    QString m_keyword2;
    QString m_transKey1;
    QString m_transKey2;

    // Four 12x12 grids: TL, TR, BL, BR
    QVector<QVector<QChar>> m_gridTL;
    QVector<QVector<QChar>> m_gridTR;
    QVector<QVector<QChar>> m_gridBL;
    QVector<QVector<QChar>> m_gridBR;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Character set for 12x12 = 144 positions */
    static const int GRID_SIZE = 12;
    static const int CHARSET_SIZE = 144;

    /** @brief Build character pool: a-z, A-Z, 0-9, punctuation, padding */
    QString buildCharset() const;

    /** @brief Build a 12x12 grid from keyword + remaining alphabet */
    void buildGrid(QVector<QVector<QChar>>& grid, const QString& keyword);

    /** @brief Find character position in grid */
    bool findInGrid(const QVector<QVector<QChar>>& grid, QChar ch, int& row, int& col) const;

    /** @brief Columnar transposition encrypt */
    QString columnarEncrypt(const QString& text, const QString& key) const;

    /** @brief Columnar transposition decrypt */
    QString columnarDecrypt(const QString& text, const QString& key) const;

    /** @brief Get column order from keyword */
    QVector<int> columnOrder(const QString& key) const;
};
