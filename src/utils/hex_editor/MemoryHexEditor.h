/**
 * @file MemoryHexEditor.h
 * @brief 十六进制内存编辑器 -- 二进制数据查看/编辑控件
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 经典三栏式十六进制编辑器(偏移 | HEX | ASCII)，支持键盘导航与编辑、
 * 字节模式搜索、自定义高亮区域、鼠标选区拖拽，以及多格式导出
 * (bin / Intel HEX / C 数组)。适用于嵌入式调试场景下查看原始内存转储、
 * 寄存器值和固件映像。
 */

#ifndef MEMORYHEXEDITOR_H
#define MEMORYHEXEDITOR_H

#include <QByteArray>
#include <QColor>
#include <QFont>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPointF>
#include <QRect>
#include <QVector>
#include <QWheelEvent>
#include <QWidget>

/**
 * @class MemoryHexEditor
 * @brief 十六进制内存编辑器控件
 *
 * 纯自绘 QWidget，通过 paintEvent 完成偏移/HEX/ASCII 三栏渲染，
 * 仅绘制可见行以保持高效。所有颜色通过 ThemeManager::color() 获取，
 * 支持主题切换自动重绘。
 */
class MemoryHexEditor : public QWidget {
    Q_OBJECT

public:
    /** @brief 显示模式枚举 */
    enum class DisplayMode {
        HexOnly,      ///< 仅显示HEX列
        HexAndAscii,  ///< HEX + ASCII 双栏(默认)
        AsciiOnly     ///< 仅显示ASCII列
    };
    Q_ENUM(DisplayMode)

    /** @brief 选区结构 */
    struct Selection {
        int startOffset = -1; ///< 选区起始字节偏移(-1表示无选区)
        int endOffset   = -1; ///< 选区结束字节偏移(-1表示无选区)
    };

    /** @brief 编辑器运行时统计数据结构 */
    struct Stats {
        quint64 totalDataLoads   = 0; ///< 累计数据加载次数
        quint64 totalByteEdits   = 0; ///< 累计字节编辑次数
        quint64 totalScrollEvents = 0; ///< 累计滚动事件次数
        quint64 totalSearches    = 0; ///< 累计搜索次数
        quint64 totalCopies      = 0; ///< 累计复制次数
        quint64 totalExports     = 0; ///< 累计导出次数
        quint64 bytesViewed      = 0; ///< 累计查看字节数
        quint64 bytesEdited      = 0; ///< 累计编辑字节数
    };

    // ---- 构造 / 析构 ----

    /** @brief 构造十六进制内存编辑器 @param parent 父控件 */
    explicit MemoryHexEditor(QWidget* parent = nullptr);

    // ---- 数据接口 ----

    /** @brief 设置要显示/编辑的二进制数据 @param data 字节数组 */
    void setData(const QByteArray& data);

    /** @brief 获取当前完整数据 @return 字节数组 */
    QByteArray data() const;

    /** @brief 设置每行显示字节数 @param bytes 8 或 16 */
    void setBytesPerLine(int bytes);

    /** @brief 获取每行字节数 @return 当前值 */
    int bytesPerLine() const;

    /** @brief 设置显示模式 @param mode 显示模式枚举 */
    void setDisplayMode(DisplayMode mode);

    /** @brief 设置只读模式 @param readOnly true禁用编辑 */
    void setReadOnly(bool readOnly);

    // ---- 高亮 / 导航 ----

    /** @brief 添加高亮区域 @param start 起始偏移 @param end 结束偏移 @param color 高亮颜色 */
    void setHighlightRange(int start, int end, const QColor& color);

    /** @brief 清除所有自定义高亮 */
    void clearHighlights();

    /** @brief 滚动到指定偏移量处 @param offset 目标字节偏移 */
    void scrollToOffset(int offset);

    // ---- 选区 ----

    /** @brief 获取当前选区 @return Selection结构 */
    Selection selection() const;

    /** @brief 获取选中的字节数据 @return 选中部分的副本 */
    QByteArray selectedBytes() const;

    // ---- 搜索 ----

    /** @brief 向后搜索字节模式 @param pattern 搜索模式 @param fromOffset 起始偏移(默认0) @return 找到的偏移，-1为未找到 */
    int findNext(const QByteArray& pattern, int fromOffset = 0);

    /** @brief 向前搜索字节模式 @param pattern 搜索模式 @param fromOffset 起始偏移(-1为末尾) @return 找到的偏移，-1为未找到 */
    int findPrev(const QByteArray& pattern, int fromOffset = -1);

