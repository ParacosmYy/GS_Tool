/**
 * @file SeriatedPlayfair.h
 * @brief 序列化Playfair密码(渐进关键字依赖网格+扩展二合字母频率分析) — Seriated Playfair Cipher with Progressive Keyword-Dependent Grid and Extended Digraph Frequency Analysis
 *
 * 功能: 实现序列化Playfair密码，支持渐进式关键字网格生成、
 *       扩展二合字母频率分析和多表替代加密。
 *
 * 协作: Vigenere8(维吉尼亚密码) / Enigma7(恩尼格玛) / AffineCipher5(仿射密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <QPair>

/**
 * @brief 序列化Playfair密码(渐进关键字依赖网格+扩展二合字母频率分析)
 */
class SeriatedPlayfair : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int keyLength = 0;
        int textSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SeriatedPlayfair(QObject *parent = nullptr);
    ~SeriatedPlayfair() override;

    void setKeyword(const QString& keyword);
    void setPeriod(int period);

    /** @brief Encrypt plaintext using seriated Playfair */
    QString encrypt(const QString& plaintext) const;

    /** @brief Decrypt ciphertext using seriated Playfair */
    QString decrypt(const QString& ciphertext) const;

    /** @brief Build 5x5 grid from keyword */
    QVector<QVector<QChar>> buildGrid(const QString& keyword) const;

    /** @brief Find character position in grid */
    QPair<int, int> findPosition(const QVector<QVector<QChar>>& grid, QChar ch) const;

    /** @brief Compute extended digraph frequency table */
    QVector<QVector<double>> digraphFrequency(const QString& text) const;

    /** @brief Prepare text: remove non-alpha, replace J with I, pad */
    QString prepareText(const QString& text) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(const QString& op, int length, double timeMs);

private:
    QString m_keyword;
    int m_period = 5;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Apply Playfair pair transformation (encrypt or decrypt) */
    QPair<QChar, QChar> transformPair(const QVector<QVector<QChar>>& grid,
                                       QChar a, QChar b, bool encrypt) const;

    /** @brief Generate seriated grids from keyword */
    QVector<QVector<QVector<QChar>>> generateGrids() const;
};
