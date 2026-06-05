/**
 * @file SequencerTypes.h
 * @brief 协议序列器类型定义 -- 步骤类型、步骤结构、执行结果、统计
 *
 * 定义 ProtocolSequencer 和 SequenceEditorWidget 共用的数据类型:
 *   - StepType 枚举: 支持的步骤类型(Send/Receive/Delay/WaitFor/Check/Loop/Branch)
 *   - SequenceStep: 单个序列步骤的完整描述
 *   - SequenceResult: 一次序列执行的汇总结果
 *   - SequencerStats: 累计运行统计(quint64 计数器)
 */

#ifndef SEQUENCERTYPES_H
#define SEQUENCERTYPES_H

#include <QByteArray>
#include <QString>
#include <QList>
#include <QtGlobal>

/**
 * @brief 步骤类型枚举
 *
 * 定义协议序列中每种步骤的行为:
 *   - Send:    向连接发送数据
 *   - Receive: 等待接收数据并匹配模式
 *   - Delay:   固定延时(毫秒)
 *   - WaitFor: 等待特定条件满足(超时可控)
 *   - Check:   校验接收数据是否符合预期
 *   - Loop:    循环执行子序列 N 次
 *   - Branch:  条件分支(根据上一步结果选择路径)
 */
enum class StepType {
    Send    = 0,  ///< 发送数据
    Receive = 1,  ///< 接收并匹配
    Delay   = 2,  ///< 延时等待
    WaitFor = 3,  ///< 等待条件
    Check   = 4,  ///< 数据校验
    Loop    = 5,  ///< 循环子序列
    Branch  = 6   ///< 条件分支
};
Q_DECLARE_METATYPE(StepType)

/**
 * @brief 单个序列步骤
 *
 * 描述协议序列中一个原子操作的完整信息。
 * 不同 StepType 使用不同字段组合:
 *   - Send:    data + delayMs(可选发送后延时)
 *   - Receive: expectedPattern + delayMs(超时)
 *   - Delay:   delayMs
 *   - Check:   expectedPattern + data(参考数据)
 *   - Loop:    loopCount + subSteps(子序列)
 */
struct SequenceStep {
    StepType type = StepType::Send;     ///< 步骤类型
    QByteArray data;                    ///< 发送/参考数据
    int delayMs = 0;                    ///< 延时/超时(ms)
    QString description;                ///< 步骤描述(用户可读)
    QString expectedPattern;            ///< 期望模式(hex串或正则)
    int loopCount = 1;                  ///< 循环次数(Loop 类型)
    QList<SequenceStep> subSteps;       ///< 子步骤列表(Loop/Branch)
};

/**
 * @brief 序列执行结果
 *
 * 一次 execute() 调用完成后的汇总报告。
 */
struct SequenceResult {
    bool success = false;               ///< 整体是否成功
    int stepsCompleted = 0;             ///< 成功完成的步骤数
    int errorsFound = 0;                ///< 遇到的错误数
    qint64 durationMs = 0;              ///< 总执行时长(ms)
    QString errorMessage;               ///< 首条错误描述(空=无错误)
};

/**
 * @brief 序列器运行统计
 *
 * 累计的 quint64 计数器，用于性能监控和状态展示。
 */
struct SequencerStats {
    quint64 sequencesExecuted = 0;      ///< 累计执行序列数
    quint64 sequencesSucceeded = 0;     ///< 累计成功序列数
    quint64 sequencesFailed = 0;        ///< 累计失败序列数
    quint64 stepsCompleted = 0;         ///< 累计完成步骤数
    quint64 stepsFailed = 0;            ///< 累计失败步骤数
    quint64 bytesSent = 0;              ///< 累计发送字节数
    quint64 bytesReceived = 0;          ///< 累计接收字节数
    quint64 totalDurationMs = 0;        ///< 累计执行时长(ms)
};

#endif // SEQUENCERTYPES_H
