#ifndef PROTOCOLVIEW_H
#define PROTOCOLVIEW_H

#include <QWidget>
#include <QTableView>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QDateTime>

// 协议解析结果展示 - 以表格形式展示解析出的帧数据
// 每行一帧，列 = 序号 + 时间 + 各字段名
class ProtocolView : public QWidget {
    Q_OBJECT

public:
    explicit ProtocolView(QWidget* parent = nullptr);

    // 添加一帧解析结果
    void addFrame(const QVariantMap& fields);

    // 清空所有解析结果
    void clear();

    // 设置显示的最大行数（旧数据自动丢弃）
    void setMaxRows(int max);

    // 获取当前行数
    int rowCount() const;

    // 获取所有解析结果（用于导出）
    QList<QVariantMap> allFrames() const;

public slots:
    // 帧解析成功
    void onFrameParsed(const QVariantMap& fields, const QByteArray& rawFrame);

    // 帧解析错误
    void onFrameError(const QString& reason, const QByteArray& rawFrame);

private:
    void setupUI();
    void updateColumnHeaders(const QVariantMap& fields);

    QTableView* m_table;
    QStandardItemModel* m_model;
    QLabel* m_statusLabel;
    QPushButton* m_clearBtn;
    QPushButton* m_exportBtn;

    int m_maxRows = 1000;
    quint64 m_totalFrames = 0;
    quint64 m_totalErrors = 0;
    QStringList m_fieldNames;   // 动态列名列表
    QList<QVariantMap> m_frames; // 保存所有帧数据

    static constexpr int kFixedColumns = 2; // 序号 + 时间
};

#endif // PROTOCOLVIEW_H
