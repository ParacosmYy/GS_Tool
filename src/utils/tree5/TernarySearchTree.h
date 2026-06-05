/**
 * @file TernarySearchTree.h
 * @brief 三叉搜索树 — 字符串存储与自动补全
 *
 * 功能: 实现三叉搜索树(Ternary Search Tree)，支持字符串插入、
 *       精确查找、前缀搜索(自动补全)和删除。每个节点存储一个
 *       字符和三个子指针(lo/eq/hi)，结合BST和Trie的优点，
 *       空间效率高且查询速度快。适用于命令补全、协议关键字
 *       匹配、AT命令自动补全等场景。
 *
 * 协作: SmartAutoComplete(智能补全) / ProtocolEngine(协议关键字)
 */
#pragma once

#include <QObject>
#include <QString>
#include <QVector>
#include <QChar>
#include <QElapsedTimer>

/**
 * @brief 三叉搜索树节点
 */
struct TSTNode {
    QChar    ch;             ///< 当前字符
    TSTNode* lo;             ///< 小于当前字符的分支
    TSTNode* eq;             ///< 等于当前字符的分支(下一字符)
    TSTNode* hi;             ///< 大于当前字符的分支
    bool     isEnd;          ///< 是否为某个单词的结束节点

    explicit TSTNode(QChar c)
        : ch(c), lo(nullptr), eq(nullptr), hi(nullptr), isEnd(false) {}
};

/**
 * @brief 三叉搜索树 — 字符串存储与自动补全
 */
class TernarySearchTree : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalInserts = 0;           ///< 累计插入次数
        quint64 totalSearches = 0;          ///< 累计搜索次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit TernarySearchTree(QObject* parent = nullptr);

    /** @brief 析构函数 — 递归释放所有节点 */
    ~TernarySearchTree();

    /**
     * @brief 插入一个字符串
     * @param word 待插入的字符串
     */
    void insert(const QString& word);

    /**
     * @brief 检查字符串是否存在(精确匹配)
     * @param word 待查找的字符串
     * @return 是否存在
     */
    bool contains(const QString& word) const;

    /**
     * @brief 前缀搜索(自动补全)
     * @param prefix 前缀字符串
     * @return 所有匹配前缀的字符串列表(按字典序)
     */
    QVector<QString> search(const QString& prefix) const;

    /**
     * @brief 删除一个字符串
     * @param word 待删除的字符串
     * @return 是否成功删除(字符串存在)
     */
    bool remove(const QString& word);

    /**
     * @brief 获取树中存储的字符串总数
     * @return 字符串数量
     */
    int size() const { return m_count; }

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 插入完成信号 @param word 插入的字符串 */
    void wordInserted(const QString& word);

    /** @brief 搜索完成信号 @param prefix 前缀 @param results 结果数 */
    void searchCompleted(const QString& prefix, int results);

private:
    /**
     * @brief 递归插入辅助函数
     * @param node 当前节点
     * @param word 字符串
     * @param idx 当前字符索引
     * @return 更新后的节点
     */
    TSTNode* insertHelper(TSTNode* node, const QString& word, int idx);

    /**
     * @brief 递归查找辅助函数
     * @param node 当前节点
     * @param word 字符串
     * @param idx 当前字符索引
     * @return 匹配的结束节点；未找到返回nullptr
     */
    TSTNode* searchHelper(TSTNode* node, const QString& word, int idx) const;

    /**
     * @brief 收集以node为根的所有后缀
     * @param node 起始节点
     * @param prefix 已匹配前缀
     * @param results 输出结果列表
     */
    void collectAll(TSTNode* node, const QString& prefix,
                    QVector<QString>& results) const;

    /**
     * @brief 递归删除节点树
     * @param node 当前节点
     */
    void deleteTree(TSTNode* node);

    TSTNode* m_root;               ///< 树根节点
    int      m_count;              ///< 存储的字符串总数

    mutable QElapsedTimer m_timer; ///< 计时器
    mutable double  m_timeSum;     ///< 累计耗时
    mutable quint64 m_totalOps;    ///< 总操作数
    mutable Stats   m_stats;       ///< 统计信息
};
