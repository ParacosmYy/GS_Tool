/**
 * @file QuickCommandBarActions.cpp
 * @brief 快捷指令栏 - 指令列表管理、持久化与统计实现
 *
 * 本文件从 QuickCommandBar.cpp 拆分而来，包含:
 *   - 指令列表管理(setCommands/addCommand/clearCommands)
 *   - 持久化存储(saveCommands/loadCommands)
 *   - 统计查询(totalCommandsSent/totalQuickSends/maxCommandLength/resetStatistics)
 *
 * UI构建与信号连接保留在 QuickCommandBar.cpp 中。
 */

#include "serial/commands/QuickCommandBar.h"

#include <QSettings>

// ============================================================================
// 指令列表管理
// ============================================================================

/**
 * @brief 设置指令列表，替换当前全部指令并重建按钮
 * @param commands 新的指令列表
 */
void QuickCommandBar::setCommands(const QList<QuickCommand>& commands)
{
    m_commands = commands;
    rebuildButtons();
}

/** @brief 获取当前指令列表的副本 */
QList<QuickCommand> QuickCommandBar::commands() const
{
    return m_commands;
}

/**
 * @brief 添加一条指令到列表末尾并重建按钮
 * @param cmd 要添加的指令
 */
void QuickCommandBar::addCommand(const QuickCommand& cmd)
{
    m_commands.append(cmd);
    rebuildButtons();
    // 新增指令后持久化到QSettings
    saveCommands();
}

/** @brief 清空所有指令并移除按钮 */
void QuickCommandBar::clearCommands()
{
    m_commands.clear();
    rebuildButtons();
    // 清空指令后持久化到QSettings
    saveCommands();
}

// ============================================================================
// 持久化存储 (QSettings)
// ============================================================================

/**
 * @brief 将当前指令列表保存到 QSettings
 *
 * 使用 "QuickCommands" 组存储，格式如下:
 *   count  = 指令总数
 *   name_0 = 第一条指令的名称
 *   data_0 = 第一条指令的数据
 *   hex_0  = "1" 或 "0"（HEX 模式开关）
 *   name_1 = 第二条指令的名称
 *   ...以此类推
 *
 * 保存前会清除该组中所有旧数据，避免残留。
 */
void QuickCommandBar::saveCommands()
{
    QSettings settings;
    settings.beginGroup("QuickCommands");

    // 清除旧数据，防止删除指令后残留
    settings.remove("");

    // 写入指令总数
    settings.setValue("count", m_commands.size());

    // 逐条写入指令
    for (int i = 0; i < m_commands.size(); ++i) {
        const auto& cmd = m_commands[i];
        settings.setValue(QString("name_%1").arg(i), cmd.name);
        settings.setValue(QString("data_%1").arg(i), cmd.data);
        settings.setValue(QString("hex_%1").arg(i), cmd.isHex ? "1" : "0");
    }

    settings.endGroup();
    settings.sync();
}

/**
 * @brief 从 QSettings 加载指令列表
 *
 * 读取 "QuickCommands" 组中保存的指令数据。
 * 如果 "count" 键不存在（首次使用或从未保存），直接返回不做任何操作。
 * 加载成功后替换内存中的指令列表并重建按钮。
 */
void QuickCommandBar::loadCommands()
{
    QSettings settings;
    settings.beginGroup("QuickCommands");

    // 检查是否有保存的数据
    if (!settings.contains("count")) {
        settings.endGroup();
        return;
    }

    const int count = settings.value("count", 0).toInt();
    QList<QuickCommand> loadedCommands;
    loadedCommands.reserve(count);

    for (int i = 0; i < count; ++i) {
        QuickCommand cmd;
        cmd.name  = settings.value(QString("name_%1").arg(i)).toString();
        cmd.data  = settings.value(QString("data_%1").arg(i)).toString();
        cmd.isHex = settings.value(QString("hex_%1").arg(i)).toString() == "1";

        // 跳过完全无效的条目（名称和数据都为空）
        if (!cmd.name.isEmpty() || !cmd.data.isEmpty()) {
            loadedCommands.append(cmd);
        }
    }

    settings.endGroup();

    // 替换当前指令列表并重建按钮
    setCommands(loadedCommands);
}

// ============================================================================
// 统计查询
// ============================================================================

/** @brief 获取快捷栏已发送的指令总次数 @return 发送次数 */
quint64 QuickCommandBar::totalCommandsSent() const
{
    return m_totalCommandsSent;
}

/** @brief 获取快捷发送累计发送的总字节数 @return 累计字节数 */
quint64 QuickCommandBar::totalQuickSends() const
{
    return m_totalQuickSends;
}

/** @brief 获取历史最大单条指令长度（字节数） @return 最大指令长度 */
quint64 QuickCommandBar::maxCommandLength() const
{
    return m_maxCommandLength;
}

/** @brief 重置所有统计计数器为零 */
void QuickCommandBar::resetStatistics()
{
    m_totalCommandsSent = 0;
    m_totalQuickSends = 0;
    m_maxCommandLength = 0;
    m_totalHexCommands = 0;
    m_totalEditDialogOpens = 0;
}
