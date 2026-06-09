/**
 * @file MorbitCode4.h
 * @brief Morbit密码(三符号Morse片段编码+穷举密钥排列搜索解密) — Morbit Code with 3-Symbol Morse Fragment Encoding and Exhaustive Key Permutation Search for Decryption
 *
 * 功能: 实现Morbit密码(Morbit cipher)，使用三符号Morse片段编码(3-symbol Morse
 *       fragment encoding)将密文映射为字母，通过穷举密钥排列搜索(exhaustive key
 *       permutation search)破解未知密钥的Morbit密文。
 *
 * 协作: HuffmanCodec6(哈夫曼编解码) / RunLengthCode8(游程编码) / ARIncoder7(算术编码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QMap>

/**
 * @brief Morbit密码(三符号Morse片段编码+穷举密钥排列搜索解密)
 */
class MorbitCode4 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int cipherLength = 0;
        int keyLength = 0;
        int permutationsTested = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit MorbitCode4(QObject *parent = nullptr);
    ~MorbitCode4() override;

    /** @brief Set the Morbit key permutation (9-digit permutation of 1-9) */
    void setKey(const QVector<int>& key);

    /** @brief Encrypt plaintext to Morbit ciphertext */
    QString encrypt(const QString& plaintext) const;

    /** @brief Decrypt Morbit ciphertext using current key */
    QString decrypt(const QString& ciphertext) const;

    /** @brief Brute-force decrypt by testing all 9! permutations */
    QVector<QPair<QVector<int>, QString>> bruteForce(const QString& ciphertext) const;

    /** @brief Score plaintext quality using English frequency analysis */
    double scorePlaintext(const QString& text) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encryptionCompleted(int inputLen, int outputLen, double timeMs);
    void bruteForceProgress(int tested, int total);
    void decryptionFound(const QVector<int>& key, const QString& text);

private:
    QVector<int> m_key;  // 9-element permutation of 1..9

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Morse code lookup for A-Z and 0-9 */
    QMap<QChar, QString> m_morseTable;

    /** @brief Fragment table: maps digit 1-9 to Morse fragment */
    QMap<int, QString> m_fragmentTable;

    /** @brief Build the standard Morse code table */
    void buildMorseTable();

    /** @brief Build the 9-entry fragment encoding table */
    void buildFragmentTable();

    /** @brief Convert text to Morse representation */
    QString textToMorse(const QString& text) const;

    /** @brief Decode Morse string back to text */
    QString morseToText(const QString& morse) const;

    /** @brief Generate next permutation in-place */
    bool nextPermutation(QVector<int>& arr) const;

    /** @brief Compute factorial */
    static qint64 factorial(int n);
};
