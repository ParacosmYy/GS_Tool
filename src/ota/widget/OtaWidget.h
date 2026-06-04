/**
 * @file OtaWidget.h
 * @brief OTA升级操作面板 -- 文件选择、拖放、协议选择、进度显示、速率/ETA、校验和、日志
 *
 * 设计: 文件拖放+浏览(.bin/.hex) | 协议选择(XMODEM/YMODEM/ZMODEM) | QPropertyAnimation进度条
 *       ByteFormat速率/ETA | CRC32校验和 | accent->success变色(400ms OutCubic) | 历史记录
 * 协作: OtaManager(业务逻辑) OtaHistoryModel(历史) ByteFormat(格式化) ThemeManager(QSS颜色)
 */

#ifndef OTAWIDGET_H
#define OTAWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QGroupBox>
#include <QTextEdit>
#include <QTreeView>
#include <QElapsedTimer>
#include <QPropertyAnimation>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QDragLeaveEvent>
#include "ota/manager/OtaManager.h"
#include "ota/history/OtaHistoryModel.h"
#include "ota/widget/AnimatedProgressBar.h"

/** @brief OTA升级面板: 文件拖放/选择+协议选择+进度动画+速率/ETA+校验和+日志+历史 */
class OtaWidget : public QWidget {
    Q_OBJECT

public:
    /** @brief 构造OTA面板 @param manager OTA管理器 @param parent 父窗口 */
    explicit OtaWidget(OtaManager* manager, QWidget* parent = nullptr);
    /** @brief 设置数据连接 @param conn 连接指针 */
    void setConnection(IConnection* conn);

    // ---- 统计访问器 ----

    /** @brief 获取传输启动计数 @return 启动次数 */
    quint64 totalTransfersStarted() const { return m_totalTransfersStarted; }

    /** @brief 获取传输完成计数 @return 完成次数 */
    quint64 totalTransfersCompleted() const { return m_totalTransfersCompleted; }

    /** @brief 获取传输失败计数 @return 失败次数 */
    quint64 totalTransfersFailed() const { return m_totalTransfersFailed; }

    /** @brief 获取累计传输字节数 @return 字节总数 */
    quint64 totalBytesTransferred() const { return m_totalBytesTransferred; }

    /** @brief 获取浏览按钮点击计数 @return 点击次数 */
    quint64 totalBrowseClicks() const { return m_totalBrowseClicks; }

    /** @brief 获取取消操作计数 @return 取消次数 */
    quint64 totalCancelOps() const { return m_totalCancelOps; }

    /** @brief 重置面板统计(不影响历史记录) */
    void resetOtaWidgetStatistics();

signals:
    void transferStarted(const QString& filename);    ///< 传输开始 @param filename 文件名
    void transferCompleted(const QString& filename, qint64 elapsed, qint64 size); ///< 传输完成
    void transferFailed(const QString& filename, const QString& error);           ///< 传输失败

private slots:
    /** @brief 浏览固件文件 */
    void onBrowseFile();

    /** @brief 开始传输 */
    void onStartTransfer();

    /** @brief 取消传输 */
    void onCancelTransfer();

    /** @brief 进度更新 @param percent 百分比 @param bytesSent 已发送字节 @param totalBytes 总字节 */
    void onProgress(int percent, qint64 bytesSent, qint64 totalBytes);

    /** @brief 传输完成(100%+变色+校验和) */
    void onTransferComplete();

    /** @brief 传输错误 @param reason 错误原因 */
    void onTransferError(const QString& reason);

    /** @brief 速率/ETA更新 @param rateBytesPerSec 速率(字节/秒) @param etaSec 预计剩余时间(秒) */
    void onTransferStats(double rateBytesPerSec, double etaSec);

    /** @brief OTA状态变化 @param state 新状态 */
    void onOtaStateChanged(OtaManager::OtaState state);

private:
    /** @brief 初始化UI */
    void setupUI();

    /** @brief 创建文件选择组(输入框+浏览+拖放提示) @return 分组QGroupBox */
    QGroupBox* setupFileGroup();

    /** @brief 创建传输配置组(协议+按钮) @return 分组QGroupBox */
    QGroupBox* setupConfigGroup();

