#ifndef OTAMANAGER_H
#define OTAMANAGER_H

#include <QObject>
#include "ota/protocols/BaseTransfer.h"
#include "ota/protocols/XModemTransfer.h"
#include "ota/protocols/YModemTransfer.h"
#include "ota/protocols/ZModemTransfer.h"
#include "connection/IConnection.h"

// OTA升级管理器 - 协调传输协议和连接
// 通过BaseTransfer*统一管理三个协议实例的信号连接
class OtaManager : public QObject {
    Q_OBJECT

public:
    explicit OtaManager(QObject* parent = nullptr);

    void setConnection(IConnection* conn);
    bool startTransfer(const QString& filePath, const QString& protocol = "xmodem-crc");
    void cancelTransfer();
    bool isTransferring() const;

signals:
    void progress(int percent, qint64 bytesSent, qint64 totalBytes);
    void transferComplete();
    void transferError(const QString& reason);

private:
    // 统一绑定BaseTransfer的三个信号到OtaManager的转发
    void connectTransferSignals(BaseTransfer* transfer);

    IConnection* m_conn = nullptr;
    XModemTransfer* m_xmodem = nullptr;
    YModemTransfer* m_ymodem = nullptr;
    ZModemTransfer* m_zmodem = nullptr;
};

#endif // OTAMANAGER_H
