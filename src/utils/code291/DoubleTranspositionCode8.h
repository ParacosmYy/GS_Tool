/**
 * @file DoubleTranspositionCode8.h
 * @brief 双重置换密码(级联矩形网格与置换密钥调度实现复合列加密) — Double Transposition with Cascaded Rectangular Grids and Permutation Key Scheduling for Compound Columnar Encryption
 *
 * 功能: 实现双重置换密码(Double transposition cipher)，采用级联矩形网格(cascaded rectangular grids)
 *       与置换密钥调度(permutation key scheduling)实现复合列加密(compound columnar encryption)。
 *
 * 协作: HillCipher8(Hill密码) / VigenereCipher7(Vigenere密码) / AffineCipher6(仿射密码)
 */
#pragma once

#include <QObject>
#include <QVector>

class DoubleTranspositionCode8 : public QObject {
    Q_OBJECT

public:
    /** @brief Encryption/decryption result */
    struct CipherResult {
        QString output;
        int gridRows1 = 0;               // First pass grid dimensions
        int gridCols1 = 0;
        int gridRows2 = 0;               // Second pass grid dimensions
        int gridCols2 = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int totalCharsProcessed = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DoubleTranspositionCode8(QObject *parent = nullptr);
    ~DoubleTranspositionCode8() override;

    /** @brief Set primary permutation key (numeric column order) */
    void setKey1(const QVector<int>& key);

    /** @brief Set secondary permutation key */
    void setKey2(const QVector<int>& key);

    /** @brief Set fill character for padding */
    void setFillChar(QChar ch);

    /** @brief Encrypt plaintext using double columnar transposition */
    CipherResult encrypt(const QString& plaintext);

    /** @brief Decrypt ciphertext using double columnar transposition */
    CipherResult decrypt(const QString& ciphertext);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationDone(const QString& op, int inputLen, int outputLen, double timeMs);

private:
    QVector<int> m_key1;
    QVector<int> m_key2;
    QChar m_fillChar = QLatin1Char('X');
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Validate and normalize a permutation key */
    QVector<int> normalizeKey(const QVector<int>& key) const;

    /** @brief Single columnar transposition encrypt */
    QString columnarEncrypt(const QString& text, const QVector<int>& key,
                             int& rows, int& cols) const;

    /** @brief Single columnar transposition decrypt */
    QString columnarDecrypt(const QString& text, const QVector<int>& key,
                             int rows, int cols) const;

    /** @brief Derive grid dimensions for given text and key */
    void gridDimensions(int textLen, int keyLen, int& rows, int& cols) const;
};
