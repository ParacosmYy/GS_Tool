/**
 * @file NetworkTopologyWidget.cpp
 * @brief 网络拓扑可视化组件实现 - 设备节点图绘制与交互
 */

#include "utils/network/NetworkTopologyWidget.h"

#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QToolTip>
#include <QtMath>

// ============================================================================
// 构造 / 析构
// ============================================================================

/** @brief 构造拓扑可视化组件 @param parent 父 widget */
NetworkTopologyWidget::NetworkTopologyWidget(QWidget *parent)
    : QWidget(parent) {
    setObjectName("NetworkTopologyWidget");
    setMouseTracking(true);
    setMinimumSize(400, 300);
}

/** @brief 析构函数 */
NetworkTopologyWidget::~NetworkTopologyWidget() = default;

// ============================================================================
// 数据接口
// ============================================================================

/**
 * @brief 批量设置设备列表（替换原有）
 * @param devices 网络设备列表
 */
void NetworkTopologyWidget::setDevices(const QVector<NetworkDevice> &devices) {
    m_devices = devices;
    m_selectedIndex = -1;
    update();
}

/**
 * @brief 设置本机网络接口列表
 * @param ifaces 接口列表
 */
void NetworkTopologyWidget::setLocalInterfaces(
    const QVector<NetworkInterface> &ifaces) {
    m_localIfaces = ifaces;
    update();
}

/**
 * @brief 追加单个设备到拓扑图
 * @param device 网络设备
 */
void NetworkTopologyWidget::addDevice(const NetworkDevice &device) {
    m_devices.append(device);
    update();
}

/** @brief 清空所有设备节点 */
void NetworkTopologyWidget::clearDevices() {
    m_devices.clear();
    m_selectedIndex = -1;
    update();
}

// ============================================================================
// 绘制
// ============================================================================

/** @brief 自绘拓扑图：本机居中、设备环绕、连线、颜色编码 */
void NetworkTopologyWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event)
    ++m_totalRepaints;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor("#1a1a2e"));

    const QPoint center(width() / 2, height() / 2);
    m_centerNode = center;

    // 根据设备数量动态计算半径
    const int maxRadius = qMin(width(), height()) / 2 - m_nodeRadius - 30;
    const int radius = m_devices.isEmpty() ? maxRadius
                      : qMin(maxRadius, qMax(80, m_nodeRadius * 3));

    calculateLayout(center, radius);

    // 先画连接线（底层）
    painter.setPen(QPen(QColor(80, 80, 120, 100), 1.5, Qt::DashLine));
    for (const auto &pos : m_nodePositions) {
        painter.drawLine(center, pos);
    }

    // 画本机节点
    drawLocalNode(painter, center);

    // 画设备节点
    for (int i = 0; i < m_devices.size(); ++i) {
        drawDeviceNode(painter, m_nodePositions[i],
                       m_devices[i], i == m_selectedIndex);
    }
}

/**
 * @brief 计算环绕布局坐标
 * @param center 画布中心
 * @param radius 环绕半径
 */
void NetworkTopologyWidget::calculateLayout(const QPoint &center, int radius) {
    m_nodePositions.clear();
    const int n = m_devices.size();
    if (n == 0) return;

    const double step = (2.0 * M_PI) / n;
    for (int i = 0; i < n; ++i) {
        const double angle = step * i - M_PI / 2;
        const int x = center.x() + static_cast<int>(radius * qCos(angle));
        const int y = center.y() + static_cast<int>(radius * qSin(angle));
        m_nodePositions.append(QPoint(x, y));
    }
}

/**
 * @brief 绘制单个设备节点
 * @param painter 画笔
 * @param pos      节点中心坐标
 * @param device   设备信息
 * @param selected 是否被选中
 */
void NetworkTopologyWidget::drawDeviceNode(QPainter &painter,
    const QPoint &pos, const NetworkDevice &device, bool selected) {
    const QColor color = statusColor(device);

    // 选中光晕
    if (selected) {
        QRadialGradient glow(pos, m_nodeRadius + 8);
        glow.setColorAt(0.0, QColor(color.red(), color.green(),
                                     color.blue(), 120));
        glow.setColorAt(1.0, QColor(color.red(), color.green(),
                                     color.blue(), 0));
        painter.setBrush(glow);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(pos, m_nodeRadius + 8, m_nodeRadius + 8);
    }

    // 节点圆形
    painter.setBrush(color.darker(140));
    painter.setPen(QPen(color, 2));
    painter.drawEllipse(pos, m_nodeRadius, m_nodeRadius);

    // IP 地址标签
    painter.setPen(Qt::white);
    QFont font = painter.font();
    font.setPixelSize(10);
    font.setBold(true);
    painter.setFont(font);
    const QRect textRect(pos.x() - m_nodeRadius, pos.y() - 6,
                         m_nodeRadius * 2, 12);
    painter.drawText(textRect, Qt::AlignCenter, device.ipAddress);

    // 主机名标签（节点下方）
    if (!device.hostname.isEmpty()) {
        font.setPixelSize(8);
        font.setBold(false);
        painter.setFont(font);
        painter.setPen(QColor(180, 180, 200));
        const QRect nameRect(pos.x() - 40, pos.y() + m_nodeRadius + 2, 80, 14);
        painter.drawText(nameRect, Qt::AlignCenter, device.hostname);
    }
}

