/**
 * @file BridgeTypes.h
 * @brief Multi-Port Bridge 数据类型定义 — 桥接方向、过滤规则、桥接规则、统计计数器
 *
 * 提供 PortBridge 和 BridgeConfigPanel 共用的枚举、结构体。
 * 纯 C++ 头文件，无 Q_OBJECT，无 QObject 继承。
 *
 * 协作关系:
 *   - PortBridge: 使用 BridgeRule 执行数据转发与过滤
 *   - BridgeConfigPanel: 编辑 BridgeRule 并展示 BridgeStats
 */
#ifndef BRIDGETYPES_H
#define BRIDGETYPES_H

#include <QString>
#include <QVector>
#include <QtTypes>

/**
 * @brief 桥接方向枚举
 *
 * 控制 source → target 的数据流方向。
 */
enum class BridgeDirection {
    Forward,      ///< 仅正向：source → target
    Backward,     ///< 仅反向：target → source
    Bidirectional ///< 双向：source ↔ target
};

/**
 * @brief 过滤器类型枚举
 *
 * 定义数据包通过桥接时的过滤策略。
 */
enum class BridgeFilterType {
    None,    ///< 不过滤，所有数据直接转发
    Prefix,  ///< 前缀匹配：仅转发以指定前缀开头的数据
    Regex,   ///< 正则表达式匹配：仅转发匹配正则的数据
    Length   ///< 长度过滤：仅转发指定长度范围的数据
};

/**
 * @brief 桥接过滤器配置
 *
 * 单条过滤规则，可组合使用。
 */
struct BridgeFilter {
    BridgeFilterType type = BridgeFilterType::None; ///< 过滤类型
    QString pattern;                                ///< 匹配模式（前缀/正则/长度范围表达式）
    bool inclusive = true;                          ///< true=白名单(匹配通过)，false=黑名单(匹配拦截)
};

/**
 * @brief 桥接规则配置
 *
 * 完整描述一条桥接连接的源/目标、方向、过滤器和启用状态。
 */
struct BridgeRule {
    QString sourceId;                         ///< 源连接唯一标识
    QString targetId;                         ///< 目标连接唯一标识
    BridgeDirection direction = BridgeDirection::Forward; ///< 数据转发方向
    QVector<BridgeFilter> filters;            ///< 过滤器列表（AND 逻辑）
    bool enabled = true;                      ///< 是否启用此规则
    QString name;                             ///< 规则名称（唯一标识）
};

/**
 * @brief 桥接统计计数器
 *
 * 由 PortBridge 在转发过程中递增，用于追踪单条规则或全局的运行时指标。
 */
struct BridgeStats {
    quint64 bytesForwarded = 0;   ///< 累计转发的字节数
    quint64 bytesFiltered = 0;    ///< 累计被过滤拦截的字节数
    quint64 bytesDropped = 0;     ///< 累计因错误丢弃的字节数
    quint64 packetsForwarded = 0; ///< 累计转发的数据包数
    quint64 errors = 0;           ///< 累计错误次数
};

#endif // BRIDGETYPES_H
