/**
 * @file ConnectionConstants.h
 * @brief 连接相关常量 — 标准波特率列表和网络连接默认参数
 *
 * 包含标准串口波特率列表、默认波特率索引，以及TCP/UDP连接的默认主机地址和端口。
 */
#ifndef CONNECTION_CONSTANTS_H
#define CONNECTION_CONSTANTS_H

#include <QString>
#include <QStringList>
#include <QtTypes>

/**
 * @brief 标准串口波特率常量 — 统一管理波特率列表和默认选项
 *
 * 涵盖从110到1000000的工业标准波特率值，升序排列。
 * 列表供 SerialConfigPanelUI 的波特率下拉框使用。
 */
namespace BaudRates {
    /// 标准串口波特率列表(升序), 涵盖从110到1000000的工业标准值
    inline const QStringList kStandardRates = {
        "110","300","600","1200","2400","4800","9600","14400",
        "19200","28800","38400","57600","56000","115200",
        "128000","230400","256000","460800","921600","1000000"
    };
    constexpr int kDefaultBaudIndex = 12; ///< 默认选中115200(列表第13项)
}

/**
 * @brief 网络连接默认参数 — 统一管理TCP/UDP连接的默认主机地址和端口
 *
 * 集中定义默认网络连接参数，避免在各Connection类中硬编码。
 */
namespace ConnectionDefaults {
    constexpr const char* kDefaultHost = "127.0.0.1";  ///< 默认TCP/UDP主机地址
    constexpr quint16 kDefaultPort = 8080;              ///< 默认TCP/UDP端口
}

#endif // CONNECTION_CONSTANTS_H
