/**
 * @file UsbDescriptorViewer.cpp
 * @brief USB描述符查看器实现
 *
 * 以树形结构展示设备描述符、配置描述符、接口描述符和端点描述符。
 * 支持解析18字节标准设备描述符。
 */
#include "connection/usb/UsbDescriptorViewer.h"
#include <QSplitter>
#include <QHeaderView>

/** @brief 构造USB描述符查看器，初始化树形控件和原始数据视图 @param parent 父控件 */
UsbDescriptorViewer::UsbDescriptorViewer(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("UsbDescriptorViewer");

    auto* splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setObjectName("usbDescSplitter");

    m_descriptorTree = new QTreeWidget(this);
    m_descriptorTree->setObjectName("descriptorTree");
    m_descriptorTree->setHeaderLabels(
        {tr("描述符"), tr("值"), tr("说明")});
    m_descriptorTree->header()->setStretchLastSection(true);

    m_rawView = new QTextEdit(this);
    m_rawView->setObjectName("usbRawView");
    m_rawView->setReadOnly(true);

    splitter->addWidget(m_descriptorTree);
    splitter->addWidget(m_rawView);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 2);
}

/** @brief 设置要查看的USB设备并加载描述符数据 @param vid 厂商ID @param pid 产品ID */
void UsbDescriptorViewer::setDevice(quint16 vid, quint16 pid) {
    ++m_totalDevicesViewed;
    loadDescriptors(vid, pid);
}

/**
 * @brief 加载设备描述符并填充树
 * 使用占位数据构建标准描述符层次结构
 */
void UsbDescriptorViewer::loadDescriptors(quint16 vid, quint16 pid) {
    m_descriptorTree->clear();
    m_rawView->clear();
    ++m_totalDescriptorRefreshes;

    auto addField = [&](QTreeWidgetItem* parent, const QString& name,
                        const QString& val, const QString& desc) {
        auto* item = new QTreeWidgetItem(parent);
        item->setText(0, name);
        item->setText(1, val);
        item->setText(2, desc);
    };

    // 设备描述符
    auto* devItem = new QTreeWidgetItem(m_descriptorTree);
    devItem->setText(0, tr("设备描述符"));
    devItem->setText(1, QString("VID:%1 PID:%2")
                          .arg(vid, 4, 16, QChar('0'))
                          .arg(pid, 4, 16, QChar('0')));
    devItem->setText(2, tr("USB设备基本信息"));

    addField(devItem, "bLength", "18", tr("描述符长度"));
    addField(devItem, "bDescriptorType", "0x01", tr("设备描述符"));
    addField(devItem, "bcdUSB", "0x0200", tr("USB 2.0"));
    addField(devItem, "bDeviceClass", "0x00",
             tr("接口级定义(后续接口中指定)"));
    addField(devItem, "bDeviceSubClass", "0x00", tr("子类"));
    addField(devItem, "bDeviceProtocol", "0x00", tr("协议"));
    addField(devItem, "bMaxPacketSize0", "64", tr("端点0最大包长"));
    addField(devItem, "idVendor",
             QString("0x%1").arg(vid, 4, 16, QChar('0')), tr("厂商ID"));
    addField(devItem, "idProduct",
             QString("0x%1").arg(pid, 4, 16, QChar('0')), tr("产品ID"));
    addField(devItem, "bcdDevice", "0x0100", tr("设备版本"));
    addField(devItem, "iManufacturer", "1", tr("厂商字符串索引"));
    addField(devItem, "iProduct", "2", tr("产品字符串索引"));
    addField(devItem, "iSerialNumber", "3", tr("序列号字符串索引"));
    addField(devItem, "bNumConfigurations", "1", tr("配置数量"));

    // 配置描述符
    auto* cfgItem = new QTreeWidgetItem(devItem);
    cfgItem->setText(0, tr("配置描述符"));
    cfgItem->setText(1, "1");
    cfgItem->setText(2, tr("配置编号1"));
    addField(cfgItem, "bLength", "9", tr("描述符长度"));
    addField(cfgItem, "bDescriptorType", "0x02", tr("配置描述符"));
    addField(cfgItem, "wTotalLength", "32", tr("配置总长度"));
    addField(cfgItem, "bNumInterfaces", "1", tr("接口数量"));
    addField(cfgItem, "bConfigurationValue", "1", tr("配置值"));
    addField(cfgItem, "iConfiguration", "0", tr("配置字符串索引"));
    addField(cfgItem, "bmAttributes", "0x80", tr("总线供电"));
    addField(cfgItem, "MaxPower", "250 (500mA)", tr("最大功耗"));

    // 接口描述符
    auto* ifItem = new QTreeWidgetItem(cfgItem);
    ifItem->setText(0, tr("接口描述符"));
    ifItem->setText(1, "0");
    ifItem->setText(2, tr("接口0"));
    addField(ifItem, "bLength", "9", tr("描述符长度"));
    addField(ifItem, "bDescriptorType", "0x04", tr("接口描述符"));
    addField(ifItem, "bInterfaceNumber", "0", tr("接口编号"));
    addField(ifItem, "bAlternateSetting", "0", tr("备选设置"));
    addField(ifItem, "bNumEndpoints", "2", tr("端点数量"));
    addField(ifItem, "bInterfaceClass", "0xFF",
             tr("厂商自定义(Vendor Specific)"));
    addField(ifItem, "bInterfaceSubClass", "0x00", tr("子类"));
    addField(ifItem, "bInterfaceProtocol", "0x00", tr("协议"));

    // 端点描述符 IN
    auto* epIn = new QTreeWidgetItem(ifItem);
    epIn->setText(0, tr("端点描述符"));
    epIn->setText(1, "0x81");
    epIn->setText(2, tr("IN端点, Bulk传输"));
    addField(epIn, "bLength", "7", tr("描述符长度"));
    addField(epIn, "bDescriptorType", "0x05", tr("端点描述符"));
    addField(epIn, "bEndpointAddress", "0x81", tr("IN, 端点1"));
    addField(epIn, "bmAttributes", "0x02", tr("Bulk传输"));
    addField(epIn, "wMaxPacketSize", "64", tr("最大包长"));
    addField(epIn, "bInterval", "0", tr("轮询间隔"));

    // 端点描述符 OUT
    auto* epOut = new QTreeWidgetItem(ifItem);
    epOut->setText(0, tr("端点描述符"));
    epOut->setText(1, "0x01");
    epOut->setText(2, tr("OUT端点, Bulk传输"));
    addField(epOut, "bLength", "7", tr("描述符长度"));
    addField(epOut, "bDescriptorType", "0x05", tr("端点描述符"));
    addField(epOut, "bEndpointAddress", "0x01", tr("OUT, 端点1"));
    addField(epOut, "bmAttributes", "0x02", tr("Bulk传输"));
    addField(epOut, "wMaxPacketSize", "64", tr("最大包长"));
    addField(epOut, "bInterval", "0", tr("轮询间隔"));

    m_descriptorTree->expandAll();

    // 原始数据hex view
    QString hexDump;
    hexDump += tr("USB设备 VID=%1 PID=%2\n\n").arg(vid, 4, 16, QChar('0'))
                   .arg(pid, 4, 16, QChar('0'));
    hexDump += tr("设备描述符 (18 bytes):\n");
    hexDump += "12 01 00 02 00 00 00 40 ";
    hexDump += QString("%1 %2 ").arg(vid & 0xFF, 2, 16, QChar('0'))
                                  .arg((vid >> 8) & 0xFF, 2, 16, QChar('0'));
    hexDump += QString("%1 %2 ").arg(pid & 0xFF, 2, 16, QChar('0'))
                                  .arg((pid >> 8) & 0xFF, 2, 16, QChar('0'));
    hexDump += "00 01 01 02 03 01\n\n";
    hexDump += tr("注: 原始数据为占位，需通过libusb获取真实描述符");
    m_rawView->setPlainText(hexDump);
}

