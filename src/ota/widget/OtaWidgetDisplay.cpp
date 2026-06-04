/**
 * @file OtaWidgetDisplay.cpp
 * @brief OTA升级面板 -- UI分组构建方法实现
 *
 * 本文件从 OtaWidget.cpp 拆分而来，包含所有UI分组的构建方法:
 *   - setupFileGroup:     文件选择分组(路径输入框、浏览按钮、文件信息标签、拖放提示)
 *   - setupConfigGroup:   传输配置分组(协议选择下拉框、开始/取消按钮)
 *   - setupProgressGroup: 进度显示分组(进度条、状态/速率/ETA/校验和标签)
 *   - setupLogGroup:      日志输出分组(只读文本编辑框)
 *   - setupHistoryGroup:  OTA历史记录分组(树形视图和清除历史按钮)
 */

#include "ota/widget/OtaWidget.h"
#include "core/widgets/AnimatedButton.h"
#include "shared/LayoutConstants.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>

// ============================================================================
// UI 分组构建方法
// ============================================================================

/**
 * @brief 创建文件选择分组
 * @return 文件选择GroupBox(包含路径输入框、浏览按钮、文件信息标签、拖放提示)
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

    // 拖放提示标签
    m_dropHintLbl = new QLabel(tr("拖放固件文件到此处"));
    m_dropHintLbl->setObjectName("otaDropHint");
    m_dropHintLbl->setAlignment(Qt::AlignCenter);
    outer->addWidget(m_dropHintLbl);
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
 * @return 进度GroupBox(包含进度条、状态/速率/ETA/校验和标签)
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

    // 第一行: 状态 + 速率 + ETA
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

    // 第二行: 校验和显示
    m_checksumLbl = new QLabel("");
    m_checksumLbl->setObjectName("otaChecksumLbl");
    layout->addWidget(m_checksumLbl);
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
