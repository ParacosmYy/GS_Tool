/**
 * @file LedMatrixSimulator.cpp
 * @brief LED矩阵模拟器 — NxM网格彩色LED显示仿真实现
 *
 * 管理rows*cols颜色网格的自定义绘制，支持鼠标交互和图像导入导出。
 * 统计重置逻辑见 LedMatrixSimulatorStats.cpp。
 */

#include "widgets/led_matrix/LedMatrixSimulator.h"

#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QtMath>

#include "core/theme/ThemeManager.h"

/**
 * @brief 构造函数 — 初始化8x8默认网格
 * @param parent 父控件指针
 */
LedMatrixSimulator::LedMatrixSimulator(QWidget* parent)
    : QWidget(parent)
    , m_rows(8)
    , m_cols(8)
    , m_interval(2)
    , m_minLedSize(12)
    , m_brightness(1.0)
    , m_shape(Shape::Circle)
{
    setObjectName("LedMatrixSimulator");
    m_grid.fill(Qt::black, m_rows * m_cols);
}

/**
 * @brief 设置网格尺寸 — 清空所有LED颜色
 * @param rows 行数(>=1)
 * @param cols 列数(>=1)
 */
void LedMatrixSimulator::setGridSize(int rows, int cols)
{
    rows = qMax(1, rows);
    cols = qMax(1, cols);

    if (rows == m_rows && cols == m_cols) {
        return;
    }

    m_rows = rows;
    m_cols = cols;
    m_grid.fill(Qt::black, m_rows * m_cols);

    const int gridSize = m_rows * m_cols;
    if (gridSize > m_stats.maxGridSize) {
        m_stats.maxGridSize = gridSize;
    }

    updateGeometry();
    update();

    emit gridSizeChanged(m_rows, m_cols);
}

/**
 * @brief 设置指定位置的LED颜色
 * @param row 行索引(0-based)
 * @param col 列索引(0-based)
 * @param color LED颜色
 */
void LedMatrixSimulator::setLed(int row, int col, const QColor& color)
{
    if (row < 0 || row >= m_rows || col < 0 || col >= m_cols) {
        return;
    }

    m_grid[row * m_cols + col] = color;
    ++m_stats.totalLedChanges;

    emit ledChanged(row, col, color);
    update();
}

/**
 * @brief 清空全部LED为关闭状态(黑色)
 */
void LedMatrixSimulator::clear()
{
    m_grid.fill(Qt::black, m_rows * m_cols);
    ++m_stats.totalClears;
    update();
}

/**
 * @brief 设置LED形状
 * @param shape 形状枚举
 */
void LedMatrixSimulator::setShape(Shape shape)
{
    if (m_shape == shape) {
        return;
    }
    m_shape = shape;
    update();
}

/**
 * @brief 设置LED间距
 * @param pixels 间距值(>=0)
 */
void LedMatrixSimulator::setInterval(int pixels)
{
    m_interval = qMax(0, pixels);
    update();
}

/**
 * @brief 设置全局亮度因子
 * @param brightness 亮度(0.0~1.0)
 */
void LedMatrixSimulator::setBrightness(double brightness)
{
    m_brightness = qBound(0.0, brightness, 1.0);
    update();
}

/**
 * @brief 从QImage导入图像到矩阵
 * @param image 源图像(自动缩放到rows*cols)
 */
void LedMatrixSimulator::fillImage(const QImage& image)
{
    if (image.isNull()) {
        return;
    }

    /* 将图像缩放到矩阵尺寸，逐像素采样 */
    const QImage scaled = image.scaled(m_cols, m_rows, Qt::IgnoreAspectRatio,
                                       Qt::SmoothTransformation);

    for (int r = 0; r < m_rows; ++r) {
        for (int c = 0; c < m_cols; ++c) {
            m_grid[r * m_cols + c] = scaled.pixelColor(c, r);
        }
    }

    ++m_stats.totalImageImports;
    ++m_stats.totalLedChanges;
    update();
}

/**
 * @brief 将当前矩阵导出为QImage
 * @return 每个LED对应一个像素的图像
 */
QImage LedMatrixSimulator::toImage() const
{
    QImage image(m_cols, m_rows, QImage::Format_ARGB32);

    for (int r = 0; r < m_rows; ++r) {
        for (int c = 0; c < m_cols; ++c) {
            image.setPixelColor(c, r, m_grid[r * m_cols + c]);
        }
    }

    return image;
}

/**
 * @brief 导出当前矩阵为PNG文件
 * @param filePath 目标文件路径
 * @return true导出成功
 */
