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

    explicit BleGattBrowser(QWidget* parent = nullptr);
    void setConnection(BleConnection* connection);

    // ---- 统计接口 ----
    quint64 totalServiceDiscoveries() const;
    quint64 totalCharacteristicReads() const;
    quint64 totalCharacteristicWrites() const;
    quint64 totalNotificationsReceived() const;
    quint64 totalDescriptorOps() const;
    quint64 totalBytesRead() const;
    quint64 totalBytesWritten() const;
    qint64 lastDiscoveryDurationMs() const;
    void resetStatistics();

signals:
    void notificationReceived(const QString& charUuid, const QByteArray& value);
    void connectionStatusChanged(bool connected);
    void characteristicReadComplete(const QString& charUuid, const QByteArray& value);
    void characteristicWritten(const QString& charUuid, qint64 bytesWritten);

private slots:
    void onTreeItemChanged();
    void onReadClicked();
    void onWriteClicked();
    void onNotifyToggled(bool checked);
    void onServicesDiscovered(const QStringList& services);
    void onRefreshClicked();
    void onConnectionStateChanged();
    void onSimulateNotification();
    void onWriteFormatChanged(int index);

private:
    void populateTree(const QStringList& services);
    QString formatHexDump(const QByteArray& data) const;
    QByteArray parseHexInput(const QString& hexStr) const;
    CharProperties getItemProperties(QTreeWidgetItem* item) const;
    void updateButtonStates();
    void addDescriptors(QTreeWidgetItem* charItem, int charIndex);
    void updateConnectionStatus();

    // ---- UI控件 ----
    QTreeWidget*  m_serviceTree;
    QTextEdit*    m_valueDisplay;
    QLabel*       m_selectedLabel;
    QLineEdit*    m_writeInput;
    QPushButton*  m_readBtn;
    QPushButton*  m_writeBtn;
    QPushButton*  m_refreshBtn;
    QCheckBox*    m_notifyCheck;
    QComboBox*    m_writeFormatCombo;
    QLabel*       m_connectionStatus;
    QLabel*       m_serviceCountLabel;

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
