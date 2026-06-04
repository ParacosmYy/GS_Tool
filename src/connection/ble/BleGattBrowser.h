/**
 * @file BleGattBrowser.h
 * @brief GATT服务浏览器 — 展示BLE设备的GATT服务/特征树并提供读写操作
 *
 * 职责: 以树形控件展示GATT层级(Service→Characteristic→Descriptor)，
 * 选中特征后可读取/写入/订阅通知其值，提供值的十六进制/文本双视图。
 */
#ifndef BLEGATTBROWSER_H
#define BLEGATTBROWSER_H

#include <QWidget>
#include <QTreeWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QTimer>
#include <QElapsedTimer>

class BleConnection;

/**
 * @brief GATT服务/特征浏览器面板
 * 左侧GATT服务树，右侧特征值显示(hex dump)，底部读写操作栏。
 */
class BleGattBrowser : public QWidget {
    Q_OBJECT

public:
    /// GATT特征属性标志
    enum CharProperty : uint32_t {
        PropRead       = 0x0001,  ///< 可读取
        PropWrite      = 0x0002,  ///< 可写入(需响应)
        PropWriteNoRsp = 0x0004,  ///< 可写入(无响应)
        PropNotify     = 0x0008,  ///< 可通知
        PropIndicate   = 0x0010,  ///< 可指示
    };
    Q_DECLARE_FLAGS(CharProperties, CharProperty)

    /** @brief 构造GATT浏览器面板 @param parent 父控件 */
    explicit BleGattBrowser(QWidget* parent = nullptr);
    /** @brief 设置BLE连接实例 @param connection BLE连接指针 */
    void setConnection(BleConnection* connection);

    // ---- 统计接口 ----
    /** @brief 获取累计服务发现次数 @return 服务发现总次数 */
    quint64 totalServiceDiscoveries() const;
    /** @brief 获取累计特征读取次数 @return 特征读取总次数 */
    quint64 totalCharacteristicReads() const;
    /** @brief 获取累计特征写入次数 @return 特征写入总次数 */
    quint64 totalCharacteristicWrites() const;
    /** @brief 获取累计通知接收次数 @return 通知接收总次数 */
    quint64 totalNotificationsReceived() const;
    /** @brief 获取累计描述符操作次数 @return 描述符操作总次数 */
    quint64 totalDescriptorOps() const;
    /** @brief 获取累计读取字节数 @return 读取字节总数 */
    quint64 totalBytesRead() const;
    /** @brief 获取累计写入字节数 @return 写入字节总数 */
    quint64 totalBytesWritten() const;
    /** @brief 获取最近一次服务发现耗时(ms) @return 最近发现耗时 */
    qint64 lastDiscoveryDurationMs() const;
    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    /** @brief 收到BLE通知 @param charUuid 特征UUID @param value 通知数据 */
    void notificationReceived(const QString& charUuid, const QByteArray& value);
    /** @brief 连接状态变化 @param connected true=已连接 */
    void connectionStatusChanged(bool connected);
    /** @brief 特征读取完成 @param charUuid 特征UUID @param value 读取到的数据 */
    void characteristicReadComplete(const QString& charUuid, const QByteArray& value);
    /** @brief 特征写入完成 @param charUuid 特征UUID @param bytesWritten 写入字节数 */
    void characteristicWritten(const QString& charUuid, qint64 bytesWritten);

private slots:
    /** @brief 树控件选中项变化回调 */
    void onTreeItemChanged();
    /** @brief 读取按钮点击回调 */
    void onReadClicked();
    /** @brief 写入按钮点击回调 */
    void onWriteClicked();
    /** @brief 通知订阅开关切换回调 @param checked 是否启用通知 */
    void onNotifyToggled(bool checked);
    /** @brief GATT服务发现完成回调 @param services 已发现的服务UUID列表 */
    void onServicesDiscovered(const QStringList& services);
    /** @brief 刷新按钮点击回调 */
    void onRefreshClicked();
    /** @brief 连接状态变化回调 */
    void onConnectionStateChanged();
    /** @brief 模拟通知数据到达回调(模拟模式下使用) */
    void onSimulateNotification();
    /** @brief 写入格式切换回调 @param index 格式索引 */
    void onWriteFormatChanged(int index);

private:
    /** @brief 填充GATT服务树 @param services 服务UUID列表 */
    void populateTree(const QStringList& services);
    /** @brief 格式化十六进制数据转储 @param data 原始数据 @return HEX转储字符串 */
    QString formatHexDump(const QByteArray& data) const;
    /** @brief 解析用户输入的HEX字符串 @param hexStr HEX字符串 @return 解析后的字节数组 */
    QByteArray parseHexInput(const QString& hexStr) const;
    /** @brief 获取树项的特征属性标志 @param item 树控件项 @return 属性标志集合 */
    CharProperties getItemProperties(QTreeWidgetItem* item) const;
    /** @brief 根据选中项更新按钮启用状态 */
    void updateButtonStates();
    /** @brief 为特征项添加描述符子项 @param charItem 特征树项 @param charIndex 特征索引 */
    void addDescriptors(QTreeWidgetItem* charItem, int charIndex);
    /** @brief 更新连接状态显示标签 */
    void updateConnectionStatus();

    // ---- UI控件 ----
    QTreeWidget*  m_serviceTree;            ///< GATT服务/特征树控件
    QTextEdit*    m_valueDisplay;           ///< 特征值显示区(HEX转储)
    QLabel*       m_selectedLabel;          ///< 当前选中特征标签
    QLineEdit*    m_writeInput;             ///< 写入数据输入框
    QPushButton*  m_readBtn;               ///< 读取按钮
    QPushButton*  m_writeBtn;              ///< 写入按钮
    QPushButton*  m_refreshBtn;            ///< 刷新按钮
    QCheckBox*    m_notifyCheck;           ///< 通知订阅开关
    QComboBox*    m_writeFormatCombo;      ///< 写入格式选择(HEX/文本)
    QLabel*       m_connectionStatus;      ///< 连接状态标签
    QLabel*       m_serviceCountLabel;     ///< 服务计数标签

    // ---- 状态 ----
    BleConnection* m_connection = nullptr;       ///< 当前BLE连接
    QString        m_selectedUuid;               ///< 当前选中特征UUID
    CharProperties m_selectedProps;              ///< 当前选中特征属性
    QStringList    m_subscribedUuids;            ///< 已订阅通知的特征UUID列表
    QTimer*        m_notificationTimer;          ///< 模拟通知定时器
    QElapsedTimer  m_discoveryTimer;             ///< 服务发现计时

    // ---- 统计计数器 ----
    quint64 m_totalServiceDiscoveries = 0;       ///< 服务发现次数
    quint64 m_totalCharacteristicReads = 0;      ///< 特征读取次数
    quint64 m_totalCharacteristicWrites = 0;     ///< 特征写入次数
    quint64 m_totalNotificationsReceived = 0;    ///< 通知接收次数
    quint64 m_totalDescriptorOps = 0;            ///< 描述符操作次数
    quint64 m_totalBytesRead = 0;                ///< 读取字节数
    quint64 m_totalBytesWritten = 0;             ///< 写入字节数
    qint64  m_lastDiscoveryDurationMs = 0;       ///< 最后发现耗时
};

Q_DECLARE_OPERATORS_FOR_FLAGS(BleGattBrowser::CharProperties)

#endif // BLEGATTBROWSER_H
