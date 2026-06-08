/**
 * @file FoursquareCode5.h
 * @brief 四方密码(10×10扩展网格+Polybius坐标压缩+关键字洗牌) — Foursquare Cipher with 10x10 Extended Grid, Polybius Coordinate Compression and Keyword Shuffle
 *
 * 功能: 实现四方密码算法，使用10×10扩展网格替代传统5×5，
 *       Polybius坐标压缩与关键字洗牌增强安全性。
 *
 * 协作: PlayfairCode3(Playfair密码) / VigenereCode4(Vigenere密码) / HillCipher2(Hill密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QChar>

/**
 * @brief 四方密码(10×10扩展网格+坐标压缩)
 */
class FoursquareCode5 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int gridSize = 10;
        int keywordLength = 0;
        int inputLength = 0;
        int outputLength = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FoursquareCode5(QObject *parent = nullptr);
    ~FoursquareCode5() override;

    /** @brief Set keyword for grid generation */
    void setKeyword(const QString& keyword1, const QString& keyword2);

    /** @brief Encrypt plaintext using foursquare cipher */
    QString encrypt(const QString& plaintext) const;

    /** @brief Decrypt ciphertext using foursquare cipher */
    QString decrypt(const QString& ciphertext) const;

    /** @brief Generate 10x10 grid from keyword */
    QVector<QVector<QChar>> generateGrid(const QString& keyword) const;

    /** @brief Polybius coordinate compression */
    QByteArray compressCoordinates(const QVector<QPair<int, int>>& coords) const;

    /** @brief Decompress to coordinates */
    QVector<QPair<int, int>> decompressCoordinates(const QByteArray& compressed) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void cipherCompleted(const QString& operation, int inputLen, int outputLen, double timeMs);

private:
    QString m_keyword1;
    QString m_keyword2;
    QVector<QVector<QChar>> m_gridTL;  // Top-left (standard)
    QVector<QVector<QChar>> m_gridTR;  // Top-right (keyword1)
    QVector<QVector<QChar>> m_gridBL;  // Bottom-left (keyword2)
    QVector<QVector<QChar>> m_gridBR;  // Bottom-right (standard)

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build standard 10x10 alphabet grid */
    void buildStandardGrid(QVector<QVector<QChar>>& grid) const;

    /** @brief Build keyword-shuffled 10x10 grid */
    void buildKeywordGrid(const QString& keyword,
                            QVector<QVector<QChar>>& grid) const;

    /** @brief Find character position in grid */
    QPair<int, int> findPosition(const QVector<QVector<QChar>>& grid,
                                   QChar ch) const;

    /** @brief Get character at grid position */
    QChar charAt(const QVector<QVector<QChar>>& grid, int row, int col) const;

    /** @brief Preprocess text: uppercase, replace non-alpha */
    QString preprocess(const QString& text) const;

    /** @brief Keyword shuffle algorithm */
    QString shuffleAlphabet(const QString& keyword) const;
};
