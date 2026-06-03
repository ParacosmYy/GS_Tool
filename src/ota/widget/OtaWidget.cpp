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

#include "ota/widget/OtaWidget.h"
#include "core/widgets/AnimatedButton.h"
#include "core/theme/Constants.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QFileDialog>
#include <QTime>
#include <QFileInfo>
#include <QDateTime>

#include "utils/data/ByteFormat.h"

// ============================================================================
// 构造 / 公开接口
// ============================================================================

/** @brief 构造OTA升级面板(文件选择+协议选择+进度条+日志) @param manager OtaManager指针 @param parent 父控件 */
OtaWidget::OtaWidget(OtaManager* manager, QWidget* parent)
    : QWidget(parent), m_manager(manager), m_progressAnim(nullptr), m_colorAnim(nullptr)
{
    setObjectName("otaWidget");

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
/** @brief 注入当前连接(OTA传输需要IConnection写入数据) @param conn 连接指针 */
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

/** @brief 初始化OTA面板UI(文件选择组+协议配置组+进度组+日志组) */
void OtaWidget::setupUI()
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(Layout::kPanelPadding, Layout::kPanelPadding,
                                   Layout::kPanelPadding, Layout::kPanelPadding);
    mainLayout->setSpacing(Layout::kPanelSpacing);

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
    outer->setSpacing(Layout::kToolbarSpacing);

    auto* fileRow = new QHBoxLayout;
    m_filePathEdit = new QLineEdit;
    m_filePathEdit->setObjectName("otaFileLabel");
    m_filePathEdit->setPlaceholderText(tr("选择固件文件 (.bin / .hex) ..."));
    m_browseBtn = new AnimatedButton(tr("浏览"));
    m_browseBtn->setObjectName("otaBrowseBtn");
    m_browseBtn->setFixedWidth(Layout::kBrowseBtnWidth);
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
    layout->setSpacing(Layout::kGroupSpacing);

    m_protocolCombo = new QComboBox;
    m_protocolCombo->setObjectName("otaProtocolCombo");
    m_protocolCombo->addItem(tr("XMODEM-CRC (推荐)"), "xmodem-crc");
    m_protocolCombo->addItem(tr("XMODEM-Checksum"), "xmodem-checksum");
    m_protocolCombo->addItem(tr("XMODEM-1K"), "xmodem-1k");
    m_protocolCombo->addItem(tr("YMODEM"), "ymodem");
    m_protocolCombo->addItem(tr("ZMODEM"), "zmodem");
    layout->addRow(tr("协议:"), m_protocolCombo);

    auto* btnLayout = new QHBoxLayout;
    m_startBtn = new AnimatedButton(tr("开始传输"));
    m_startBtn->setObjectName("otaStartBtn");
    m_startBtn->setFixedHeight(Layout::kInputHeight);
    m_cancelBtn = new AnimatedButton(tr("取消"));
    m_cancelBtn->setObjectName("otaCancelBtn");
    m_cancelBtn->setFixedHeight(Layout::kInputHeight);
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
    layout->setSpacing(Layout::kControlSpacing);

    m_progressBar = new AnimatedProgressBar;
    m_progressBar->setObjectName("otaProgress");
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setTextVisible(true);
    m_progressBar->setFixedHeight(OtaLayout::kProgressBarHeight);
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
    m_logView->setMaximumHeight(OtaLayout::kLogViewMaxHeight);
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
    layout->setSpacing(Layout::kControlSpacing);

    m_historyModel = new OtaHistoryModel(this);
    m_historyView = new QTreeView(this);
    m_historyView->setObjectName("otaHistoryView");
    m_historyView->setModel(m_historyModel);
    m_historyView->setRootIsDecorated(false);
    m_historyView->setAlternatingRowColors(true);
    m_historyView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_historyView->setColumnWidth(OtaHistoryModel::ColTime, OtaLayout::kHistoryColTime);
    m_historyView->setColumnWidth(OtaHistoryModel::ColFileName, OtaLayout::kHistoryColFileName);
    m_historyView->setColumnWidth(OtaHistoryModel::ColProtocol, OtaLayout::kHistoryColProtocol);
    m_historyView->setColumnWidth(OtaHistoryModel::ColSize, OtaLayout::kHistoryColSize);
    m_historyView->setColumnWidth(OtaHistoryModel::ColDuration, OtaLayout::kHistoryColDuration);
    layout->addWidget(m_historyView);

    auto* btnRow = new QHBoxLayout;
    m_clearHistoryBtn = new AnimatedButton(tr("清除历史"));
    m_clearHistoryBtn->setObjectName("otaClearHistory");
    m_clearHistoryBtn->setFixedHeight(Layout::kMinButtonHeight);
    connect(m_clearHistoryBtn, &QPushButton::clicked, this, [this]() { m_historyModel->clearHistory(); });
    btnRow->addStretch();
    btnRow->addWidget(m_clearHistoryBtn);
    layout->addLayout(btnRow);
    return group;
}

// ============================================================================
// 槽函数 -- 文件浏览 / 传输控制
// ============================================================================

/** @brief 浏览文件按钮回调：打开文件对话框选择固件文件(.bin/.hex/.fw) */
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

/** @brief 开始传输按钮回调：校验文件路径后调用OtaManager启动传输 */
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
    QString typeStr = (fwType == OtaManager::FirmwareType::IntelHex) ? tr("HEX->BIN") : tr("BIN");
    appendLog(tr("开始传输: %1 [%2, %3], 协议: %4")
        .arg(m_currentFileName, typeStr, ByteFormat::formatSize(m_currentFileSize), protocol));

    m_transferTimer.start();
    m_lastBytesSent = 0;
    setTransferring(true);
    ++m_totalTransfersStarted;  ///< 统计: 传输启动

    // 发射传输开始信号，供Toast通知使用
    emit transferStarted(m_currentFileName);

    if (!m_manager->startTransfer(filePath, protocol)) {
        setTransferring(false);
        appendLog(tr("传输启动失败"));
        ++m_totalTransfersFailed;  ///< 统计: 传输启动失败
        // 启动失败也通知Toast
        emit transferFailed(m_currentFileName, tr("传输启动失败"));
    }
}

/** @brief 取消传输按钮回调：中止当前OTA传输并恢复UI状态 */
void OtaWidget::onCancelTransfer()
{
    m_manager->cancelTransfer();
    appendLog(tr("用户已取消传输"));
    setTransferring(false);
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

/** @brief 重置OTA面板统计计数器(不影响历史记录) */
void OtaWidget::resetOtaWidgetStatistics()
{
    m_totalTransfersStarted = 0;
    m_totalTransfersCompleted = 0;
    m_totalTransfersFailed = 0;
    m_totalBytesTransferred = 0;
}
