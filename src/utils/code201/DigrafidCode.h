/**
 * @file DigrafidCode.h
 * @brief 双字母分数密码(双图替换+分数坐标转置) — Digrafid Cipher Combining Digraph Substitution with Fractional Coordinate Transposition
 *
 * 功能: 实现Digrafid密码，支持双图替换表、分数坐标分解、
 *       行列转置加密/解密和密钥矩阵自定义。
 *
 * 协作: PlayfairCipher4(Playfair密码) / ADFGVX3(ADFGVX密码) / BifidCode5(Bifid密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <QString>

/**
 * @brief 双字母分数密码(双图替换+分数坐标转置)
 */
class DigrafidCode : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOperations = 0;
        int inputLength = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DigrafidCode(QObject *parent = nullptr);
    ~DigrafidCode() override;

    /** @brief Set the 3x3 key for fractional coordinates */
    void setKey(const QString& key);

    /** @brief Set period for transposition (0 = use full text length) */
    void setPeriod(int period);

    /** @brief Encrypt plaintext */
    QString encrypt(const QString& plaintext);

    /** @brief Decrypt ciphertext */
    QString decrypt(const QString& ciphertext);

    /** @brief Build digraph substitution table from key */
    void buildSubstitutionTable(const QString& key);

    /** @brief Fractional coordinate decomposition for a digraph */
    QPair<QVector<int>, QVector<int>> fractionate(QChar a, QChar b) const;

    /** @brief Transpose fractional coordinates by period */
    QVector<int> transpose(const QVector<int>& coords, int period) const;

    /** @brief Reverse transpose fractional coordinates */
    QVector<int> reverseTranspose(const QVector<int>& coords, int period) const;

    /** @brief Defractionate coordinates back to digraphs */
    QVector<QPair<QChar, QChar>> defractionate(const QVector<int>& coords) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(const QString& op, int length, double timeMs);

private:
    int m_period = 0;
    QString m_key;

    // Substitution tables
    QVector<QVector<int>> m_rowTable;     // row coordinate per char pair
    QVector<QVector<int>> m_colTable;     // col coordinate per char pair
    QString m_charSet;                     // 27-char alphabet (A-Z + #)

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Map character to index in charset */
    int charToIdx(QChar c) const;

    /** @brief Map index back to character */
    QChar idxToChar(int idx) const;
};
