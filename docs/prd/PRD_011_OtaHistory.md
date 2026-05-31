# PRD-011: OTA历史记录模型

## 背景
OTA固件升级需要记录每次升级的历史(文件名、协议、时间、结果、耗时)，方便追溯和排查升级失败问题。

## 需求列表

| ID | 需求描述 | 优先级 | 涉及模块 |
|----|---------|--------|---------|
| R1 | OtaHistoryModel OTA历史记录数据模型 | P0 | ota/ |
| R2 | 支持添加/查询/清除历史记录 | P0 | ota/ |
| R3 | 历史持久化到QSettings(JSON) | P0 | ota/ |
| R4 | OtaWidget集成历史记录表格 | P1 | ota/ |

## 接口设计

```cpp
struct OtaRecord {
    QString fileName;
    QString protocol;
    qint64 fileSize;
    QDateTime startTime;
    qint64 durationMs;
    bool success;
    QString errorMessage;
};

class OtaHistoryModel : public QAbstractTableModel {
    void addRecord(const OtaRecord& record);
    void clearHistory();
    // 持久化到SettingsManager
};
```

## 验收标准
1. OTA完成(成功或失败)自动记录
2. 历史列表显示文件名/协议/时间/结果
3. 重启后历史不丢失