/**
 * @brief 绘制本机节点（居中，特殊样式）
 * @param painter 画笔
 * @param center  中心坐标
 */
void NetworkTopologyWidget::drawLocalNode(QPainter &painter,
    const QPoint &center) {
    const int r = m_nodeRadius + 6;

    // 外圈光晕
    QRadialGradient glow(center, r + 12);
    glow.setColorAt(0.0, QColor(100, 180, 255, 80));
    glow.setColorAt(1.0, QColor(100, 180, 255, 0));
    painter.setBrush(glow);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(center, r + 12, r + 12);

    // 主圆形
    painter.setBrush(QColor(60, 120, 200));
    painter.setPen(QPen(QColor(100, 180, 255), 2.5));
    painter.drawEllipse(center, r, r);

    // 标签
    painter.setPen(Qt::white);
    QFont font = painter.font();
    font.setPixelSize(11);
    font.setBold(true);
    painter.setFont(font);
    painter.drawText(QRect(center.x() - r, center.y() - 7, r * 2, 14),
                     Qt::AlignCenter, tr("Local"));

    // 显示第一个接口的 IP
    if (!m_localIfaces.isEmpty()) {
        font.setPixelSize(8);
        font.setBold(false);
        painter.setFont(font);
        painter.setPen(QColor(180, 210, 255));
        const QRect ipRect(center.x() - r, center.y() + 6, r * 2, 12);
        painter.drawText(ipRect, Qt::AlignCenter,
                         m_localIfaces.first().ipAddress);
    }
}

// ============================================================================
// 鼠标交互
// ============================================================================

/** @brief 鼠标移动：更新 Tooltip 显示设备详情 */
void NetworkTopologyWidget::mouseMoveEvent(QMouseEvent *event) {
    const NetworkDevice *dev = deviceAt(event->pos());
    if (dev) {
        QStringList tip;
        tip << tr("IP: %1").arg(dev->ipAddress);
        if (!dev->macAddress.isEmpty())
            tip << tr("MAC: %1").arg(dev->macAddress);
        if (!dev->hostname.isEmpty())
            tip << tr("Host: %1").arg(dev->hostname);
        if (dev->port > 0)
            tip << tr("Port: %1").arg(dev->port);
        if (!dev->service.isEmpty())
            tip << tr("Service: %1").arg(dev->service);
        tip << tr("Status: %1").arg(dev->isOnline ? tr("Online")
                                                   : tr("Offline"));
        tip << tr("Response: %1 ms").arg(dev->responseTimeMs);
        QToolTip::showText(event->globalPosition().toPoint(),
                           tip.join("\n"), this);
    } else {
        QToolTip::hideText();
    }
    QWidget::mouseMoveEvent(event);
}

/** @brief 鼠标点击：选中设备并发射信号 */
void NetworkTopologyWidget::mousePressEvent(QMouseEvent *event) {
    const NetworkDevice *dev = deviceAt(event->pos());
    if (dev) {
        ++m_totalDeviceClicks;
        // 查找索引
        for (int i = 0; i < m_devices.size(); ++i) {
            if (m_devices[i].ipAddress == dev->ipAddress) {
                m_selectedIndex = i;
                break;
            }
        }
        emit deviceClicked(*dev);
        update();
    } else {
        m_selectedIndex = -1;
        update();
    }
    QWidget::mousePressEvent(event);
}

// ============================================================================
// 内部工具
// ============================================================================

/**
 * @brief 查找指定坐标处的设备
 * @param pos 鼠标坐标
 * @return 设备指针，未命中返回 nullptr
 */
const NetworkDevice *NetworkTopologyWidget::deviceAt(const QPoint &pos) const {
    for (int i = 0; i < m_nodePositions.size(); ++i) {
        const QPoint diff = pos - m_nodePositions[i];
        if (diff.x() * diff.x() + diff.y() * diff.y()
                <= m_nodeRadius * m_nodeRadius) {
            return &m_devices[i];
        }
    }
    return nullptr;
}

/**
 * @brief 根据设备在线状态返回颜色
 * @param device 设备信息
 * @return 状态颜色（绿/红/灰）
 */
QColor NetworkTopologyWidget::statusColor(const NetworkDevice &device) const {
    if (device.isOnline)  return QColor(76, 175, 80);   // 在线=绿
    if (!device.macAddress.isEmpty()) return QColor(244, 67, 54); // 离线=红
    return QColor(158, 158, 158);                       // 未知=灰
}
