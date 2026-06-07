/**
 * @file FoursquareCode2.h
 * @brief 四方密码变体(关键词依赖网格布局+互易二合字母分析) — Four-Square Cipher Variant with Keyword-Dependent Grid Layout and Reciprocal Digraph Analysis
 *
 * 功能: 实现四方密码变体，支持关键词依赖的网格布局、
 *       互易二合字母分析和加解密操作。
 *
 * 协作: PolybiusCipher3(波利比奥斯方阵) / VigenereCipher4(维吉尼亚) / HillCipher2(希尔密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <QString>

/**
 * @brief 四方密码变体(关键词依赖网格布局+互易二合字母分析)
 */
class FoursquareCode2 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int textLength = 0;
        int digraphCount = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FoursquareCode2(QObject *parent = nullptr);
    ~FoursquareCode2() override;

    void setKeyword1(const QString& kw);
    void setKeyword2(const QString& kw);

    /** @brief Encrypt plaintext using four-square cipher */
    QString encrypt(const QString& plaintext) const;

    /** @brief Decrypt ciphertext using four-square cipher */
    QString decrypt(const QString& ciphertext) const;

    /** @brief Build a 5x5 key square from keyword */
    QVector<QVector<int>> buildSquare(const QString& keyword) const;

    /** @brief Find row,col position of letter in square */
    QPair<int, int> findPosition(const QVector<QVector<int>>& square, int letter) const;

    /** @brief Analyze reciprocal digraphs in text */
    QVector<QPair<QChar, QChar>> analyzeDigraphs(const QString& text) const;

    /** @brief Prepare text: uppercase, remove non-alpha, pad for even length */
    QString prepareText(const QString& text) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(const QString& op, int length, double timeMs);

private:
    QString m_keyword1;
    QString m_keyword2;
    QVector<QVector<int>> m_squareTL;  // top-left (standard)
    QVector<QVector<int>> m_squareTR;  // top-right (keyword1)
    QVector<QVector<int>> m_squareBL;  // bottom-left (keyword2)
    QVector<QVector<int>> m_squareBR;  // bottom-right (standard)

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build standard 5x5 square (A-Z, skip J) */
    QVector<QVector<int>> buildStandardSquare() const;

    /** @brief Encode digraph pair (r1,c1) and (r2,c2) */
    QPair<int, int> encodePair(const QPair<int,int>& p1,
                                const QPair<int,int>& p2,
                                bool encrypt) const;

    /** @brief Map letter index to QChar (J->I) */
    static QChar idxToChar(int idx);
    static int charToIdx(QChar c);
};
