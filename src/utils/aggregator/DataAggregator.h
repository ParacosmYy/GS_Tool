#pragma once
#include <QObject>
#include <QMap>
#include <QString>
#include <QVariant>
#include <QList>
#include <QTimer>

/**
 * @brief 数据聚合器 — 多源滑动窗口聚合计算
 *
 * 支持多个独立数据源，每个数据源可单独配置聚合函数和窗口大小。
 * 聚合类型包括: 求和/均值/最小值/最大值/计数/首值/末值。
 */
class DataAggregator : public QObject {
    Q_OBJECT
public:
    /** @brief 聚合函数类型枚举 */
    enum AggregateFunc { Sum, Average, Min, Max, Count, First, Last };
    Q_ENUM(AggregateFunc)

    /** @brief 构造数据聚合器 @param parent 父对象 */
    explicit DataAggregator(QObject *parent = nullptr);
    /** @brief 析构函数 */
    ~DataAggregator() override;

    /** @brief 添加数据源并指定聚合函数和窗口大小 @param name 数据源名称 @param func 聚合函数类型 @param windowSize 滑动窗口大小 */
    void addSource(const QString &name, AggregateFunc func, int windowSize = 100);
    /** @brief 移除指定名称的数据源 @param name 数据源名称 */
    void removeSource(const QString &name);
    /** @brief 向指定数据源输入一个新值，触发聚合计算并发射结果 @param source 数据源名称 @param value 新数据值 */
    void feedValue(const QString &source, double value);
    /** @brief 获取指定数据源的聚合结果 @param source 数据源名称 @return 聚合结果值，不存在返回0.0 */
    double aggregateResult(const QString &source) const;
    /** @brief 获取所有数据源的聚合结果 @return 数据源名称到聚合结果的映射 */
    QMap<QString, double> allResults() const;
    /** @brief 获取所有数据源名称列表 @return 数据源名称列表 */
    QStringList sources() const;
    /** @brief 设置指定数据源的滑动窗口大小 @param source 数据源名称 @param size 新窗口大小 */
    void setWindowSize(const QString &source, int size);
    /** @brief 设置指定数据源的聚合函数 @param source 数据源名称 @param func 聚合函数类型 */
    void setAggregateFunc(const QString &source, AggregateFunc func);
    /** @brief 重置指定数据源的缓冲区和结果 @param source 数据源名称 */
    void resetSource(const QString &source);
    /** @brief 重置所有数据源的缓冲区和结果 */
    void resetAll();

signals:
    /** @brief 数据源聚合完成信号 @param source 数据源名称 @param result 聚合结果 */
    void valueAggregated(const QString &source, double result);
    /** @brief 数据源添加信号 @param name 数据源名称 */
    void sourceAdded(const QString &name);
    /** @brief 数据源移除信号 @param name 数据源名称 */
    void sourceRemoved(const QString &name);

private:
    /** @brief 对指定数据源执行聚合计算 @param source 数据源名称 */
    void computeAggregate(const QString &source);
    /** @brief 数据源配置结构: 聚合函数+窗口大小+值缓冲区+结果 */
    struct SourceConfig { AggregateFunc func; int windowSize; QList<double> values; double result = 0.0; };
    QMap<QString, SourceConfig> m_sources;
};
