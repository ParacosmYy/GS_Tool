/**
 * @file UsbConnection.cpp
 * @brief USB连接核心属性与配置 — 类型/名称/状态/write/configure
 *
 * 传输方法(Bulk/Interrupt/Control)实现在 UsbConnectionTransfer.cpp 中。
 * 描述符读取方法实现在 UsbConnectionDescriptors.cpp 中。
 * 生命周期方法(open/close/setDevice)实现在 UsbConnectionLifecycle.cpp 中。
 *
 * @see UsbConnectionLifecycle.cpp — 生命周期方法
 * @see UsbConnectionTransfer.cpp — 传输方法实现
 * @see UsbConnectionDescriptors.cpp — 描述符读取方法实现
 */
#include "connection/usb/UsbConnection.h"
#include "connection/usb/UsbLibraryLoader.h"

/** @brief 构造函数，初始化USB连接基类 @param parent 父对象指针 */
UsbConnection::UsbConnection(QObject* parent)
    : IConnection(parent)
{
}

/** @brief 析构函数，关闭连接并释放USB资源 */
UsbConnection::~UsbConnection() {
    close();
}

/** @brief 获取连接类型 @return 固定返回ConnectionType::Usb */
ConnectionType UsbConnection::type() const {
    return ConnectionType::Usb;
}

/** @brief 获取连接显示名称，格式为USB:VID:PID @return 十六进制格式的VID:PID字符串 */
QString UsbConnection::name() const {
    return tr("USB:%1:%2").arg(m_vid, 4, 16, QChar('0'))
                       .arg(m_pid, 4, 16, QChar('0'));
}

/** @brief 获取当前连接状态 @return 当前连接状态枚举值 */
ConnectionState UsbConnection::state() const {
    return m_state;
}

/** @brief 通过默认bulk OUT端点写入数据 @param data 待发送的字节数据 @return 实际传输字节数，失败返回-1 */
qint64 UsbConnection::write(const QByteArray& data) {
    if (m_state != ConnectionState::Connected) { return -1; }
    if (!m_devHandle) { return -1; }

    auto& loader = UsbLibraryLoader::instance();
    int transferred = 0;

    /* 使用默认OUT端点(0x01)进行bulk传输 */
    int result = loader.bulkTransfer(
        m_devHandle,
        0x01,  /* 默认bulk OUT端点 */
        reinterpret_cast<unsigned char*>(const_cast<char*>(data.constData())),
        data.size(),
        &transferred,
        m_timeout);

    if (result != 0) {
        emit errorOccurred(tr("USB写入失败: 错误码 %1").arg(result));
        ++m_errorCount;
        return -1;
    }

    ++m_totalTransfers;
    ++m_bulkTransferCount;
    m_totalBytesSent += static_cast<quint64>(transferred);
    emit bytesWritten(transferred);
    return transferred;
}

/** @brief 通过参数映射配置USB连接属性(VID/PID/接口/超时) @param params 配置参数键值对 */
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
    if (params.contains("timeout")) {
        m_timeout = static_cast<unsigned int>(params["timeout"].toUInt());
    }
}

// open()/close()/setDevice() 见 UsbConnectionLifecycle.cpp
// 描述符读取方法 — 已拆分至 UsbConnectionDescriptors.cpp
// 接口管理方法(claim/release/detachKernel) — 已拆分至 UsbConnectionInterface.cpp
