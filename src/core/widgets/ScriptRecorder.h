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

enum class ScriptActionType { SendData, Delay, Connect, Disconnect, Comment };
struct ScriptAction {
    ScriptActionType type; QString data; bool isHex = false; qint64 timestamp = 0;
};

class ScriptRecorder : public QWidget {
    Q_OBJECT
public:
    explicit ScriptRecorder(QWidget* parent = nullptr);
    bool isRecording() const { return m_recording; }
    bool isPlaying() const { return m_playing; }
    QVector<ScriptAction> script() const;
    void loadScript(const QVector<ScriptAction>& actions);
    void clearScript();
public slots:
    void startRecording(); void stopRecording();
    void startPlayback(); void stopPlayback();
    void recordSendAction(const QString& data, bool isHex);
signals:
    void recordingChanged(bool); void playbackChanged(bool);
    void playbackSendRequested(const QString& data, bool isHex);
    void playbackProgress(int current, int total);
private:
    void refreshList(); QString actionIcon(ScriptActionType) const;
    void executeNextAction(); void saveToFile(const QString&); void loadFromFile(const QString&);
    QListWidget* m_actionList = nullptr;
    QPushButton* m_recordBtn = nullptr; QPushButton* m_playBtn = nullptr;
    QPushButton* m_stopBtn = nullptr; QPushButton* m_saveBtn = nullptr;
    QPushButton* m_loadBtn = nullptr; QPushButton* m_clearBtn = nullptr;
    QLabel* m_statusLabel = nullptr; QSlider* m_speedSlider = nullptr;
    QVector<ScriptAction> m_actions;
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
