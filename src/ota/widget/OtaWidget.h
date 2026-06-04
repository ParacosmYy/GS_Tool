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
    quint64 totalTransfersStarted() const { return m_totalTransfersStarted; }   ///< 传输启动计数
    quint64 totalTransfersCompleted() const { return m_totalTransfersCompleted; } ///< 传输完成计数
    quint64 totalTransfersFailed() const { return m_totalTransfersFailed; }     ///< 传输失败计数
    quint64 totalBytesTransferred() const { return m_totalBytesTransferred; }   ///< 累计字节
    quint64 totalBrowseClicks() const { return m_totalBrowseClicks; }           ///< 浏览按钮点击计数
    quint64 totalCancelOps() const { return m_totalCancelOps; }                 ///< 取消操作计数
    void resetOtaWidgetStatistics(); ///< 重置面板统计(不影响历史记录)

signals:
    void transferStarted(const QString& filename);    ///< 传输开始 @param filename 文件名
    void transferCompleted(const QString& filename, qint64 elapsed, qint64 size); ///< 传输完成
    void transferFailed(const QString& filename, const QString& error);           ///< 传输失败

private slots:
    void onBrowseFile();     ///< 浏览固件文件
    void onStartTransfer();  ///< 开始传输
    void onCancelTransfer(); ///< 取消传输
    void onProgress(int percent, qint64 bytesSent, qint64 totalBytes); ///< 进度更新
    void onTransferComplete();          ///< 传输完成(100%+变色+校验和)
    void onTransferError(const QString& reason); ///< 传输错误
    void onTransferStats(double rateBytesPerSec, double etaSec); ///< 速率/ETA
    void onOtaStateChanged(OtaManager::OtaState state);          ///< OTA状态变化

private:
    void setupUI();           ///< 初始化UI
    QGroupBox* setupFileGroup();     ///< 文件选择组(输入框+浏览+拖放提示)
    QGroupBox* setupConfigGroup();   ///< 传输配置组(协议+按钮)
    QGroupBox* setupProgressGroup(); ///< 进度显示组(进度条+速率+ETA+校验和)
    QGroupBox* setupLogGroup();      ///< 日志输出组
    QGroupBox* setupHistoryGroup();  ///< OTA历史记录组(树视图+清除)
    void appendLog(const QString& msg);             ///< 追加时间戳日志
    void setTransferring(bool transferring);         ///< 切换传输UI状态
    void startCompletionAnimation();                 ///< accent->success变色
    void handleDroppedFile(const QString& filePath); ///< 处理拖入文件

    // ---- 拖放事件重写 ----
    void dragEnterEvent(QDragEnterEvent* event) override;  ///< 拖入: 校验文件后缀
    void dragMoveEvent(QDragMoveEvent* event) override;    ///< 拖动: 持续接受
    void dropEvent(QDropEvent* event) override;            ///< 放下: 提取路径
    void dragLeaveEvent(QDragLeaveEvent* event) override;  ///< 拖离: 恢复样式

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
