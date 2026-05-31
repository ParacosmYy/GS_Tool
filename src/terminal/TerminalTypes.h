#ifndef TERMINALTYPES_H
#define TERMINALTYPES_H

#include <QByteArray>
#include <QDateTime>
#include "core/Constants.h"

// 共享终端数据类型 - 供 TerminalModel 和 DataExporter 共同使用
// 避免基础设施层(DataExporter)反向依赖表现层(TerminalModel)

// 单条终端数据记录
struct TerminalLine {
    QByteArray data;          // 原始数据
    DataDirection direction;  // 收/发方向
    QDateTime timestamp;      // 时间戳(精确到ms)
};

#endif // TERMINALTYPES_H