bool LedMatrixSimulator::exportToPng(const QString& filePath)
{
    /* 以4倍分辨率渲染矩阵，使导出图片清晰可见 */
    const int scale = 4;
    const int ledSize = 16 * scale;
    const int gap = m_interval * scale;
    const int cellSize = ledSize + gap;
    const int imgW = m_cols * cellSize + gap;
    const int imgH = m_rows * cellSize + gap;

    QImage image(imgW, imgH, QImage::Format_ARGB32);
    image.fill(Qt::black);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);

    for (int r = 0; r < m_rows; ++r) {
        for (int c = 0; c < m_cols; ++c) {
            const QColor& color = m_grid[r * m_cols + c];
            const qreal cx = gap + c * cellSize + ledSize / 2.0;
            const qreal cy = gap + r * cellSize + ledSize / 2.0;

            drawSingleLed(painter, cx, cy, ledSize, color);
        }
    }

    painter.end();
    ++m_stats.totalExports;

    return image.save(filePath, "PNG");
}

/**
 * @brief 获取统计数据的只读引用
 * @return Stats常量引用
 */
const LedMatrixSimulator::Stats& LedMatrixSimulator::stats() const
{
    return m_stats;
}

/**
 * @brief 获取当前行数
 * @return 行数
 */
int LedMatrixSimulator::rows() const
{
    return m_rows;
}

/**
 * @brief 获取当前列数
 * @return 列数
 */
int LedMatrixSimulator::cols() const
{
    return m_cols;
}

/**
 * @brief 获取指定位置LED颜色
 * @param row 行
 * @param col 列
 * @return LED颜色(越界返回黑色)
 */
QColor LedMatrixSimulator::ledColor(int row, int col) const
{
    if (row < 0 || row >= m_rows || col < 0 || col >= m_cols) {
        return Qt::black;
    }
    return m_grid[row * m_cols + col];
}

/**
 * @brief 获取当前LED形状
 * @return 形状枚举
 */
LedMatrixSimulator::Shape LedMatrixSimulator::shape() const
{
    return m_shape;
}

/**
 * @brief 获取当前LED间距
 * @return 间距(像素)
 */
int LedMatrixSimulator::interval() const
{
    return m_interval;
}

/**
 * @brief 获取当前亮度因子
 * @return 亮度(0.0~1.0)
 */
double LedMatrixSimulator::brightness() const
{
    return m_brightness;
}

/**
 * @brief 绘制事件 — 背景→LED网格
 * @param event 绘制事件
 */
void LedMatrixSimulator::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    ++m_stats.totalPaints;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    auto& theme = ThemeManager::instance();

    /* 背景 */
    painter.fillRect(rect(), theme.color(ThemeManager::SemanticColor::BgPrimary));

    const int ledSize = computeLedSize();
    if (ledSize < 2) {
        painter.end();
        return;
    }

    const QPointF offset = computeGridOffset(ledSize);

    /* 绘制每个LED */
    for (int r = 0; r < m_rows; ++r) {
        for (int c = 0; c < m_cols; ++c) {
            const QColor& rawColor = m_grid[r * m_cols + c];
            const QPointF center = offset + QPointF(
                m_interval + c * (ledSize + m_interval) + ledSize / 2.0,
                m_interval + r * (ledSize + m_interval) + ledSize / 2.0);

            /* 应用亮度因子 */
            QColor color = rawColor;
            if (m_brightness < 1.0) {
                color = QColor::fromHsvF(color.hueF(), color.saturationF(),
                                         color.valueF() * m_brightness,
                                         color.alphaF());
            }

            drawSingleLed(painter, center.x(), center.y(), ledSize, color);
        }
    }

    painter.end();
}

/**
 * @brief 鼠标按下事件 — 左键切换LED、右键取色
 * @param event 鼠标事件
 */
void LedMatrixSimulator::mousePressEvent(QMouseEvent* event)
{
    const int ledSize = computeLedSize();
    if (ledSize < 2) {
        return;
    }

    const auto [row, col] = posToRowCol(event->position(), ledSize);
    if (row < 0) {
        return;
    }

    ++m_stats.totalMouseClicks;

    if (event->button() == Qt::LeftButton) {
        /* 左键: 切换LED — 黑色变绿，其他变黑色 */
        QColor current = m_grid[row * m_cols + col];
        QColor newColor = (current == Qt::black) ? QColor(0, 255, 0) : Qt::black;
        setLed(row, col, newColor);
    }
}

/**
 * @brief 滚轮事件 — 调整LED最小尺寸
 * @param event 滚轮事件
 */
void LedMatrixSimulator::wheelEvent(QWheelEvent* event)
{
    const int delta = (event->angleDelta().y() > 0) ? 2 : -2;
    m_minLedSize = qBound(4, m_minLedSize + delta, 64);
    update();
    event->accept();
}

/**
 * @brief 建议最小尺寸
 * @return 基于网格和最小LED尺寸计算
 */
