/**
 * @file AnnotationManager.cpp
 * @brief 数据标注管理器实现 — CRUD、搜索过滤、持久化、撤销重做
 */

#include "utils/annotation/AnnotationManager.h"

#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QUuid>
#include <algorithm>

/** @brief 撤销栈最大深度 */
static constexpr int kMaxUndoDepth = 50;

/* ── 构造 / 清理 ── */

/** @brief 构造管理器 @param parent 父对象 */
AnnotationManager::AnnotationManager(QObject* parent)
    : QObject(parent)
{
}

/** @brief 清除所有标注（不可撤销） */
void AnnotationManager::clear()
{
    m_annotations.clear();
    m_undoStack.clear();
    m_redoStack.clear();
    m_stats.activeCount = 0;
}

/* ── CRUD ── */

/** @brief 添加标注 @param annotation 标注数据 @return 添加后的标注 */
DataAnnotation AnnotationManager::addAnnotation(const DataAnnotation& annotation)
{
    DataAnnotation a = annotation;
    if (a.id.isEmpty()) {
        a.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    }
    if (!a.timestamp.isValid()) {
        a.timestamp = QDateTime::currentDateTime();
    }
    m_annotations.append(a);

    ++m_stats.totalAdded;
    m_stats.activeCount = static_cast<int>(m_annotations.size());
    if (m_stats.activeCount > m_stats.peakCount) {
        m_stats.peakCount = m_stats.activeCount;
    }
    pushUndo(AnnotationAction::Add, DataAnnotation{}, a);
    emit annotationAdded(a);
    return a;
}

/** @brief 移除标注 @param id 标注 ID @return 是否成功 */
bool AnnotationManager::removeAnnotation(const QString& id)
{
    for (int i = 0; i < m_annotations.size(); ++i) {
        if (m_annotations.at(i).id == id) {
            DataAnnotation before = m_annotations.at(i);
            m_annotations.removeAt(i);
            ++m_stats.totalRemoved;
            m_stats.activeCount = static_cast<int>(m_annotations.size());
            pushUndo(AnnotationAction::Remove, before, DataAnnotation{});
            emit annotationRemoved(id);
            return true;
        }
    }
    return false;
}

/** @brief 更新标注 @param id 标注 ID @param annotation 新数据 @return 是否成功 */
bool AnnotationManager::updateAnnotation(const QString& id,
                                          const DataAnnotation& annotation)
{
    for (int i = 0; i < m_annotations.size(); ++i) {
        if (m_annotations.at(i).id == id) {
            DataAnnotation before = m_annotations.at(i);
            DataAnnotation after = annotation;
            after.id = id;
            m_annotations[i] = after;
            ++m_stats.totalUpdated;
            pushUndo(AnnotationAction::Update, before, after);
            emit annotationUpdated(after);
            return true;
        }
    }
    return false;
}

/** @brief 获取标注 @param id 标注 ID @return 标注数据 */
DataAnnotation AnnotationManager::annotation(const QString& id) const
{
    for (const auto& a : m_annotations) {
        if (a.id == id) return a;
    }
    return DataAnnotation{};
}

/** @brief 获取所有标注 @return 标注列表 */
QList<DataAnnotation> AnnotationManager::allAnnotations() const
{
    return m_annotations;
}

/* ── 过滤 / 搜索 ── */

/** @brief 按类型过滤 @param type 标注类型 @return 匹配的标注 */
QList<DataAnnotation> AnnotationManager::filterByType(AnnotationType type) const
{
    QList<DataAnnotation> result;
    for (const auto& a : m_annotations) {
        if (a.type == type) result.append(a);
    }
    return result;
}

/** @brief 按分类过滤 @param category 分类名称 @return 匹配的标注 */
QList<DataAnnotation> AnnotationManager::filterByCategory(
    const QString& category) const
{
    QList<DataAnnotation> result;
    for (const auto& a : m_annotations) {
        if (a.category.compare(category, Qt::CaseInsensitive) == 0) {
            result.append(a);
        }
    }
    return result;
}

