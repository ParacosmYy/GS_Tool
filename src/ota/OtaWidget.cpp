#include "ota/OtaWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QFileDialog>
#include <QElapsedTimer>
#include <QTime>
#include <QFileInfo>
#include <QDateTime>

OtaWidget::OtaWidget(OtaManager* manager, QWidget* parent)
    : QWidget(parent)
    , m_manager(manager)
{
    setupUI();

    // 连接OTA管理器信号
    connect(m_manager, &OtaManager::progress,
            this, &OtaWidget::onProgress);
    connect(m_manager, &OtaManager::transferComplete,
            this, &OtaWidget::onTransferComplete);
    connect(m_manager, &OtaManager::transferError,
            this, &OtaWidget::onTransferError);
}

void OtaWidget::setConnection(IConnection* conn)
{
    m_manager->setConnection(conn);
}

void OtaWidget::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(12);

    // ---- 文件选择组 ----
    auto* fileGroup = new QGroupBox(tr("固件文件"));
    auto* fileLayout = new QHBoxLayout(fileGroup);

    m_filePathEdit = new QLineEdit;
    m_filePathEdit->setPlaceholderText(tr("选择固件文件 (.bin / .hex) ..."));

    m_browseBtn = new QPushButton(tr("浏览"));
    m_browseBtn->setFixedWidth(80);
    connect(m_browseBtn, &QPushButton::clicked,
            this, &OtaWidget::onBrowseFile);

    fileLayout->addWidget(m_filePathEdit, 1);
    fileLayout->addWidget(m_browseBtn);
    mainLayout->addWidget(fileGroup);

    // ---- 传输配置组 ----
    auto* configGroup = new QGroupBox(tr("传输设置"));
    auto* configLayout = new QFormLayout(configGroup);
    configLayout->setSpacing(8);

    m_protocolCombo = new QComboBox;
    m_protocolCombo->addItem(tr("XMODEM-CRC (推荐)"), "xmodem-crc");
    m_protocolCombo->addItem(tr("XMODEM-Checksum"), "xmodem-checksum");
    m_protocolCombo->addItem(tr("XMODEM-1K"), "xmodem-1k");
    m_protocolCombo->addItem(tr("YMODEM"), "ymodem");
    m_protocolCombo->addItem(tr("ZMODEM"), "zmodem");
    configLayout->addRow(tr("协议:"), m_protocolCombo);

    auto* btnLayout = new QHBoxLayout;
    m_startBtn = new QPushButton(tr("开始传输"));
    m_startBtn->setObjectName("otaStartBtn");
    m_startBtn->setFixedHeight(32);

    m_cancelBtn = new QPushButton(tr("取消"));
    m_cancelBtn->setObjectName("otaCancelBtn");
    m_cancelBtn->setFixedHeight(32);
    m_cancelBtn->setEnabled(false);

    btnLayout->addWidget(m_startBtn, 1);
    btnLayout->addWidget(m_cancelBtn, 1);
    configLayout->addRow(btnLayout);

    connect(m_startBtn, &QPushButton::clicked,
            this, &OtaWidget::onStartTransfer);
    connect(m_cancelBtn, &QPushButton::clicked,
            this, &OtaWidget::onCancelTransfer);

    mainLayout->addWidget(configGroup);

    // ---- 进度显示组 ----
    auto* progressGroup = new QGroupBox(tr("传输进度"));
    auto* progressLayout = new QVBoxLayout(progressGroup);
    progressLayout->setSpacing(6);

    m_progressBar = new QProgressBar;
    m_progressBar->setObjectName("otaProgress");
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setTextVisible(true);
    m_progressBar->setFixedHeight(24);
    progressLayout->addWidget(m_progressBar);

    // 状态信息行
    auto* statsLayout = new QHBoxLayout;
    m_statusLbl = new QLabel(tr("就绪"));
    m_speedLbl = new QLabel("");
    m_etaLbl = new QLabel("");
    m_speedLbl->setAlignment(Qt::AlignCenter);
    m_etaLbl->setAlignment(Qt::AlignRight);
    statsLayout->addWidget(m_statusLbl, 1);
    statsLayout->addWidget(m_speedLbl, 1);
    statsLayout->addWidget(m_etaLbl, 1);
    progressLayout->addLayout(statsLayout);

    mainLayout->addWidget(progressGroup);

    // ---- 日志输出 ----
    auto* logGroup = new QGroupBox(tr("传输日志"));
    auto* logLayout = new QVBoxLayout(logGroup);

    m_logView = new QTextEdit;
    m_logView->setObjectName("otaLogView");
    m_logView->setReadOnly(true);
    m_logView->setMaximumHeight(160);
    logLayout->addWidget(m_logView);

    mainLayout->addWidget(logGroup, 1);

    // ---- 历史记录组 ----
    auto* historyGroup = new QGroupBox(tr("OTA历史记录"));
    auto* historyLayout = new QVBoxLayout(historyGroup);
    historyLayout->setSpacing(6);

    m_historyModel = new OtaHistoryModel(this);

    m_historyView = new QTreeView;
    m_historyView->setObjectName("otaHistoryView");
    m_historyView->setModel(m_historyModel);
    m_historyView->setRootIsDecorated(false);
    m_historyView->setAlternatingRowColors(true);
    m_historyView->setSelectionMode(QAbstractItemView::SingleSelection);
    // 列宽设置
    m_historyView->setColumnWidth(OtaHistoryModel::ColTime, 150);
    m_historyView->setColumnWidth(OtaHistoryModel::ColFileName, 160);
    m_historyView->setColumnWidth(OtaHistoryModel::ColProtocol, 80);
    m_historyView->setColumnWidth(OtaHistoryModel::ColSize, 80);
    m_historyView->setColumnWidth(OtaHistoryModel::ColDuration, 80);
    historyLayout->addWidget(m_historyView);

    auto* histBtnLayout = new QHBoxLayout;
    m_clearHistoryBtn = new QPushButton(tr("清除历史"));
    m_clearHistoryBtn->setObjectName("otaClearHistory");
    m_clearHistoryBtn->setFixedHeight(28);
    connect(m_clearHistoryBtn, &QPushButton::clicked, this, [this]() {
        m_historyModel->clearHistory();
    });
    histBtnLayout->addStretch();
    histBtnLayout->addWidget(m_clearHistoryBtn);
    historyLayout->addLayout(histBtnLayout);

    mainLayout->addWidget(historyGroup, 1);
}

