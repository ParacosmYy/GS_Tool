/**
 * @file TriggerAction.cpp
 * @brief 触发器动作执行器实现 — 根据 ActionType 分发执行动作
 */

#include "automation/TriggerAction.h"

/**
 * @brief 构造函数
 * @param parent 父对象
 */
TriggerAction::TriggerAction(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 执行动作
 *
 * 根据 actionType 分发到不同的执行路径:
 *   - SendData: 优先调用回调函数，否则发送 sendDataRequested 信号
 *   - StartRecording: 发送 startRecordingRequested 信号
 *   - StopRecording: 发送 stopRecordingRequested 信号
 *   - ShowToast: 发送 showToastRequested 信号
 *   - PlaySound: 预留桩（TODO: 接入音频模块）
 *
 * @param actionType 动作类型（对应 ActionType 枚举值）
 * @param actionData 动作附加数据
 */
void TriggerAction::execute(int actionType, const QByteArray& actionData)
{
    const auto type = static_cast<ActionType>(actionType);

    switch (type) {
    case ActionType::SendData: {
        /* 发送数据: 优先使用回调，否则发射信号 */
        if (m_sendCallback) {
            m_sendCallback(actionData);
        } else {
            emit sendDataRequested(actionData);
        }
        break;
    }
    case ActionType::StartRecording: {
        emit startRecordingRequested();
        break;
    }
    case ActionType::StopRecording: {
        emit stopRecordingRequested();
        break;
    }
    case ActionType::ShowToast: {
        emit showToastRequested(QString::fromUtf8(actionData));
        break;
    }
    case ActionType::PlaySound: {
        /* TODO: 接入音频播放模块 */
        break;
    }
    }
}

/**
 * @brief 设置数据发送回调函数
 * @param callback 发送数据的回调函数
 */
void TriggerAction::setSendCallback(std::function<void(QByteArray)> callback)
{
    m_sendCallback = std::move(callback);
}
