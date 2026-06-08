/**
 * @file SeriatedPlayfair4.h
 * @brief 序列化Playfair密码(关键词驱动Polybius网格+扩展二字母组替换与填充优化) — Seriated Playfair Cipher with Keyword-Driven Polybius Grid and Extended Digraph Substitution with Filler Optimization
 *
 * 功能: 实现序列化Playfair密码(Seriated Playfair cipher)，采用关键词驱动(keyword-driven)
 *       构建Polybius网格(Polybius grid)，支持扩展二字母组替换(extended digraph substitution)
 *       与填充字符优化(filler optimization)，用于经典密码教学与分析。
 *
 * 协作: VigenereCipher3(维吉尼亚密码) / Aes256Engine5(AES加密) / RsaEngine4(RSA加密)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QChar>
#include <QString>

/**
 * @brief 序列化Playfair密码(关键词驱动Polybius网格+扩展二字母组替换与填充优化)
 */
class SeriatedPlayfair4 : public QObject {
    Q_OBJECT

public:
    /** @brief Grid cell with row/col indices */
    struct GridCell {
        QChar ch;
        int row = 0;
        int col = 0;
    };

    /** @brief Cipher operation result */
    struct CipherResult {
        QString text;
        int numDigraphs = 0;
        int numFillers = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int gridSize = 5;
        int numEncryptions = 0;
        int numDecryptions = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SeriatedPlayfair4(QObject *parent = nullptr);
    ~SeriatedPlayfair4() override;

    /** @brief Set keyword for grid generation */
    void setKeyword(const QString& keyword);

    /** @brief Set filler character for odd-length digraphs */
    void setFillerChar(QChar ch);

    /** @brief Set grid size (5 for 5x5 I/J merged, 6 for 6x6 with digits) */
    void setGridSize(int size);

    /** @brief Build the Polybius grid from keyword */
    void buildGrid();

    /** @brief Encrypt plaintext */
    CipherResult encrypt(const QString& plaintext);

    /** @brief Decrypt ciphertext */
    CipherResult decrypt(const QString& ciphertext);

    /** @brief Get current grid as 2D char array */
    QVector<QVector<QChar>> grid() const;

    /** @brief Find position of character in grid */
    GridCell findChar(QChar ch) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void gridBuilt(int size, const QString& keyword);
    void encryptCompleted(int digraphs, int fillers, double timeMs);
    void decryptCompleted(int digraphs, double timeMs);

private:
    QString m_keyword;
    QChar m_fillerChar = QLatin1Char('X');
    int m_gridSize = 5;

    QVector<QVector<QChar>> m_grid;     // [size x size]
    QVector<QChar> m_alphabet;          // full alphabet used

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Generate alphabet for current grid size */
    QVector<QChar> generateAlphabet() const;

    /** @brief Preprocess text: upper case, remove non-alpha, split into digraphs */
    QVector<QPair<QChar, QChar>> preprocessText(const QString& text, bool isEncrypt) const;

    /** @brief Apply Playfair substitution rule to one digraph */
    QPair<QChar, QChar> substituteDigraph(QChar a, QChar b, bool encrypt) const;

    /** @brief Handle I/J merging for 5x5 grid */
    QChar normalizeChar(QChar ch) const;
};
