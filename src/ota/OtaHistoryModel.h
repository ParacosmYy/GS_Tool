#ifndef OTAHISTORYMODEL_H
#define OTAHISTORYMODEL_H

#include <QAbstractTableModel>
#include <QDateTime>
#include <QVector>

// OTA升级历史记录
struct OtaRecord {
    QString fileName;       // 固件文件名
    QString protocol;       // 协议: xmodem-crc / ymodem / zmodem
    qint64 fileSize = 0;    // 文件大小(字节)
    QDateTime startTime;    // 开始时间
    qint64 durationMs = 0;  // 传输耗时(毫秒)
    bool success = false;   // 是否成功
    QString errorMessage;   // 失败原因(成功时为空)
};

// OTA历史记录表格模型
// 提供增删查、持久化到SettingsManager
class OtaHistoryModel : public QAbstractTableModel {
    Q_OBJECT

public:
    enum Column {
        ColTime = 0,
        ColFileName,
        ColProtocol,
        ColSize,
        ColDuration,
        ColResult,
        ColCount
    };

    explicit OtaHistoryModel(QObject* parent = nullptr);

    // QAbstractTableModel接口
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    // 记录操作
    void addRecord(const OtaRecord& record);
    void clearHistory();

    // 获取记录
    const OtaRecord& record(int row) const;
    int count() const;

    // 持久化
    void saveToSettings();
    void loadFromSettings();

private:
    QVector<OtaRecord> m_records;
    static constexpr int kMaxRecords = 200;
};

#endif // OTAHISTORYMODEL_H
