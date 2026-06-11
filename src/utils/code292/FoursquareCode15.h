/**
 * @file FoursquareCode15.h
 * @brief 四方密码(二部坐标映射与双表交叉引用实现增强型波利比乌斯加密) — Foursquare Code with Bipartite Coordinate Mapping and Dual-tableau Cross-reference for Strengthened Polybius Encryption
 *
 * 功能: 实现四方密码(Foursquare cipher)，采用二部坐标映射(bipartite coordinate mapping)
 *       与双表交叉引用(dual-tableau cross-reference)实现增强型波利比乌斯加密(strengthened Polybius encryption)。
 *
 * 协作: PlayfairCipher8(Playfair密码) / HillCipher10(Hill密码) / ADFGVCipher7(ADFGVX密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

class FoursquareCode15 : public QObject {
    Q_OBJECT

public:
    /** @brief Encryption result */
    struct CipherResult {
        QString ciphertext;
        int inputLength = 0;
        int outputLength = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int encryptCount = 0;
        int decryptCount = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FoursquareCode15(QObject *parent = nullptr);
    ~FoursquareCode15() override;

    /** @brief Set the two keyword pairs (kw1 for TL/BR, kw2 for TR/BL) */
    void setKeywords(const QString& kw1, const QString& kw2);

    /** @brief Encrypt plaintext using foursquare cipher */
    CipherResult encrypt(const QString& plaintext) const;

    /** @brief Decrypt ciphertext using foursquare cipher */
    CipherResult decrypt(const QString& ciphertext) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encryptDone(int inLen, int outLen, double timeMs);
    void decryptDone(int inLen, int outLen, double timeMs);

private:
    static constexpr int TABLE_SIZE = 5;
    static constexpr int ALPHA_SIZE = 25;   // I/J merged

    /** @brief A 5×5 Polybius square */
    struct Tableau {
        int grid[TABLE_SIZE][TABLE_SIZE];    // letter index 0-24
        int position[25][2];                 // [letter] -> (row, col)
    };

    QString m_kw1;
    QString m_kw2;
    Tableau m_tl;       // Top-left (plain square)
    Tableau m_tr;       // Top-right (key1 square)
    Tableau m_bl;       // Bottom-left (key2 square)
    Tableau m_br;       // Bottom-right (plain square)
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build a 5×5 tableau from keyword */
    void buildTableau(const QString& keyword, Tableau& tab) const;

    /** @brief Build standard A-Z (I=J) tableau */
    void buildStandardTableau(Tableau& tab) const;

    /** @brief Normalize text: uppercase, replace J with I, remove non-alpha */
    QString normalize(const QString& text) const;

    /** @brief Pad text to even length */
    QString padEven(const QString& text) const;

    /** @brief Map letter char to index 0-24 (J→I) */
    int charToIndex(QChar ch) const;

    /** @brief Map index back to letter */
    QChar indexToChar(int idx) const;
};
