/**
 * @file FoursquareCode3.h
 * @brief 四方密码(扩展Polybius方阵+关键字驱动坐标映射) — Foursquare Cipher with Extended Polybius Square and Keyword-Driven Coordinate Mapping
 *
 * 功能: 实现四方密码编解码，支持扩展Polybius方阵(含字母+数字)、
 *       关键字驱动的坐标映射和自定义方阵填充。
 *
 * 协作: VigenereCipher5(维吉尼亚) / PlayfairCipher2(Playfair) / Aes256Crypto7(AES)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <QPair>

/**
 * @brief 四方密码(扩展Polybius方阵+关键字驱动坐标映射)
 */
class FoursquareCode3 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int inputLength = 0;
        int outputLength = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FoursquareCode3(QObject *parent = nullptr);
    ~FoursquareCode3() override;

    void setKeyword1(const QString& kw);
    void setKeyword2(const QString& kw);

    /** @brief Encrypt plaintext using foursquare cipher */
    QString encrypt(const QString& plaintext) const;

    /** @brief Decrypt ciphertext using foursquare cipher */
    QString decrypt(const QString& ciphertext) const;

    /** @brief Build a Polybius square from a keyword */
    QVector<QVector<QChar>> buildSquare(const QString& keyword) const;

    /** @brief Map a character to (row, col) in a square */
    QPair<int, int> findPosition(const QVector<QVector<QChar>>& square, QChar ch) const;

    /** @brief Get character at (row, col) in a square */
    QChar charAt(const QVector<QVector<QChar>>& square, int row, int col) const;

    /** @brief Prepare text: uppercase, filter valid chars, pad if needed */
    QString prepareText(const QString& text) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void cipherCompleted(const QString& mode, int length, double timeMs);

private:
    QString m_keyword1;
    QString m_keyword2;

    // Extended alphabet: A-Z + 0-9 (6x6 square), omit Q
    static constexpr int SQUARE_SIZE = 6;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build the default alphabet sequence (skip 'Q') */
    static QString defaultAlphabet();

    /** @brief Build square from keyword merged with default alphabet */
    QVector<QVector<QChar>> buildKeySquare(const QString& keyword) const;

    /** @brief Process digraph pair with four squares */
    QString processPair(QChar a, QChar b,
                        const QVector<QVector<QChar>>& sq1,
                        const QVector<QVector<QChar>>& sq2,
                        bool encrypt) const;
};
