/**
 * @file BspTree2d.h
 * @brief 二维BSP树 — 空间划分与范围查询
 *
 * 功能: 基于交替轴分割的二维空间划分树，支持点插入、
 *       矩形范围查询，统计插入/查询次数/耗时。
 */
#ifndef BSPTREE2D_H
#define BSPTREE2D_H

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @class BspTree2d
 * @brief 二维BSP空间划分树，支持矩形范围查询
 *
 * 使用交替X/Y轴中值分割策略，将二维点集组织为二叉树，
 * 支持高效的范围查询操作。
 */
class BspTree2d : public QObject {
    Q_OBJECT
public:
    /** 操作统计 */
    struct Stats {
        quint64 totalInserts = 0;       ///< 总插入次数
        quint64 totalQueries = 0;       ///< 总查询次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /**
     * @brief 构造函数
     * @param maxLeafSize 叶节点最大容量，超过则分裂
     * @param parent QObject父对象
     */
    explicit BspTree2d(int maxLeafSize = 16, QObject* parent = nullptr);

    ~BspTree2d();

    /**
     * @brief 插入二维数据点
     * @param x X坐标
     * @param y Y坐标
     * @param data 关联数据
     * @return 插入是否成功
     */
    bool insert(double x, double y, int data);

    /**
     * @brief 矩形范围查询
     * @param xMin 矩形左边界
     * @param yMin 矩形下边界
     * @param xMax 矩形右边界
     * @param yMax 矩形上边界
     * @return 范围内的所有点及其关联数据
     */
    QVector<QPair<QPair<double,double>,int>> rangeQuery(
        double xMin, double yMin, double xMax, double yMax);

    /** @brief 获取统计信息 */
    const Stats& stats() { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

    /** @brief 清空树中所有数据 */
    void clear();

    /** @brief 获取树中点数量 */
    int size() { return m_size; }

signals:
    /** @brief 查询完成信号 @param resultCount 结果数量 */
    void queryCompleted(int resultCount);

private:
    /** 树节点 */
    struct Node {
        double splitPos = 0.0;     ///< 分割位置
        int    splitAxis = 0;      ///< 分割轴(0=X, 1=Y)
        bool   isLeaf = true;      ///< 是否为叶节点
        Node*  left = nullptr;     ///< 左子树
        Node*  right = nullptr;    ///< 右子树
        QVector<QPair<QPair<double,double>,int>> points; ///< 叶节点存储的点
    };

    void deleteNode(Node* node);
    void splitNode(Node* node, int depth);
    void insertImpl(Node* node, double x, double y, int data, int depth);
    void rangeQueryImpl(Node* node, double xMin, double yMin,
                        double xMax, double yMax,
                        QVector<QPair<QPair<double,double>,int>>& results);

    Node*  m_root;           ///< 树根节点
    int    m_maxLeafSize;    ///< 叶节点最大容量
    int    m_size;           ///< 点总数
    Stats  m_stats;          ///< 统计信息
    double m_timeSum;        ///< 累计耗时
};

#endif // BSPTREE2D_H