void OtaWidget::onBrowseFile()
{
    QString filter = tr("固件文件 (*.bin *.hex);;二进制文件 (*.bin);;Intel HEX (*.hex);;所有文件 (*.*)");
    QString path = QFileDialog::getOpenFileName(this, tr("选择固件文件"),
                                                 QString(), filter);
    if (!path.isEmpty()) {
        m_filePathEdit->setText(path);
        appendLog(tr("已选择文件: %1").arg(path));
    }
}

void OtaWidget::onStartTransfer()
{
    QString filePath = m_filePathEdit->text().trimmed();
    if (filePath.isEmpty()) {
        appendLog(tr("错误: 未选择固件文件"));
        return;
    }

    QFileInfo fileInfo(filePath);
    m_currentFileName = fileInfo.fileName();
    m_currentProtocol = m_protocolCombo->currentData().toString();
    m_currentFileSize = fileInfo.size();
    m_transferStartTime = QDateTime::currentDateTime();

    QString protocol = m_protocolCombo->currentData().toString();
    appendLog(tr("开始传输: %1, 协议: %2").arg(filePath, protocol));

    m_transferTimer.start();
    m_lastBytesSent = 0;
    setTransferring(true);

    if (!m_manager->startTransfer(filePath, protocol)) {
        setTransferring(false);
        appendLog(tr("传输启动失败"));
    }
}

void OtaWidget::onCancelTransfer()
{
    m_manager->cancelTransfer();
    appendLog(tr("用户已取消传输"));
    setTransferring(false);
}

void OtaWidget::onProgress(int percent, qint64 bytesSent, qint64 totalBytes)
{
    m_progressBar->setValue(percent);
    m_statusLbl->setText(tr("传输中: %1%").arg(percent));

    // 计算速率和ETA
    qint64 elapsed = m_transferTimer.elapsed();
    if (elapsed > 500) {
        double speed = (bytesSent * 1000.0) / elapsed;
        QString speedStr;
        if (speed < 1024) {
            speedStr = QString("%1 B/s").arg(speed, 0, 'f', 0);
        } else if (speed < 1024 * 1024) {
            speedStr = QString("%1 KB/s").arg(speed / 1024.0, 0, 'f', 1);
        } else {
            speedStr = QString("%1 MB/s").arg(speed / (1024.0 * 1024.0), 0, 'f', 2);
        }
        m_speedLbl->setText(speedStr);

        // ETA计算
        if (bytesSent > 0 && bytesSent < totalBytes) {
            qint64 remaining = totalBytes - bytesSent;
            qint64 etaMs = static_cast<qint64>((remaining * elapsed) / bytesSent);
            int secs = static_cast<int>(etaMs / 1000);
            QTime eta(0, 0);
            eta = eta.addSecs(secs);
            m_etaLbl->setText(tr("ETA: %1").arg(eta.toString("mm:ss")));
        }
    }
}

void OtaWidget::onTransferComplete()
{
    setTransferring(false);
    m_progressBar->setValue(100);
    m_statusLbl->setText(tr("传输完成"));
    m_etaLbl->setText("");

    qint64 elapsed = m_transferTimer.elapsed();
    appendLog(tr("传输完成，耗时 %1s").arg(elapsed / 1000.0, 0, 'f', 1));

    // 记录成功历史
    OtaRecord rec;
    rec.fileName = m_currentFileName;
    rec.protocol = m_currentProtocol;
    rec.fileSize = m_currentFileSize;
    rec.startTime = m_transferStartTime;
    rec.durationMs = elapsed;
    rec.success = true;
    m_historyModel->addRecord(rec);
}

void OtaWidget::onTransferError(const QString& reason)
{
    setTransferring(false);
    m_statusLbl->setText(tr("错误: %1").arg(reason));
    appendLog(tr("错误: %1").arg(reason));

    // 记录失败历史
    OtaRecord rec;
    rec.fileName = m_currentFileName;
    rec.protocol = m_currentProtocol;
    rec.fileSize = m_currentFileSize;
    rec.startTime = m_transferStartTime;
    rec.durationMs = m_transferTimer.elapsed();
    rec.success = false;
    rec.errorMessage = reason;
    m_historyModel->addRecord(rec);
}

void OtaWidget::appendLog(const QString& msg)
{
    QString timestamp = QTime::currentTime().toString("HH:mm:ss");
    m_logView->append(QString("[%1] %2").arg(timestamp, msg));
}

void OtaWidget::setTransferring(bool transferring)
{
    m_startBtn->setEnabled(!transferring);
    m_cancelBtn->setEnabled(transferring);
    m_browseBtn->setEnabled(!transferring);
    m_protocolCombo->setEnabled(!transferring);
    m_filePathEdit->setEnabled(!transferring);

    if (transferring) {
        m_progressBar->setValue(0);
        m_speedLbl->setText("");
        m_etaLbl->setText("");
    }
}
