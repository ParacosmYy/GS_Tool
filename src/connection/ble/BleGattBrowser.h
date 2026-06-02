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

class BleConnection;

/**
 * @brief GATT服务/特征浏览器面板
 *
 * 左侧为GATT服务树，右侧为特征值显示和操作按钮。
 * 通过setConnection()绑定BleConnection实例。
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

private:
    /** @brief GATT服务/特征树控件 */
    QTreeWidget* m_serviceTree;

    /** @brief 特征值显示区域 */
    QTextEdit* m_valueDisplay;

    /** @brief 读取特征值按钮 */
    QPushButton* m_readBtn;

    /** @brief 写入特征值按钮 */
    QPushButton* m_writeBtn;

    /** @brief 当前绑定的BLE连接 */
    BleConnection* m_connection = nullptr;
};

#endif // BLEGATTBROWSER_H
