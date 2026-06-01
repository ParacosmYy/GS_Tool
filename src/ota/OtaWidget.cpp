/**
 * @file OtaWidget.cpp
 * @brief OTA升级操作面板实现
 *
 * 实现要点:
 *   1. 进度条填充使用 QPropertyAnimation（平滑过渡，避免跳变）
 *   2. 传输完成时进度条从 accent 色变为 success 色（400ms OutCubic）
 *   3. 所有控件设置 objectName，便于 QSS 选择器精准匹配
 *   4. 所有用户可见文字使用 tr() 包裹，支持国际化
 */

#include "ota/OtaWidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QFileDialog>
#include <QTime>
#include <QFileInfo>
#include <QDateTime>

#include "core/ThemeManager.h"
#include "utils/ByteFormat.h"

// ============================================================================
// 构造 / 公开接口
// ============================================================================

OtaWidget::OtaWidget(OtaManager* manager, QWidget* parent)
    : QWidget(parent), m_manager(manager), m_progressAnim(nullptr), m_colorAnim(nullptr)
{
    setupUI();
    connect(m_manager, &OtaManager::progress, this, &OtaWidget::onProgress);
    connect(m_manager, &OtaManager::transferComplete, this, &OtaWidget::onTransferComplete);
    connect(m_manager, &OtaManager::transferError, this, &OtaWidget::onTransferError);
    connect(m_manager, &OtaManager::transferStats, this, &OtaWidget::onTransferStats);
    connect(m_manager, &OtaManager::otaStateChanged, this, &OtaWidget::onOtaStateChanged);
}

/**
 * @brief 设置数据连接，转发到 OtaManager
 * @param conn 新的数据连接
 *
 * 安全机制: 如果当前有活跃传输，先警告用户并自动取消，
 * 避免传输协议持有已失效的连接导致数据损坏
 */
void OtaWidget::setConnection(IConnection* conn)
{
    if (m_manager->isTransferring()) {
        appendLog(tr("警告: 活跃传输期间切换连接，已自动取消当前传输"));
    }
    m_manager->setConnection(conn);
}

// ============================================================================
// UI 初始化
// ============================================================================

void OtaWidget::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(12);

    mainLayout->addWidget(setupFileGroup());
    mainLayout->addWidget(setupConfigGroup());
    mainLayout->addWidget(setupProgressGroup());
    mainLayout->addWidget(setupLogGroup(), 1);
    mainLayout->addWidget(setupHistoryGroup(), 1);
}

/**
 * @brief 创建文件选择分组
 * @return 文件选择GroupBox(包含路径输入框、浏览按钮、文件信息标签)
 */
QGroupBox* OtaWidget::setupFileGroup()
{
    auto* group = new QGroupBox(tr("固件文件"));
    group->setObjectName("otaFileGroup");
    auto* outer = new QVBoxLayout(group);
    outer->setSpacing(4);

    auto* fileRow = new QHBoxLayout;
    m_filePathEdit = new QLineEdit;
    m_filePathEdit->setObjectName("otaFileLabel");
    m_filePathEdit->setPlaceholderText(tr("选择固件文件 (.bin / .hex) ..."));
    m_browseBtn = new QPushButton(tr("浏览"));
    m_browseBtn->setObjectName("otaBrowseBtn");
    m_browseBtn->setFixedWidth(80);
    connect(m_browseBtn, &QPushButton::clicked, this, &OtaWidget::onBrowseFile);
    fileRow->addWidget(m_filePathEdit, 1);
    fileRow->addWidget(m_browseBtn);
    outer->addLayout(fileRow);

    m_fileInfoLbl = new QLabel(tr("未选择文件"));
    m_fileInfoLbl->setObjectName("otaFileInfo");
    outer->addWidget(m_fileInfoLbl);
    return group;
}

/**
 * @brief 创建传输配置分组
 * @return 配置GroupBox(包含协议选择下拉框、开始/取消按钮)
 */
QGroupBox* OtaWidget::setupConfigGroup()
{
    auto* group = new QGroupBox(tr("传输设置"));
    group->setObjectName("otaConfigGroup");
    auto* layout = new QFormLayout(group);
    layout->setSpacing(8);

    m_protocolCombo = new QComboBox;
    m_protocolCombo->setObjectName("otaProtocolCombo");
    m_protocolCombo->addItem(tr("XMODEM-CRC (推荐)"), "xmodem-crc");
    m_protocolCombo->addItem(tr("XMODEM-Checksum"), "xmodem-checksum");
    m_protocolCombo->addItem(tr("XMODEM-1K"), "xmodem-1k");
    m_protocolCombo->addItem(tr("YMODEM"), "ymodem");
    m_protocolCombo->addItem(tr("ZMODEM"), "zmodem");
    layout->addRow(tr("协议:"), m_protocolCombo);

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
    layout->addRow(btnLayout);
    connect(m_startBtn, &QPushButton::clicked, this, &OtaWidget::onStartTransfer);
    connect(m_cancelBtn, &QPushButton::clicked, this, &OtaWidget::onCancelTransfer);
    return group;
}

