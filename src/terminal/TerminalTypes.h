/**
 * @file TerminalTypes.h
 * @brief 共享终端数据类型定义 — 供TerminalModel/DataExporter/TerminalWidget共同使用
 *
 * 避免基础设施层(DataExporter)反向依赖表现层(TerminalModel)。
 * 包含TerminalLine结构体和DataDirection枚举。
 */
#ifndef TERMINALTYPES_H
#define TERMINALTYPES_H

#include <QByteArray>
#include <QDateTime>
#include <QString>
#include "core/Constants.h"

// 共享终端数据类型 - 供 TerminalModel、DataExporter、TerminalWidget 共同使用
// 避免基础设施层(DataExporter)反向依赖表现层(TerminalModel)

// 单条终端数据记录
struct TerminalLine {
    QByteArray data;          ///< 原始数据
    DataDirection direction;  ///< 收/发方向
    QDateTime timestamp;      ///< 时间戳(精确到ms)
};

/**
 * @brief 缓存行的完整信息，避免paintEvent中调用lineAt()访问环形缓冲区
 *
 * 每次缓存更新时一次性填充，渲染时只读缓存即可。
 * 格式化后的文本已根据当前DisplayMode转换完成。
 * 放在TerminalTypes.h中供TerminalWidget、TerminalSelectionManager、TerminalSearchManager共享使用。
 */
struct CachedLine {
    QString text;            ///< 按当前DisplayMode格式化后的文本
    DataDirection direction; ///< 收/发方向，用于选择文字颜色
    qint64 timestamp;        ///< epoch毫秒时间戳，用于时间戳显示
};

#endif // TERMINALTYPES_H