    // ---- 导出 ----

    /** @brief 导出为原始二进制文件 @param filePath 目标路径 @return 成功true */
    bool exportToBin(const QString& filePath);

    /** @brief 导出为 Intel HEX 格式文件 @param filePath 目标路径 @return 成功true */
    bool exportToHex(const QString& filePath);

    /** @brief 导出为 C 语言 uint8_t 数组 @param filePath 目标路径 @return 成功true */
    bool exportToCpp(const QString& filePath);

    // ---- 统计 ----

    /** @brief 获取统计数据 @return Stats常量引用 */
    const Stats& stats() const;

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    /** @brief 数据内容发生变更信号 */
    void dataChanged();

    /** @brief 选区变更信号 @param start 起始偏移 @param end 结束偏移 */
    void selectionChanged(int start, int end);

    /** @brief 单字节编辑信号 @param offset 偏移 @param oldVal 旧值 @param newVal 新值 */
    void byteEdited(int offset, uint8_t oldVal, uint8_t newVal);

    /** @brief 搜索命中信号 @param offset 匹配偏移 */
    void searchFound(int offset);

protected:
    /** @brief 自绘事件 -- 渲染偏移/HEX/ASCII三栏 */
    void paintEvent(QPaintEvent* event) override;

    /** @brief 鼠标按下 -- 设置光标/开始选区 */
    void mousePressEvent(QMouseEvent* event) override;

    /** @brief 鼠标移动 -- 拖拽扩展选区 */
    void mouseMoveEvent(QMouseEvent* event) override;

    /** @brief 键盘按下 -- 导航/编辑/快捷键 */
    void keyPressEvent(QKeyEvent* event) override;

    /** @brief 鼠标滚轮 -- 上下滚动 */
    void wheelEvent(QWheelEvent* event) override;

    /** @brief 最小尺寸提示 */
    QSize minimumSizeHint() const override;

private:
    /** @brief 自定义高亮区域结构 */
    struct Highlight {
        int start;      ///< 起始偏移
        int end;        ///< 结束偏移
        QColor color;   ///< 高亮颜色
    };

    // ---- 数据成员 ----
    QByteArray m_data;              ///< 当前数据缓冲区
    int m_bytesPerLine    = 16;     ///< 每行字节数(8或16)
    DisplayMode m_displayMode = DisplayMode::HexAndAscii; ///< 显示模式
    bool m_readOnly       = false;  ///< 只读标记
    int m_scrollOffset    = 0;      ///< 垂直滚动偏移(行数)
    int m_cursorPos       = 0;      ///< 光标所在字节偏移
    bool m_cursorInHex    = true;   ///< 光标在HEX区(true)还是ASCII区
    int m_nibbleIndex     = 0;      ///< 当前半字节索引(0=高4位,1=低4位)
    Selection m_selection;          ///< 当前选区
    QVector<Highlight> m_highlights; ///< 自定义高亮列表

    // ---- 布局度量 ----
    int m_charWidth       = 0;      ///< 单字符像素宽度
    int m_lineHeight      = 0;      ///< 单行像素高度
    int m_addressWidth    = 0;      ///< 地址栏像素宽度
    int m_hexAreaWidth    = 0;      ///< HEX区域像素宽度
    int m_asciiAreaWidth  = 0;      ///< ASCII区域像素宽度
    int m_areaGap         = 16;     ///< 区域间间隔像素
    int m_leftMargin      = 4;      ///< 左边距像素
    int m_topMargin       = 4;      ///< 上边距像素
    QFont m_monoFont;              ///< 等宽字体

    // ---- 统计 ----
    Stats m_stats;                  ///< 运行时统计

    // ---- 内部辅助方法 ----
    int totalLines() const;         ///< 数据总行数
    int visibleLines() const;       ///< 可见行数
    int lineAtY(int y) const;       ///< Y坐标对应行号
    int byteAtPos(const QPoint& pos) const; ///< 像素坐标对应字节偏移
    void ensureCursorVisible();     ///< 确保光标在可见区域
    void calculateLayout();         ///< 根据字体度量计算布局尺寸
    int maxScrollOffset() const;    ///< 最大滚动偏移(行数)

    /** @brief 判断字节是否可显示ASCII @param byte 输入字节 @return 可显示字符或'.' */
    static char toPrintable(uint8_t byte);
};

#endif // MEMORYHEXEDITOR_H
