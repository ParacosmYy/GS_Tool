/**
 * @file PolluxCode6.h
 * @brief Pollux密码(扩展莫尔斯元素集约束传播密钥空间归约) — Pollux Cipher with Extended Morse Element Set and Constraint Propagation for Efficient Key Space Reduction
 *
 * 功能: 实现Pollux密码(Pollux cipher)，采用扩展莫尔斯元素集(extended Morse
 *       element set)和约束传播(constraint propagation)进行高效密钥空间归约
 *       (key space reduction)，支持点/dot、划/dash、字符间隔/letter space
 *       三种基础元素。
 *
 * 协作: Huffman9(哈夫曼编码) / EnigmaCipher7(恩尼格玛密码) / CaesarCode6(凯撒密码)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief Pollux密码(扩展莫尔斯元素集约束传播密钥空间归约)
 */
class PolluxCode6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int keyLength = 0;
        int numEncryptions = 0;
        int numDecryptions = 0;
        int keyspaceReductionPercent = 0;
        double avgProcessingTimeMs = 0.0;
    };

    /** @brief Morse element types: dot, dash, letter space */
    enum class MorseElement : quint8 {
        Dot = 0,
        Dash = 1,
        LetterSpace = 2
    };

    /** @brief Key entry: maps a digit to a Morse element */
    struct KeyEntry {
        int digit = 0;
        MorseElement element = MorseElement::Dot;
    };

    explicit PolluxCode6(QObject *parent = nullptr);
    ~PolluxCode6() override;

    /** @brief Set encryption/decryption key */
    void setKey(const QVector<KeyEntry>& key);

    /** @brief Generate random key with specified length */
    QVector<KeyEntry> generateKey(int length) const;

    /** @brief Encrypt plaintext string to digit sequence */
    QVector<int> encrypt(const QString& plaintext) const;

    /** @brief Decrypt digit sequence to plaintext via constraint propagation */
    QString decrypt(const QVector<int>& ciphertext) const;

    /** @brief Reduce keyspace using known plaintext constraints */
    QVector<QVector<KeyEntry>> reduceKeyspace(const QVector<int>& ciphertext,
                                               const QString& knownPlaintext) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encryptionCompleted(int inputLen, int outputLen, double timeMs);
    void decryptionCompleted(int cipherLen, QString result, double timeMs);
    void keyspaceReduced(int originalSize, int reducedSize);

private:
    QVector<KeyEntry> m_key;
    int m_keyLength = 10;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Convert character to Morse code (dot/dash sequence) */
    QVector<MorseElement> charToMorse(QChar ch) const;

    /** @brief Convert Morse element sequence back to character */
    QChar morseToChar(const QVector<MorseElement>& elements) const;

    /** @brief Build constraint set from known plaintext-ciphertext pair */
    QVector<QVector<MorseElement>> buildConstraints(
        const QVector<int>& cipher, const QString& known) const;

    /** @brief Check if a key satisfies all constraints */
    bool satisfiesConstraints(const QVector<KeyEntry>& key,
                               const QVector<QVector<MorseElement>>& constraints) const;

    /** @brief Insert word-space into Morse stream at letter boundaries */
    QVector<MorseElement> insertLetterSpaces(const QVector<MorseElement>& raw) const;
};