/**
 * @brief 创建进度显示分组
 * @return 进度GroupBox(包含进度条、状态/速率/ETA标签)
 */
QGroupBox* OtaWidget::setupProgressGroup()
{
    auto* group = new QGroupBox(tr("传输进度"));
    group->setObjectName("otaProgressGroup");
    auto* layout = new QVBoxLayout(group);
    layout->setSpacing(6);

    m_progressBar = new AnimatedProgressBar;
    m_progressBar->setObjectName("otaProgress");
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setTextVisible(true);
    m_progressBar->setFixedHeight(24);
    layout->addWidget(m_progressBar);

    auto* statsLayout = new QHBoxLayout;
    m_statusLbl = new QLabel(tr("就绪"));
    m_statusLbl->setObjectName("otaStatusLbl");
    m_speedLbl = new QLabel(""); m_speedLbl->setObjectName("otaSpeedLbl");
    m_etaLbl = new QLabel(""); m_etaLbl->setObjectName("otaEtaLbl");
    m_speedLbl->setAlignment(Qt::AlignCenter);
    m_etaLbl->setAlignment(Qt::AlignRight);
    statsLayout->addWidget(m_statusLbl, 1);
    statsLayout->addWidget(m_speedLbl, 1);
    statsLayout->addWidget(m_etaLbl, 1);
    layout->addLayout(statsLayout);
    return group;
}

/**
 * @brief 创建日志输出分组
 * @return 日志GroupBox(包含只读文本编辑框)
 */
QGroupBox* OtaWidget::setupLogGroup()
{
    auto* group = new QGroupBox(tr("传输日志"));
    group->setObjectName("otaLogGroup");
    auto* layout = new QVBoxLayout(group);
    m_logView = new QTextEdit;
    m_logView->setObjectName("otaLogView");
    m_logView->setReadOnly(true);
    m_logView->setMaximumHeight(160);
    layout->addWidget(m_logView);
    return group;
}

/**
 * @brief 创建OTA历史记录分组
 * @return 历史记录GroupBox(包含树形视图和清除历史按钮)
 */
QGroupBox* OtaWidget::setupHistoryGroup()
{
    auto* group = new QGroupBox(tr("OTA历史记录"));
    group->setObjectName("otaHistoryGroup");
    auto* layout = new QVBoxLayout(group);
    layout->setSpacing(6);

    m_historyModel = new OtaHistoryModel(this);
    m_historyView = new QTreeView;
    m_historyView->setObjectName("otaHistoryView");
    m_historyView->setModel(m_historyModel);
    m_historyView->setRootIsDecorated(false);
    m_historyView->setAlternatingRowColors(true);
    m_historyView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_historyView->setColumnWidth(OtaHistoryModel::ColTime, 150);
    m_historyView->setColumnWidth(OtaHistoryModel::ColFileName, 160);
    m_historyView->setColumnWidth(OtaHistoryModel::ColProtocol, 80);
    m_historyView->setColumnWidth(OtaHistoryModel::ColSize, 80);
    m_historyView->setColumnWidth(OtaHistoryModel::ColDuration, 80);
    layout->addWidget(m_historyView);

    auto* btnRow = new QHBoxLayout;
    m_clearHistoryBtn = new QPushButton(tr("清除历史"));
    m_clearHistoryBtn->setObjectName("otaClearHistory");
    m_clearHistoryBtn->setFixedHeight(28);
    connect(m_clearHistoryBtn, &QPushButton::clicked, this, [this]() { m_historyModel->clearHistory(); });
    btnRow->addStretch();
    btnRow->addWidget(m_clearHistoryBtn);
    layout->addLayout(btnRow);
    return group;
}

// ============================================================================
// 槽函数 -- 文件浏览 / 传输控制
// ============================================================================

