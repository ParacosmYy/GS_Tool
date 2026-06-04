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

// loadDescriptors/addDeviceDescriptor/统计getter/resetStatistics已移至 UsbDescriptorViewerTree.cpp
