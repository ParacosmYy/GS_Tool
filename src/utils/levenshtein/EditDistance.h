/**
 * @file EditDistance.h
 * @brief 编辑距离(Levenshtein Distance)计算器
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <QPair>

/**
 * @class EditDistance
 * @brief 编辑距离计算 — 支持Levenshtein/Damerau-Levenshtein/加权编辑距离
 *
 * 用于拼写检查、模糊匹配、DNA序列比对、版本对比等场景。
 * 支持自定义操作代价和回溯编辑路径。
 */
class EditDistance : public QObject
{
    Q_OBJECT

public:
    /** @brief 编辑操作类型 */
    enum Operation {
        None,    /**< 无操作(匹配) */
        Insert,  /**< 插入 */
        Delete,  /**< 删除 */
        Replace, /**< 替换 */
        Transpose /**< 交换(Damerau) */
    };

    /** @brief 编辑步骤 */
    struct EditStep {
        Operation op;    /**< 操作类型 */
        int pos1;        /**< 串1中的位置 */
        int pos2;        /**< 串2中的位置 */
        QChar ch1;       /**< 串1中的字符 */
        QChar ch2;       /**< 串2中的字符 */
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalComputations = 0;  /**< 总计算次数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 构造函数 */
    explicit EditDistance(QObject* parent = nullptr);

    /**
     * @brief 计算Levenshtein距离
     * @param s1 字符串1
     * @param s2 字符串2
     * @return 最小编辑距离
     */
    int levenshtein(const QString& s1, const QString& s2) const;

    /**
     * @brief 计算Damerau-Levenshtein距离(支持相邻交换)
     * @param s1 字符串1
     * @param s2 字符串2
     * @return 最小编辑距离
     */
    int damerauLevenshtein(const QString& s1, const QString& s2) const;

    /**
     * @brief 计算编辑距离并返回编辑路径
     * @param s1 字符串1
     * @param s2 字符串2
     * @return (距离, 编辑步骤列表)
     */
    QPair<int, QVector<EditStep>> editPath(const QString& s1, const QString& s2) const;

    /**
     * @brief 归一化编辑距离(0~1)
     * @param s1 字符串1
     * @param s2 字符串2
     * @return 归一化距离
     */
    double normalizedDistance(const QString& s1, const QString& s2) const;

    /**
     * @brief 编辑相似度(1 - 归一化距离)
     * @param s1 字符串1
     * @param s2 字符串2
     * @return 相似度(0~1)
     */
    double similarity(const QString& s1, const QString& s2) const;

    /**
     * @brief 在候选列表中找最相似的字符串
     * @param target 目标字符串
     * @param candidates 候选列表
     * @param maxDistance 最大距离阈值
     * @return (最佳匹配, 距离)
     */
    QPair<QString, int> bestMatch(const QString& target,
                                   const QVector<QString>& candidates,
                                   int maxDistance = -1) const;

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 计算完成信号 */
    void computationCompleted(int distance);

private:
    mutable Stats m_stats;
    mutable double m_timeSum;
};
