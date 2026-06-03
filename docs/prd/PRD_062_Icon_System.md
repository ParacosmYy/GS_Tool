# PRD-062: IconManager SVG图标系统 — Lucide图标集缓存着色

## 背景
项目UI现代化需要统一的图标体系。当前各面板/按钮缺少图标或使用零散的文本标记，视觉一致性差。引入IconManager单例，基于Lucide开源图标集提供SVG加载、缓存和动态着色能力。通过QSvgRenderer渲染到QPixmap，再用QPainter CompositionMode_SourceIn实现主题色着色，避免维护多套颜色的SVG文件。

## 需求列表
| ID | 需求描述 | 优先级 | 涉及模块 |
|----|---------|--------|---------|
| R1 | IconManager单例: icon(name)/pixmap(name, size)/clearCache() | P0 | core/theme/ |
| R2 | SVG着色管线: QSvgRenderer → QPixmap → QPainter SourceIn | P0 | core/theme/ |
| R3 | 50个Lucide SVG图标导入resources/icons/lucide/ | P0 | resources/ |
| R4 | 主题集成: themeChanged → clearCache + setTintColor(TextPrimary) | P0 | core/theme/ |
| R5 | 图标分类目录: nav/connection/terminal/data/files/actions/status/layout/special | P1 | resources/ |
| R6 | QRC资源注册 | P0 | resources/resources.qrc |

## 接口设计

### IconManager类
```cpp
/**
 * @brief SVG图标管理器 -- 单例，负责图标加载/缓存/着色
 *
 * 着色管线:
 *   1. QSvgRenderer从.qrc加载SVG原始内容
 *   2. 渲染到QPixmap(默认devicePixelRatio适配)
 *   3. QPainter以CompositionMode_SourceIn叠加tintColor
 *   4. 结果缓存到m_cache，避免重复渲染
 *
 * 主题切换时自动clearCache()，下次请求以新tintColor重新渲染。
 */
class IconManager : public QObject {
    Q_OBJECT

public:
    static IconManager& instance();

    /** @brief 获取着色后的QIcon
     * @param name 图标名(如"connection/serial-port")
     * @param size 请求尺寸(默认QSize(20,20))
     */
    QIcon icon(const QString& name, const QSize& size = QSize(20, 20));

    /** @brief 获取着色后的QPixmap */
    QPixmap pixmap(const QString& name, const QSize& size = QSize(20, 20));

    /** @brief 设置全局着色颜色(默认ThemeManager::TextPrimary) */
    void setTintColor(const QColor& color);

    /** @brief 清空缓存(主题切换时调用) */
    void clearCache();

private slots:
    void onThemeChanged();

private:
    IconManager(QObject* parent = nullptr);
    QPixmap renderSvg(const QString& name, const QSize& size);

    QColor m_tintColor;                              ///< 当前着色颜色
    QMap<QString, QIcon> m_cache;                    ///< name → QIcon缓存
    QMap<QString, QPixmap> m_pixmapCache;            ///< name@size → QPixmap缓存
};
```

### 图标分类(78个Lucide图标)
```
resources/icons/lucide/
  nav/         (10) home, panel-left, panel-right, layout-grid, list, menu, chevron-*
  connection/  (12) serial-port, wifi, bluetooth, usb, link, unlink, plug, ...
  terminal/    (8)  terminal, monitor, code, scroll-text, ...
  data/        (8)  database, bar-chart, activity, signal, ...
  files/       (8)  folder, file-text, save, download, upload, ...
  actions/     (12) play, pause, stop, copy, trash, search, ...
  status/      (5)  check, x, alert-triangle, info, loader
  layout/      (6)  maximize, minimize, columns, rows, split, ...
  special/     (9)  settings, palette, sun, moon, zap, ...
```

## 依赖的公共组件
- ThemeManager (core/theme/ThemeManager.h) — color(TextPrimary)作为默认tintColor, themeChanged信号
- Qt SVG模块 (QSvgRenderer) — SVG文件解析渲染

## 设计模式
- **单例模式**: IconManager全局唯一实例，通过instance()访问
- **缓存模式**: QMap缓存已渲染图标，避免重复SVG解析和着色
- **观察者模式**: 监听ThemeManager::themeChanged自动clearCache + setTintColor

## 影响范围
| 文件 | 变更类型 | 风险 |
|------|---------|------|
| src/core/theme/IconManager.h | 新增 | 无 |
| src/core/theme/IconManager.cpp | 新增 | 无 |
| resources/icons/lucide/*.svg | 新增(78个) | 无 |
| resources/resources.qrc | 修改(注册图标) | 低 |
| CMakeLists.txt | 修改(Qt6::Svg) | 低 |

## 验收标准
1. IconManager::instance()返回同一对象(单例验证)
2. icon("connection/serial-port")返回非空QIcon，颜色为TextPrimary
3. 主题切换后图标颜色自动更新(Light→Dark→OneDark三轮)
4. clearCache()后m_cache和m_pixmapCache均为空
5. 连续调用icon()100次无性能劣化(缓存命中)
6. 78个Lucide SVG图标全部注册在QRC中
7. CMakeLists.txt包含Qt6::Svg模块
8. .h ≤ 200行, .cpp ≤ 500行
9. 编译零错误
