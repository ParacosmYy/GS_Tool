/**
 * @file FoursquareCode10.h
 * @brief 四方密码(18x18扩展网格三关键字多层替换编码) — Foursquare Cipher with Extended 18x18 Grid and Triple-keyword Driven Multi-layer Substitution Encoding
 *
 * 功能: 实现四方密码(Foursquare cipher)的扩展版本，采用18x18网格
 *       和三关键字驱动的多层替换(multi-layer substitution)编码，
 *       适用于高强度文本加密与密码学教学演示。
 *
 * 协作: PlayfairCipher6(Playfair密码) / VigenereCipher5(维吉尼亚) / HillCipher7(Hill密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QChar>

/**
 * @brief 四方密码(18x18扩展网格三关键字多层替换编码)
 */
class FoursquareCode10 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numEncryptions = 0;
        int numDecryptions = 0;
        int gridSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FoursquareCode10(QObject *parent = nullptr);
    ~FoursquareCode10() override;

    /** @brief Set three keywords for the three cipher squares */
    void setKeywords(const QString& kw1, const QString& kw2, const QString& kw3);

    /** @brief Encrypt plaintext using extended foursquare cipher */
    QString encrypt(const QString& plaintext);

    /** @brief Decrypt ciphertext using extended foursquare cipher */
    QString decrypt(const QString& ciphertext);

    /** @brief Get the current grid at position (0-3) */
    QVector<QChar> grid(int index) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encryptionCompleted(int inputLen, int outputLen, double timeMs);
    void decryptionCompleted(int inputLen, int outputLen, double timeMs);

private:
    static const int GRID_SIZE = 18;
    static const int ALPHABET_SIZE = 324; // 18*18

    QString m_kw1, m_kw2, m_kw3;
    QVector<QVector<QChar>> m_grids;   // 4 grids: TL plain, TR kw1, BL kw2, BR kw3

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build the extended alphabet for 18x18 grid */
    QVector<QChar> buildExtendedAlphabet() const;

    /** @brief Build a grid from keyword filling */
    QVector<QChar> buildGrid(const QString& keyword);

    /** @brief Build default plain grid */
    QVector<QChar> buildPlainGrid();

    /** @brief Find character position in grid */
    bool findInGrid(const QVector<QChar>& grid, QChar ch, int& row, int& col) const;

    /** @brief Prepare text for encryption (pad, normalize) */
    QString prepareText(const QString& text) const;

    /** @brief Process a digraph through the foursquare */
    QPair<QChar, QChar> processDigraph(QChar a, QChar b, bool encrypt) const;
};
