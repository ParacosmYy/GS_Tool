/**
 * @file TriggerAction.cpp
 * @brief 触发器动作执行器实现 — 骨架文件
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
 *   - SendData: 调用回调或发送信号
 *   - StartRecording/StopRecording: 发送录制信号
 *   - ShowToast/PlaySound: 发送提示信号
 *
 * @param actionType 动作类型（对应 ActionType 枚举值）
 * @param actionData 动作附加数据
 */
void TriggerAction::execute(int actionType, const QByteArray& actionData)
{
    Q_UNUSED(actionType)
    Q_UNUSED(actionData)
    // TODO: 根据 actionType 分发动作
}

/**
 * @brief 设置数据发送回调函数
 * @param callback 发送数据的回调函数
 */
void TriggerAction::setSendCallback(std::function<void(QByteArray)> callback)
{
    m_sendCallback = callback;
}
