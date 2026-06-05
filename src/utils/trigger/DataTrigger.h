/**
 * @file DataTrigger.h
 * @brief 数据触发器 — 条件触发动作引擎
 *
 * 功能: 支持阈值/范围/变化率/模式匹配4种触发条件，
 *       触发后执行回调或发射信号。
 *
 * 协作: DataThresholdMonitor(阈值) / TriggerEngine(自动化)
 */
#ifndef DATATRIGGER_H
#define DATATRIGGER_H

#include <QObject>
#include <QMap>
#include <QString>
#include <QList>
#include <functional>

class DataTrigger : public QObject {
    Q_OBJECT
public:
    enum class TriggerCondition {
        AboveThreshold,    ///< 高于阈值
        BelowThreshold,    ///< 低于阈值
        InRange,           ///< 在范围内
        OutOfRange,        ///< 超出范围
        RateOfChange,      ///< 变化率超限
        PatternMatch       ///< 模式匹配
    };
    Q_ENUM(TriggerCondition)

    struct TriggerRule {
        QString name;
        TriggerCondition condition;
        double param1 = 0.0;
        double param2 = 0.0;
        bool enabled = true;
        bool oneShot = false;
        bool fired = false;
    };

    struct Stats {
        quint64 totalEvaluations = 0;
        quint64 totalFirings = 0;
        QMap<QString, quint64> firingsByRule;
        double peakValue = 0.0;
    };

    explicit DataTrigger(QObject* parent = nullptr);

    void addRule(const TriggerRule& rule);
    void removeRule(const QString& name);
    void evaluate(double value);
    void evaluatePattern(const QByteArray& data);
    void resetAllFired();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void triggered(const QString& ruleName, double value);

private:
    bool checkRule(const TriggerRule& rule, double value);

    QMap<QString, TriggerRule> m_rules;
    double m_prevValue;
    Stats m_stats;
};

#endif // DATATRIGGER_H
