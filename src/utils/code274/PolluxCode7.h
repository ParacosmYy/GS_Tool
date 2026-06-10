/**
 * @file PolluxCode7.h
 * @brief Pollux密码(约束剪枝密钥空间与摩尔斯元素频率分析高效解密) — Pollux Cipher with Constraint-based Key Space Pruning and Morse Element Frequency Analysis for Efficient Decryption
 *
 * 功能: 实现Pollux密码(Pollux cipher)，采用约束剪枝密钥空间
 *       (constraint-based key space pruning)与摩尔斯元素频率分析
 *       (Morse element frequency analysis)实现高效解密(efficient decryption)。
 *
 * 协作: CaesarCipher6(凯撒) / VigenereCipher7(维吉尼亚) / PlayfairCipher6(普莱费尔)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief Pollux密码(约束剪枝密钥空间与摩尔斯元素频率分析)
 */
class PolluxCode7 : public QObject {
    Q_OBJECT

public:
    /** @brief Morse element type */
    enum MorseElement { Dot = 0, Dash = 1, LetterSep = 2, WordSep = 3 };

    /** @brief Decryption result with confidence */
    struct DecryptResult {
        QString plaintext;
        double confidence = 0.0;
        int keysExplored = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int messageLength = 0;
        int keysExplored = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit PolluxCode7(QObject *parent = nullptr);
    ~PolluxCode7() override;

    /** @brief Encrypt plaintext using Pollux cipher with given key */
    QString encrypt(const QString& plaintext, const QVector<int>& key);

    /** @brief Decrypt ciphertext with known key */
    QString decrypt(const QString& ciphertext, const QVector<int>& key);

    /** @brief Auto-decrypt using constraint pruning + frequency analysis */
    DecryptResult autoDecrypt(const QString& ciphertext);

    /** @brief Set maximum search depth for pruning */
    void setMaxSearchDepth(int depth);

    /** @brief Get Morse code table (letter → morse string) */
    QMap<QChar, QString> morseTable() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void decryptionDone(const QString& result, double confidence, double timeMs);

private:
    int m_maxSearchDepth = 10000;

    // Morse code lookup: letter -> Morse string (".-" notation)
    QMap<QChar, QString> m_morseEncode;
    // Reverse: Morse string -> letter
    QMap<QString, QChar> m_morseDecode;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build standard Morse code tables */
    void buildMorseTables();

    /** @brief Convert plaintext to Morse element sequence */
    QVector<MorseElement> textToMorse(const QString& text) const;

    /** @brief Convert Morse element sequence back to text */
    QString morseToText(const QVector<MorseElement>& elements) const;

    /** @brief Prune key candidates using element frequency constraints */
    QVector<QVector<int>> pruneKeySpace(const QString& ciphertext) const;

    /** @brief Score candidate plaintext by English letter frequency */
    double scorePlaintext(const QString& candidate) const;

    /** @brief Recursive DFS with pruning for key search */
    bool dfsSearch(const QVector<int>& cipher, int pos, QVector<int>& key,
                   QVector<MorseElement>& decoded, QString& bestText,
                   double& bestScore, int& explored);
};
