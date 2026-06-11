/**
 * @file FoursquareCode16.h
 * @brief 四方密码(同音替换与Polybius坐标抖动实现频率隐藏增强表加密) — Foursquare Code with Homophonic Substitution and Polybius Coordinate Jitter for Frequency-Hiding Strengthened Tableaux Encryption
 *
 * 功能: 实现四方密码(Foursquare cipher)，采用同音替换(homophonic substitution)
 *       与Polybius坐标抖动(Polybius coordinate jitter)实现频率隐藏增强表加密(frequency-hiding strengthened tableaux encryption)。
 *
 * 协作: VigenereCipher(维吉尼亚密码) / PlayfairCipher(普莱费尔密码) / SubstitutionCipher(替换密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <QMap>

class FoursquareCode16 : public QObject {
    Q_OBJECT

public:
    /** @brief Encryption configuration */
    struct Config {
        QString key1;
        QString key2;
        QString alphabet = QStringLiteral("ABCDEFGHIKLMNOPQRSTUVWXYZ");  // 25 chars (J=I)
        bool homophonicEnabled = true;
        double jitterStrength = 0.3;     // Coordinate jitter magnitude [0,1)
    };

    /** @brief Encryption result */
    struct EncryptResult {
        QString ciphertext;
        int length = 0;
        int digraphCount = 0;
        double elapsedMs = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int totalCharsEncrypted = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FoursquareCode16(QObject *parent = nullptr);
    ~FoursquareCode16() override;

    void setConfig(const Config& config);

    /** @brief Encrypt plaintext with Foursquare + homophonic + jitter */
    EncryptResult encrypt(const QString& plaintext);

    /** @brief Decrypt ciphertext back to plaintext */
    EncryptResult decrypt(const QString& ciphertext);

    /** @brief Build a Polybius square from key */
    QVector<QVector<QChar>> buildSquare(const QString& key) const;

    /** @brief Frequency analysis of text */
    QMap<QChar, int> frequencyAnalysis(const QString& text) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encryptDone(int length, int digraphs, double timeMs);

private:
    Config m_config;
    QVector<QVector<QChar>> m_tlSquare;    // Top-left (plain alphabet)
    QVector<QVector<QChar>> m_trSquare;    // Top-right (key1)
    QVector<QVector<QChar>> m_blSquare;    // Bottom-left (key2)
    QVector<QVector<QChar>> m_brSquare;    // Bottom-right (plain alphabet)
    QMap<QChar, QVector<int>> m_homophones; // Homophone map: char -> list of codes
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Remove duplicates and build 25-char key alphabet */
    QString prepareKey(const QString& key) const;

    /** @brief Find character position in a Polybius square */
    QPair<int, int> findPosition(const QVector<QVector<QChar>>& square, QChar ch) const;

    /** @brief Preprocess text: uppercase, J->I, pad odd length */
    QString preprocess(const QString& text) const;

    /** @brief Build homophone table from frequency analysis */
    void buildHomophoneTable(const QString& referenceText);

    /** @brief Apply Polybius coordinate jitter */
    QPair<int, int> applyJitter(int row, int col) const;

    /** @brief Remove jitter from coordinates */
    QPair<int, int> removeJitter(int row, int col) const;
};
