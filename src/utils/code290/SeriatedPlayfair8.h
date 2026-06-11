/**
 * @file SeriatedPlayfair8.h
 * @brief 序列化Playfair密码(渐进关键字旋转与双字母频率平衡增强波利比乌斯加密) — Seriated Playfair with Progressive Keyword Rotation and Digraph Frequency Balancing for Enhanced Polybius Encryption
 *
 * 功能: 实现序列化Playfair密码(Seriated Playfair cipher)，采用渐进关键字旋转(progressive keyword rotation)
 *       与双字母频率平衡(digraph frequency balancing)增强波利比乌斯加密(Polybius encryption)。
 *
 * 协作: AesBlock10(AES) / ChaCha20(流密码) / EnigmaMachine10(恩尼格玛)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

class SeriatedPlayfair8 : public QObject {
    Q_OBJECT

public:
    /** @brief Encryption result with metadata */
    struct EncResult {
        QString cipherText;
        int gridRotations = 0;
        double digraphBalance = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int charsEncrypted = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit SeriatedPlayfair8(QObject *parent = nullptr);
    ~SeriatedPlayfair8() override;

    void setKeyword(const QString& keyword);
    void setRotationPeriod(int period);
    void setFillOrder(int order);

    /** @brief Encrypt plaintext */
    EncResult encrypt(const QString& plaintext);

    /** @brief Decrypt ciphertext */
    EncResult decrypt(const QString& ciphertext);

    /** @brief Analyze digraph frequency distribution */
    QVector<QPair<QString,double>> analyzeDigraphFrequency(const QString& text) const;

    /** @brief Get current 5×5 grid state */
    QVector<QVector<QChar>> getGrid() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encryptDone(int len, double balance, double timeMs);

private:
    QString m_keyword;
    int m_rotationPeriod = 10;       // Rotate grid every N digraphs
    int m_fillOrder = 0;             // 0=row, 1=col, 2=spiral
    Stats m_stats;
    double m_timeSum = 0.0;

    QVector<QVector<QChar>> m_grid;  // 5×5 Polybius grid
    QVector<int> m_positionMap;       // QChar -> grid position

    /** @brief Build initial grid from keyword */
    void buildGrid(const QString& keyword);

    /** @brief Rotate grid by shifting rows/columns */
    void rotateGrid(int step);

    /** @brief Find position of character in grid */
    QPair<int,int> findPosition(QChar ch) const;

    /** @brief Apply Playfair digraph rule at given grid state */
    QPair<QChar,QChar> encodeDigraph(QChar a, QChar b) const;

    /** @brief Apply reverse Playfair digraph rule */
    QPair<QChar,QChar> decodeDigraph(QChar a, QChar b) const;

    /** @brief Prepare text: pad repeated letters, make even length */
    QString prepareText(const QString& text) const;

    /** @brief Normalize character to A-Z (J→I) */
    QChar normalize(QChar ch) const;
};
