/**
 * @file MorbitCode10.h
 * @brief Morbit密码(同音莫尔斯映射与随机符号选择实现可手工操作的字段加密) — Morbit Cipher with Homophonic Morse Mapping and Stochastic Symbol Selection for Randomized Hand-Operable Field Encryption
 *
 * 功能: 实现Morbit密码(Morbit cipher)，采用同音莫尔斯映射(homophonic Morse mapping)
 *       与随机符号选择(stochastic symbol selection)实现可手工操作的字段加密(randomized hand-operable field encryption)。
 *
 * 协作: CaesarCipher(凯撒密码) / SubstitutionCipher(替换密码) / VigenereCipher(维吉尼亚密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <QHash>

class MorbitCode10 : public QObject {
    Q_OBJECT

public:
    /** @brief Morse element with homophonic variants */
    struct MorseSymbol {
        QString dot;        // Primary dot representation
        QString dash;       // Primary dash representation
        QString separator;  // Letter separator
        QVector<QString> dotVariants;   // Homophonic dot variants
        QVector<QString> dashVariants;  // Homophonic dash variants
    };

    /** @brief Encryption result */
    struct EncryptResult {
        QString ciphertext;
        QString morseSequence;
        int symbolsUsed = 0;
        double elapsedMs = 0.0;
    };

    /** @brief Decryption result */
    struct DecryptResult {
        QString plaintext;
        bool success = false;
        int errors = 0;
        double elapsedMs = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalEncryptions = 0;
        quint64 totalDecryptions = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit MorbitCode10(QObject *parent = nullptr);
    ~MorbitCode10() override;

    /** @brief Set 9-digit key for Morbit grid */
    void setKey(const QString& key);

    /** @brief Enable/disable homophonic randomization */
    void setHomophonicEnabled(bool enabled);

    /** @brief Encrypt plaintext to Morbit ciphertext */
    EncryptResult encrypt(const QString& plaintext);

    /** @brief Decrypt Morbit ciphertext */
    DecryptResult decrypt(const QString& ciphertext);

    /** @brief Text to Morse code */
    QString textToMorse(const QString& text) const;

    /** @brief Morse to digit pairs via Morbit key */
    QString morseToDigits(const QString& morse) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encryptDone(int length, double timeMs);
    void decryptDone(int length, bool success, double timeMs);

private:
    QString m_key;
    bool m_homophonic = true;
    QHash<QChar, QString> m_charToMorse;
    QHash<QString, QChar> m_morseToChar;
    QHash<QChar, int> m_morseGrid;     // Morbit digit grid mapping
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build standard Morse lookup tables */
    void buildMorseTable();

    /** @brief Build Morbit 3x3 grid from key */
    void buildMorbitGrid();

    /** @brief Select random homophonic variant */
    QString randomVariant(const QVector<QString>& variants) const;

    /** @brief Map Morse pair (. or -) to digit via grid */
    QChar morsePairToDigit(QChar first, QChar second) const;
};
