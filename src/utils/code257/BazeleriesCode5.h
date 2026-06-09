/**
 * @file BazeleriesCode5.h
 * @brief 巴泽里密码(嵌套列置换+密钥派生乱序读取) — Bazeleries Cipher with Nested Columnar Transposition and Key-derived Permutation with Disrupted Reading Order
 *
 * 功能: 实现巴泽里密码(Bazeleries cipher)，采用嵌套列置换(nested
 *       columnar transposition)结合密钥派生置换(key-derived permutation)
 *       与乱序读取(disrupted reading order)进行加密解密。
 *
 * 协作: ADFGVX3(ADFGVX密码) / Playfair4(Playfair密码) / Vigenere2(Vigenere密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief 巴泽里密码(嵌套列置换+密钥派生乱序读取)
 */
class BazeleriesCode5 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int inputLength = 0;
        int outputLength = 0;
        int numColumns = 0;
        int numRows = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BazeleriesCode5(QObject *parent = nullptr);
    ~BazeleriesCode5() override;

    /** @brief Set primary key for first transposition */
    void setPrimaryKey(const QString& key);

    /** @brief Set secondary key for nested transposition */
    void setSecondaryKey(const QString& key);

    /** @brief Set disrupted reading pattern (e.g. "132" for column read order) */
    void setDisruptionPattern(const QString& pattern);

    /** @brief Encrypt plaintext */
    QString encrypt(const QString& plaintext);

    /** @brief Decrypt ciphertext */
    QString decrypt(const QString& ciphertext);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void cipherCompleted(bool encrypt, int inLen, int outLen, double timeMs);

private:
    QString m_primaryKey;
    QString m_secondaryKey;
    QString m_disruption;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Derive column permutation from key */
    QVector<int> keyPermutation(const QString& key) const;

    /** @brief Single columnar transposition */
    QString columnarTranspose(const QString& text, const QString& key, bool encrypt) const;

    /** @brief Apply disrupted reading order to grid */
    QString disruptedRead(const QString& text, int cols, const QVector<int>& order, bool byRow) const;

    /** @brief Fill disrupted pattern */
    QVector<QVector<QChar>> buildDisruptedGrid(const QString& text, int rows, int cols,
                                                const QVector<int>& disruptOrder) const;
};
