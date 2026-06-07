/**
 * @file TrifidCode.h
 * @brief 三分密码(3D立方体分数编码+层转位) — Trifid Cipher with 3D Fractional Encoding using 3x3x3 Cube and Layer Transposition
 *
 * 功能: 实现三分密码算法，支持3x3x3立体方阵映射、层内分数编码、
 *       层转位混淆和可配置周期长度。
 *
 * 协作: BifidCipher3(双分密码) / PlayfairCipher4(Playfair密码) / HillCipher3(Hill密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <QMap>

/**
 * @brief 三分密码器(3D立方体分数编码+层转位)
 */
class TrifidCode : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalOperations = 0;
        int period = 0;
        int inputLength = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit TrifidCode(QObject *parent = nullptr);
    ~TrifidCode() override;

    void setKey(const QString& key);
    void setPeriod(int period);

    /** @brief 加密明文 */
    QString encrypt(const QString& plaintext);

    /** @brief 解密密文 */
    QString decrypt(const QString& ciphertext);

    /** @brief 获取3x3x3立方体映射(字符→层/行/列) */
    QMap<QChar, QVector<int>> cubeMapping() const { return m_encodeMap; }

    /** @brief 获取反向映射(层/行/列→字符) */
    QMap<QString, QChar> reverseMapping() const { return m_decodeMap; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(const QString& op, int length, double timeMs);

private:
    int m_period = 5;
    QString m_alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ.";

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Forward map: char -> {layer, row, col} */
    QMap<QChar, QVector<int>> m_encodeMap;
    /** @brief Reverse map: "layer,row,col" -> char */
    QMap<QString, QChar> m_decodeMap;

    /** @brief Build 3x3x3 cube from key */
    void buildCube(const QString& key);

    /** @brief Encode a single char to (layer, row, col) */
    QVector<int> encodeChar(QChar c) const;

    /** @brief Decode (layer, row, col) back to char */
    QChar decodeCoord(int layer, int row, int col) const;

    /** @brief Perform layer transposition on coordinate triples */
    QVector<int> transpose(const QVector<int>& coords, int period) const;

    /** @brief Inverse transposition for decryption */
    QVector<int> inverseTranspose(const QVector<int>& coords, int period) const;

    /** @brief Preprocess text: uppercase, filter valid chars */
    QString preprocess(const QString& text) const;
};
