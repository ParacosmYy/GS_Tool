/**
 * @file WaveformPreviewWidget.h
 * @brief 波形预览控件 — 轻量级实时数据波形显示
 *
 * 接收原始字节数据流，按选定格式解析为数值序列，使用QPainter绘制
 * 滚动波形。支持自动缩放、冻结/恢复、触发模式、数据格式切换。
 * 适用于ADC读数、传感器数据、信号波形的实时预览场景。
 */

#ifndef WAVEFORMPREVIEWWIDGET_H
#define WAVEFORMPREVIEWWIDGET_H

#include <QWidget>
#include <QByteArray>
#include <QVector>

class QLabel;
class QPushButton;
class QComboBox;

/**
 * @class WaveformPreviewWidget
 * @brief 轻量级实时波形预览控件
 * @details 维护固定长度缓冲区(默认200点)，feedData()将原始字节按格式解析为
 *          double序列追加到缓冲区并触发重绘。支持触发模式(Auto/Normal/Single)
 *          和冻结显示。所有颜色通过ThemeManager语义色获取。
 */
class WaveformPreviewWidget : public QWidget {
    Q_OBJECT

public:
    /** @brief 数据解析格式 */
    enum class DataFormat {
        Uint8,      ///< 单字节无符号
        Int16LE,    ///< 16位有符号小端
        Int16BE,    ///< 16位有符号大端
        Uint16LE,   ///< 16位无符号小端
        Float32LE   ///< 32位浮点小端
    };
    Q_ENUM(DataFormat)

    /** @brief 触发模式 */
    enum class TriggerMode {
        Auto,       ///< 自动滚动
        Normal,     ///< 正常触发
        Single      ///< 单次触发
    };
    Q_ENUM(TriggerMode)

    /** @brief 运行统计数据结构体 */
    struct Stats {
        quint64 totalPointsReceived = 0;   ///< 累计接收数据点数
        quint64 totalFramesDisplayed = 0;  ///< 累计渲染帧数
        quint64 totalFreezeCount = 0;      ///< 累计冻结次数
        quint64 totalTriggersFired = 0;    ///< 累计触发次数
        double  peakValue = 0.0;           ///< 历史峰值
        double  minValue = 0.0;            ///< 历史最小值
        bool    hasValue = false;           ///< 是否已有值(用于首次最小值初始化)
        quint64 totalBytesFed = 0;         ///< 累计喂入字节数
    };

    /** @brief 构造波形预览控件 @param parent 父控件指针 */
    explicit WaveformPreviewWidget(QWidget* parent = nullptr);

    /** @brief 喂入原始字节数据，按当前格式解析并追加到缓冲区 @param data 原始字节 */
    void feedData(const QByteArray& data);

    /** @brief 设置数据解析格式 @param format 目标格式 */
    void setDataFormat(DataFormat format);

    /** @brief 设置最大显示点数 @param points 最大点数(>=10) */
    void setMaxPoints(int points);

    /** @brief 冻结或恢复显示 @param freeze true=冻结 false=恢复 */
    void setFreeze(bool freeze);

    /** @brief 设置触发模式和触发电平 @param mode 触发模式 @param level 触发电平 */
    void setTrigger(TriggerMode mode, double level = 0.0);

    /** @brief 获取统计数据的只读引用 @return Stats常量引用 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    /** @brief 新数据点解析完成 @param value 数值 */
    void pointReceived(double value);

    /** @brief 触发条件满足 @param level 触发电平 */
    void triggerFired(double level);

    /** @brief 统计数据已更新 */
    void statsUpdated();

protected:
    /** @brief 绘制事件 — 绘制网格、波形、触发线、信息文本 @param event 绘制事件 */
    void paintEvent(QPaintEvent* event) override;

private slots:
    /** @brief 格式下拉框选项变更处理 @param index 新选项索引 */
    void onFormatChanged(int index);

    /** @brief 冻结按钮点击切换处理 */
    void onFreezeToggled();

private:
    /** @brief 初始化UI布局(叠加层：右上角控件 + 左下角信息) */
    void setupUI();

    /** @brief 将原始字节按m_format解析为double追加到m_buffer @param data 原始字节 */
    void parseBytes(const QByteArray& data);

    /** @brief 绘制波形折线 @param painter 画笔指针 */
    void drawWaveform(QPainter* painter);

    /** @brief 绘制网格线(5水平+垂直分割) @param painter 画笔指针 */
    void drawGrid(QPainter* painter);

    /** @brief 绘制触发水平线 @param painter 画笔指针 */
    void drawTriggerLine(QPainter* painter);

    /** @brief 根据缓冲区数据计算Y轴自动范围(含10%边距) */
    void computeYRange();

    QVector<double> m_buffer;       ///< 波形数据缓冲区
    int m_maxPoints;                ///< 最大显示点数
    DataFormat m_format;            ///< 数据解析格式
    TriggerMode m_triggerMode;      ///< 触发模式
    double m_triggerLevel;          ///< 触发电平
    bool m_frozen;                  ///< 是否冻结显示
    double m_yMin;                  ///< Y轴当前最小值
    double m_yMax;                  ///< Y轴当前最大值
    bool m_singleTriggered;         ///< Single模式是否已触发

    QLabel* m_infoLabel;            ///< 左下角信息标签(当前值/范围)
    QPushButton* m_freezeBtn;       ///< 右上角冻结按钮
    QComboBox* m_formatCombo;       ///< 右上角格式选择下拉框

    Stats m_stats;                  ///< 运行统计
};

#endif // WAVEFORMPREVIEWWIDGET_H
