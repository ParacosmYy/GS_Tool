#include "ota/OtaManager.h"

OtaManager::OtaManager(QObject* parent)
    : QObject(parent)
    , m_xmodem(new XModemTransfer(this))
    , m_ymodem(new YModemTransfer(this))
{
    connect(m_xmodem, &XModemTransfer::progress,
            this, &OtaManager::progress);
    connect(m_xmodem, &XModemTransfer::transferComplete,
            this, &OtaManager::transferComplete);
    connect(m_xmodem, &XModemTransfer::transferError,
            this, &OtaManager::transferError);

    connect(m_ymodem, &YModemTransfer::progress,
            this, &OtaManager::progress);
    connect(m_ymodem, &YModemTransfer::transferComplete,
            this, &OtaManager::transferComplete);
    connect(m_ymodem, &YModemTransfer::transferError,
            this, &OtaManager::transferError);
}

void OtaManager::setConnection(IConnection* conn)
{
    m_conn = conn;
    m_xmodem->setConnection(conn);
    m_ymodem->setConnection(conn);
}

bool OtaManager::startTransfer(const QString& filePath, const QString& protocol)
{
    if (!m_conn) {
        emit transferError("No connection available");
        return false;
    }

    if (protocol == "ymodem") {
        m_ymodem->setFilePath(filePath);
        return m_ymodem->start();
    }

    // XMODEM模式选择
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
    if (m_xmodem->isRunning()) {
        m_xmodem->cancel();
    }
    if (m_ymodem->isRunning()) {
        m_ymodem->cancel();
    }
}

bool OtaManager::isTransferring() const
{
    return m_xmodem->isRunning() || m_ymodem->isRunning();
}
