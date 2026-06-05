/**
 * @file SimulatorTypes.h
 * @brief 设备模拟器公共类型定义 -- 响应模式、匹配策略、响应规则、统计数据结构
 *
 * 本文件是 DeviceSimulator 和 DeviceSimulatorPanel 的共享类型头文件，
 * 不依赖 QObject，可被非 Qt 模块引用。
 */
#ifndef SIMULATORTYPES_H
#define SIMULATORTYPES_H

#include <QByteArray>
#include <QVector>
#include <QString>
#include <QtGlobal>

/**
 * @brief 响应生成模式 -- 决定匹配到命令后如何构造响应数据
 */
enum class ResponseMode : int {
    Fixed      = 0,  ///< 固定内容: 每次返回 fixedData
    Incremental = 1,  ///< 递增模式: 每次返回后 fixedData 末字节自增
    Random     = 2,  ///< 随机模式: 返回随机长度的随机字节
    Scripted   = 3   ///< 脚本模式: 按 scriptedResponses 列表循环输出
};

/**
 * @brief 命令匹配策略 -- 决定如何将收到的数据与命令模式关联
 */
enum class MatchStrategy : int {
    Exact  = 0,  ///< 精确匹配: 输入与 commandPattern 完全一致
    Prefix = 1,  ///< 前缀匹配: 输入以 commandPattern 开头即匹配
    Regex  = 2   ///< 正则匹配: commandPattern 作为正则表达式
};

/**
 * @brief 单条命令-响应规则 -- 描述一组匹配条件及对应的响应行为
 */
struct SimResponse {
    ResponseMode mode = ResponseMode::Fixed;    ///< 响应生成模式
    MatchStrategy strategy = MatchStrategy::Exact; ///< 命令匹配策略
    QByteArray commandPattern;                  ///< 命令模式(精确串/前缀串/正则表达式)
    QByteArray fixedData;                       ///< Fixed/Incremental 模式下的固定响应数据
    QVector<QByteArray> scriptedResponses;      ///< Scripted 模式下的响应列表(循环输出)
    int minDelayMs = 10;                        ///< 最小响应延迟(ms)
    int maxDelayMs = 50;                        ///< 最大响应延迟(ms)
    int randomMinLen = 4;                       ///< Random 模式最小长度
    int randomMaxLen = 16;                      ///< Random 模式最大长度
    bool enabled = true;                        ///< 规则是否启用
};

/**
 * @brief 设备模拟器运行统计数据
 */
struct SimulatorRunStats {
    quint64 commandsReceived = 0;    ///< 累计接收的命令总数
    quint64 responsesSent = 0;       ///< 累计发送的响应总数
    quint64 commandsUnmatched = 0;   ///< 累计未匹配到任何规则的命令数
    quint64 totalBytesSent = 0;      ///< 累计发送的总字节数
    quint64 totalBytesReceived = 0;  ///< 累计接收的总字节数
    double avgResponseDelayMs = 0.0; ///< 平均响应延迟(ms)
    quint64 echoCount = 0;           ///< 回显模式触发次数
    quint64 noiseCount = 0;          ///< 噪声注入次数
};

#endif // SIMULATORTYPES_H