void OtaWidget::onBrowseFile()
{
    QString filter = tr("固件文件 (*.bin *.hex);;二进制文件 (*.bin);;Intel HEX (*.hex);;所有文件 (*.*)");
    QString path = QFileDialog::getOpenFileName(this, tr("选择固件文件"), QString(), filter);
    if (path.isEmpty()) return;
    m_filePathEdit->setText(path);
    QFileInfo info(path);
    QString sizeStr = ByteFormat::formatSize(info.size());
    m_fileInfoLbl->setText(tr("类型: %1 | 大小: %2").arg(info.suffix().toUpper(), sizeStr));
    appendLog(tr("已选择文件: %1 (%2)").arg(path, sizeStr));
}

void OtaWidget::onStartTransfer()
{
    QString filePath = m_filePathEdit->text().trimmed();
    if (filePath.isEmpty()) { appendLog(tr("错误: 未选择固件文件")); return; }

    QString errorMsg;
    if (!m_manager->validateFilePath(filePath, errorMsg)) {
        appendLog(tr("文件验证失败: %1").arg(errorMsg)); return;
    }

    QFileInfo fileInfo(filePath);
    m_currentFileName = fileInfo.fileName();
    m_currentProtocol = m_protocolCombo->currentData().toString();
    m_currentFileSize = fileInfo.size();
    m_transferStartTime = QDateTime::currentDateTime();
    QString protocol = m_protocolCombo->currentData().toString();

    auto fwType = m_manager->detectFirmwareType(filePath);
    QString typeStr = (fwType == OtaManager::FirmwareType::IntelHex) ? "HEX->BIN" : "BIN";
    appendLog(tr("开始传输: %1 [%2, %3], 协议: %4")
        .arg(m_currentFileName, typeStr, ByteFormat::formatSize(m_currentFileSize), protocol));

    m_transferTimer.start();
    m_lastBytesSent = 0;
    setTransferring(true);

    // 发射传输开始信号，供Toast通知使用
    emit transferStarted(m_currentFileName);

    if (!m_manager->startTransfer(filePath, protocol)) {
        setTransferring(false);
        appendLog(tr("传输启动失败"));
        // 启动失败也通知Toast
        emit transferFailed(m_currentFileName, tr("传输启动失败"));
    }
}

void OtaWidget::onCancelTransfer()
{
    m_manager->cancelTransfer();
    appendLog(tr("用户已取消传输"));
    setTransferring(false);
}

// ============================================================================
// 槽函数 -- 进度 / 速率 / 状态
// ============================================================================

/** @brief 进度更新 -- QPropertyAnimation 平滑填充进度条 */
void OtaWidget::onProgress(int percent, qint64 bytesSent, qint64 totalBytes)
{
    Q_UNUSED(bytesSent)
    Q_UNUSED(totalBytes)

    if (m_progressAnim && m_progressAnim->state() == QAbstractAnimation::Running) m_progressAnim->stop();
    if (m_progressAnim) m_progressAnim->deleteLater();  // 延迟销毁，避免动画信号回调访问已释放对象
    m_progressAnim = nullptr;

    int oldValue = m_progressBar->value();
    if (oldValue != percent) {
        m_progressAnim = new QPropertyAnimation(m_progressBar, "value");
        m_progressAnim->setStartValue(oldValue);
        m_progressAnim->setEndValue(percent);
        m_progressAnim->setDuration(qMin(qAbs(percent - oldValue) * 10, 500));
        m_progressAnim->setEasingCurve(QEasingCurve::OutCubic);
        // DeleteWhenStopped 自动销毁动画，连接 destroyed 信号清空指针避免悬挂
        connect(m_progressAnim, &QObject::destroyed, this, [this]() { m_progressAnim = nullptr; });
        m_progressAnim->start(QAbstractAnimation::DeleteWhenStopped);
    }
    m_statusLbl->setText(tr("传输中: %1%").arg(percent));
}

/** @brief 速率和ETA更新 -- 由 OtaManager::transferStats 信号驱动 */
void OtaWidget::onTransferStats(double rateBytesPerSec, double etaSec)
{
    qint64 rate = static_cast<qint64>(rateBytesPerSec);
    m_speedLbl->setText(ByteFormat::formatRate(rate, 1.0));

    if (etaSec > 0) {
        qint64 etaMs = static_cast<qint64>(etaSec * 1000.0);
        m_etaLbl->setText(tr("ETA: %1").arg(ByteFormat::formatDuration(etaMs)));
    } else {
        m_etaLbl->setText(tr("ETA: --"));
    }
}

