/**
 * @file UsbConnection.cpp
 * @brief USB连接实现
 *
 * 通过libusb与USB设备通信。TODO: 集成libusb实现。
 */
#include "connection/usb/UsbConnection.h"

UsbConnection::UsbConnection(QObject* parent)
    : IConnection(parent)
{
}

UsbConnection::~UsbConnection() {
    close();
}

ConnectionType UsbConnection::type() const {
    // TODO: 在Constants.h中添加 Usb 到 ConnectionType 枚举
    return ConnectionType::Usb;
}

QString UsbConnection::name() const {
    return tr("USB:%1:%2").arg(m_vid, 4, 16, QChar('0'))
                       .arg(m_pid, 4, 16, QChar('0'));
}

ConnectionState UsbConnection::state() const {
    return m_state;
}

bool UsbConnection::open() {
    if (m_vid == 0 || m_pid == 0) {
        emit errorOccurred(tr("未设置USB设备VID/PID"));
        return false;
    }

    // TODO: libusb_open_device_with_vid_pid
    m_state = ConnectionState::Connected;
    emit stateChanged(m_state);
    return true;
}

void UsbConnection::close() {
    if (m_state == ConnectionState::Connected) {
        // TODO: libusb_release_interface + libusb_close
        m_state = ConnectionState::Disconnected;
        emit stateChanged(m_state);
    }
}

qint64 UsbConnection::write(const QByteArray& data) {
    if (m_state != ConnectionState::Connected) { return -1; }
    // TODO: 使用默认端点进行bulk传输
    QByteArray response = bulkTransfer(0x01, data);
    Q_UNUSED(response)
    return data.size();
}

void UsbConnection::configure(const QVariantMap& params) {
    if (params.contains("vid")) {
        m_vid = static_cast<quint16>(params["vid"].toUInt());
    }
    if (params.contains("pid")) {
        m_pid = static_cast<quint16>(params["pid"].toUInt());
    }
    if (params.contains("interface")) {
        m_interface = params["interface"].toInt();
    }
}

bool UsbConnection::setDevice(quint16 vid, quint16 pid) {
    m_vid = vid;
    m_pid = pid;
    // TODO: 搜索设备是否存在
    return true;
}

bool UsbConnection::claimInterface(int interface) {
    m_interface = interface;
    // TODO: libusb_claim_interface
    return true;
}

void UsbConnection::releaseInterface(int interface) {
    Q_UNUSED(interface)
    // TODO: libusb_release_interface
}

QByteArray UsbConnection::bulkTransfer(int endpoint,
                                        const QByteArray& data) {
    Q_UNUSED(endpoint)
    Q_UNUSED(data)
    // TODO: libusb_bulk_transfer
    return QByteArray();
}

QByteArray UsbConnection::interruptTransfer(int endpoint,
                                             const QByteArray& data) {
    Q_UNUSED(endpoint)
    Q_UNUSED(data)
    // TODO: libusb_interrupt_transfer
    return QByteArray();
}

QByteArray UsbConnection::controlTransfer(quint8 requestType,
                                           quint8 request,
                                           quint16 value,
                                           quint16 index,
                                           const QByteArray& data) {
    Q_UNUSED(requestType)
    Q_UNUSED(request)
    Q_UNUSED(value)
    Q_UNUSED(index)
    Q_UNUSED(data)
    // TODO: libusb_control_transfer
    return QByteArray();
}
