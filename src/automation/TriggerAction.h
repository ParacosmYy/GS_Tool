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
};

#endif // TRIGGERACTION_H
