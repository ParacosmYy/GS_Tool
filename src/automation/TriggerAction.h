/**
 * @file TriggerAction.h
 * @brief 触发器动作执行器 — 响应触发器命中信号执行对应动作
 *
 * 根据触发器的动作类型（发送数据/录制/提示等）执行具体操作。
 * 通过信号通知外部模块执行实际动作，自身不直接操作硬件或 UI。
 *
 * 协作关系:
 *   - TriggerEngine: 发出 actionRequired 信号驱动本类
 *   - TriggerManager: 管理本类实例的生命周期
 *   - SendController: 响应 sendDataRequested 信号发送数据
 */
#ifndef TRIGGERACTION_H
#define TRIGGERACTION_H

#include <QObject>
#include <QByteArray>
#include <QMap>
#include <functional>
#include "automation/TriggerRule.h"

/**
 * @brief 触发器动作执行器
 *
 * 接收动作类型和附加数据，通过信号或回调分发到对应的执行模块。
 * 支持通过 setSendCallback 设置数据发送回调函数。
 */
class TriggerAction : public QObject {
    Q_OBJECT

public:
    /** @brief 构造函数 */
    explicit TriggerAction(QObject* parent = nullptr);

    /**
     * @brief 执行动作
     * @param actionType 动作类型（对应 ActionType 枚举值）
     * @param actionData 动作附加数据
     */
    void execute(int actionType, const QByteArray& actionData);

    /**
     * @brief 设置数据发送回调函数
     * @param callback 发送数据的回调函数
     */
    void setSendCallback(std::function<void(QByteArray)> callback);

    /**
     * @brief 获取累计执行动作次数
     * @return 所有类型的动作执行总次数
     */
    quint64 totalExecCount() const;

    /**
     * @brief 获取指定类型动作的执行次数
     * @param actionType 动作类型
     * @return 该类型动作的执行次数
     */
    quint64 execCountByType(int actionType) const;

    /**
     * @brief 获取累计发送数据字节数（仅 SendData 类型）
     * @return 已发送字节总数
     */
    quint64 totalSendBytes() const;

    /**
     * @brief 获取累计动作执行错误次数(SendData回调失败/PlaySound失败等)
     * @return 错误总次数
     */
    quint64 totalActionErrors() const;

    /**
     * @brief 获取累计回调函数设置次数
     * @return setSendCallback调用次数
     */
    quint64 totalCallbacksSet() const;

    /**
     * @brief 重置执行统计
     */
    void resetExecStatistics();

signals:
    /** @brief 请求数据发送信号 */
    void sendDataRequested(const QByteArray& data);

    /** @brief 请求开始录制信号 */
    void startRecordingRequested();

    /** @brief 请求停止录制信号 */
    void stopRecordingRequested();

    /**
     * @brief 请求显示提示消息信号
     * @param message 消息内容
     */
    void showToastRequested(const QString& message);

private:
    std::function<void(QByteArray)> m_sendCallback;  ///< 数据发送回调函数

    /** @brief 累计执行动作总次数 */
    quint64 m_totalExecCount = 0;
    /** @brief 各类型动作执行次数 */
    QMap<int, quint64> m_execCountByType;
    /** @brief SendData 动作累计发送字节总数 */
    quint64 m_totalSendBytes = 0;
    /** @brief 累计动作执行错误次数 */
    quint64 m_totalActionErrors = 0;
    /** @brief 累计回调函数设置次数 */
    quint64 m_totalCallbacksSet = 0;
};

#endif // TRIGGERACTION_H
