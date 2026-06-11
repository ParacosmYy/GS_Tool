/**
 * @file MorbitCode8.h
 * @brief Morbit密码(扩展符号表与多层替换手操野战加密) — Morbit Cipher with Extended Symbol Alphabet and Multi-layer Substitution for Hand-operable Field Encryption
 *
 * 功能: 实现Morbit密码(Morbit cipher)，采用扩展符号表(extended symbol alphabet)
 *       与多层替换(multi-layer substitution)实现手操野战加密(hand-operable field encryption)。
 *
 * 协作: EnigmaMachine8(恩尼格玛) / AffineCipher6(仿射密码) / PlayfairCipher7(Playfair密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <QMap>

/**
 * @brief Morbit密码(扩展符号表与多层替换手操野战加密)
 */
class MorbitCode8 : public QObject {
    Q_OBJECT

public:
    /** @brief Cipher configuration */
    struct MorbitConfig {
        QString key = QStringLiteral("MORBIT");
        int numLayers = 3;              // Number of substitution layers
        int symbolBase = 10;            // Symbol alphabet size (10=digit, 26=alpha)
    };

    /** @brief Encryption result */
    struct EncryptResult {
        QString cipherText;
        QString intermediateDigit;      // Digits before final substitution
        int symbolBase = 10;
        int numLayers = 3;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int totalCharsEncrypted = 0;
        int totalCharsDecrypted = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit MorbitCode8(QObject *parent = nullptr);
    ~MorbitCode8() override;

    void setConfig(const MorbitConfig& cfg);

    /** @brief Encrypt plaintext using Morbit with multi-layer substitution */
    EncryptResult encrypt(const QString& plainText);

    /** @brief Decrypt ciphertext back to plaintext */
    QString decrypt(const QString& cipherText);

    /** @brief Generate a random Morbit key */
    QString generateKey(int length = 9) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encryptDone(int plainLen, int cipherLen, double timeMs);
    void decryptDone(int cipherLen, int plainLen, double timeMs);

private:
    MorbitConfig m_config;
    Stats m_stats;
    double m_timeSum = 0.0;

    QMap<QChar, int> m_polybiusMap;
    QMap<int, QChar> m_polybiusReverse;
    QVector<QVector<int>> m_layerSubs;
    QVector<QVector<int>> m_layerSubsInv;

    /** @brief Build Polybius square from key */
    void buildPolybiusSquare();

    /** @brief Generate layer substitution tables from key */
    void buildLayerTables();

    /** @brief Polybius encode a character to digit pair */
    QVector<int> polybiusEncode(QChar ch) const;

    /** @brief Polybius decode digit pair to character */
    QChar polybiusDecode(int row, int col) const;

    /** @brief Apply forward substitution layer */
    QVector<int> applyLayer(const QVector<int>& digits, int layer) const;

    /** @brief Apply inverse substitution layer */
    QVector<int> applyLayerInverse(const QVector<int>& digits, int layer) const;

    /** @brief Convert digits to symbol string */
    QString digitsToSymbols(const QVector<int>& digits) const;

    /** @brief Convert symbol string to digits */
    QVector<int> symbolsToDigits(const QString& sym) const;
};
