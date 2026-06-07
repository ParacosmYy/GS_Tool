/**
 * @file TwoSquareCode2.h
 * @brief 双方阵密码(纵横配对网格+交叉坐标映射) — Two-Square Cipher with Vertical/Horizontal Paired Grids and Cross-Coordinate Digraph Mapping
 *
 * 功能: 实现双方阵密码算法，支持垂直/水平配对网格、
 *       交叉坐标双字母映射和密钥矩阵生成。
 *
 * 协作: HillCipher2(Hill密码) / Playfair3(Playfair密码) / Vigenere4(Vigenere密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <QPair>

/**
 * @brief 双方阵密码(纵横配对网格+交叉坐标映射)
 */
class TwoSquareCode2 : public QObject {
    Q_OBJECT

public:
    /** @brief Grid layout mode */
    enum class Layout { Vertical, Horizontal };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOperations = 0;
        int inputLength = 0;
        int outputLength = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit TwoSquareCode2(QObject *parent = nullptr);
    ~TwoSquareCode2() override;

    void setKey1(const QString& key);
    void setKey2(const QString& key);
    void setLayout(Layout layout);
    void setAlphabet(const QString& alpha);

    /** @brief Build 5x5 key square from keyword */
    QVector<QVector<QChar>> buildKeySquare(const QString& key) const;

    /** @brief Encrypt plaintext using two-square digraph mapping */
    QString encrypt(const QString& plaintext) const;

    /** @brief Decrypt ciphertext using two-square digraph mapping */
    QString decrypt(const QString& ciphertext) const;

    /** @brief Find character position in key square */
    QPair<int, int> findPosition(const QVector<QVector<QChar>>& square, QChar ch) const;

    /** @brief Map digraph through cross-coordinate rule */
    QPair<QChar, QChar> mapDigraph(QChar a, QChar b, bool enc) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(const QString& op, int inLen, int outLen, double timeMs);

private:
    QString m_key1;
    QString m_key2;
    Layout m_layout = Layout::Vertical;
    QString m_alphabet;

    QVector<QVector<QChar>> m_square1;
    QVector<QVector<QChar>> m_square2;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Preprocess text: upper-case, replace J->I, pad odd */
    QString preprocess(const QString& text) const;

    /** @brief Build look-up table for alphabet position */
    int charIndex(QChar ch) const;
};
