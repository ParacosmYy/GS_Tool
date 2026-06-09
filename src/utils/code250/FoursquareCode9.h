/**
 * @file FoursquareCode9.h
 * @brief 四方密码(关键词驱动扩展网格+三层转置坐标对编码) — Foursquare Cipher with Keyword-Driven Extended Grid and Triple-Layer Transposition with Coordinate Pair Encoding
 *
 * 功能: 实现四方密码(Foursquare Cipher)，使用关键词驱动的扩展网格
 *       (keyword-driven extended grid)构建4个Polybius方阵，三层转置
 *       (triple-layer transposition)对坐标对进行行列置换编码。
 *
 * 协作: PlayfairCipher3(Playfair密码) / BifidCipher4(Bifid密码) / ADFGVX5(ADFGVX密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief 四方密码(关键词扩展网格+三层转置)
 */
class FoursquareCode9 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int inputLength = 0;
        int outputLength = 0;
        int numTranspositions = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FoursquareCode9(QObject *parent = nullptr);
    ~FoursquareCode9() override;

    /** @brief Set keywords for TL and BR squares */
    void setKeywords(const QString& kw1, const QString& kw2);

    /** @brief Encrypt plaintext */
    QString encrypt(const QString& plaintext);

    /** @brief Decrypt ciphertext */
    QString decrypt(const QString& ciphertext);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void cipherCompleted(bool encrypt, int length, double timeMs);

private:
    static const int GRID = 5;
    QString m_keyword1;
    QString m_keyword2;

    // Four 5x5 grids: TL(plain), TR(kw1), BL(kw2), BR(plain)
    QVector<QVector<int>> m_gridTL;
    QVector<QVector<int>> m_gridTR;
    QVector<QVector<int>> m_gridBL;
    QVector<QVector<int>> m_gridBR;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build a 5x5 Polybius grid from keyword */
    void buildGrid(const QString& keyword, QVector<QVector<int>>& grid);

    /** @brief Find (row, col) of char in grid */
    void findPosition(const QVector<QVector<int>>& grid, int ch,
                       int& row, int& col) const;

    /** @brief Triple-layer transposition on coordinate pairs */
    QVector<int> tripleTranspose(const QVector<int>& coords, bool encrypt) const;

    /** @brief Preprocess text: upper, replace J->I, pad */
    QString preprocess(const QString& text) const;

    /** @brief Map char to 0-24 (A-Z minus J) */
    static int charToIndex(QChar c);

    /** @brief Map index 0-24 back to char */
    static QChar indexToChar(int idx);
};
