/**
 * @file TriggerAction.cpp
 * @brief 触发器动作执行器实现 — 根据 ActionType 分发执行动作
 */

#include "automation/TriggerAction.h"

#include <QProcess>
#include <QApplication>

/**
 * @brief 构造函数
 * @param parent 父对象
 */
TriggerAction::TriggerAction(QObject* parent)
    : QObject(parent)
{
}

/** @brief 执行动作，根据ActionType分发到不同执行路径 @param actionType 动作类型(对应ActionType枚举值) @param actionData 动作附加数据 */
void TriggerAction::execute(int actionType, const QByteArray& actionData)
{
    const auto type = static_cast<ActionType>(actionType);

    /* 更新执行统计 */
    ++m_stats.totalExecutions;
    ++m_execCountByType[actionType];

    switch (type) {
    case ActionType::SendData: {
        /* 累计发送字节数 */
        m_stats.totalSendBytes += actionData.size();
        if (actionData.isEmpty()) ++m_stats.failedExecutions;
        else ++m_stats.successfulExecutions;
        /* 发送数据: 优先使用回调，否则发射信号 */
        if (m_sendCallback) {
            m_sendCallback(actionData);
        } else {
            emit sendDataRequested(actionData);
        }
        break;
    }
    case ActionType::StartRecording: {
        ++m_stats.successfulExecutions;
        emit startRecordingRequested();
        break;
    }
    case ActionType::StopRecording: {
        ++m_stats.successfulExecutions;
        emit stopRecordingRequested();
        break;
    }
    case ActionType::ShowToast: {
        ++m_stats.successfulExecutions;
        emit showToastRequested(QString::fromUtf8(actionData));
        break;
    }
    case ActionType::PlaySound: {
        /* 播放系统提示音 — 使用平台原生音频接口 */
        QString soundFile = QString::fromUtf8(actionData);
        if (soundFile.isEmpty()) {
            /* 默认系统提示音 */
            QApplication::beep();
            ++m_stats.successfulExecutions;
        } else {
#ifdef Q_OS_WIN
            /* Windows: 使用 PowerShell 播放音频文件 */
            QProcess::startDetached(
                "powershell",
                QStringList() << "-NoProfile" << "-Command"
                << QString("(New-Object Media.SoundPlayer '%1').PlaySync()")
                   .arg(soundFile));
            ++m_stats.successfulExecutions;
#else
            /* Linux: 使用 aplay 或 paplay */
            QProcess::startDetached("aplay", QStringList() << soundFile);
            ++m_stats.successfulExecutions;
#endif
        }
        break;
    }
    }
}

/** @brief 设置数据发送回调函数 @param callback 发送数据的回调函数 */
void TriggerAction::setSendCallback(std::function<void(QByteArray)> callback)
{
    ++m_stats.totalCallbacksSet;
    m_sendCallback = std::move(callback);
}

/** @brief 获取指定类型动作的执行次数 @param actionType 动作类型 @return 该类型动作的执行次数 */
quint64 TriggerAction::execCountByType(int actionType) const
{
    return m_execCountByType.value(actionType, 0);
}
