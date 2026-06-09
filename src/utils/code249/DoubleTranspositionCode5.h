/**
 * @file DoubleTranspositionCode5.h
 * @brief 双重置换密码(Myszkowski重复关键字处理+交错偏移列式回读) — Double Transposition Cipher with Myszkowski-style Repeated Keyword Handling and Columnar Read-back with Staggered Offset
 *
 * 功能: 实现双重置换密码(Double Transposition Cipher)，使用Myszkowski风格
 *       重复关键字处理(Myszkowski repeated keyword)支持关键字中重复字母的
 *       并行列读取，交错偏移(staggered offset)列式回读增强安全性。
 *
 * 协作: HillCipher4(Hill密码) / VigenereCipher3(Vigenere密码) / RailFenceCode2(栅栏密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief 双重置换密码(Myszkowski重复关键字+交错偏移回读)
 */
class DoubleTranspositionCode5 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int inputLength = 0;
        int keyLength1 = 0;
        int keyLength2 = 0;
        int numEncryptions = 0;
        int numDecryptions = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DoubleTranspositionCode5(QObject *parent = nullptr);
    ~DoubleTranspositionCode5() override;

    /** @brief Set primary and secondary keywords */
    void setKeywords(const QString& key1, const QString& key2);

    /** @brief Set staggered read-back offset */
    void setStaggerOffset(int offset);

    /** @brief Encrypt plaintext via double columnar transposition */
    QString encrypt(const QString& plaintext);

    /** @brief Decrypt ciphertext via inverse double transposition */
    QString decrypt(const QString& ciphertext);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encryptionCompleted(int length, double timeMs);
    void decryptionCompleted(int length, double timeMs);

private:
    QString m_key1;
    QString m_key2;
    int m_staggerOffset = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build column order from keyword (Myszkowski-style) */
    QVector<QVector<int>> buildMyszkowskiOrder(const QString& keyword) const;

    /** @brief Single columnar transposition with Myszkowski read */
    QString columnarTransposition(const QString& text,
                                   const QString& keyword,
                                   bool encrypt) const;

    /** @brief Write text into grid rows */
    QVector<QVector<QChar>> fillGrid(const QString& text, int cols) const;

    /** @brief Read grid columns in keyword order (Myszkowski merge for ties) */
    QString readGridMyszkowski(const QVector<QVector<QChar>>& grid,
                                const QVector<QVector<int>>& order) const;

    /** @brief Inverse Myszkowski read: scatter ciphertext back into grid */
    QString inverseMyszkowskiRead(const QString& cipher, int rows, int cols,
                                    const QVector<QVector<int>>& order) const;

    /** @brief Apply staggered offset to column indices */
    QVector<int> applyStagger(const QVector<int>& cols) const;
};
