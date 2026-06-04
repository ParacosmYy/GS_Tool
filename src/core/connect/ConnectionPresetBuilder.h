/**
 * @file ConnectionPresetBuilder.h
 * @brief 连接默认参数构造器 - 根据连接类型生成默认连接参数
 */

#ifndef CONNECTIONPRESETBUILDER_H
#define CONNECTIONPRESETBUILDER_H

#include <QVariantMap>
#include "shared/AppConstants.h"

/**
 * @brief 连接默认参数构造器
 *
 * 仅负责为不同连接类型生成默认参数映射，避免默认值散落在多个调用点。
 * 网络类型包括普通 UDP、UDP 组播等共享参数入口。
 */
class ConnectionPresetBuilder {
public:
    /** @brief 根据连接类型生成默认参数 */
    static QVariantMap build(ConnectionType type);
};

#endif // CONNECTIONPRESETBUILDER_H
