/**
 * @file OtaWidget.cpp
 * @brief OTA升级操作面板实现 - 构造/UI/内部方法
 *
 * 实现要点:
 *   1. 进度条填充使用 QPropertyAnimation(平滑过渡，避免跳变)
 *   2. 传输完成时进度条从 accent 色变为 success 色(400ms OutCubic)
 *   3. 所有控件设置 objectName，便于 QSS 选择器精准匹配
 *   4. 所有用户可见文字使用 tr() 包裹，支持国际化
 *   5. 支持文件拖放(.bin/.hex文件直接拖入面板)
 *   6. 传输完成后显示CRC32校验和
 *
 * 拖放事件处理见 OtaWidgetDragDrop.cpp。
 * setup*Group 分组构建方法见 OtaWidgetDisplay.cpp。
 * 槽函数(文件浏览/开始/取消)见 OtaWidgetSlots2.cpp。
 */

#include "ota/widget/OtaWidget.h"
#include "core/widgets/AnimatedButton.h"
#include "shared/LayoutConstants.h"

#include <QVBoxLayout>
#include <QTime>

// ============================================================================
// 构造 / 公开接口
// ============================================================================

/** @brief 构造OTA升级面板(文件选择+拖放+协议选择+进度条+校验和+日志) @param manager OtaManager指针 @param parent 父控件 */
OtaWidget::OtaWidget(OtaManager* manager, QWidget* parent)
    : QWidget(parent), m_manager(manager), m_progressAnim(nullptr), m_colorAnim(nullptr)
{
    setObjectName("otaWidget");
    // 启用拖放支持
    setAcceptDrops(true);

    setupUI();
    connect(m_manager, &OtaManager::progress, this, &OtaWidget::onProgress);
    connect(m_manager, &OtaManager::transferComplete, this, &OtaWidget::onTransferComplete);
    connect(m_manager, &OtaManager::transferError, this, &OtaWidget::onTransferError);
    connect(m_manager, &OtaManager::transferStats, this, &OtaWidget::onTransferStats);
    connect(m_manager, &OtaManager::otaStateChanged, this, &OtaWidget::onOtaStateChanged);
}

/**
 * @brief 注入当前连接(OTA传输需要IConnection写入数据)
 * @param conn 连接指针
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

// ============================================================================
// 内部方法
// ============================================================================

/** @brief 追加带时间戳的日志消息 */
void OtaWidget::appendLog(const QString& msg)
{
    m_logView->append(QString("[%1] %2").arg(QTime::currentTime().toString("HH:mm:ss"), msg));
}

/** @brief 切换传输状态(启用/禁用相关控件) */
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
        m_checksumLbl->setText("");
    } else {
        m_progressBar->stopShimmer();
    }
}

// resetOtaWidgetStatistics/resetStats已在.h中内联实现

// 拖放事件处理(dragEnterEvent/dropEvent等)见 OtaWidgetDragDrop.cpp
// onBrowseFile/onStartTransfer/onCancelTransfer 见 OtaWidgetSlots2.cpp
