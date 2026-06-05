/**
 * @file AnnotationTypes.h
 * @brief 数据标注工具类型定义 — 标注类型枚举、标注数据结构
 *
 * 定义 AnnotationType 枚举和 AnnotationEntry 结构体，供 AnnotationManager
 * 和 AnnotationWidget 共享使用。支持 Marker / Region / Event / Measurement
 * 四种标注类型。
 *
 * 所有类型置于 annotation 命名空间，避免与旧版 DataAnnotation 冲突。
 */

#ifndef ANNOTATION_TYPES_H
#define ANNOTATION_TYPES_H

#include <QColor>
#include <QDateTime>
#include <QString>
#include <QUuid>
#include <QList>

namespace annotation {

/**
 * @brief 标注类型枚举
 *
 * 区分不同种类的数据标注，影响时间轴渲染和交互方式。
 */
enum class Type {
    Marker      = 0,  ///< 单点标记 — 时间轴上绘制竖线
    Region      = 1,  ///< 区域标注 — 时间轴上绘制色块区间
    Event       = 2,  ///< 事件标注 — 关键事件快照
    Measurement = 3   ///< 测量标注 — 起止区间 + 数值差
};

/**
 * @brief 单条数据标注结构体
 *
 * 存储标注的所有元信息：位置、类型、标签、颜色、分类和时间戳。
 * id 由 AnnotationManager 自动生成（UUID 字符串）。
 */
struct Entry {
    QString     id;            ///< 唯一标识符（UUID）
    Type        type;          ///< 标注类型
    qint64      startPos;      ///< 起始位置（字节偏移 / 采样索引）
    qint64      endPos;        ///< 结束位置（仅 Region/Measurement 有效）
    QString     label;         ///< 用户自定义标签
    QString     category;      ///< 分类名称（自由文本）
    QColor      color;         ///< 显示颜色
    QDateTime   timestamp;     ///< 创建时间

    /** @brief 默认构造 — 生成 UUID 并填充默认值 */
    Entry()
        : type(Type::Marker)
        , startPos(0)
        , endPos(0)
        , color(Qt::yellow)
        , timestamp(QDateTime::currentDateTime())
    {
        id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    }

    /** @brief 判断是否为区域类型标注 */
    bool isRegion() const {
        return type == Type::Region || type == Type::Measurement;
    }

    /** @brief 获取标注类型的显示名称 */
    static QString typeName(Type t) {
        switch (t) {
        case Type::Marker:      return QStringLiteral("Marker");
        case Type::Region:      return QStringLiteral("Region");
        case Type::Event:       return QStringLiteral("Event");
        case Type::Measurement: return QStringLiteral("Measurement");
        }
        return QStringLiteral("Unknown");
    }

    /** @brief 获取所有支持的标注类型列表 */
    static QList<Type> allTypes() {
        return { Type::Marker, Type::Region, Type::Event, Type::Measurement };
    }
};

/** @brief 撤销/重做操作记录 */
struct Action {
    enum Kind { Add, Remove, Update };
    Kind    kind;      ///< 操作类型
    Entry   before;    ///< 操作前状态
    Entry   after;     ///< 操作后状态
};

} // namespace annotation

/* ── 全局类型别名 — 保持对外接口简洁 ── */
using AnnotationType  = annotation::Type;
using DataAnnotation  = annotation::Entry;
using AnnotationAction = annotation::Action;

#endif // ANNOTATION_TYPES_H