/** @brief 按时间范围过滤 @param from 起始 @param to 结束 @return 匹配的标注 */
QList<DataAnnotation> AnnotationManager::filterByTimeRange(
    const QDateTime& from, const QDateTime& to) const
{
    QList<DataAnnotation> result;
    for (const auto& a : m_annotations) {
        if (a.timestamp >= from && a.timestamp <= to) {
            result.append(a);
        }
    }
    return result;
}

/** @brief 关键字搜索（匹配 label 和 category） @param keyword 关键字 @return 匹配标注 */
QList<DataAnnotation> AnnotationManager::search(const QString& keyword) const
{
    ++m_stats.totalSearches;
    QList<DataAnnotation> result;
    for (const auto& a : m_annotations) {
        if (a.label.contains(keyword, Qt::CaseInsensitive)
            || a.category.contains(keyword, Qt::CaseInsensitive)) {
            result.append(a);
        }
    }
    return result;
}

/** @brief 检测重叠 @param annotation 目标注 @return 重叠标注列表 */
QList<DataAnnotation> AnnotationManager::findOverlaps(
    const DataAnnotation& annotation) const
{
    QList<DataAnnotation> result;
    for (const auto& a : m_annotations) {
        if (a.id == annotation.id) continue;
        if (!a.isRegion() || !annotation.isRegion()) continue;
        /* 区间重叠: max(start1,start2) <= min(end1,end2) */
        qint64 overlapStart = std::max(a.startPos, annotation.startPos);
        qint64 overlapEnd   = std::min(a.endPos, annotation.endPos);
        if (overlapStart <= overlapEnd) {
            result.append(a);
        }
    }
    return result;
}

/* ── 持久化 ── */

/** @brief 序列化单条标注为 JSON 对象 */
static QJsonObject annotationToJson(const DataAnnotation& a)
{
    QJsonObject obj;
    obj[QStringLiteral("id")]       = a.id;
    obj[QStringLiteral("type")]     = static_cast<int>(a.type);
    obj[QStringLiteral("startPos")] = static_cast<qint64>(a.startPos);
    obj[QStringLiteral("endPos")]   = static_cast<qint64>(a.endPos);
    obj[QStringLiteral("label")]    = a.label;
    obj[QStringLiteral("category")] = a.category;
    obj[QStringLiteral("color")]    = a.color.name();
    obj[QStringLiteral("timestamp")] = a.timestamp.toString(Qt::ISODateWithMs);
    return obj;
}

/** @brief 从 JSON 对象反序列化标注 */
static DataAnnotation jsonToAnnotation(const QJsonObject& obj)
{
    DataAnnotation a;
    a.id       = obj[QStringLiteral("id")].toString();
    a.type     = static_cast<AnnotationType>(
        obj[QStringLiteral("type")].toInt(static_cast<int>(AnnotationType::Marker)));
    a.startPos = obj[QStringLiteral("startPos")].toInteger();
    a.endPos   = obj[QStringLiteral("endPos")].toInteger();
    a.label    = obj[QStringLiteral("label")].toString();
    a.category = obj[QStringLiteral("category")].toString();
    a.color    = QColor(obj[QStringLiteral("color")].toString());
    a.timestamp = QDateTime::fromString(
        obj[QStringLiteral("timestamp")].toString(), Qt::ISODateWithMs);
    return a;
}

/** @brief 保存到 JSON @param filePath 文件路径 @return 是否成功 */
bool AnnotationManager::saveToJson(const QString& filePath) const
{
    QJsonArray arr;
    for (const auto& a : m_annotations) {
        arr.append(annotationToJson(a));
    }
    QJsonDocument doc(arr);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) return false;
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    ++m_stats.totalExports;
    return true;
}

/** @brief 从 JSON 加载 @param filePath 文件路径 @return 是否成功 */
bool AnnotationManager::loadFromJson(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) return false;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    if (!doc.isArray()) return false;

    const QJsonArray arr = doc.array();
    for (const QJsonValue& val : arr) {
        m_annotations.append(jsonToAnnotation(val.toObject()));
    }
    ++m_stats.totalImports;
    m_stats.activeCount = static_cast<int>(m_annotations.size());
    if (m_stats.activeCount > m_stats.peakCount) {
        m_stats.peakCount = m_stats.activeCount;
    }
    /* 加载后清空撤销栈 */
    m_undoStack.clear();
    m_redoStack.clear();
    return true;
}

