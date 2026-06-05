/**
 * @file PersistentSegmentTree.h
 * @brief 可持久化(函数式)线段树,支持版本历史回溯
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 可持久化线段树
 *
 * 每次修改操作产生新版本,保留所有历史版本。
 * 支持区间求和、区间最值、版本间查询和回滚。
 * 节点采用指针池分配,避免频繁new/delete。
 */
class PersistentSegmentTree : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalQueries = 0;           ///< 总查询次数
        int totalUpdates = 0;           ///< 总更新次数
        double avgProcessingTimeMs = 0.0;///< 平均处理时间(ms)
    };

    explicit PersistentSegmentTree(QObject* parent = nullptr);

    /**
     * @brief 从初始数组构建可持久化线段树
     * @param values 初始值数组
     * @return 初始版本号(0)
     */
    int build(const QVector<double>& values);

    /**
     * @brief 单点更新,产生新版本
     * @param version 基于的版本号
     * @param index 更新位置
     * @param value 新值
     * @return 新版本号
     */
    int update(int version, int index, double value);

    /**
     * @brief 区间求和查询
     * @param version 版本号
     * @param left 区间左端(含)
     * @param right 区间右端(含)
     * @return 区间和
     */
    double rangeSum(int version, int left, int right);

    /**
     * @brief 区间最小值查询
     * @param version 版本号
     * @param left 区间左端(含)
     * @param right 区间右端(含)
     * @return 最小值
     */
    double rangeMin(int version, int left, int right);

    /**
     * @brief 区间最大值查询
     * @param version 版本号
     * @param left 区间左端(含)
     * @param right 区间右端(含)
     * @return 最大值
     */
    double rangeMax(int version, int left, int right);

    /**
     * @brief 获取当前版本数
     * @return 版本数量
     */
    int versionCount() const;

    /**
     * @brief 获取指定版本的单点值
     * @param version 版本号
     * @param index 位置索引
     * @return 该位置的值
     */
    double pointQuery(int version, int index);

    Stats stats() const;
    void resetStatistics();

signals:
    /** @brief 新版本创建信号 */
    void versionCreated(int version);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief 线段树节点 */
    struct Node {
        double sum = 0.0;   ///< 区间和
        double minV = 0.0;  ///< 区间最小值
        double maxV = 0.0;  ///< 区间最大值
        int left = -1;      ///< 左子节点索引(-1表示null)
        int right = -1;     ///< 右子节点索引
    };

    QVector<Node> m_pool;               ///< 节点池
    QVector<int> m_roots;               ///< 各版本根节点索引
    int m_size = 0;                      ///< 叶节点数量

    int buildRec(int lo, int hi, const QVector<double>& values);
    int updateRec(int nodeIdx, int lo, int hi, int pos, double val);
    double sumRec(int nodeIdx, int lo, int hi, int ql, int qr) const;
    double minRec(int nodeIdx, int lo, int hi, int ql, int qr) const;
    double maxRec(int nodeIdx, int lo, int hi, int ql, int qr) const;
    int allocNode();
};
