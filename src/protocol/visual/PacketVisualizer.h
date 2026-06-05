/**
 * @file PacketVisualizer.h
 * @brief 数据包结构可视化控件 -- 彩色字段块图展示协议帧结构
 *
 * 将原始数据包以水平彩色块图形式可视化，每个字段对应一段着色矩形，
 * 支持字节级/位级视图切换、鼠标悬停详情、点击选中高亮、缩放/自适应。
 *
 * 协作: ThemeManager(语义色板) / 协议分析组件(setData/setFieldDefs)
 */

#ifndef PACKETVISUALIZER_H
#define PACKETVISUALIZER_H

#include <QWidget>
#include <QByteArray>
#include <QList>
#include <QColor>
#include <QString>

/**
 * @class PacketVisualizer
 * @brief 数据包结构可视化控件
 *
 * 以彩色水平块图展示协议帧结构，支持字段定义管理、鼠标交互、缩放和视图切换。
 * 绘制逻辑拆分在 PacketVisualizerPaint.cpp，统计重置在 PacketVisualizerStats.cpp。
 */
class PacketVisualizer : public QWidget {
    Q_OBJECT

public:
    /** @brief 字段定义: 名称/偏移/长度/颜色/描述 */
    struct FieldDef {
        QString name;           ///< 字段名称(如 "Header", "Length", "CRC")
        int offset = 0;         ///< 起始偏移(字节或位)
        int length = 1;         ///< 字段长度(字节或位)
        QColor color;           ///< 字段着色
        QString description;    ///< 悬停提示详细描述
    };

    /** @brief 运行统计数据 */
    struct Stats {
        quint64 totalPacketsVisualized = 0;  ///< 累计可视化数据包数
        quint64 totalFieldsRendered = 0;     ///< 累计渲染字段数
        quint64 totalZoomOperations = 0;     ///< 累计缩放操作次数
        quint64 totalFieldClicks = 0;        ///< 累计字段点击次数
    };

    /** @brief 视图模式: 字节级(默认) / 位级 */
    enum class ViewMode { ByteLevel, BitLevel };
    Q_ENUM(ViewMode)

    explicit PacketVisualizer(QWidget *parent = nullptr);
    ~PacketVisualizer() override;

    // ---- 数据设置 ----
    void setData(const QByteArray &data);                ///< 设置原始数据包
    void setFieldDefs(const QList<FieldDef> &defs);      ///< 设置字段定义列表
    void addFieldDef(const FieldDef &def);               ///< 添加字段定义
    bool removeFieldDef(int index);                      ///< 移除字段定义(按索引)
    bool modifyFieldDef(int index, const FieldDef &def); ///< 修改字段定义(按索引)
    void clearFieldDefs();                               ///< 清空所有字段定义

    // ---- 视图控制 ----
    void setViewMode(ViewMode mode);   ///< 设置视图模式
    ViewMode viewMode() const;         ///< 获取当前视图模式
    void zoomIn();                     ///< 放大(增大像素/字节比率)
    void zoomOut();                    ///< 缩小
    void zoomToFit();                  ///< 自动适应控件宽度
    void setZoomLevel(double level);   ///< 直接设置缩放级别
    double zoomLevel() const;          ///< 获取当前缩放级别

    // ---- 查询 ----
    int selectedFieldIndex() const;              ///< 当前选中字段索引(-1=无)
    const QList<FieldDef>& fieldDefs() const;    ///< 字段定义列表
    QByteArray data() const;                     ///< 原始数据

    // ---- 统计 ----
    const Stats& stats() const;   ///< 获取统计数据
    void resetStatistics();       ///< 重置所有统计计数器

signals:
    void fieldClicked(int index);      ///< 字段被点击
    void fieldHovered(int index);      ///< 字段被悬停(-1=离开)
    void packetRendered(int fieldCount); ///< 渲染完成

protected:
    void paintEvent(QPaintEvent *event) override;       ///< 绘制事件
    void mouseMoveEvent(QMouseEvent *event) override;   ///< 鼠标移动(悬停)
    void mousePressEvent(QMouseEvent *event) override;  ///< 鼠标点击(选中)
    QSize minimumSizeHint() const override;             ///< 最小尺寸
    QSize sizeHint() const override;                    ///< 推荐尺寸

private:
    int hitTestField(int pixelX) const;       ///< 像素坐标 → 字段索引
    int totalPacketLength() const;            ///< 数据包总长度(字节或位)
    QString fieldHexValue(const FieldDef &def) const;  ///< 字段十六进制值
    double computeFitZoom() const;            ///< 计算自适应缩放级别

    QByteArray m_data;                ///< 原始数据包内容
    QList<FieldDef> m_fieldDefs;      ///< 字段定义列表
    ViewMode m_viewMode = ViewMode::ByteLevel;  ///< 当前视图模式

    int m_selectedIndex = -1;         ///< 选中字段索引(-1=无)
    int m_hoveredIndex = -1;          ///< 悬停字段索引(-1=无)

    double m_zoomLevel = 16.0;        ///< 缩放级别(每单位像素数)
    double m_scrollOffset = 0.0;      ///< 水平滚动偏移(像素)
    bool m_autoFit = true;            ///< 自动适应宽度标志

    static constexpr int kRowHeight = 80;       ///< 每行最小高度(像素)
    static constexpr double kMinZoom = 4.0;     ///< 最小缩放级别
    static constexpr double kMaxZoom = 128.0;   ///< 最大缩放级别
    static constexpr double kDefaultZoom = 16.0; ///< 默认缩放级别

    Stats m_stats;                    ///< 聚合统计结构体
};

#endif // PACKETVISUALIZER_H
