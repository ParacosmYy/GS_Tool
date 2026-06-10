/**
 * @file SeriatedPlayfair7.h
 * @brief 串联普莱费尔密码(渐进关键字旋转与列置换的双重Bifid增强加密) — Seriated Playfair with Progressive Keyword Rotation and Columnar Seriation for Enhanced Bifid-style Encryption
 *
 * 功能: 实现串联普莱费尔密码(Seriated Playfair cipher)，采用渐进关键字旋转(progressive keyword rotation)
 *       与列置换(columnar seriation)实现双重Bifid增强加密(enhanced Bifid-style encryption)。
 *
 * 协作: AesEncrypt3(AES加密) / XteaCipher5(XTEA加密) / Base64Codec6(编解码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <QChar>

/**
 * @brief 串联普莱费尔密码(渐进关键字旋转与列置换)
 */
class SeriatedPlayfair7 : public QObject {
    Q_OBJECT

public:
    /** @brief Encryption parameters */
    struct Params {
        QString keyword;              // Primary keyword for grid generation
        int period = 7;               // Seriation period for Bifid-style split
        int rotationStep = 3;         // Keyword rotation step between blocks
        bool columnarSeriation = true;// Enable columnar transposition
    };

    /** @brief Cipher result */
    struct CipherResult {
        QString text;                 // Encrypted/decrypted text
        int blockCount = 0;
        int rotationsApplied = 0;
        double processingTimeMs = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numEncrypts = 0;
        int numDecrypts = 0;
        int totalChars = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SeriatedPlayfair7(QObject *parent = nullptr);
    ~SeriatedPlayfair7() override;

    /** @brief Set cipher parameters */
    void setParams(const Params& params);

    /** @brief Encrypt plaintext using seriated Playfair */
    CipherResult encrypt(const QString& plaintext);

    /** @brief Decrypt ciphertext using seriated Playfair */
    CipherResult decrypt(const QString& ciphertext);

    /** @brief Generate 5x5 Playfair grid from keyword */
    QVector<QVector<QChar>> generateGrid(const QString& keyword) const;

    /** @brief Get current grid for inspection */
    QVector<QVector<QChar>> currentGrid() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encryptionDone(int chars, int blocks, double timeMs);
    void decryptionDone(int chars, int blocks, double timeMs);

private:
    Params m_params;
    QVector<QVector<QChar>> m_grid;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Preprocess text: uppercase, replace J->I, pad digraphs */
    QString preprocess(const QString& text) const;

    /** @brief Find character position in grid */
    void findPosition(QChar ch, int& row, int& col) const;

    /** @brief Apply Playfair digraph substitution */
    QPair<QChar, QChar> substituteDigraph(QChar a, QChar b, bool encrypt) const;

    /** @brief Apply Bifid-style seriation split */
    QVector<QChar> seriateSplit(const QVector<QChar>& chars, int period) const;

    /** @brief Reverse seriation (recombine) */
    QVector<QChar> seriateMerge(const QVector<QChar>& chars, int period) const;

    /** @brief Rotate keyword by step positions */
    QString rotateKeyword(const QString& keyword, int step) const;

    /** @brief Columnar transposition */
    QVector<QChar> columnarTranspose(const QVector<QChar>& chars, bool encrypt) const;

    /** @brief Build 25-letter alphabet for grid (excluding J) */
    QVector<QChar> buildAlphabet(const QString& keyword) const;
};