    /** @brief 创建进度显示组(进度条+速率+ETA+校验和) @return 分组QGroupBox */
    QGroupBox* setupProgressGroup();

    /** @brief 创建日志输出组 @return 分组QGroupBox */
    QGroupBox* setupLogGroup();

    /** @brief 创建OTA历史记录组(树视图+清除) @return 分组QGroupBox */
    QGroupBox* setupHistoryGroup();

    /** @brief 追加时间戳日志 @param msg 日志消息 */
    void appendLog(const QString& msg);

    /** @brief 切换传输UI状态 @param transferring true=传输中 */
    void setTransferring(bool transferring);

    /** @brief 启动accent->success变色动画 */
    void startCompletionAnimation();

    /** @brief 处理拖入文件 @param filePath 文件路径 */
    void handleDroppedFile(const QString& filePath);

    // ---- 拖放事件重写 ----

    /** @brief 拖入: 校验文件后缀 @param event 拖入事件 */
    void dragEnterEvent(QDragEnterEvent* event) override;

    /** @brief 拖动: 持续接受 @param event 拖动事件 */
    void dragMoveEvent(QDragMoveEvent* event) override;

    /** @brief 放下: 提取文件路径 @param event 放下事件 */
    void dropEvent(QDropEvent* event) override;

    /** @brief 拖离: 恢复样式 @param event 拖离事件 */
    void dragLeaveEvent(QDragLeaveEvent* event) override;

    OtaManager* m_manager;              ///< OTA管理器(业务逻辑层)

    // ---- 文件选择组 ----
    QLineEdit* m_filePathEdit;          ///< 文件路径输入框
    QPushButton* m_browseBtn;           ///< 浏览按钮
    QLabel* m_dropHintLbl;              ///< 拖放提示标签

    // ---- 传输控制组 ----
    QComboBox* m_protocolCombo;         ///< 协议下拉框
    QPushButton* m_startBtn;            ///< 开始按钮
    QPushButton* m_cancelBtn;           ///< 取消按钮

    // ---- 进度显示组 ----
    AnimatedProgressBar* m_progressBar; ///< 进度条(shimmer流动)
    QLabel* m_statusLbl;                ///< 状态标签
    QLabel* m_speedLbl;                 ///< 速率标签
    QLabel* m_etaLbl;                   ///< ETA标签
    QLabel* m_fileInfoLbl;              ///< 文件信息标签
    QLabel* m_checksumLbl;              ///< CRC32校验和标签

    // ---- 日志输出组 ----
    QTextEdit* m_logView;               ///< 日志文本框

    // ---- 历史记录组 ----
    OtaHistoryModel* m_historyModel;    ///< 历史数据模型
    QTreeView* m_historyView;           ///< 历史树视图
    QPushButton* m_clearHistoryBtn;     ///< 清除历史按钮

    // ---- 传输计时 ----
    QElapsedTimer m_transferTimer;      ///< 传输计时器
    qint64 m_lastBytesSent = 0;         ///< 上次速率计算的字节数

    // ---- 当前传输信息 ----
    QString m_currentFileName;          ///< 当前文件名
    QString m_currentProtocol;          ///< 当前协议
    qint64 m_currentFileSize = 0;       ///< 当前文件大小
    QDateTime m_transferStartTime;      ///< 传输开始时间

    // ---- 动画 ----
    QPropertyAnimation* m_progressAnim; ///< 进度值动画
    QPropertyAnimation* m_colorAnim;    ///< 完成变色动画

    // ---- 拖放状态 ----
    bool m_dragHovering = false;        ///< 文件悬停标志

    // ---- 统计计数器 ----
    quint64 m_totalTransfersStarted = 0;    ///< 启动总次数
    quint64 m_totalTransfersCompleted = 0;  ///< 完成总次数
    quint64 m_totalTransfersFailed = 0;     ///< 失败总次数
    quint64 m_totalBytesTransferred = 0;    ///< 累计字节数
    quint64 m_totalBrowseClicks = 0;        ///< 浏览按钮点击总次数
    quint64 m_totalCancelOps = 0;           ///< 取消操作总次数
};

#endif // OTAWIDGET_H
