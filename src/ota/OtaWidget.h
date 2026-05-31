/**
 * @file OtaWidget.h
 * @brief OTA升级操作面板 -- 文件选择、协议选择、进度显示、速率/ETA、日志输出
 *
 * 设计要点:
 *   1. 提供文件选择（.bin/.hex）、协议选择（XMODEM/YMODEM/ZMODEM）、传输控制
 *   2. 进度条填充使用 QPropertyAnimation 实现平滑过渡
 *   3. 传输完成时进度条颜色从 Accent 变为 Success（400ms OutCubic 动画）
 *   4. 传输速率和ETA使用 ByteFormat 工具格式化
 *   5. 传输日志和OTA历史记录展示
 *
 * 协作关系:
 *   - OtaManager: 业务逻辑层，负责实际OTA传输和协议处理
 *   - OtaHistoryModel: 历史记录数据模型
 *   - ByteFormat: 字节格式化工具（速率/大小/时间）
 *   - ThemeManager: 通过 QSS 获取进度条颜色
 *
 * 设计模式:
 *   - 观察者模式: 监听 OtaManager 的 progress/transferStats/transferComplete/transferError 信号
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
#include "ota/OtaManager.h"
#include "ota/OtaHistoryModel.h"
#include "ota/AnimatedProgressBar.h"

/**
 * @brief OTA升级操作面板
 *
 * 职责:
 *   1. 文件选择和路径显示
 *   2. 协议选择（XMODEM-CRC/Checksum/1K, YMODEM, ZMODEM）
 *   3. 传输进度显示（带平滑动画的进度条 + 百分比文字）
 *   4. 传输速率和ETA实时显示（使用ByteFormat格式化）
 *   5. 传输完成/失败状态反馈（进度条变色动画）
 *   6. 传输日志实时输出（带时间戳）
 *   7. OTA历史记录查看和清除
 */
class OtaWidget : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief 构造OTA面板
     * @param manager OTA管理器实例（业务逻辑层）
     * @param parent 父窗口
     */
    explicit OtaWidget(OtaManager* manager, QWidget* parent = nullptr);

    /**
     * @brief 设置关联的连接（切换连接时调用）
     * @param conn 连接接口指针
     */
    void setConnection(IConnection* conn);

signals:
    /**
     * @brief 传输开始信号 -- MainWindow可连接到ToastWidget显示通知
     * @param filename 固件文件名（不含路径）
     */
    void transferStarted(const QString& filename);

    /**
     * @brief 传输完成信号 -- MainWindow可连接到ToastWidget显示成功通知
     * @param filename 固件文件名（不含路径）
     * @param elapsed 传输耗时（毫秒）
     * @param size 传输文件大小（字节）
     */
    void transferCompleted(const QString& filename, int elapsed, int size);

    /**
     * @brief 传输失败信号 -- MainWindow可连接到ToastWidget显示错误通知
     * @param filename 固件文件名（不含路径）
     * @param error 错误原因描述
     */
    void transferFailed(const QString& filename, const QString& error);

private slots:
    /** @brief 浏览固件文件按钮点击 */
    void onBrowseFile();

    /** @brief 开始传输按钮点击 */
    void onStartTransfer();

    /** @brief 取消传输按钮点击 */
    void onCancelTransfer();

    /**
     * @brief 传输进度更新
     * @param percent 进度百分比 (0-100)
     * @param bytesSent 已发送字节数
     * @param totalBytes 总字节数
     */
    void onProgress(int percent, qint64 bytesSent, qint64 totalBytes);

    /** @brief 传输完成处理 -- 设置进度条100%、触发变色动画 */
    void onTransferComplete();

    /**
     * @brief 传输错误处理
     * @param reason 错误原因描述
     */
    void onTransferError(const QString& reason);

    /**
     * @brief 传输速率和ETA更新
     * @param rateBytesPerSec 当前传输速率(字节/秒)
     * @param etaSec 预计剩余时间(秒)
     */
    void onTransferStats(double rateBytesPerSec, double etaSec);

    /**
     * @brief OTA状态变化处理
     * @param state 新的OTA状态
     */
    void onOtaStateChanged(OtaManager::OtaState state);

private:
    /** @brief 初始化UI布局和所有子控件 */
    void setupUI();

    /**
     * @brief 追加带时间戳的日志消息
     * @param msg 日志内容
     */
    void appendLog(const QString& msg);

    /**
     * @brief 切换传输状态（启用/禁用相关控件）
     * @param transferring true 为传输中，false 为空闲
     */
    void setTransferring(bool transferring);

    /**
     * @brief 启动进度条完成变色动画
     *
     * 传输完成后将进度条样式从 accent 色渐变为 success 色。
     * 使用 QPropertyAnimation 对自定义 progressColor 属性进行动画，
     * 持续 400ms，缓动曲线 OutCubic。
     */
    void startCompletionAnimation();

    OtaManager* m_manager;              ///< OTA管理器（业务逻辑层）

    // ---- 文件选择组 ----
    QLineEdit* m_filePathEdit;          ///< 固件文件路径输入框
    QPushButton* m_browseBtn;           ///< 浏览按钮

    // ---- 协议选择组 ----
    QComboBox* m_protocolCombo;         ///< 协议选择下拉框

    // ---- 传输控制组 ----
    QPushButton* m_startBtn;            ///< 开始传输按钮
    QPushButton* m_cancelBtn;           ///< 取消传输按钮

    // ---- 进度显示组 ----
    AnimatedProgressBar* m_progressBar; ///< 传输进度条（带shimmer流动效果）
    QLabel* m_statusLbl;                ///< 状态文字标签
    QLabel* m_speedLbl;                 ///< 传输速率标签
    QLabel* m_etaLbl;                   ///< 预计剩余时间标签
    QLabel* m_fileInfoLbl;              ///< 文件信息标签(类型+大小)

    // ---- 日志输出组 ----
    QTextEdit* m_logView;               ///< 传输日志文本框

    // ---- 历史记录组 ----
    OtaHistoryModel* m_historyModel;    ///< OTA历史数据模型
    QTreeView* m_historyView;           ///< 历史记录树形视图
    QPushButton* m_clearHistoryBtn;     ///< 清除历史按钮

    // ---- 传输计时 ----
    QElapsedTimer m_transferTimer;      ///< 传输耗时计时器
    qint64 m_lastBytesSent = 0;         ///< 上次速率计算的已发送字节数

    // ---- 当前传输信息（用于记录历史） ----
    QString m_currentFileName;          ///< 当前传输文件名
    QString m_currentProtocol;          ///< 当前传输协议
    qint64 m_currentFileSize = 0;       ///< 当前文件大小（字节）
    QDateTime m_transferStartTime;      ///< 传输开始时间

    // ---- 进度条动画 ----
    QPropertyAnimation* m_progressAnim; ///< 进度条值动画（平滑填充）
    QPropertyAnimation* m_colorAnim;    ///< 进度条完成变色动画（accent -> success）
};

#endif // OTAWIDGET_H
