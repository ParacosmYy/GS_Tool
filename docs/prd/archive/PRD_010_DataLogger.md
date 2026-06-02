# PRD-010: 数据日志记录和回放

## 背景
调试嵌入式设备时，数据流需要录制和回放以便分析问题。DataLogger记录串口/TCP收发数据到文件，支持带时间戳的回放，帮助复现问题场景。

## 需求列表

| ID | 需求描述 | 优先级 | 涉及模块 |
|----|---------|--------|---------|
| R1 | DataLogger日志记录器(开始/停止/暂停录制) | P0 | utils/ |
| R2 | 支持二进制日志格式(.edl)，包含时间戳+方向+数据 | P0 | utils/ |
| R3 | 日志回放功能，按时间间隔重现数据流 | P0 | utils/ |
| R4 | 回放速率可调(0.5x/1x/2x/4x/最大速度) | P1 | utils/ |
| R5 | MainWindow集成录制/回放控制按钮到工具栏 | P1 | core/ |

## 接口设计

```cpp
class DataLogger : public QObject {
    Q_OBJECT
public:
    void startRecording(const QString& filePath);
    void stopRecording();
    void pauseRecording();
    void resumeRecording();
    bool isRecording() const;

    void startPlayback(const QString& filePath);
    void stopPlayback();
    void pausePlayback();
    void setPlaybackSpeed(qreal speed);

signals:
    void recordingStarted();
    void recordingStopped(const QString& filePath);
    void playbackData(const QByteArray& data, qint64 direction);
    void playbackProgress(qreal percent);
    void playbackFinished();
    void error(const QString& reason);
};
```

## 日志文件格式 (.edl - EmbedDebug Log)

二进制格式:
```
Header: "EDL" magic (3B) + version (1B) + recordCount (4B)
Records: timestamp(8B, ms since start) + direction(1B, 0=RX 1=TX) + length(4B) + data(NB)
```

## 依赖
- `IConnection` — 数据来源(通过信号接入)
- `RingBuffer` — 回放缓冲

## 设计模式
- **观察者**: 通过信号通知UI录制/回放状态
- **策略**: 录制和回放是两种独立策略

## 验收标准
1. 录制串口收发数据到.edl文件
2. 回放时按原始时间间隔重现数据
3. 回放速率可调
4. 支持暂停/恢复录制和回放
5. 日志文件包含完整的时间戳和方向信息
