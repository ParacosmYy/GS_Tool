#ifndef OTAMANAGER_H
#define OTAMANAGER_H

#include <QObject>
#include "ota/protocols/XModemTransfer.h"
#include "ota/protocols/YModemTransfer.h"
#include "connection/IConnection.h"

// OTA升级管理器 - 协调传输协议和连接
// 支持自动检测协议(通过接收方响应)和手动指定
class OtaManager : public QObject {
    Q_OBJECT

public:
    explicit OtaManager(QObject* parent = nullptr);

    // 设置传输连接
    void setConnection(IConnection* conn);

    // 开始OTA传输
    // protocol: "xmodem-checksum" / "xmodem-crc" / "xmodem-1k" / "ymodem"
    bool startTransfer(const QString& filePath, const QString& protocol = "xmodem-crc");

    // 取消传输
    void cancelTransfer();

    // 是否正在传输
    bool isTransferring() const;

signals:
    void progress(int percent, qint64 bytesSent, qint64 totalBytes);
    void transferComplete();
    void transferError(const QString& reason);

private:
    IConnection* m_conn = nullptr;
    XModemTransfer* m_xmodem = nullptr;
    YModemTransfer* m_ymodem = nullptr;
};

#endif // OTAMANAGER_H
