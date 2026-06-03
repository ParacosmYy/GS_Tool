/**
 * @file ConnectionConstants.h
 * @brief 连接相关常量 - 标准波特率列表和网络连接默认参数
 *
 * 公共基础层中的连接默认值入口。
 */
#ifndef SHARED_CONNECTION_CONSTANTS_H
#define SHARED_CONNECTION_CONSTANTS_H

#include <QString>
#include <QStringList>
#include <QtTypes>

namespace BaudRates {
    inline const QStringList kStandardRates = {
        "110","300","600","1200","2400","4800","9600","14400",
        "19200","28800","38400","57600","56000","115200",
        "128000","230400","256000","460800","921600","1000000"
    };
    constexpr int kDefaultBaudIndex = 12;
}

namespace ConnectionDefaults {
    constexpr const char* kDefaultHost = "127.0.0.1";
    constexpr quint16 kDefaultPort = 8080;
}

#endif // SHARED_CONNECTION_CONSTANTS_H
