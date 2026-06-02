/**
 * @file BleGattBrowser.cpp
 * @brief GATT服务浏览器实现
 */

#include "connection/ble/BleGattBrowser.h"
#include "connection/ble/BleConnection.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>

BleGattBrowser::BleGattBrowser(QWidget* parent)
    : QWidget(parent)
    , m_serviceTree(new QTreeWidget(this))
    , m_valueDisplay(new QTextEdit(this))
    , m_readBtn(new QPushButton(tr("读取"), this))
    , m_writeBtn(new QPushButton(tr("写入"), this))
{
    setObjectName("BleGattBrowser");

    m_serviceTree->setHeaderLabel(tr("GATT 服务/特征"));
    m_valueDisplay->setReadOnly(true);
    m_valueDisplay->setPlaceholderText(tr("选择特征后点击读取查看值"));

    // 按钮布局
    auto btnLayout = new QHBoxLayout();
    btnLayout->addWidget(m_readBtn);
    btnLayout->addWidget(m_writeBtn);
    btnLayout->addStretch();

    // 主布局: 左树右值
    auto mainLayout = new QHBoxLayout(this);
    mainLayout->addWidget(m_serviceTree, 1);
    auto rightLayout = new QVBoxLayout();
    rightLayout->addWidget(m_valueDisplay, 1);
    rightLayout->addLayout(btnLayout);
    mainLayout->addLayout(rightLayout, 1);
}

void BleGattBrowser::setConnection(BleConnection* connection)
{
    m_connection = connection;
    // TODO: 连接信号，刷新服务树
}
