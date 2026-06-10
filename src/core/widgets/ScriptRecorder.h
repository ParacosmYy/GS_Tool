/**
 * @file ScriptRecorder.h
 * @brief 脚本录制器 — 记录用户操作序列为可回放脚本
 * 录制发送数据/延时/连接操作，保存JSON脚本，支持回放。
 * 设计: 命令(Command) | 协作: SendController */
#ifndef SCRIPT_RECORDER_H
#define SCRIPT_RECORDER_H
#include <QWidget>
#include <QVector>
#include <QElapsedTimer>
class QListWidget; class QPushButton; class QLabel; class QSlider;

/** @brief 脚本动作类型枚举 */
enum class ScriptActionType { SendData, Delay, Connect, Disconnect, Comment };
/** @brief 脚本动作数据结构 */
struct RecordedScriptAction {
    ScriptActionType type; ///< 动作类型
    QString data;          ///< 动作数据(发送内容/延时毫秒/注释等)
    bool isHex = false;    ///< 是否为十六进制格式(仅SendData)
    qint64 timestamp = 0;  ///< 录制时间戳(ms)
};

/**
 * @brief 脚本录制器 — 记录用户操作序列为可回放脚本
 *
 * 录制发送数据/延时/连接操作，保存JSON脚本，支持回放。
 * 设计: 命令(Command) | 协作: SendController
 */
class ScriptRecorder : public QWidget {
    Q_OBJECT
public:
    /** @brief 构造脚本录制器 @param parent 父控件 */
    explicit ScriptRecorder(QWidget* parent = nullptr);
    /** @brief 查询是否正在录制 @return true=录制中 */
    bool isRecording() const { return m_recording; }
    /** @brief 查询是否正在回放 @return true=回放中 */
    bool isPlaying() const { return m_playing; }
    /** @brief 获取当前脚本动作列表 @return 动作列表 */
    QVector<RecordedScriptAction> script() const;
    /** @brief 加载外部脚本动作列表 @param actions 动作列表 */
    void loadScript(const QVector<RecordedScriptAction>& actions);
    /** @brief 清空当前脚本 */
    void clearScript();
public slots:
    /** @brief 开始录制 */
    void startRecording();
    /** @brief 停止录制 */
    void stopRecording();
    /** @brief 开始回放 */
    void startPlayback();
    /** @brief 停止回放 */
    void stopPlayback();
    /** @brief 记录一条发送动作 @param data 发送数据 @param isHex 是否为十六进制 */
    void recordSendAction(const QString& data, bool isHex);
signals:
    /** @brief 录制状态变更信号 @param recording 是否录制中 */
    void recordingChanged(bool recording);
    /** @brief 回放状态变更信号 @param playing 是否回放中 */
    void playbackChanged(bool playing);
    /** @brief 回放请求发送数据 @param data 发送数据 @param isHex 是否十六进制 */
    void playbackSendRequested(const QString& data, bool isHex);
    /** @brief 回放进度更新 @param current 当前步骤 @param total 总步骤数 */
    void playbackProgress(int current, int total);
private:
    /** @brief 刷新动作列表显示 */
    void refreshList();
    /** @brief 获取动作类型对应的图标字符 @param type 动作类型 @return 图标字符串 */
    QString actionIcon(ScriptActionType type) const;
    /** @brief 执行下一条回放动作 */
    void executeNextAction();
    /** @brief 保存脚本到文件 @param filePath 文件路径 */
    void saveToFile(const QString& filePath);
    /** @brief 从文件加载脚本 @param filePath 文件路径 */
    void loadFromFile(const QString& filePath);
    QListWidget* m_actionList = nullptr;
    QPushButton* m_recordBtn = nullptr; QPushButton* m_playBtn = nullptr;
    QPushButton* m_stopBtn = nullptr; QPushButton* m_saveBtn = nullptr;
    QPushButton* m_loadBtn = nullptr; QPushButton* m_clearBtn = nullptr;
    QLabel* m_statusLabel = nullptr; QSlider* m_speedSlider = nullptr;
    QVector<RecordedScriptAction> m_actions;
    bool m_recording = false; bool m_playing = false; int m_playbackIndex = 0;
    QElapsedTimer m_recordTimer; class QTimer* m_playbackTimer = nullptr;

    // ---- 统计计数器 ----
    quint64 m_totalRecords = 0;     ///< 总录制动作数
    quint64 m_totalPlaybacks = 0;   ///< 总回放次数
    quint64 m_totalSends = 0;       ///< 总发送次数(录制+回放)

public:
    /** @brief 获取总录制动作数 @return 录制计数 */
    quint64 totalRecords() const { return m_totalRecords; }
    /** @brief 获取总回放次数 @return 回放计数 */
    quint64 totalPlaybacks() const { return m_totalPlaybacks; }
    /** @brief 获取总发送次数 @return 发送计数 */
    quint64 totalSends() const { return m_totalSends; }
    /** @brief 重置脚本统计计数器 */
    void resetScriptStatistics();
};
#endif