/** @brief 导出为 CSV @param filePath 文件路径 @return 是否成功 */
bool AnnotationManager::exportToCsv(const QString& filePath) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) return false;
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    /* 表头 */
    out << QStringLiteral("ID,Type,StartPos,EndPos,Label,Category,Color,Timestamp\n");
    /* 数据行 */
    for (const auto& a : m_annotations) {
        out << a.id << QStringLiteral(",")
            << DataAnnotation::typeName(a.type) << QStringLiteral(",")
            << a.startPos << QStringLiteral(",")
            << a.endPos << QStringLiteral(",")
            << QStringLiteral("\"%1\"").arg(a.label) << QStringLiteral(",")
            << QStringLiteral("\"%1\"").arg(a.category) << QStringLiteral(",")
            << a.color.name() << QStringLiteral(",")
            << a.timestamp.toString(Qt::ISODate) << QStringLiteral("\n");
    }
    file.close();
    ++m_stats.totalExports;
    return true;
}

/* ── 撤销 / 重做 ── */

/** @brief 压入撤销栈 @param action 操作类型 @param before 操作前 @param after 操作后 */
void AnnotationManager::pushUndo(AnnotationAction::Kind action,
                                  const DataAnnotation& before,
                                  const DataAnnotation& after)
{
    AnnotationAction act;
    act.kind = action;
    act.before = before;
    act.after  = after;
    m_undoStack.append(act);
    if (m_undoStack.size() > kMaxUndoDepth) {
        m_undoStack.removeFirst();
    }
    m_redoStack.clear();
}

/** @brief 撤销 @return 是否成功 */
bool AnnotationManager::undo()
{
    if (m_undoStack.isEmpty()) return false;
    AnnotationAction act = m_undoStack.takeLast();
    m_redoStack.append(act);

    switch (act.kind) {
    case AnnotationAction::Add:
        /* 撤销添加 → 移除 */
        for (int i = 0; i < m_annotations.size(); ++i) {
            if (m_annotations.at(i).id == act.after.id) {
                m_annotations.removeAt(i);
                break;
            }
        }
        break;
    case AnnotationAction::Remove:
        /* 撤销移除 → 重新添加 */
        m_annotations.append(act.before);
        break;
    case AnnotationAction::Update:
        /* 撤销更新 → 还原旧值 */
        for (int i = 0; i < m_annotations.size(); ++i) {
            if (m_annotations.at(i).id == act.after.id) {
                m_annotations[i] = act.before;
                break;
            }
        }
        break;
    }
    m_stats.activeCount = static_cast<int>(m_annotations.size());
    return true;
}

/** @brief 重做 @return 是否成功 */
bool AnnotationManager::redo()
{
    if (m_redoStack.isEmpty()) return false;
    AnnotationAction act = m_redoStack.takeLast();
    m_undoStack.append(act);

    switch (act.kind) {

    case AnnotationAction::Add:
        m_annotations.append(act.after);
        break;
    case AnnotationAction::Remove:
        for (int i = 0; i < m_annotations.size(); ++i) {
            if (m_annotations.at(i).id == act.before.id) {
                m_annotations.removeAt(i);
                break;
            }
        }
        break;
    case AnnotationAction::Update:
        for (int i = 0; i < m_annotations.size(); ++i) {
            if (m_annotations.at(i).id == act.before.id) {
                m_annotations[i] = act.after;
                break;
            }
        }
        break;
    }
    m_stats.activeCount = static_cast<int>(m_annotations.size());
    return true;
}

/** @brief 判断是否可撤销 */
bool AnnotationManager::canUndo() const { return !m_undoStack.isEmpty(); }

/** @brief 判断是否可重做 */
bool AnnotationManager::canRedo() const { return !m_redoStack.isEmpty(); }