/** @brief OTA状态变化 -- 记录关键状态到日志 */
void OtaWidget::onOtaStateChanged(OtaManager::OtaState state)
{
    switch (state) {
    case OtaManager::OtaState::Selecting:  appendLog(tr("正在验证固件文件...")); break;
    case OtaManager::OtaState::Verifying:  appendLog(tr("正在校验传输数据...")); break;
    case OtaManager::OtaState::Complete:   m_statusLbl->setText(tr("传输完成")); break;
    default: break;
    }
}

// ============================================================================
// 槽函数 -- 传输完成 / 错误
// ============================================================================

/** @brief 传输完成 -- 进度条100% + 变色动画 + 记录历史 */
void OtaWidget::onTransferComplete()
{
    setTransferring(false);
    m_progressBar->setValue(100);
    m_statusLbl->setText(tr("传输完成"));
    m_speedLbl->setText("");
    m_etaLbl->setText("");

    qint64 elapsed = m_transferTimer.elapsed();
    appendLog(tr("传输完成，耗时 %1").arg(ByteFormat::formatDuration(elapsed)));
    startCompletionAnimation();

    // 发射传输完成信号，供Toast通知使用
    emit transferCompleted(m_currentFileName,
                           static_cast<int>(elapsed),
                           static_cast<int>(m_currentFileSize));

    OtaRecord rec;
    rec.fileName = m_currentFileName;
    rec.protocol = m_currentProtocol;
    rec.fileSize = m_currentFileSize;
    rec.startTime = m_transferStartTime;
    rec.durationMs = elapsed;
    rec.success = true;
    m_historyModel->addRecord(rec);
}

/** @brief 传输错误 -- 重置状态 + 记录失败历史 */
void OtaWidget::onTransferError(const QString& reason)
{
    setTransferring(false);
    m_statusLbl->setText(tr("错误: %1").arg(reason));
    appendLog(tr("错误: %1").arg(reason));

    // 发射传输失败信号，供Toast通知使用
    emit transferFailed(m_currentFileName, reason);

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

// ============================================================================
// 内部方法
// ============================================================================

/** @brief 追加带时间戳的日志消息 */
void OtaWidget::appendLog(const QString& msg)
{
    m_logView->append(QString("[%1] %2").arg(QTime::currentTime().toString("HH:mm:ss"), msg));
}

/** @brief 切换传输状态（启用/禁用相关控件） */
void OtaWidget::setTransferring(bool transferring)
{
    m_startBtn->setEnabled(!transferring);
    m_cancelBtn->setEnabled(transferring);
    m_browseBtn->setEnabled(!transferring);
    m_protocolCombo->setEnabled(!transferring);
    m_filePathEdit->setEnabled(!transferring);
    if (transferring) {
        m_progressBar->setValue(0);
        m_progressBar->resetChunkColor();  // 恢复QSS主题默认颜色
        m_progressBar->startShimmer();
        m_speedLbl->setText("");
        m_etaLbl->setText("");
    } else {
        m_progressBar->stopShimmer();
    }
}

/**
 * @brief 启动进度条完成变色动画（accent -> success，400ms OutCubic）
 *
 * 从 ThemeManager 获取 accent 和 success 语义色，采用两阶段渐变:
 *   阶段1: 立即设置中间混合色（50% accent + 50% success）
 *   阶段2: 400ms 后切换为最终 success 色
 * 颜色通过 setChunkColor() 设置，布局属性由 QSS 主题文件控制。
 */
void OtaWidget::startCompletionAnimation()
{
    auto& theme = ThemeManager::instance();
    QColor accent = theme.color(ThemeManager::SemanticColor::Accent);
    QColor success = theme.color(ThemeManager::SemanticColor::Success);

    // 中间混合色（50% accent + 50% success）
    QColor mid = QColor::fromRgbF(
        accent.redF() * 0.5 + success.redF() * 0.5,
        accent.greenF() * 0.5 + success.greenF() * 0.5,
        accent.blueF() * 0.5 + success.blueF() * 0.5);

    if (m_colorAnim && m_colorAnim->state() == QAbstractAnimation::Running) m_colorAnim->stop();
    if (m_colorAnim) m_colorAnim->deleteLater();  // 延迟销毁，避免动画信号回调访问已释放对象
    m_colorAnim = nullptr;

    // 阶段1: 立即设置中间混合色（布局属性由QSS主题控制）
    m_progressBar->setChunkColor(mid);

    // 阶段2: 400ms 后切换为最终 success 色（使用QTimer替代QPropertyAnimation误用）
    QTimer::singleShot(400, this, [this, success]() {
        m_progressBar->setChunkColor(success);
    });
}
