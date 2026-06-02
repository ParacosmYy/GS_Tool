/**
 * @file BleGattBrowser.h
 * @brief GATT服务浏览器 — 展示BLE设备的GATT服务/特征树并提供读写操作
 *
 * 职责: 以树形控件展示GATT层级，选中特征后可读取/写入其值，
 * 提供值的十六进制/文本双视图。
 */
#ifndef BLEGATTBROWSER_H
#define BLEGATTBROWSER_H

#include <QWidget>
#include <QTreeWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>

class BleConnection;
class BleGattModel;

/**
 * @brief GATT服务/特征浏览器面板
 *
 * 左侧为GATT服务树，右侧为特征值显示(十六进制dump)，
 * 底部为读写操作按钮和写入值输入框。
 */
class BleGattBrowser : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief 构造GATT浏览器
     * @param parent 父控件
     */
    explicit BleGattBrowser(QWidget* parent = nullptr);

    /**
     * @brief 绑定BLE连接实例
     * @param connection BLE连接对象
     */
    void setConnection(BleConnection* connection);

private slots:
    /** @brief 树控件选中项变化处理 */
    void onTreeItemChanged();

    /** @brief 读取按钮点击处理 */
    void onReadClicked();

    /** @brief 写入按钮点击处理 */
    void onWriteClicked();

    /** @brief 服务发现完成后刷新树 */
    void onServicesDiscovered(const QStringList& services);

private:
    /**
     * @brief 生成模拟GATT树数据并填充到控件
     * @param services 服务UUID列表
     */
    void populateTree(const QStringList& services);

    /**
     * @brief 将字节数据格式化为十六进制dump显示
     * @param data 原始字节
     * @return 格式化后的十六进制文本
     */
    QString formatHexDump(const QByteArray& data) const;

    /** @brief GATT服务/特征树控件 */
    QTreeWidget* m_serviceTree;

    /** @brief 特征值显示区域（十六进制dump） */
    QTextEdit* m_valueDisplay;

    /** @brief 当前选中特征的UUID标签 */
    QLabel* m_selectedLabel;

    /** @brief 写入值输入框 */
    QLineEdit* m_writeInput;

    /** @brief 读取特征值按钮 */
    QPushButton* m_readBtn;

    /** @brief 写入特征值按钮 */
    QPushButton* m_writeBtn;

    /** @brief 当前绑定的BLE连接 */
    BleConnection* m_connection = nullptr;

    /** @brief 当前选中特征UUID */
    QString m_selectedUuid;
};

#endif // BLEGATTBROWSER_H
