#ifndef OTAWIDGET_H
#define OTAWIDGET_H

#include <QWidget>
#include <QProgressBar>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QGroupBox>
#include <QTextEdit>
#include <QElapsedTimer>
#include "ota/OtaManager.h"

// OTA升级操作面板
// 提供文件选择、协议选择、进度显示、日志输出
class OtaWidget : public QWidget {
    Q_OBJECT

public:
    explicit OtaWidget(OtaManager* manager, QWidget* parent = nullptr);

    // 设置关联的连接（切换连接时调用）
    void setConnection(IConnection* conn);

private slots:
    void onBrowseFile();
    void onStartTransfer();
    void onCancelTransfer();
    void onProgress(int percent, qint64 bytesSent, qint64 totalBytes);
    void onTransferComplete();
    void onTransferError(const QString& reason);

private:
    void setupUI();
    void appendLog(const QString& msg);
    void setTransferring(bool transferring);

    OtaManager* m_manager;

    // 文件选择
    QLineEdit* m_filePathEdit;
    QPushButton* m_browseBtn;

    // 协议选择
    QComboBox* m_protocolCombo;

    // 传输控制
    QPushButton* m_startBtn;
    QPushButton* m_cancelBtn;

    // 进度显示
    QProgressBar* m_progressBar;
    QLabel* m_statusLbl;
    QLabel* m_speedLbl;
    QLabel* m_etaLbl;

    // 日志
    QTextEdit* m_logView;

    // 传输计时
    QElapsedTimer m_transferTimer;
    qint64 m_lastBytesSent = 0;
};

#endif // OTAWIDGET_H
