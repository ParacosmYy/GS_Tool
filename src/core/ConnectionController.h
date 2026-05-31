#ifndef CONNECTIONCONTROLLER_H
#define CONNECTIONCONTROLLER_H

#include <QObject>
#include <QVariantMap>
#include "connection/IConnection.h"
#include "core/ConnectionManager.h"

class SendController;
class OtaManager;
class RecordingController;

// 连接控制器 - 从MainWindow中提取的连接管理业务逻辑
// 职责:
//   1. 创建/断开串口连接（从SerialConfigPanel读取参数）
//   2. 创建网络连接（TCP/UDP）
//   3. 将连接状态同步给SendController/OtaManager/RecordingController
//   4. 转发连接状态变化和数据接收信号到MainWindow用于UI更新
class ConnectionController : public QObject {
    Q_OBJECT

public:
    explicit ConnectionController(ConnectionManager* connMgr, QObject* parent = nullptr);

    // 依赖注入: 设置需要连接状态通知的对象
    void setSendController(SendController* ctrl);
    void setOtaManager(OtaManager* mgr);
    void setRecordingController(RecordingController* ctrl);

    // 串口连接/断开
    void connectSerial(const QVariantMap& serialParams);
    void disconnectSerial();

    // 网络连接
    void connectNetwork(ConnectionType type);

    // 获取当前连接
    IConnection* currentConnection() const;

    // 运行时线路控制（DTR/RTS）- 透传给当前串口连接
    void setDtr(bool enabled);
    void setRts(bool enabled);

signals:
    // 通知MainWindow更新UI
    void connectionStateChanged(ConnectionState state, const QString& connName);
    void dataReceived(const QByteArray& data);
    void statusBarUpdateRequested();

    // 连接失败通知（MainWindow显示QMessageBox）
    void connectionFailed(const QString& title, const QString& message);

private slots:
    void onConnectionStateChanged(ConnectionState state);
    void onDataReceived(const QByteArray& data);

private:
    void connectSignals(IConnection* conn);

    ConnectionManager* m_connManager;
    IConnection* m_currentConn = nullptr;

    SendController* m_sendController = nullptr;
    OtaManager* m_otaManager = nullptr;
    RecordingController* m_recordingController = nullptr;
};

#endif // CONNECTIONCONTROLLER_H
