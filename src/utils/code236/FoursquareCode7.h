/**
 * @file FoursquareCode7.h
 * @brief 四方密码(扩展14x14栅格+三层列转置+Polybius坐标编码) — Foursquare Cipher with Extended 14x14 Grid and Triple-Layer Columnar Transposition with Polybius Coordinate Encoding
 *
 * 功能: 实现四方密码(Foursquare cipher)扩展变体，采用14x14扩展栅格(extended 14x14 grid)
 *       和三层列转置(triple-layer columnar transposition)，结合Polybius坐标编码
 *       (Polybius coordinate encoding)实现多层级加密。
 *
 * 协作: HillCipher5(Hill密码) / PlayfairCode4(Playfair密码) / AffineCipher3(仿射密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QChar>

/**
 * @brief 四方密码(扩展14x14栅格+三层列转置+Polybius坐标编码)
 */
class FoursquareCode7 : public QObject {
    Q_OBJECT

public:
    /** @brief Cipher operation mode */
    enum Mode { Encrypt = 0, Decrypt = 1 };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int inputLength = 0;
        int outputLength = 0;
        int numTranspositionLayers = 3;
        int gridSize = 14;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FoursquareCode7(QObject *parent = nullptr);
    ~FoursquareCode7() override;

    /** @brief Set primary keyword for grid TL */
    void setKeyword1(const QString& kw);

    /** @brief Set secondary keyword for grid BR */
    void setKeyword2(const QString& kw);

    /** @brief Set transposition keys (3 layers) */
    void setTranspositionKeys(const QVector<QString>& keys);

    /** @brief Encrypt plaintext */
    QString encrypt(const QString& plaintext);

    /** @brief Decrypt ciphertext */
    QString decrypt(const QString& ciphertext);

    /** @brief Get current grid state (for debug) */
    QVector<QVector<QChar>> grid(int index) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encryptCompleted(int inLen, int outLen, double timeMs);
    void decryptCompleted(int inLen, int outLen, double timeMs);

private:
    QString m_keyword1;
    QString m_keyword2;
    QVector<QString> m_transKeys;

    QVector<QVector<QChar>> m_gridTL;  // top-left
    QVector<QVector<QChar>> m_gridTR;  // top-right
    QVector<QVector<QChar>> m_gridBL;  // bottom-left
    QVector<QVector<QChar>> m_gridBR;  // bottom-right

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build 14x14 Polybius grid from keyword */
    void buildGrid(const QString& keyword, QVector<QVector<QChar>>& grid);

    /** @brief Find character position in grid */
    bool findInGrid(const QVector<QVector<QChar>>& grid, QChar ch, int& row, int& col) const;

    /** @brief Columnar transposition (single layer) */
    QString columnarTranspose(const QString& text, const QString& key, bool encrypt) const;

    /** @brief Apply triple-layer columnar transposition */
    QString tripleTranspose(const QString& text, bool encrypt) const;

    /** @brief Pad text to even length */
    QString padText(const QString& text) const;

    /** @brief Convert text to grid coordinates */
    QVector<QPair<int, int>> toCoordinates(const QString& text) const;

    /** @brief Get alphabet character for grid position */
    QChar gridChar(int row, int col) const;

    /** @brief Get transposition column order from key */
    QVector<int> columnOrder(const QString& key) const;
};
