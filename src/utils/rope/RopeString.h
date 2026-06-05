/**
 * @file RopeString.h
 * @brief Rope数据结构 — 高效字符串操作
 *
 * 功能: 实现Rope数据结构，一种基于二叉树的字符串表示方法。
 *       支持O(log n)的插入/删除/子串操作，适用于频繁编辑的大文本。
 *       统计插入/删除次数/平均耗时。
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

/**
 * @class RopeString
 * @brief Rope数据结构 — 基于平衡二叉树的字符串
 *
 * 将字符串分割为叶子节点，通过内部节点组织为二叉树。
 * 插入/删除操作通过分裂和合并节点实现，保持近似平衡。
 * 叶子节点最大长度为LEAF_SIZE(默认64字符)。
 */
class RopeString : public QObject
{
    Q_OBJECT

public:
    /** @brief 运行统计信息 */
    struct Stats {
        quint64 totalInserts = 0;    ///< 总插入次数
        quint64 totalRemoves = 0;    ///< 总删除次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /**
     * @brief 构造函数
     * @param parent QObject父对象
     */
    explicit RopeString(QObject* parent = nullptr);

    /**
     * @brief 带初始字符串的构造函数
     * @param text 初始字符串
     * @param parent QObject父对象
     */
    explicit RopeString(const QString& text, QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~RopeString();

    /**
     * @brief 在指定位置插入字符串
     * @param pos 插入位置(0-based)
     * @param str 待插入字符串
     */
    void insert(int pos, const QString& str);

    /**
     * @brief 删除指定范围的字符
     * @param pos 起始位置(0-based)
     * @param len 删除长度
     */
    void remove(int pos, int len);

    /**
     * @brief 获取子串
     * @param pos 起始位置(0-based)
     * @param len 子串长度
     * @return 子字符串
     */
    QString substring(int pos, int len) const;

    /**
     * @brief 获取总字符串长度
     * @return 字符数
     */
    int length() const;

    /**
     * @brief 将Rope转换为平坦字符串
     * @return 完整字符串
     */
    QString toString() const;

    /** @brief 获取统计信息 */
    Stats stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 结构变化信号(插入/删除后发射) */
    void structureChanged();

private:
    /** @brief Rope节点(内部节点或叶子节点) */
    struct Node {
        int     weight = 0;   ///< 左子树的总字符数(内部节点)或叶子字符串长度
        QString data;          ///< 叶子节点数据(非空时为叶子)
        Node*   left = nullptr;
        Node*   right = nullptr;

        /** @brief 判断是否为叶子节点 */
        bool isLeaf() const { return left == nullptr && right == nullptr; }
    };

    /** @brief 叶子节点最大长度 */
    static constexpr int kLeafSize = 64;

    /** @brief 从字符串构建Rope */
    Node* buildRope(const QString& text, int start, int end) const;

    /** @brief 在节点中指定位置插入字符串 */
    Node* insertNode(Node* node, int pos, const QString& str);

    /** @brief 从节点中删除指定范围 */
    Node* removeRange(Node* node, int pos, int len);

    /** @brief 收集子串 */
    void substringHelper(Node* node, int pos, int len,
                         int& skipped, QString& result) const;

    /** @brief 收集全部字符串 */
    void toStringHelper(Node* node, QString& result) const;

    /** @brief 计算节点总字符数 */
    int computeTotalLength(Node* node) const;

    /** @brief 计算节点权重(左子树字符总数) */
    void updateWeight(Node* node);

    /** @brief 分裂过长叶子节点 */
    Node* splitLeaf(Node* node);

    /** @brief 递归释放节点 */
    void destroyNode(Node* node);

    /** @brief 递归复制节点 */
    Node* cloneNode(Node* node) const;

    Node*  m_root = nullptr;       ///< 根节点
    mutable Stats m_stats;         ///< 统计信息
    mutable double m_timeSum = 0.0; ///< 累计耗时
    int m_totalOps = 0;            ///< 总操作次数
};
