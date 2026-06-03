# PRD-063: 空状态与加载状态组件 — EmptyStateWidget/LoadingSpinner/SkeletonWidget

## 背景
当前面板在无数据或加载中时缺乏友好的状态提示。用户打开DataStatistics面板看到空白，SearchBar搜索无结果时没有反馈。需要提供三个通用状态组件: 空状态(EmptyStateWidget)、加载旋转(Loadingspinner)、骨架屏(SkeletonWidget)，全部基于ThemeManager::color()实现主题适配，统一应用到所有需要状态提示的面板。

## 需求列表
| ID | 需求描述 | 优先级 | 涉及模块 |
|----|---------|--------|---------|
| R1 | EmptyStateWidget: icon(48px) + title(bold 14px) + description(muted 12px) + action按钮 | P0 | core/widgets/ |
| R2 | LoadingSpinner: QPainter弧线旋转动画, 1000ms linear loop | P0 | core/widgets/ |
| R3 | SkeletonWidget: 灰色块 + shimmer渐变动画, 1500ms linear loop | P1 | core/widgets/ |
| R4 | 应用到无连接状态(SerialConfigPanel空态) | P0 | serial/ |
| R5 | 应用到搜索无结果(TerminalSearchBar) | P0 | terminal/ |
| R6 | 应用到数据面板加载中(DataStatistics) | P1 | serial/ |

## 接口设计

### EmptyStateWidget
```cpp
/**
 * @brief 空状态提示组件 -- 居中显示图标+标题+描述+操作按钮
 *
 * 用法:
 *   auto* empty = new EmptyStateWidget(this);
 *   empty->setup("inbox", tr("暂无数据"), tr("连接串口后数据将显示在此"),
 *                tr("立即连接"), this, [this](){ openConnection(); });
 */
class EmptyStateWidget : public QWidget {
    Q_OBJECT

public:
    explicit EmptyStateWidget(QWidget* parent = nullptr);

    /** @brief 配置空状态显示内容
     * @param iconName IconManager图标名
     * @param title 标题(bold 14px TextPrimary)
     * @param description 描述(muted 12px TextSecondary)
     * @param actionText 按钮文本(空则不显示按钮)
     * @param parent 按钮信号接收对象
     * @param callback 按钮点击回调
     */
    void setup(const QString& iconName, const QString& title,
               const QString& description,
               const QString& actionText = QString(),
               QWidget* parent = nullptr,
               std::function<void()> callback = nullptr);

private slots:
    void onThemeChanged();

private:
    QLabel* m_iconLabel;
    QLabel* m_titleLabel;
    QLabel* m_descLabel;
    QPushButton* m_actionBtn = nullptr;
};
```

### LoadingSpinner
```cpp
/**
 * @brief 加载旋转动画 -- QPainter绘制弧线, 持续旋转
 */
class LoadingSpinner : public QWidget {
    Q_OBJECT
    Q_PROPERTY(int rotation READ rotation WRITE setRotation)

public:
    explicit LoadingSpinner(int size = 32, QWidget* parent = nullptr);

private:
    void paintEvent(QPaintEvent* event) override;
    int rotation() const;
    void setRotation(int angle);

    QPropertyAnimation* m_anim;
    int m_rotation = 0;
    int m_lineWidth = 3;
};
```

### SkeletonWidget
```cpp
/**
 * @brief 骨架屏组件 -- 模拟内容布局的灰色块 + shimmer高光扫过动画
 */
class SkeletonWidget : public QWidget {
    Q_OBJECT

public:
    /** @brief 定义一行骨架的形状 */
    struct Bone {
        int widthPercent;   ///< 宽度占比(0-100)
        int height;         ///< 像素高度
        int topMargin;      ///< 上边距
    };

    explicit SkeletonWidget(QWidget* parent = nullptr);

    /** @brief 设置骨架行布局 */
    void setBones(const QList<Bone>& bones);

private:
    void paintEvent(QPaintEvent* event) override;

    QList<Bone> m_bones;
    QPropertyAnimation* m_shimmerAnim;
    qreal m_shimmerOffset = 0.0;
};
```

## 依赖的公共组件
- ThemeManager (core/theme/ThemeManager.h) — color()获取BgTertiary/TextPrimary/TextSecondary/TextMuted/Accent
- IconManager (core/theme/IconManager.h) — EmptyStateWidget加载图标
- AnimatedButton (core/widgets/AnimatedButton.h) — EmptyStateWidget操作按钮

## 设计模式
- **策略模式**: EmptyStateWidget通过setup()动态配置显示内容
- **属性动画**: LoadingSpinner通过QPropertyAnimation驱动rotation属性
- **观察者模式**: 所有组件监听themeChanged重绘

## 影响范围
| 文件 | 变更类型 | 风险 |
|------|---------|------|
| src/core/widgets/EmptyStateWidget.h/cpp | 新增 | 无 |
| src/core/widgets/LoadingSpinner.h/cpp | 新增 | 无 |
| src/core/widgets/SkeletonWidget.h/cpp | 新增 | 无 |
| src/serial/SerialConfigPanel.cpp | 修改(接入空态) | 低 |
| src/serial/DataStatistics.cpp | 修改(接入加载态) | 低 |
| src/terminal/search/TerminalSearchBar.cpp | 修改(接入无结果态) | 低 |

## 验收标准
1. EmptyStateWidget居中显示48px图标 + bold标题 + muted描述，按钮可点击
2. LoadingSpinner绘制弧线旋转动画，1000ms一圈，无卡顿
3. SkeletonWidget显示灰色块，shimmer高光从左到右扫过(1500ms)
4. 3套主题切换后所有状态组件颜色正确(BgTertiary/TextMuted等)
5. SerialConfigPanel未连接时显示"暂无连接"空状态
6. SearchBar搜索无匹配时显示"无匹配结果"空状态
7. .h ≤ 200行, .cpp ≤ 500行
8. 编译零错误
