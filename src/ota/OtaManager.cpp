#include "ota/OtaManager.h"

OtaManager::OtaManager(QObject* parent)
    : QObject(parent)
    , m_xmodem(new XModemTransfer(this))
{
    connect(m_xmodem, &XModemTransfer::progress,
            this, &OtaManager::progress);
    connect(m_xmodem, &XModemTransfer::transferComplete,
            this, &OtaManager::transferComplete);
    connect(m_xmodem, &XModemTransfer::transferError,
            this, &OtaManager::transferError);
}

void OtaManager::setConnection(IConnection* conn)
{
    m_conn = conn;
    m_xmodem->setConnection(conn);
}

bool OtaManager::startTransfer(const QString& filePath, const QString& protocol)
{
    if (!m_conn) {
        emit transferError("No connection available");
        return false;
    }

    // 设置传输模式
    if (protocol == "xmodem-checksum") {
        m_xmodem->setMode(XModemTransfer::Checksum);
    } else if (protocol == "xmodem-1k") {
        m_xmodem->setMode(XModemTransfer::OneK);
    } else {
        m_xmodem->setMode(XModemTransfer::CRC);
    }

    m_xmodem->setFilePath(filePath);
    return m_xmodem->start();
}

void OtaManager::cancelTransfer()
{
    m_xmodem->cancel();
}

bool OtaManager::isTransferring() const
{
    return m_xmodem->isRunning();
}
