/**
 * @file OtaWidget.cpp
 * @brief OTA升级操作面板实现
 *
 * 实现要点:
 *   1. 进度条填充使用 QPropertyAnimation(平滑过渡，避免跳变)
 *   2. 传输完成时进度条从 accent 色变为 success 色(400ms OutCubic)
 *   3. 所有控件设置 objectName，便于 QSS 选择器精准匹配
 *   4. 所有用户可见文字使用 tr() 包裹，支持国际化
 *   5. 支持文件拖放(.bin/.hex文件直接拖入面板)
 *   6. 传输完成后显示CRC32校验和
 */

#include "ota/widget/OtaWidget.h"
#include "core/widgets/AnimatedButton.h"
#include "shared/LayoutConstants.h"

#include <QVBoxLayout>
#include <QTime>
#include <QFileInfo>
#include <QMimeData>
#include <QUrl>

#include "utils/data/ByteFormat.h"

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
// 拖放事件处理
// ============================================================================

/** @brief 拖入事件: 检查MIME类型是否包含文件URL，且文件后缀为.bin/.hex @param event 拖入事件 */
void OtaWidget::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls()) {
        const QList<QUrl> urls = event->mimeData()->urls();
        if (!urls.isEmpty()) {
            QString suffix = QFileInfo(urls.first().toLocalFile()).suffix().toLower();
            if (suffix == "bin" || suffix == "hex" || suffix == "ihex" || suffix == "fw") {
                event->acceptProposedAction();
                m_dragHovering = true;
                // 视觉反馈: 显示拖放提示标签高亮
                if (m_dropHintLbl) {
                    m_dropHintLbl->setText(tr("释放以选择此固件文件"));
                }
                return;
            }
        }
    }
    event->ignore();
}

/** @brief 拖动事件: 持续接受有效拖放 @param event 拖动事件 */
void OtaWidget::dragMoveEvent(QDragMoveEvent* event)
{
    if (m_dragHovering) {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

/** @brief 放下事件: 提取第一个文件路径并加载 @param event 放下事件 */
void OtaWidget::dropEvent(QDropEvent* event)
{
    m_dragHovering = false;
    if (m_dropHintLbl) {
        m_dropHintLbl->setText(tr("拖放固件文件到此处"));
    }

    const QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty()) return;

    QString filePath = urls.first().toLocalFile();
    if (!filePath.isEmpty()) {
        handleDroppedFile(filePath);
    }
    event->acceptProposedAction();
}

/** @brief 拖离事件: 恢复拖放提示标签文字 @param event 拖离事件 */
void OtaWidget::dragLeaveEvent(QDragLeaveEvent* event)
{
    Q_UNUSED(event)
    m_dragHovering = false;
    if (m_dropHintLbl) {
        m_dropHintLbl->setText(tr("拖放固件文件到此处"));
    }
}

/** @brief 处理拖入的固件文件: 更新路径输入框、文件信息标签并记录日志 @param filePath 拖入的文件路径 */
void OtaWidget::handleDroppedFile(const QString& filePath)
{
    // 传输中不允许切换文件
    if (m_manager->isTransferring()) {
        appendLog(tr("警告: 传输进行中，无法更换文件"));
        return;
    }

    m_filePathEdit->setText(filePath);
    QFileInfo info(filePath);
    QString sizeStr = ByteFormat::formatSize(info.size());
    m_fileInfoLbl->setText(tr("类型: %1 | 大小: %2").arg(info.suffix().toUpper(), sizeStr));
    appendLog(tr("已拖入文件: %1 (%2)").arg(filePath, sizeStr));
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

// setup*Group 分组构建方法见 OtaWidgetDisplay.cpp
// 槽函数(文件浏览/开始/取消)见 OtaWidgetSlots2.cpp

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

/** @brief 重置OTA面板统计计数器(不影响历史记录) */
void OtaWidget::resetOtaWidgetStatistics()
{
    m_totalTransfersStarted = 0;
    m_totalTransfersCompleted = 0;
    m_totalTransfersFailed = 0;
    m_totalBytesTransferred = 0;
}

// onBrowseFile/onStartTransfer/onCancelTransfer 见 OtaWidgetSlots2.cpp
