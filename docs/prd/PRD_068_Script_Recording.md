# PRD-068: 脚本录制回放 — 记录用户操作为可序列化脚本

## 背景
用户在调试嵌入式设备时常需重复相同的操作序列(发送命令→切换面板→修改配置→等待响应)。手动重复效率低且容易出错。引入脚本录制回放功能，将用户操作记录为JSON格式的ScriptAction序列，支持单步回放和变速(0.5x/1x/2x/5x)，方便自动化测试和回归验证。集成到现有TriggerListPanel中，不新增独立面板。

## 需求列表
| ID | 需求描述 | 优先级 | 涉及模块 |
|----|---------|--------|---------|
| R1 | ScriptModel: JSON序列化的ScriptAction序列 | P0 | serial/commands/ |
| R2 | ScriptRecorder: 监听SendController/NavigationController记录操作 | P0 | serial/commands/ |
| R3 | ScriptPlayer: QTimer::singleShot回放, 0.5x/1x/2x/5x变速 | P0 | serial/commands/ |
| R4 | TriggerListPanel UI集成: 录制/停止/播放按钮 + 速度选择 | P1 | serial/commands/ |
| R5 | .edscript文件格式(JSON)的保存和加载 | P1 | serial/commands/ |

## 接口设计

### ScriptAction数据模型
```cpp
/**
 * @brief 脚本动作类型
 */
enum class ScriptActionType {
    Send,           ///< 发送数据
    PanelSwitch,    ///< 切换面板
    ConfigChange,   ///< 修改配置项
    Wait            ///< 等待指定毫秒数
};

/**
 * @brief 脚本动作 -- 一个可序列化的用户操作记录
 */
struct ScriptAction {
    ScriptActionType type;   ///< 动作类型
    QVariantMap data;        ///< 动作数据(内容因type不同)
    int delayMs = 100;       ///< 与前一个动作的间隔毫秒数

    /** @brief 序列化为JSON */
    QJsonObject toJson() const;
    /** @brief 从JSON反序列化 */
    static ScriptAction fromJson(const QJsonObject& obj);
};

// data字段约定:
// Send:        { "text": "AT+RST", "isHex": false }
// PanelSwitch: { "panelId": "serial-config" }
// ConfigChange:{ "key": "baudRate", "value": "115200" }
// Wait:        { "ms": 500 }
```

### ScriptRecorder类
```cpp
/**
 * @brief 脚本录制器 -- 监听用户操作并记录为ScriptAction序列
 */
class ScriptRecorder : public QObject {
    Q_OBJECT

public:
    explicit ScriptRecorder(QObject* parent = nullptr);

    void startRecording();
    void stopRecording();
    bool isRecording() const;

    /** @brief 获取录制结果 */
    QList<ScriptAction> recordedActions() const;

    /** @brief 保存到.edscript文件 */
    bool saveToFile(const QString& path) const;
    /** @brief 从.edscript文件加载 */
    bool loadFromFile(const QString& path);

signals:
    void recordingStarted();
    void recordingStopped(int actionCount);
    void actionRecorded(const ScriptAction& action);

private slots:
    void onDataSent(const QByteArray& data, bool isHex);
    void onPanelSwitched(const QString& panelId);

private:
    QList<ScriptAction> m_actions;
    QElapsedTimer m_timer;    ///< 计算动作间隔
    bool m_recording = false;
};
```

### ScriptPlayer类
```cpp
/**
 * @brief 脚本回放器 -- 按延时序列执行ScriptAction
 */
class ScriptPlayer : public QObject {
    Q_OBJECT

public:
    explicit ScriptPlayer(QObject* parent = nullptr);

    /** @brief 设置回放速度倍率 */
    void setSpeed(qreal multiplier);  // 0.5/1.0/2.0/5.0

    void play(const QList<ScriptAction>& actions);
    void stop();
    bool isPlaying() const;

signals:
    void playbackStarted();
    void playbackFinished();
    void playbackProgress(int current, int total);
    void actionExecuted(const ScriptAction& action);

private:
    void executeNext();

    QList<ScriptAction> m_actions;
    int m_currentIndex = 0;
    qreal m_speed = 1.0;
    bool m_playing = false;
};
```

## 依赖的公共组件
- SendController (serial/commands/SendController.h) — dataSent信号录制发送动作
- NavigationController (core/NavigationController.h) — panelSwitched信号录制切换动作
- SendHistory (serial/commands/SendHistory.h) — 录制参考
- TriggerListPanel (serial/commands/) — UI容器

## 设计模式
- **命令模式**: ScriptAction封装操作为可序列化对象
- **观察者模式**: ScriptRecorder订阅SendController/NavigationController信号
- **状态模式**: 录制中/回放中/空闲三种状态

## 影响范围
| 文件 | 变更类型 | 风险 |
|------|---------|------|
| src/serial/commands/ScriptAction.h | 新增 | 无 |
| src/serial/commands/ScriptRecorder.h/cpp | 新增 | 无 |
| src/serial/commands/ScriptPlayer.h/cpp | 新增 | 无 |
| src/serial/commands/TriggerListPanel.cpp | 修改(集成UI) | 低 |
| resources/themes/*.qss | 修改(录制状态样式) | 低 |

## 验收标准
1. 点击录制按钮后，发送3条命令+切换2个面板，停止录制后生成5个ScriptAction
2. ScriptAction序列可序列化为JSON并保存为.edscript文件
3. 加载.edscript文件后回放，命令按原始间隔发送(支持0.5x/1x/2x/5x变速)
4. 回放过程中进度条显示current/total
5. 回放中途可停止，不残留未执行动作
6. 未连接串口时回放Send动作显示错误提示，不崩溃
7. .h ≤ 200行, .cpp ≤ 500行
8. 编译零错误
