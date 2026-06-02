/**
 * @file UsbDescriptorViewer.cpp
 * @brief USB描述符查看器实现
 *
 * 通过libusb获取设备描述符并以树形结构展示。
 */
#include "connection/usb/UsbDescriptorViewer.h"
#include <QSplitter>
#include <QHeaderView>

UsbDescriptorViewer::UsbDescriptorViewer(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("UsbDescriptorViewer");

    auto* splitter = new QSplitter(Qt::Horizontal, this);

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

void UsbDescriptorViewer::setDevice(quint16 vid, quint16 pid) {
    loadDescriptors(vid, pid);
}

void UsbDescriptorViewer::loadDescriptors(quint16 vid, quint16 pid) {
    m_descriptorTree->clear();
    m_rawView->clear();

    // TODO: 使用libusb获取真实描述符数据
    // 占位：创建标准描述符树结构

    // 设备描述符
    auto* devItem = new QTreeWidgetItem(m_descriptorTree);
    devItem->setText(0, tr("设备描述符"));
    devItem->setText(1, QString("VID:%1 PID:%2")
                          .arg(vid, 4, 16, QChar('0'))
                          .arg(pid, 4, 16, QChar('0')));
    devItem->setText(2, tr("USB设备基本信息"));

    // 设备描述符字段
    auto addField = [&](QTreeWidgetItem* parent, const QString& name,
                        const QString& val, const QString& desc) {
        auto* item = new QTreeWidgetItem(parent);
        item->setText(0, name);
        item->setText(1, val);
        item->setText(2, desc);
    };

    addField(devItem, "bLength", "18", tr("描述符长度"));
    addField(devItem, "bDescriptorType", "1", tr("设备描述符类型"));
    addField(devItem, "idVendor",
             QString("0x%1").arg(vid, 4, 16, QChar('0')), tr("厂商ID"));
    addField(devItem, "idProduct",
             QString("0x%1").arg(pid, 4, 16, QChar('0')), tr("产品ID"));

    // 配置描述符（占位）
    auto* cfgItem = new QTreeWidgetItem(devItem);
    cfgItem->setText(0, tr("配置描述符"));
    cfgItem->setText(1, "1");
    cfgItem->setText(2, tr("配置编号1"));

    // 接口描述符（占位）
    auto* ifItem = new QTreeWidgetItem(cfgItem);
    ifItem->setText(0, tr("接口描述符"));
    ifItem->setText(1, "0");
    ifItem->setText(2, tr("接口0"));

    // 端点描述符（占位）
    auto* epItem = new QTreeWidgetItem(ifItem);
    epItem->setText(0, tr("端点描述符"));
    epItem->setText(1, "0x81");
    epItem->setText(2, tr("IN端点, Bulk传输"));

    m_descriptorTree->expandAll();

    m_rawView->setPlainText(
        tr("USB设备 VID=%1 PID=%2\n原始描述符数据尚未获取")
            .arg(vid, 4, 16, QChar('0'))
            .arg(pid, 4, 16, QChar('0')));
}

void UsbDescriptorViewer::addDeviceDescriptor(QTreeWidgetItem* parent,
                                               const QByteArray& raw) {
    if (raw.size() < 18) { return; }
    Q_UNUSED(parent)
    // TODO: 解析18字节设备描述符
}
