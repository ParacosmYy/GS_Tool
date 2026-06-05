/**
 * @file DeviceSimulator.h
 * @brief 设备模拟器核心引擎 -- 接收命令、匹配规则、生成响应
 *
 * DeviceSimulator 是一个 QObject，模拟串口设备的命令-响应交互。
 * 支持精确/前缀/正则三种命令匹配，Fixed/Incremental/Random/Scripted 四种响应模式，
 * 以及可配置的随机延迟、自动回显和噪声注入。
 *
 * 信号:
 *   - responseSent(QByteArray): 响应已发送
 *   - commandReceived(QByteArray): 命令已接收
 */
#ifndef DEVICESIMULATOR_H
#define DEVICESIMULATOR_H

#include <QObject>
#include <QTimer>
#include <QVector>
#include <QRandomGenerator>
#include <QElapsedTimer>
#include <QRegularExpression>

#include "utils/simulator/SimulatorTypes.h"

/**
 * @brief 设备模拟器核心引擎 -- 模拟串口设备的命令-响应行为
 *
 * 使用方式:
 *   1. addResponse() 添加命令-响应规则
 *   2. setAutoEcho() / setNoiseRate() 配置附加行为
 *   3. feedData() 输入命令数据 → 匹配规则 → 延迟后 emit responseSent
 *   4. stats() 查看运行统计
 */
class DeviceSimulator : public QObject {
    Q_OBJECT

public:
    /** @brief 构造设备模拟器 @param parent 父对象 */
    explicit DeviceSimulator(QObject* parent = nullptr);

    // ── 规则管理 ──

    /** @brief 添加命令-响应规则 @param rule 响应规则 */
    void addResponse(const SimResponse& rule);
    /** @brief 移除指定索引的规则 @param index 规则索引 */
    void removeResponse(int index);
    /** @brief 替换指定索引的规则 @param index 规则索引 @param rule 新规则 */
    void setResponse(int index, const SimResponse& rule);
    /** @brief 获取所有规则(只读) */
    const QVector<SimResponse>& responses() const;
    /** @brief 清除所有规则 */
    void clearResponses();

    // ── 全局配置 ──

    /** @brief 设置自动回显模式(收到的数据原样回传) @param enable 是否启用 */
    void setAutoEcho(bool enable);
    /** @brief 查询自动回显是否启用 */
    bool autoEcho() const;
    /** @brief 设置噪声注入率(每条响应附加随机字节的概率, 0.0~1.0) */
    void setNoiseRate(double rate);
    /** @brief 查询噪声注入率 */
    double noiseRate() const;
    /** @brief 设置全局默认延迟范围(ms) */
    void setDefaultDelay(int minMs, int maxMs);

    // ── 核心交互 ──

    /** @brief 喂入命令数据(触发匹配→延迟→发送响应) @param data 收到的命令数据 */
    void feedData(const QByteArray& data);

    // ── 统计 ──

    /** @brief 获取运行统计数据 */
    const SimulatorRunStats& stats() const;
    /** @brief 重置统计数据 */
    void resetStats();

signals:
    /** @brief 响应已发送 @param data 响应数据 */
    void responseSent(const QByteArray& data);
    /** @brief 命令已接收 @param data 命令数据 */
    void commandReceived(const QByteArray& data);

private:
    /** @brief 查找匹配的规则(Exact > Prefix > Regex 优先级) */
    int findMatchingRule(const QByteArray& input) const;
    /** @brief 根据规则模式生成响应数据 */
    QByteArray generateResponse(const SimResponse& rule, int ruleIndex);
    /** @brief 生成随机字节序列 */
    QByteArray generateRandomBytes(int minLen, int maxLen) const;
    /** @brief 注入噪声(附加随机字节) */
    QByteArray injectNoise(const QByteArray& data);
    /** @brief 计算规则延迟(ms) */
    int calcDelay(const SimResponse& rule) const;

    QVector<SimResponse> m_rules;            ///< 响应规则列表
    QVector<int> m_scriptCursors;            ///< Scripted 模式下每条规则当前游标
    bool m_autoEcho = false;                 ///< 自动回显开关
    double m_noiseRate = 0.0;                ///< 噪声注入率(0.0~1.0)
    int m_defaultMinDelayMs = 10;            ///< 全局默认最小延迟
    int m_defaultMaxDelayMs = 50;            ///< 全局默认最大延迟
    mutable QRandomGenerator m_rng;          ///< 随机数生成器(const方法可用)
    QElapsedTimer m_timer;                   ///< 运行计时
    qint64 m_delayAccum = 0;                 ///< 延迟累计(用于均值)
    quint64 m_delayCount = 0;                ///< 延迟采样次数
    SimulatorRunStats m_stats;               ///< 运行统计
};

#endif // DEVICESIMULATOR_H
