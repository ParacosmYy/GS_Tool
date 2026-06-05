/**
 * @file DataThresholdMonitor.h
 * @brief 数据阈值监控组件 — 当数据值越界时发出告警
 *
 * 功能: 可配置多条阈值规则(名称/最小值/最大值/启用状态)，
 *       支持绝对值和百分比阈值，实时统计违规次数/峰值/谷值。
 *
 * 协作: DataAggregator(数据流监控) / AutomationModule(触发器联动)
 */
#ifndef DATATHRESHOLDMONITOR_H
#define DATATHRESHOLDMONITOR_H

#include <QObject>
#include <QMap>
#include <QString>
#include <QList>

/**
 * @brief 数据阈值监控组件 — 检测数据越界并发出告警
 */
class DataThresholdMonitor : public QObject {
    Q_OBJECT

public:
    /** @brief 阈值状态 */
    enum class ThresholdState {
        BelowMin,   ///< 低于最小值
        Normal,     ///< 正常范围内
        AboveMax,   ///< 高于最大值
        Violation   ///< 违规(通用)
    };
    Q_ENUM(ThresholdState)

    /** @brief 阈值规则 */
    struct ThresholdRule {
        QString name;           ///< 规则名称
        double  minValue = 0.0; ///< 最小阈值
        double  maxValue = 0.0; ///< 最大阈值
        bool    enabled  = true;///< 是否启用
        bool    isPercentage = false; ///< 是否为百分比模式(相对于参考值)
        double  referenceValue = 0.0; ///< 百分比模式的参考基准值
    };

    /** @brief 统计 */
    struct Stats {
        quint64 totalChecks     = 0;  ///< 累计检查次数
        quint64 totalViolations = 0;  ///< 累计违规次数
        double  peakValue       = 0.0;///< 峰值
        double  lowestValue     = 0.0;///< 谷值
        int     activeRules     = 0;  ///< 当前活跃规则数
        QMap<QString, quint64> violationsByRule; ///< 每条规则的违规次数
    };

    explicit DataThresholdMonitor(QObject* parent = nullptr);

    /** @brief 添加阈值规则 @param name 规则名 @param min 最小值 @param max 最大值 */
    void addRule(const QString& name, double min, double max);

    /** @brief 移除阈值规则 @param name 规则名 */
    void removeRule(const QString& name);

    /** @brief 检查单个值是否越界 @param ruleName 规则名 @param value 待检值 @return 阈值状态 */
    ThresholdState checkValue(const QString& ruleName, double value);

    /** @brief 用所有规则检查一组值 @param values 规则名→值的映射 @return 规则名→状态映射 */
    QMap<QString, ThresholdState> checkAll(const QMap<QString, double>& values);

    /** @brief 设置规则为百分比模式 @param name 规则名 @param refValue 参考基准值 */
    void setRulePercentage(const QString& name, double refValue);

    /** @brief 启用/禁用规则 @param name 规则名 @param enabled 是否启用 */
    void setRuleEnabled(const QString& name, bool enabled);

    /** @brief 全局告警开关 @param enabled 是否启用告警信号 */
    void setAlertEnabled(bool enabled);

    /** @brief 获取所有规则 @return 规则列表 */
    QList<ThresholdRule> rules() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 阈值违规 @param rule 规则名 @param value 当前值 @param direction 越界方向("above"/"below") */
    void thresholdViolated(const QString& rule, double value, const QString& direction);
    /** @brief 值回到正常范围 @param rule 规则名 */
    void valueInRange(const QString& rule);

private:
    double effectiveMin(const ThresholdRule& rule) const;
    double effectiveMax(const ThresholdRule& rule) const;

    QMap<QString, ThresholdRule> m_rules;  ///< 规则映射(名称→规则)
    bool m_alertEnabled;                    ///< 告警开关

    Stats m_stats;
};

#endif // DATATHRESHOLDMONITOR_H
