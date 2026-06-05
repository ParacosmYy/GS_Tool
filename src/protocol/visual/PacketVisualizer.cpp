/**
 * @file PacketVisualizer.cpp
 * @brief 数据包结构可视化控件实现 -- 构造/析构/数据管理/鼠标交互
 *
 * 包含构造/析构、setData/setFieldDefs/addFieldDef/removeFieldDef/modifyFieldDef、
 * setViewMode/zoomIn/zoomOut/zoomToFit 等数据与视图管理方法，
 * 以及鼠标事件处理和辅助方法。
 * 绘制逻辑在 PacketVisualizerPaint.cpp。
 * Stats 重置逻辑在 PacketVisualizerStats.cpp。
 */

#include "protocol/visual/PacketVisualizer.h"

#include <QMouseEvent>
#include <QToolTip>
#include <algorithm>

// ==================== 构造 / 析构 ====================

/** @brief 构造函数，初始化控件属性和鼠标追踪 @param parent 父Widget */
PacketVisualizer::PacketVisualizer(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("PacketVisualizer");
    setMinimumHeight(kRowHeight);
    setMouseTracking(true);   ///< 启用鼠标追踪以支持悬停检测
}

/** @brief 析构函数 */
PacketVisualizer::~PacketVisualizer() = default;

// ==================== 数据设置接口 ====================

/** @brief 设置原始数据包内容 @param data 原始字节数据 */
void PacketVisualizer::setData(const QByteArray &data)
{
    m_data = data;
    ++m_stats.totalPacketsVisualized;
    update();
}

/** @brief 设置字段定义列表(替换全部) @param defs 字段定义列表 */
void PacketVisualizer::setFieldDefs(const QList<FieldDef> &defs)
{
    m_fieldDefs = defs;
    m_selectedIndex = -1;
    m_hoveredIndex = -1;
    update();
}

/** @brief 添加单个字段定义到列表末尾 @param def 字段定义 */
void PacketVisualizer::addFieldDef(const FieldDef &def)
{
    m_fieldDefs.append(def);
    update();
}

/**
 * @brief 移除指定索引的字段定义
 * @param index 字段索引
 * @return true=移除成功, false=索引越界
 */
bool PacketVisualizer::removeFieldDef(int index)
{
    if (index < 0 || index >= m_fieldDefs.size()) {
        return false;
    }
    m_fieldDefs.removeAt(index);
    /* 调整选中/悬停索引 */
    if (m_selectedIndex == index) {
        m_selectedIndex = -1;
    } else if (m_selectedIndex > index) {
        --m_selectedIndex;
    }
    if (m_hoveredIndex == index) {
        m_hoveredIndex = -1;
    } else if (m_hoveredIndex > index) {
        --m_hoveredIndex;
    }
    update();
    return true;
}

/**
 * @brief 修改指定索引的字段定义
 * @param index 字段索引
 * @param def 新的字段定义
 * @return true=修改成功, false=索引越界
 */
bool PacketVisualizer::modifyFieldDef(int index, const FieldDef &def)
{
    if (index < 0 || index >= m_fieldDefs.size()) {
        return false;
    }
    m_fieldDefs[index] = def;
    update();
    return true;
}

/** @brief 清空所有字段定义和选中状态 */
void PacketVisualizer::clearFieldDefs()
{
    m_fieldDefs.clear();
    m_selectedIndex = -1;
    m_hoveredIndex = -1;
    update();
}

// ==================== 视图控制接口 ====================

/** @brief 设置视图模式(字节级/位级) @param mode 视图模式 */
void PacketVisualizer::setViewMode(ViewMode mode)
{
    if (m_viewMode != mode) {
        m_viewMode = mode;
        if (m_autoFit) {
            m_zoomLevel = computeFitZoom();
        }
        update();
    }
}

/** @brief 获取当前视图模式 @return 视图模式 */
PacketVisualizer::ViewMode PacketVisualizer::viewMode() const
{
    return m_viewMode;
}

/** @brief 放大视图(缩放级别乘以1.5) */
void PacketVisualizer::zoomIn()
{
    m_autoFit = false;
    m_zoomLevel = qMin(m_zoomLevel * 1.5, kMaxZoom);
    ++m_stats.totalZoomOperations;
    update();
}

/** @brief 缩小视图(缩放级别除以1.5) */
void PacketVisualizer::zoomOut()
{
    m_autoFit = false;
    m_zoomLevel = qMax(m_zoomLevel / 1.5, kMinZoom);
    ++m_stats.totalZoomOperations;
    update();
}

/** @brief 自动适应控件宽度 */
void PacketVisualizer::zoomToFit()
{
    m_autoFit = true;
    m_zoomLevel = computeFitZoom();
    ++m_stats.totalZoomOperations;
    update();
}

/** @brief 直接设置缩放级别 @param level 每单位(字节或位)对应的像素数 */
void PacketVisualizer::setZoomLevel(double level)
{
    m_autoFit = false;
    m_zoomLevel = qBound(kMinZoom, level, kMaxZoom);
    ++m_stats.totalZoomOperations;
    update();
}

/** @brief 获取当前缩放级别 @return 每单位像素数 */
double PacketVisualizer::zoomLevel() const
{
    return m_zoomLevel;
}

// ==================== 查询接口 ====================

/** @brief 获取当前选中字段索引 @return 字段索引, -1表示无选中 */
int PacketVisualizer::selectedFieldIndex() const
{
    return m_selectedIndex;
}

/** @brief 获取字段定义列表 @return 字段定义列表的只读引用 */
const QList<PacketVisualizer::FieldDef>& PacketVisualizer::fieldDefs() const
{
    return m_fieldDefs;
}

