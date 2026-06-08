/**
 * @file BazeleriesCode3.h
 * @brief Bazeleries密码(嵌套分数网格+ADFGVX坐标编码行列密钥) — Bazeleries Cipher with Nested Fractionation Grids and ADFGVX-style Coordinate Encoding with Row/Column Keys
 *
 * 功能: 实现Bazeleries密码，使用嵌套分数化网格(nested fractionation grids)进行字符映射，
 *       结合ADFGVX风格的坐标编码(coordinate encoding)配合行列密钥(row/column keys)进行加解密。
 *
 * 协作: SubstitutionCipher1(替换密码) / TranspositionCipher2(置换密码) / VigenereCipher4(维吉尼亚)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief Bazeleries密码(嵌套分数网格+ADFGVX坐标编码)
 */
class BazeleriesCode3 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int inputLength = 0;
        int outputLength = 0;
        int gridSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BazeleriesCode3(QObject *parent = nullptr);
    ~BazeleriesCode3() override;

    /** @brief Set row key and column key (permutation keys) */
    void setKeys(const QString& rowKey, const QString& colKey);

    /** @brief Set alphabet for the fractionation grid (default: A-Z + 0-9) */
    void setAlphabet(const QString& alphabet);

    /** @brief Encrypt plaintext */
    QString encrypt(const QString& plaintext);

    /** @brief Decrypt ciphertext */
    QString decrypt(const QString& ciphertext);

    /** @brief Get current fractionation grid */
    QVector<QString> grid() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encryptionCompleted(int inputLen, int outputLen, double timeMs);
    void decryptionCompleted(int inputLen, int outputLen, double timeMs);

private:
    QString m_rowKey;
    QString m_colKey;
    QString m_alphabet;
    int m_gridSize = 6;

    // ADFGVX coordinate labels
    static const QString COORD_LABELS;

    // Fractionation grid: 6x6 = 36 chars
    QVector<QVector<QChar>> m_grid;
    QVector<int> m_rowPerm;
    QVector<int> m_colPerm;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build the fractionation grid from alphabet and keys */
    void buildGrid();

    /** @brief Generate permutation from key string */
    QVector<int> keyPermutation(const QString& key) const;

    /** @brief Find character position in grid */
    bool findInGrid(QChar ch, int& row, int& col) const;

    /** @brief Encode character to ADFGVX coordinate pair */
    QString encodeCoord(QChar ch) const;

    /** @brief Decode ADFGVX coordinate pair to character */
    QChar decodeCoord(const QString& coord) const;

    /** @brief Columnar transposition encrypt */
    QString columnarEncrypt(const QString& text) const;

    /** @brief Columnar transposition decrypt */
    QString columnarDecrypt(const QString& text, int originalRows) const;
};
