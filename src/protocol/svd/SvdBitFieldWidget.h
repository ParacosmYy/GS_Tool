/**
 * @file SvdBitFieldWidget.h
 * @brief SVD位字段可视化控件 — 以彩色矩形块展示寄存器位字段分布
 *
 * 职责: 将SVD寄存器字段以水平彩色块可视化，每个字段宽度与位宽成正比，
 * 颜色按访问类型区分(读=蓝/写=绿/读写=橙)，支持悬停提示和点击选中。
 *
 * 协作: SvdViewerWidget(集成) / SvdTypes(数据结构) / ThemeManager(语义色板)
 */
#ifndef SVDBITFIELDWIDGET_H
#define SVDBITFIELDWIDGET_H

#include <QWidget>
#include <QList>
#include <QColor>
#include <QString>

struct SvdRegister;

/**
 * @brief SVD位字段可视化控件
 *
 * 以水平彩色块图展示寄存器位字段分布，字段宽度与位宽成正比。
 * 颜色按访问类型编码: read=蓝 / write=绿 / read-write=橙。
 * 支持鼠标悬停提示和点击选中。
 */
class SvdBitFieldWidget : public QWidget {
    Q_OBJECT

public:
    /** @brief 位字段可视数据项 */
    struct FieldItem {
        QString name;           ///< 字段名称
        int bitOffset = 0;      ///< 起始位偏移
        int bitWidth = 1;       ///< 位宽
        QString access;         ///< 访问权限(read/write/read-write)
        QString description;    ///< 字段描述
        quint32 value = 0;      ///< 当前值(用于显示)
    };

    /** @brief 构造位字段可视化控件 @param parent 父控件 */
    explicit SvdBitFieldWidget(QWidget* parent = nullptr);

    /** @brief 设置寄存器数据，从SvdRegister提取字段并重绘 @param reg SVD寄存器结构体 */
    void setRegister(const SvdRegister& reg);

    /** @brief 设置寄存器总位宽 @param bits 位宽(通常8/16/32) */
    void setTotalBits(int bits);

    /** @brief 设置字段值(更新显示) @param value 寄存器值 */
    void setFieldValue(quint32 value);

    /** @brief 清除所有字段 */
    void clear();

    /** @brief 获取当前选中字段索引 @return 索引，-1表示无选中 */
    int selectedFieldIndex() const;

    // ---- 统计接口 ----
    /** @brief 获取累计字段悬停次数 @return 悬停总次数 */
    quint64 totalFieldHoverCount() const { return m_totalFieldHoverCount; }
    /** @brief 获取累计字段点击次数 @return 点击总次数 */
    quint64 totalFieldClickCount() const { return m_totalFieldClickCount; }
    /** @brief 获取累计绘制次数 @return 绘制总次数 */
    quint64 totalPaintCount() const { return m_totalPaintCount; }
    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    /** @brief 字段被点击 @param index 字段索引 */
    void fieldClicked(int index);
    /** @brief 字段被悬停 @param index 字段索引，-1表示鼠标离开 */
    void fieldHovered(int index);

protected:
    /** @brief 自定义绘制事件 */
    void paintEvent(QPaintEvent* event) override;
    /** @brief 鼠标移动事件(悬停检测) */
    void mouseMoveEvent(QMouseEvent* event) override;
    /** @brief 鼠标点击事件(选中检测) */
    void mousePressEvent(QMouseEvent* event) override;
    /** @brief 建议最小尺寸 */
    QSize minimumSizeHint() const override;
    /** @brief 建议尺寸 */
    QSize sizeHint() const override;

private:
    /** @brief 根据访问类型返回对应颜色 @param access 访问类型字符串 @return 颜色 */
    static QColor accessColor(const QString& access);
    /** @brief 根据像素X坐标计算命中的字段索引 @param pixelX 像素X坐标 @return 字段索引，-1未命中 */
    int hitTestField(int pixelX) const;
    /** @brief 生成字段的悬停提示文本 @param field 字段数据 @return HTML格式提示 */
    QString fieldTooltip(const FieldItem& field) const;

    QList<FieldItem> m_fields;          ///< 字段可视数据列表
    int m_totalBits = 32;               ///< 寄存器总位宽
    quint32 m_registerValue = 0;        ///< 当前寄存器值
    int m_selectedIndex = -1;           ///< 选中字段索引
    int m_hoveredIndex = -1;            ///< 悬停字段索引

    static constexpr int kRowHeight = 56;       ///< 行高(像素)
    static constexpr int kTopMargin = 20;       ///< 顶部边距(刻度标尺区域)
    static constexpr int kSideMargin = 10;      ///< 左右边距

    quint64 m_totalFieldHoverCount = 0; ///< 累计字段悬停次数
    quint64 m_totalFieldClickCount = 0; ///< 累计字段点击次数
    quint64 m_totalPaintCount = 0;      ///< 累计绘制次数
};

#endif // SVDBITFIELDWIDGET_H