/** @brief 获取原始数据 @return 原始字节数据 */
QByteArray PacketVisualizer::data() const
{
    return m_data;
}

// ==================== 统计接口 ====================

/** @brief 获取统计数据 @return Stats常量引用 */
const PacketVisualizer::Stats& PacketVisualizer::stats() const
{
    return m_stats;
}

/** @brief 建议最小尺寸 @return 宽度200 x 高度kRowHeight */
QSize PacketVisualizer::minimumSizeHint() const
{
    return QSize(200, kRowHeight);
}

/** @brief 建议尺寸 @return 宽度400 x 高度kRowHeight */
QSize PacketVisualizer::sizeHint() const
{
    return QSize(400, kRowHeight);
}

// ==================== 鼠标事件 ====================

/**
 * @brief 鼠标移动事件处理
 *
 * 检测鼠标是否悬停在某个字段上，更新悬停索引并显示工具提示。
 * @param event 鼠标事件
 */
void PacketVisualizer::mouseMoveEvent(QMouseEvent *event)
{
    int newHovered = hitTestField(event->pos().x());

    if (newHovered != m_hoveredIndex) {
        m_hoveredIndex = newHovered;
        emit fieldHovered(newHovered);
        update();  ///< 触发重绘以更新悬停高亮

        /* 显示/隐藏工具提示 */
        if (newHovered >= 0 && newHovered < m_fieldDefs.size()) {
            const auto &def = m_fieldDefs[newHovered];
            QString tooltip = tr("<b>%1</b><br>"
                                "Offset: %2<br>"
                                "Length: %3<br>"
                                "Value: %4<br>"
                                "%5")
                                  .arg(def.name)
                                  .arg(def.offset)
                                  .arg(def.length)
                                  .arg(fieldHexValue(def))
                                  .arg(def.description);
            QToolTip::showText(event->globalPosition().toPoint(), tooltip, this);
        } else {
            QToolTip::hideText();
        }
    }
}

/**
 * @brief 鼠标点击事件处理
 *
 * 检测点击是否在某个字段上，更新选中索引并发射信号。
 * @param event 鼠标事件
 */
void PacketVisualizer::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        int clickedIndex = hitTestField(event->pos().x());
        if (clickedIndex >= 0) {
            m_selectedIndex = clickedIndex;
            ++m_stats.totalFieldClicks;
            emit fieldClicked(clickedIndex);
        } else {
            m_selectedIndex = -1;
        }
        update();
    }
}

// ==================== 私有辅助方法 ====================

/**
 * @brief 根据像素X坐标计算命中的字段索引
 * @param pixelX 像素X坐标
 * @return 字段索引, -1表示未命中
 */
int PacketVisualizer::hitTestField(int pixelX) const
{
    if (m_data.isEmpty() && m_fieldDefs.isEmpty()) {
        return -1;
    }

    /* 将像素转换为偏移量 */
    double adjustedX = pixelX - m_scrollOffset;
    double offset = adjustedX / m_zoomLevel;

    for (int i = 0; i < m_fieldDefs.size(); ++i) {
        const auto &def = m_fieldDefs[i];
        if (offset >= def.offset && offset < (def.offset + def.length)) {
            return i;
        }
    }
    return -1;
}

/**
 * @brief 计算数据包总长度(字节或位)
 *
 * 有字段定义时取最大 offset+length，否则取数据长度。
 * @return 总长度
 */
int PacketVisualizer::totalPacketLength() const
{
    if (!m_fieldDefs.isEmpty()) {
        int maxEnd = 0;
        for (const auto &def : m_fieldDefs) {
            maxEnd = qMax(maxEnd, def.offset + def.length);
        }
        return maxEnd;
    }
    if (m_viewMode == ViewMode::BitLevel) {
        return m_data.size() * 8;
    }
    return qMax(1, m_data.size());
}

/**
 * @brief 获取字段的十六进制值字符串
 * @param def 字段定义
 * @return 十六进制字符串
 */
QString PacketVisualizer::fieldHexValue(const FieldDef &def) const
{
    if (m_data.isEmpty()) {
        return QStringLiteral("--");
    }

    if (m_viewMode == ViewMode::BitLevel) {
        /* 位级模式: 提取对应字节范围 */
        int startByte = def.offset / 8;
        int endByte = (def.offset + def.length - 1) / 8;
        endByte = qMin(endByte, m_data.size() - 1);
        startByte = qBound(0, startByte, m_data.size() - 1);

        QStringList hexParts;
        for (int i = startByte; i <= endByte; ++i) {
            hexParts << QString("%1").arg(static_cast<unsigned char>(m_data[i]), 2, 16, QChar('0')).toUpper();
        }
        return hexParts.join(" ");
    }

    /* 字节级模式 */
    int start = qBound(0, def.offset, m_data.size() - 1);
    int end = qBound(0, def.offset + def.length - 1, m_data.size() - 1);

    QStringList hexParts;
    for (int i = start; i <= end; ++i) {
        hexParts << QString("%1").arg(static_cast<unsigned char>(m_data[i]), 2, 16, QChar('0')).toUpper();
    }
    return hexParts.join(" ");
}

/**
 * @brief 计算自适应缩放级别
 *
 * 使数据包恰好填充控件宽度(留出边距)。
 * @return 适合控件宽度的缩放级别
 */
double PacketVisualizer::computeFitZoom() const
{
    int totalLen = totalPacketLength();
    if (totalLen <= 0) {
        return kDefaultZoom;
    }
    int availableWidth = qMax(100, width() - 20);  ///< 左右各10像素边距
    return qBound(kMinZoom, static_cast<double>(availableWidth) / totalLen, kMaxZoom);
}

// paintEvent 实现在 PacketVisualizerPaint.cpp
// resetStatistics 实现在 PacketVisualizerStats.cpp
