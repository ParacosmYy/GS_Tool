/**
 * @file BazeleriesCode2.h
 * @brief Bazeleries密码(组合分馏扩散+交替行列置换) — Bazeleries Cipher with Combined Fractionation and Diffusion Using Alternating Column-Row Transposition
 *
 * 功能: 实现Bazeleries密码，支持组合分馏与扩散操作，
 *       交替列行置换实现加密解密。
 *
 * 协作: PlayfairCipher1(Playfair) / ADFGVX3(ADFGVX) / Bifid4(双分密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief Bazeleries密码(分馏扩散+交替置换)
 */
class BazeleriesCode2 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int inputLength = 0;
        int outputLength = 0;
        int numRows = 0;
        int numCols = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BazeleriesCode2(QObject *parent = nullptr);
    ~BazeleriesCode2() override;

    /** @brief Set transposition grid dimensions */
    void setParameters(int cols = 7, int fracDepth = 2);

    /** @brief Set custom substitution key */
    void setKey(const QString& key);

    /** @brief Encrypt plaintext */
    QString encrypt(const QString& plaintext);

    /** @brief Decrypt ciphertext */
    QString decrypt(const QString& ciphertext);

    /** @brief Build substitution alphabet from key */
    QString buildAlphabet(const QString& key) const;

    /** @brief Fractionate text into coordinate pairs */
    QVector<int> fractionate(const QString& text) const;

    /** @brief Defractionate coordinates back to text */
    QString defractionate(const QVector<int>& coords) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encryptionCompleted(int inLen, int outLen, double timeMs);

private:
    int m_cols = 7;
    int m_fracDepth = 2;
    QString m_key;
    QString m_alphabet;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Column transposition */
    QString columnTranspose(const QString& text, int rows, int cols) const;

    /** @brief Row transposition */
    QString rowTranspose(const QString& text, int rows, int cols) const;

    /** @brief Inverse column transposition */
    QString inverseColumnTranspose(const QString& text, int rows, int cols) const;

    /** @brief Inverse row transposition */
    QString inverseRowTranspose(const QString& text, int rows, int cols) const;

    /** @brief Substitute single character */
    QChar substitute(QChar c, bool invert) const;
};