QSize LedMatrixSimulator::minimumSizeHint() const
{
    const int cell = m_minLedSize + m_interval;
    return QSize(m_cols * cell + m_interval, m_rows * cell + m_interval);
}

/**
 * @brief 根据控件尺寸计算每个LED的像素大小
 * @return LED边长(>=2)
 */
int LedMatrixSimulator::computeLedSize() const
{
    const int gapTotal_w = (m_cols + 1) * m_interval;
    const int gapTotal_h = (m_rows + 1) * m_interval;
    const int avail_w = width() - gapTotal_w;
    const int avail_h = height() - gapTotal_h;

    if (avail_w <= 0 || avail_h <= 0) {
        return m_minLedSize;
    }

    const int sizeByW = avail_w / m_cols;
    const int sizeByH = avail_h / m_rows;
    return qMax(m_minLedSize, qMin(sizeByW, sizeByH));
}

/**
 * @brief 计算网格绘制区域的总偏移量(居中对齐)
 * @param ledSize LED大小
 * @return 左上角偏移
 */
QPointF LedMatrixSimulator::computeGridOffset(int ledSize) const
{
    const qreal gridW = m_cols * (ledSize + m_interval) + m_interval;
    const qreal gridH = m_rows * (ledSize + m_interval) + m_interval;
    return QPointF((width() - gridW) / 2.0, (height() - gridH) / 2.0);
}

/**
 * @brief 根据鼠标位置计算对应的行列索引
 * @param pos 鼠标位置
 * @param ledSize LED大小
 * @return 行列对(-1表示越界)
 */
QPair<int, int> LedMatrixSimulator::posToRowCol(const QPointF& pos, int ledSize) const
{
    const QPointF offset = computeGridOffset(ledSize);
    const qreal relX = pos.x() - offset.x() - m_interval;
    const qreal relY = pos.y() - offset.y() - m_interval;

    const int col = static_cast<int>(relX / (ledSize + m_interval));
    const int row = static_cast<int>(relY / (ledSize + m_interval));

    /* 检查是否在LED区域内(而非间距区域) */
    if (row < 0 || row >= m_rows || col < 0 || col >= m_cols) {
        return {-1, -1};
    }

    const qreal localX = relX - col * (ledSize + m_interval);
    const qreal localY = relY - row * (ledSize + m_interval);
    if (localX < 0 || localX > ledSize || localY < 0 || localY > ledSize) {
        return {-1, -1};
    }

    return {row, col};
}

/**
 * @brief 绘制单个LED单元(根据当前Shape绘制对应形状)
 * @param painter 画笔引用
 * @param cx 中心X
 * @param cy 中心Y
 * @param size LED大小
 * @param color LED颜色(已应用亮度)
 */
void LedMatrixSimulator::drawSingleLed(QPainter& painter, qreal cx, qreal cy,
                                        int size, const QColor& color) const
{
    const qreal half = size / 2.0;
    painter.setPen(Qt::NoPen);

    switch (m_shape) {
    case Shape::Circle: {
        /* 径向渐变模拟LED发光效果 */
        QRadialGradient gradient(QPointF(cx, cy), half);
        if (color == Qt::black) {
            /* 关闭状态: 暗灰色 */
            auto& theme = ThemeManager::instance();
            QColor offColor = theme.color(ThemeManager::SemanticColor::Border);
            gradient.setColorAt(0.0, offColor.darker(150));
            gradient.setColorAt(1.0, offColor.darker(200));
        } else {
            /* 开启状态: 中心高亮 → 原色 → 暗边缘 */
            gradient.setColorAt(0.0, color.lighter(160));
            gradient.setColorAt(0.6, color);
            gradient.setColorAt(1.0, color.darker(200));
        }
        painter.setBrush(gradient);
        painter.drawEllipse(QPointF(cx, cy), half, half);
        break;
    }
    case Shape::Square: {
        if (color == Qt::black) {
            auto& theme = ThemeManager::instance();
            QColor offColor = theme.color(ThemeManager::SemanticColor::Border);
            painter.setBrush(offColor.darker(150));
        } else {
            painter.setBrush(color);
        }
        painter.drawRect(QRectF(cx - half, cy - half, size, size));
        break;
    }
    case Shape::RoundedSquare: {
        if (color == Qt::black) {
            auto& theme = ThemeManager::instance();
            QColor offColor = theme.color(ThemeManager::SemanticColor::Border);
            painter.setBrush(offColor.darker(150));
        } else {
            painter.setBrush(color);
        }
        const qreal radius = size * 0.2;
        painter.drawRoundedRect(QRectF(cx - half, cy - half, size, size),
                                radius, radius);
        break;
    }
    }
}