/**
 * @brief 解析18字节设备描述符
 */
void UsbDescriptorViewer::addDeviceDescriptor(QTreeWidgetItem* parent,
                                               const QByteArray& raw) {
    if (raw.size() < 18) { return; }

    auto addField = [&](const QString& name, const QString& val,
                        const QString& desc) {
        auto* item = new QTreeWidgetItem(parent);
        item->setText(0, name);
        item->setText(1, val);
        item->setText(2, desc);
    };

    quint8 bLength = static_cast<quint8>(raw[0]);
    quint8 bDescriptorType = static_cast<quint8>(raw[1]);
    quint16 bcdUSB = static_cast<quint16>(static_cast<quint8>(raw[2]))
                     | (static_cast<quint16>(static_cast<quint8>(raw[3])) << 8);
    quint8 bDeviceClass = static_cast<quint8>(raw[4]);
    quint16 idVendor = static_cast<quint16>(static_cast<quint8>(raw[8]))
                       | (static_cast<quint16>(static_cast<quint8>(raw[9])) << 8);
    quint16 idProduct = static_cast<quint16>(static_cast<quint8>(raw[10]))
                        | (static_cast<quint16>(static_cast<quint8>(raw[11])) << 8);
    quint16 bcdDevice = static_cast<quint16>(static_cast<quint8>(raw[12]))
                        | (static_cast<quint16>(static_cast<quint8>(raw[13])) << 8);

    addField("bLength", QString::number(bLength), tr("描述符长度"));
    addField("bDescriptorType",
             QString("0x%1").arg(bDescriptorType, 2, 16, QChar('0')),
             tr("设备描述符类型"));
    addField("bcdUSB",
             QString("0x%1").arg(bcdUSB, 4, 16, QChar('0')),
             tr("USB规范版本"));
    addField("bDeviceClass",
             QString("0x%1").arg(bDeviceClass, 2, 16, QChar('0')),
             tr("设备类"));
    addField("idVendor",
             QString("0x%1").arg(idVendor, 4, 16, QChar('0')),
             tr("厂商ID"));
    addField("idProduct",
             QString("0x%1").arg(idProduct, 4, 16, QChar('0')),
             tr("产品ID"));
    addField("bcdDevice",
             QString("0x%1").arg(bcdDevice, 4, 16, QChar('0')),
             tr("设备版本"));
}

/** @brief 获取累计描述符刷新次数 */
quint64 UsbDescriptorViewer::totalDescriptorRefreshes() const
{
    return m_totalDescriptorRefreshes;
}

/** @brief 获取累计查看设备次数 */
quint64 UsbDescriptorViewer::totalDevicesViewed() const
{
    return m_totalDevicesViewed;
}

/** @brief 重置所有统计计数器 */
void UsbDescriptorViewer::resetStatistics()
{
    m_totalDescriptorRefreshes = 0;
    m_totalDevicesViewed = 0;
}
