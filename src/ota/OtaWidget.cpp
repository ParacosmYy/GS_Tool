#include "ota/OtaWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QFileDialog>
#include <QElapsedTimer>
#include <QTime>

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
    auto* fileGroup = new QGroupBox(tr("Firmware File"));
    auto* fileLayout = new QHBoxLayout(fileGroup);

    m_filePathEdit = new QLineEdit;
    m_filePathEdit->setPlaceholderText(tr("Select firmware file (.bin / .hex) ..."));

    m_browseBtn = new QPushButton(tr("Browse"));
    m_browseBtn->setFixedWidth(80);
    connect(m_browseBtn, &QPushButton::clicked,
            this, &OtaWidget::onBrowseFile);

    fileLayout->addWidget(m_filePathEdit, 1);
    fileLayout->addWidget(m_browseBtn);
    mainLayout->addWidget(fileGroup);

    // ---- 传输配置组 ----
    auto* configGroup = new QGroupBox(tr("Transfer Settings"));
    auto* configLayout = new QFormLayout(configGroup);
    configLayout->setSpacing(8);

    m_protocolCombo = new QComboBox;
    m_protocolCombo->addItem(tr("XMODEM-CRC (Recommended)"), "xmodem-crc");
    m_protocolCombo->addItem(tr("XMODEM-Checksum"), "xmodem-checksum");
    m_protocolCombo->addItem(tr("XMODEM-1K"), "xmodem-1k");
    m_protocolCombo->addItem(tr("YMODEM"), "ymodem");
    m_protocolCombo->addItem(tr("ZMODEM"), "zmodem");
    configLayout->addRow(tr("Protocol:"), m_protocolCombo);

    auto* btnLayout = new QHBoxLayout;
    m_startBtn = new QPushButton(tr("Start Transfer"));
    m_startBtn->setFixedHeight(32);
    m_startBtn->setStyleSheet(
        "QPushButton { background-color: #89b4fa; color: #1e1e2e; "
        "  border: none; border-radius: 4px; font-weight: bold; }"
        "QPushButton:hover { background-color: #b4d0fb; }"
        "QPushButton:pressed { background-color: #74a8f7; }"
        "QPushButton:disabled { background-color: #45475a; color: #6c7086; }"
    );

    m_cancelBtn = new QPushButton(tr("Cancel"));
    m_cancelBtn->setFixedHeight(32);
    m_cancelBtn->setEnabled(false);
    m_cancelBtn->setStyleSheet(
        "QPushButton { background-color: transparent; color: #f38ba8; "
        "  border: 1px solid #f38ba8; border-radius: 4px; }"
        "QPushButton:hover { background-color: rgba(243,139,168,0.15); }"
        "QPushButton:pressed { background-color: rgba(243,139,168,0.3); }"
        "QPushButton:disabled { color: #45475a; border-color: #45475a; }"
    );

    btnLayout->addWidget(m_startBtn, 1);
    btnLayout->addWidget(m_cancelBtn, 1);
    configLayout->addRow(btnLayout);

    connect(m_startBtn, &QPushButton::clicked,
            this, &OtaWidget::onStartTransfer);
    connect(m_cancelBtn, &QPushButton::clicked,
            this, &OtaWidget::onCancelTransfer);

    mainLayout->addWidget(configGroup);

    // ---- 进度显示组 ----
    auto* progressGroup = new QGroupBox(tr("Progress"));
    auto* progressLayout = new QVBoxLayout(progressGroup);
    progressLayout->setSpacing(6);

    m_progressBar = new QProgressBar;
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setTextVisible(true);
    m_progressBar->setFixedHeight(24);
    m_progressBar->setStyleSheet(
        "QProgressBar { background-color: #313244; border: none; "
        "  border-radius: 4px; text-align: center; color: #cdd6f4; }"
        "QProgressBar::chunk { background-color: #89b4fa; border-radius: 4px; }"
    );
    progressLayout->addWidget(m_progressBar);

    // 状态信息行
    auto* statsLayout = new QHBoxLayout;
    m_statusLbl = new QLabel(tr("Ready"));
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
    auto* logGroup = new QGroupBox(tr("Transfer Log"));
    auto* logLayout = new QVBoxLayout(logGroup);

    m_logView = new QTextEdit;
    m_logView->setReadOnly(true);
    m_logView->setMaximumHeight(160);
    m_logView->setStyleSheet(
        "QTextEdit { background-color: #1e1e2e; color: #a6adc8; "
        "  border: 1px solid #313244; border-radius: 4px; "
        "  font-family: Consolas, 'Courier New', monospace; font-size: 12px; }"
    );
    logLayout->addWidget(m_logView);

    mainLayout->addWidget(logGroup, 1);
}

void OtaWidget::onBrowseFile()
{
    QString filter = tr("Firmware files (*.bin *.hex);;Binary files (*.bin);;Intel HEX (*.hex);;All files (*.*)");
    QString path = QFileDialog::getOpenFileName(this, tr("Select Firmware File"),
                                                 QString(), filter);
    if (!path.isEmpty()) {
        m_filePathEdit->setText(path);
        appendLog(tr("Selected file: %1").arg(path));
    }
}

void OtaWidget::onStartTransfer()
{
    QString filePath = m_filePathEdit->text().trimmed();
    if (filePath.isEmpty()) {
        appendLog(tr("Error: No firmware file selected"));
        return;
    }

    QString protocol = m_protocolCombo->currentData().toString();
    appendLog(tr("Starting transfer: %1, Protocol: %2").arg(filePath, protocol));

    m_transferTimer.start();
    m_lastBytesSent = 0;
    setTransferring(true);

    if (!m_manager->startTransfer(filePath, protocol)) {
        setTransferring(false);
        appendLog(tr("Failed to start transfer"));
    }
}

void OtaWidget::onCancelTransfer()
{
    m_manager->cancelTransfer();
    appendLog(tr("Transfer cancelled by user"));
    setTransferring(false);
}

void OtaWidget::onProgress(int percent, qint64 bytesSent, qint64 totalBytes)
{
    m_progressBar->setValue(percent);
    m_statusLbl->setText(tr("Transferring: %1%").arg(percent));

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
    m_statusLbl->setText(tr("Transfer Complete"));
    m_etaLbl->setText("");

    qint64 elapsed = m_transferTimer.elapsed();
    appendLog(tr("Transfer completed in %1s").arg(elapsed / 1000.0, 0, 'f', 1));
}

void OtaWidget::onTransferError(const QString& reason)
{
    setTransferring(false);
    m_statusLbl->setText(tr("Error: %1").arg(reason));
    appendLog(tr("Error: %1").arg(reason));
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
