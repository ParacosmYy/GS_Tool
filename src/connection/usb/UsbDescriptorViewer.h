/**
 * @file UsbDescriptorViewer.h
 * @brief USB描述符查看器 — 可视化展示USB设备描述符
 *
 * 以树形结构展示设备描述符、配置描述符、接口描述符和端点描述符。
 */
#ifndef USB_DESCRIPTOR_VIEWER_H
#define USB_DESCRIPTOR_VIEWER_H

#include <QWidget>
#include <QTreeWidget>
#include <QTextEdit>
#include <QElapsedTimer>

/**
 * @brief USB描述符查看器控件
 * 加载并展示指定USB设备的完整描述符层次结构。
 */
class UsbDescriptorViewer : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief 构造USB描述符查看器
     * @param parent 父控件
     */
    explicit UsbDescriptorViewer(QWidget* parent = nullptr);

    /**
     * @brief 设置要查看的USB设备
     * @param vid 厂商ID
     * @param pid 产品ID
     */
    void setDevice(quint16 vid, quint16 pid);

    /** @brief 获取累计描述符刷新次数 @return 刷新总次数 */
    quint64 totalDescriptorRefreshes() const;

    /** @brief 获取累计查看设备次数 @return 查看总次数 */
    quint64 totalDevicesViewed() const;

    /** @brief 获取累计树更新次数 @return 树更新总次数 */
    quint64 totalTreeUpdates() const { return m_totalTreeUpdates; }

    /** @brief 获取平均描述符解析耗时(ms) @return 平均解析时间 */
    double avgParseTimeMs() const;

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

private:
    /**
     * @brief 加载设备描述符并填充树
     * @param vid 厂商ID
     * @param pid 产品ID
     */
    void loadDescriptors(quint16 vid, quint16 pid);

    /**
     * @brief 添加设备描述符节点
     * @param parent 父节点
     * @param raw 原始描述符数据
     */
    void addDeviceDescriptor(QTreeWidgetItem* parent,
                              const QByteArray& raw);

    QTreeWidget* m_descriptorTree = nullptr; ///< 描述符结构树
    QTextEdit*   m_rawView       = nullptr; ///< 原始数据视图

    // ---- 统计计数器 ----
    quint64 m_totalDescriptorRefreshes = 0;  ///< 累计描述符刷新次数
    quint64 m_totalDevicesViewed = 0;        ///< 累计查看设备次数
    quint64 m_totalNodesAdded = 0;           ///< 累计添加的树节点总数(设备/配置/接口/端点)
    quint64 m_totalRawBytesViewed = 0;       ///< 累计查看的原始描述符字节数
    quint64 m_totalTreeUpdates = 0;          ///< 累计树更新次数
    qint64  m_totalParseTimeUs = 0;          ///< 累计解析总耗时(微秒)

public:
    /** @brief 获取累计添加树节点总数 @return 节点计数 */
    quint64 totalNodesAdded() const { return m_totalNodesAdded; }
    /** @brief 获取累计查看原始描述符字节数 @return 字节数 */
    quint64 totalRawBytesViewed() const { return m_totalRawBytesViewed; }
};

#endif // USB_DESCRIPTOR_VIEWER_H
