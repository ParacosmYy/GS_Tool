/**
 * @file PersistentTree2.h
 * @brief 持久化平衡BST — 路径拷贝 + 版本化查询 + 版本GC
 *
 * 功能: 实现持久化(Persistent)平衡二叉搜索树，每次修改操作
 *       保留历史版本，通过路径拷贝实现O(log n)空间开销。
 *       支持版本化查询(在任意历史版本上查找)、版本GC回收、
 *       版本diff比较。适用于撤销/重做系统、时间旅行调试、
 *       数据库MVCC、版本化配置管理。
 *
 * 协作: StateTracker(状态跟踪) / SettingsManager(配置管理)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QMap>
#include <QSet>

/**
 * @brief 持久化平衡BST — 路径拷贝实现
 *
 * 核心思想: 每次插入/删除操作创建从根到修改节点的路径副本，
 * 未修改的子树直接共享引用。空间开销O(log n)每次操作。
 */
class PersistentTree2 : public QObject {
    Q_OBJECT

public:
    /** @brief 键类型 */
    using Key = double;
    /** @brief 值类型 */
    using Value = double;

    /** @brief 版本号类型 */
    using Version = int;

    /** @brief 版本信息 */
    struct VersionInfo {
        Version version = -1;       ///< 版本号
        int rootNode = -1;          ///< 根节点索引
        int nodeCount = 0;          ///< 该版本节点总数
        int treeSize = 0;           ///< 逻辑大小
        QString description;        ///< 版本描述
    };

    /** @brief 查询结果 */
    struct LookupResult {
        bool found = false;         ///< 是否找到
        Key key = 0.0;             ///< 查询键
        Value value = 0.0;        ///< 找到的值
        int depth = 0;            ///< 查找深度
    };

    /** @brief 范围查询结果 */
    struct RangeResult {
        QVector<QPair<Key, Value>> entries;  ///< 范围内的键值对
        int count = 0;                       ///< 结果数
    };

    /** @brief 操作统计 */
    struct Stats {
        quint64 totalInsertions = 0;     ///< 总插入次数
        quint64 totalDeletions = 0;      ///< 总删除次数
        quint64 totalLookups = 0;        ///< 总查询次数
        quint64 totalNodesCreated = 0;   ///< 总创建节点数
        quint64 totalNodesGCd = 0;       ///< 总GC回收节点数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit PersistentTree2(QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~PersistentTree2() override;

    // ── 修改操作(创建新版本) ──

    /**
     * @brief 插入键值对(创建新版本)
     * @param key 键
     * @param value 值
     * @param description 版本描述
     * @return 新版本号
     */
    Version insert(Key key, Value value,
                   const QString& description = QString());

    /**
     * @brief 删除键(创建新版本)
     * @param key 键
     * @param description 版本描述
     * @return 新版本号(-1表示键不存在)
     */
    Version remove(Key key,
                   const QString& description = QString());

    // ── 查询操作(只读) ──

    /**
     * @brief 在指定版本中查找键
     * @param key 键
     * @param version 版本号(-1表示最新)
     * @return 查找结果
     */
    LookupResult lookup(Key key, Version version = -1) const;

    /**
     * @brief 在指定版本中范围查询
     * @param low 下界(含)
     * @param high 上界(含)
     * @param version 版本号(-1表示最新)
     * @return 范围内键值对
     */
    RangeResult rangeQuery(Key low, Key high,
                           Version version = -1) const;

    /**
     * @brief 获取指定版本中树的大小
     * @param version 版本号
     * @return 元素个数
     */
    int size(Version version = -1) const;

    // ── 版本管理 ──

    /**
     * @brief 获取当前最新版本号
     * @return 版本号
     */
    Version currentVersion() const;

    /**
     * @brief 获取所有版本信息列表
     * @return 版本列表
     */
    QVector<VersionInfo> versionHistory() const;

    /**
     * @brief 获取指定版本信息
     * @param version 版本号
     * @return 版本信息
     */
    VersionInfo versionInfo(Version version) const;

    /**
     * @brief 回滚到指定版本(创建新版本指向旧根)
     * @param version 目标版本
     * @param description 描述
     * @return 新版本号
     */
    Version rollback(Version version,
                     const QString& description = QString());

    /**
     * @brief 垃圾回收: 删除不可达版本
     * @param keepVersions 需要保留的版本号集合
     * @return 回收的节点数
     */
    int garbageCollect(const QSet<Version>& keepVersions);

    // ── 统计 ──

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 新版本创建 @param version 版本号 @param desc 描述 */
    void versionCreated(Version version, const QString& desc);
    /** @brief 节点被GC回收 @param count 回收数 */
    void nodesGarbageCollected(int count);

private:
    /**
     * @brief 树节点(持久化, 不可变)
     */
    struct Node {
        Key key = 0.0;              ///< 键
        Value value = 0.0;          ///< 值
        int left = -1;              ///< 左子节点索引(-1为空)
        int right = -1;             ///< 右子节点索引(-1为空)
        int height = 1;             ///< AVL高度
        int refCount = 0;           ///< 引用计数
    };

    /**
     * @brief 插入到子树(返回新节点索引)
     */
    int insertImpl(int nodeIdx, Key key, Value value);

    /**
     * @brief 从子树删除(返回新节点索引)
     */
    int removeImpl(int nodeIdx, Key key);

    /**
     * @brief 查找子树
     */
    LookupResult lookupImpl(int nodeIdx, Key key, int depth) const;

    /**
     * @brief 范围查询
     */
    void rangeQueryImpl(int nodeIdx, Key low, Key high,
                        QVector<QPair<Key, Value>>& result) const;

    /* AVL平衡操作 */
    int balance(int nodeIdx);
    int rotateLeft(int nodeIdx);
    int rotateRight(int nodeIdx);
    int getHeight(int nodeIdx) const;
    int getBalanceFactor(int nodeIdx) const;
    int updateHeight(int nodeIdx);

    /* 节点管理 */
    int allocateNode(Key key, Value value, int left, int right);
    int cloneNode(int nodeIdx);

    /* 引用计数管理 */
    void addRef(int nodeIdx);
    void releaseRef(int nodeIdx);

    QVector<Node> m_nodes;               ///< 节点池
    QMap<Version, VersionInfo> m_versions; ///< 版本映射
    Version m_nextVersion;                ///< 下一版本号

    mutable Stats m_stats;                        ///< 操作统计
    mutable double m_timeSum = 0.0;               ///< 累计耗时
};
