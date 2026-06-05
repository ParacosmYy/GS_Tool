/**
 * @file NetworkTopologyWidget.h
 * @brief 网络拓扑可视化组件 - 设备节点图绘制与交互
 *
 * 职责:
 *   1. 以 QPainter 自绘方式展示网络拓扑图
 *   2. 本机居中，已发现设备环绕排列
 *   3. 颜色编码：在线=绿、离线=红、未知=灰
 *   4. 鼠标悬停显示设备详情 Tooltip
 *   5. 点击选中设备
 */

#pragma once
#include <QWidget>
#include <QVector>
#include <QString>
#include <QPoint>

#include "utils/network/NetworkTypes.h"

/**
 * @brief 网络拓扑可视化组件
 *
 * 自绘 QWidget，将扫描发现的网络设备以节点图形式呈现。
 * 本机接口居于画面中心，其余设备按发现顺序环绕排列，
 * 连线表示同一子网内的逻辑连接。
 */
class NetworkTopologyWidget : public QWidget {
    Q_OBJECT
public:
    /**
     * @brief 构造拓扑可视化组件
     * @param parent 父 widget
     */
    explicit NetworkTopologyWidget(QWidget *parent = nullptr);

    /** @brief 析构函数 */
    ~NetworkTopologyWidget() override;

    /**
     * @brief 批量设置设备列表（替换原有）
     * @param devices 网络设备列表
     */
    void setDevices(const QVector<NetworkDevice> &devices);

    /**
     * @brief 设置本机网络接口列表
     * @param ifaces 接口列表
     */
    void setLocalInterfaces(const QVector<NetworkInterface> &ifaces);

    /**
     * @brief 追加单个设备到拓扑图
     * @param device 网络设备
     */
    void addDevice(const NetworkDevice &device);

    /** @brief 清空所有设备节点 */
    void clearDevices();

    // ---- 统计接口 ----
    /** @brief 获取累计重绘次数 @return 计数 */
    quint64 totalRepaints() const;
    /** @brief 获取累计设备点击次数 @return 计数 */
    quint64 totalDeviceClicks() const;
    /** @brief 重置拓扑组件统计计数器 */
    void resetStatistics();

signals:
    /** @brief 设备被点击选中 @param device 被选中的设备 */
    void deviceClicked(const NetworkDevice &device);

protected:
    /** @brief 自绘拓扑图 */
    void paintEvent(QPaintEvent *event) override;

    /** @brief 鼠标移动事件，更新 Tooltip */
    void mouseMoveEvent(QMouseEvent *event) override;

    /** @brief 鼠标点击事件，选中设备 */
    void mousePressEvent(QMouseEvent *event) override;

private:
    /**
     * @brief 计算所有节点的布局坐标
     * @param center 画布中心点
     * @param radius 环绕半径
     */
    void calculateLayout(const QPoint &center, int radius);

    /**
     * @brief 绘制单个设备节点
     * @param painter 画笔
     * @param pos      节点中心坐标
     * @param device   设备信息
     * @param selected 是否被选中
     */
    void drawDeviceNode(QPainter &painter, const QPoint &pos,
                        const NetworkDevice &device, bool selected);

    /**
     * @brief 绘制本机节点（居中，特殊样式）
     * @param painter 画笔
     * @param center  中心坐标
     */
    void drawLocalNode(QPainter &painter, const QPoint &center);

    /**
     * @brief 查找指定坐标处的设备
     * @param pos 鼠标坐标
     * @return 设备指针，未命中返回 nullptr
     */
    const NetworkDevice *deviceAt(const QPoint &pos) const;

    /**
     * @brief 根据设备在线状态返回颜色
     * @param device 设备信息
     * @return 状态颜色
     */
    QColor statusColor(const NetworkDevice &device) const;

    QVector<NetworkDevice>  m_devices;       ///< 已发现设备列表
    QVector<NetworkInterface> m_localIfaces; ///< 本机接口列表
    QVector<QPoint>         m_nodePositions; ///< 各设备节点坐标
    QPoint                  m_centerNode;    ///< 本机节点坐标
    int                     m_selectedIndex = -1; ///< 选中设备索引，-1 无选中
    int                     m_nodeRadius = 28;    ///< 节点圆形半径

    // ---- 统计计数器 ----
    quint64 m_totalRepaints = 0;     ///< 累计重绘次数
    quint64 m_totalDeviceClicks = 0; ///< 累计设备点击次数
};
