/**
 * @file DigrafidCode4.h
 * @brief Digrafid密码(二连字替换+分数混合群转置) — Digrafid Cipher with Digram Substitution and Fractional Mixed-Group Transposition across a Rectangular Key Grid
 *
 * 功能: 实现Digrafid密码(Digrafid cipher)，采用二连字替换(digram substitution)将明文
 *       字符对映射到行列坐标，通过分数混合群转置(fractional mixed-group transposition)
 *       在矩形密钥网格上进行行列重排，实现经典多表替换加密。
 *
 * 协作: ADFGVX3(ADFGVX密码) / Playfair2(Playfair密码) / Bifid1(Bifid密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief Digrafid密码(二连字替换+分数混合群转置)
 */
class DigrafidCode4 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int gridSize = 0;
        int numEncryptions = 0;
        int numDecryptions = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DigrafidCode4(QObject *parent = nullptr);
    ~DigrafidCode4() override;

    /** @brief Set rectangular grid dimensions (rows x cols = alphabet size) */
    void setGridSize(int rows, int cols);

    /** @brief Set primary mixed alphabet key */
    void setKey(const QString& key);

    /** @brief Set period for fractional transposition */
    void setPeriod(int period);

    /** @brief Encrypt plaintext */
    QString encrypt(const QString& plaintext);

    /** @brief Decrypt ciphertext */
    QString decrypt(const QString& ciphertext);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encryptionCompleted(int inLen, int outLen, double timeMs);
    void decryptionCompleted(int inLen, int outLen, double timeMs);

private:
    int m_rows = 3;
    int m_cols = 9;
    int m_period = 5;

    QString m_key;
    QString m_alphabet;
    QVector<QChar> m_grid;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build mixed-alphabet grid from key */
    void buildGrid();

    /** @brief Find row and col of character in grid */
    bool findPosition(QChar ch, int& row, int& col) const;

    /** @brief Perform fractional transposition on coordinate pairs */
    QVector<int> transpose(const QVector<int>& coords, int period) const;

    /** @brief Reverse fractional transposition */
    QVector<int> reverseTranspose(const QVector<int>& coords, int period) const;
};
