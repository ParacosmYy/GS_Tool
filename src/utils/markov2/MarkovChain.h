/**
 * @file MarkovChain.h
 * @brief 一阶马尔可夫链引擎 — 状态转移概率矩阵建模与预测
 *
 * 提供一阶马尔可夫链的状态转移概率矩阵构建、下一状态预测、
 * 稳态分布计算等功能。适用于嵌入式调试场景中的模式预测、
 * 信号状态转移分析和异常行为检测。
 */
#ifndef MARKOVCHAIN2_H
#define MARKOVCHAIN2_H

#include <QObject>
#include <QMap>
#include <QVector>

/**
 * @class MarkovChain
 * @brief 一阶马尔可夫链引擎
 *
 * 典型用法:
 * @code
 *   MarkovChain mc;
 *   mc.addTransition("A", "B");
 *   mc.addTransition("B", "C");
 *   QString next = mc.predictNext("A");
 *   auto dist = mc.stationaryDistribution();
 * @endcode
 */
class MarkovChain : public QObject {
    Q_OBJECT

public:
    /** @brief 操作统计结构 */
    struct Stats {
        quint64 totalSteps = 0;           ///< 总转移步数
        quint64 totalStates = 0;          ///< 已注册状态总数
        double  avgProcessingTime = 0.0;  ///< 平均处理时间(us)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit MarkovChain(QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~MarkovChain() override;

    // ── 核心操作 ──

    /**
     * @brief 添加一次状态转移记录
     * @param from 源状态
     * @param to   目标状态
     */
    void addTransition(const QString& from, const QString& to);

    /**
     * @brief 预测下一状态(最高概率)
     * @param current 当前状态
     * @return 最可能的下一状态; 无数据返回空串
     */
    QString predictNext(const QString& current) const;

    /**
     * @brief 获取指定状态的转移概率分布
     * @param state 源状态
     * @return 目标状态→概率映射; 无数据返回空Map
     */
    QMap<QString, double> transitionProbabilities(const QString& state) const;

    /**
     * @brief 计算稳态分布(幂迭代法)
     * @param iterations 迭代次数, 默认100
     * @return 状态→稳态概率映射
     */
    QMap<QString, double> stationaryDistribution(int iterations = 100) const;

    // ── 批量操作 ──

    /**
     * @brief 批量添加转移序列(按顺序两两配对)
     * @param states 状态序列(至少2个元素)
     */
    void addSequence(const QVector<QString>& states);

    /** @brief 清除所有转移记录和状态 */
    void clear();

    // ── 查询 ──

    /** @brief 获取所有已注册状态列表 */
    QVector<QString> states() const;

    /** @brief 获取指定状态的转移计数 */
    quint64 transitionCount(const QString& from) const;

    // ── 统计 ──

    /** @brief 获取当前统计数据快照 */
    Stats stats() const;

    /** @brief 重置所有统计计数器归零 */
    void resetStatistics();

signals:
    /** @brief 转移添加完成信号 @param from 源状态 @param to 目标状态 */
    void transitionAdded(const QString& from, const QString& to);
    /** @brief 状态新增信号 @param state 新状态名称 */
    void stateAdded(const QString& state);
    /** @brief 错误信号 @param errorMessage 错误描述 */
    void error(const QString& errorMessage);

private:
    /** @brief 确保状态已注册到索引表 */
    void ensureState(const QString& state);

    /** @brief 更新平均处理时间(us) */
    void updateAvgTime(qint64 elapsedUs);

    /** @brief 转移计数: transitions_[from][to] = count */
    QMap<QString, QMap<QString, quint64>> m_transitions;

    /** @brief 每个源状态的总转移计数 */
    QMap<QString, quint64> m_rowTotals;

    /** @brief 所有已注册状态列表 */
    QVector<QString> m_stateList;

    /** @brief 操作统计 */
    Stats m_stats;
};

#endif // MARKOVCHAIN2_H
