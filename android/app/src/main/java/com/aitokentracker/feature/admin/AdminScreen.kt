/**
 * Author: AI Token Tracker Engineering Team
 * Maintainer: Project Owner
 * Purpose: Present the administrator's read-only team observatory.
 * Module: Android feature / administrator presentation.
 *
 * The screen renders sanitized projections from TrackerViewModel. It never
 * creates work events, changes roles, downloads exports, or handles secrets.
 */
package com.aitokentracker.feature.admin

import androidx.compose.foundation.BorderStroke
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.WindowInsets
import androidx.compose.foundation.layout.asPaddingValues
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.safeDrawing
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.Card
import androidx.compose.material3.CardDefaults
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.LayoutDirection
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.aitokentracker.data.AdminMember
import com.aitokentracker.data.AdminMemberActivity
import com.aitokentracker.data.AdminOverview
import com.aitokentracker.data.AppLog
import com.aitokentracker.data.UserProfile
import com.aitokentracker.data.UsageRecord
import com.aitokentracker.data.WorkEvent

/** Render team aggregates and selected-member activity for an admin account. */
@Composable
internal fun AdminScreen(
    user: UserProfile,
    overview: AdminOverview?,
    members: List<AdminMember>,
    selectedMemberId: Long?,
    selectedActivity: AdminMemberActivity?,
    loading: Boolean,
    error: String?,
    message: String?,
    onRefresh: () -> Unit,
    onLogout: () -> Unit,
    onSelectMember: (Long) -> Unit,
) {
    val safePadding = WindowInsets.safeDrawing.asPaddingValues()
    LazyColumn(
        modifier = Modifier.fillMaxSize(),
        contentPadding = PaddingValues(
            start = 22.dp + safePadding.calculateLeftPadding(LayoutDirection.Ltr),
            top = 22.dp + safePadding.calculateTopPadding(),
            end = 22.dp + safePadding.calculateRightPadding(LayoutDirection.Ltr),
            bottom = 22.dp + safePadding.calculateBottomPadding(),
        ),
        verticalArrangement = Arrangement.spacedBy(14.dp),
    ) {
        item { AdminHeader(user, onRefresh, onLogout) }
        if (message != null) item { AdminBanner(message, isError = false) }
        if (error != null) item { AdminBanner(error, isError = true) }
        item { AdminHero(user, overview) }
        item { AdminMetrics(overview) }
        item { AdminModelMix(overview) }
        item { AdminTrend(overview) }
        item {
            AdminMembers(
                members = members,
                selectedMemberId = selectedMemberId,
                loading = loading,
                onSelectMember = onSelectMember,
            )
        }
        item { AdminMemberActivityCard(selectedActivity, loading) }
        item {
            Text(
                text = "只读管理员视图 · 总览、成员选择和活动明细均由 Windows RBAC 与审计边界保护。",
                modifier = Modifier.padding(horizontal = 4.dp, vertical = 8.dp),
                color = MaterialTheme.colorScheme.onSurfaceVariant.copy(alpha = .72f),
                style = MaterialTheme.typography.labelSmall,
                lineHeight = 18.sp,
            )
        }
    }
}

@Composable
private fun AdminHeader(user: UserProfile, onRefresh: () -> Unit, onLogout: () -> Unit) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .padding(bottom = 8.dp),
        verticalAlignment = Alignment.CenterVertically,
        horizontalArrangement = Arrangement.SpaceBetween,
    ) {
        Column {
            Text(
                text = "AI TOKEN / CONTROL PLANE",
                color = MaterialTheme.colorScheme.primary,
                style = MaterialTheme.typography.labelMedium,
                letterSpacing = 1.6.sp,
            )
            Spacer(Modifier.height(4.dp))
            Text(user.username, style = MaterialTheme.typography.titleMedium)
        }
        Row(verticalAlignment = Alignment.CenterVertically) {
            TextButton(onClick = onRefresh) { Text("刷新") }
            TextButton(
                onClick = onLogout,
                colors = ButtonDefaults.textButtonColors(
                    contentColor = MaterialTheme.colorScheme.onSurfaceVariant,
                ),
            ) { Text("退出") }
        }
    }
}

@Composable
private fun AdminHero(user: UserProfile, overview: AdminOverview?) {
    Card(
        colors = CardDefaults.cardColors(containerColor = Color(0xD91A1C26)),
        shape = androidx.compose.foundation.shape.RoundedCornerShape(24.dp),
    ) {
        Column(modifier = Modifier.padding(22.dp)) {
            Text(
                text = "TEAM SIGNAL",
                color = MaterialTheme.colorScheme.primary,
                style = MaterialTheme.typography.labelMedium,
                letterSpacing = 1.6.sp,
            )
            Spacer(Modifier.height(12.dp))
            Text(
                text = "看见整个团队的\nAI 工作流。",
                style = MaterialTheme.typography.headlineLarge,
                lineHeight = 40.sp,
            )
            Spacer(Modifier.height(10.dp))
            Text(
                text = "${user.role.uppercase()} · ${overview?.totalUsers ?: 0} 个账号 · 只读观测",
                color = MaterialTheme.colorScheme.onSurfaceVariant,
                style = MaterialTheme.typography.bodySmall,
            )
        }
    }
}

@Composable
private fun AdminMetrics(overview: AdminOverview?) {
    val usage = overview?.usage
    val events = overview?.workEvents
    Column(verticalArrangement = Arrangement.spacedBy(10.dp)) {
        Row(horizontalArrangement = Arrangement.spacedBy(10.dp)) {
            AdminMetric("团队 token", formatTokens(usage?.totalTokens), Modifier.weight(1f))
            AdminMetric("账号数", formatTokens(overview?.totalUsers), Modifier.weight(1f))
        }
        Row(horizontalArrangement = Arrangement.spacedBy(10.dp)) {
            AdminMetric("成功信号", formatTokens(events?.success), Modifier.weight(1f))
            AdminMetric("失败信号", formatTokens(events?.failure), Modifier.weight(1f))
        }
    }
}

@Composable
private fun AdminMetric(label: String, value: String, modifier: Modifier) {
    Card(
        modifier = modifier,
        colors = CardDefaults.cardColors(containerColor = Color(0xD9171921)),
        shape = androidx.compose.foundation.shape.RoundedCornerShape(16.dp),
    ) {
        Column(modifier = Modifier.padding(16.dp)) {
            Text(label, color = MaterialTheme.colorScheme.onSurfaceVariant, style = MaterialTheme.typography.labelMedium)
            Spacer(Modifier.height(8.dp))
            Text(value, color = MaterialTheme.colorScheme.onBackground, style = MaterialTheme.typography.headlineSmall)
        }
    }
}

@Composable
private fun AdminModelMix(overview: AdminOverview?) {
    AdminSection(title = "团队模型占比", eyebrow = "BY MODEL") {
        val models = overview?.byModel.orEmpty()
        if (models.isEmpty()) {
            AdminEmpty("还没有团队 token 记录。")
        } else {
            val total = models.sumOf { it.totalTokens }.coerceAtLeast(1L)
            models.take(6).forEach { model ->
                Column {
                    Row(
                        modifier = Modifier.fillMaxWidth(),
                        horizontalArrangement = Arrangement.SpaceBetween,
                    ) {
                        Text(model.model, style = MaterialTheme.typography.bodyMedium)
                        Text(formatTokens(model.totalTokens), color = MaterialTheme.colorScheme.primary, style = MaterialTheme.typography.labelLarge)
                    }
                    Spacer(Modifier.height(6.dp))
                    Box(
                        modifier = Modifier
                            .fillMaxWidth()
                            .height(6.dp)
                            .background(Color.White.copy(alpha = .08f), androidx.compose.foundation.shape.RoundedCornerShape(99.dp)),
                    ) {
                        Box(
                            modifier = Modifier
                                .fillMaxWidth((model.totalTokens.toFloat() / total).coerceIn(.08f, 1f))
                                .height(6.dp)
                                .background(MaterialTheme.colorScheme.primary, androidx.compose.foundation.shape.RoundedCornerShape(99.dp)),
                        )
                    }
                }
                Spacer(Modifier.height(10.dp))
            }
        }
    }
}

@Composable
private fun AdminTrend(overview: AdminOverview?) {
    AdminSection(title = "团队趋势", eyebrow = "LOCAL DAY") {
        val trend = overview?.trend.orEmpty().takeLast(8)
        if (trend.isEmpty()) {
            AdminEmpty("完成一次团队调用后，这里会出现趋势。")
        } else {
            val maximum = trend.maxOf { it.totalTokens }.coerceAtLeast(1L)
            trend.forEach { day ->
                Row(verticalAlignment = Alignment.CenterVertically) {
                    Text(day.day.takeLast(5), modifier = Modifier.width(48.dp), color = MaterialTheme.colorScheme.onSurfaceVariant, style = MaterialTheme.typography.labelSmall)
                    Box(modifier = Modifier.weight(1f).height(8.dp).background(Color.White.copy(alpha = .08f), androidx.compose.foundation.shape.RoundedCornerShape(99.dp))) {
                        Box(modifier = Modifier.fillMaxWidth((day.totalTokens.toFloat() / maximum).coerceIn(.04f, 1f)).height(8.dp).background(MaterialTheme.colorScheme.primary, androidx.compose.foundation.shape.RoundedCornerShape(99.dp)))
                    }
                    Text(formatTokens(day.totalTokens), modifier = Modifier.width(64.dp).padding(start = 8.dp), color = MaterialTheme.colorScheme.primary, style = MaterialTheme.typography.labelSmall)
                }
                Spacer(Modifier.height(8.dp))
            }
        }
    }
}

@Composable
private fun AdminMembers(
    members: List<AdminMember>,
    selectedMemberId: Long?,
    loading: Boolean,
    onSelectMember: (Long) -> Unit,
) {
    AdminSection(title = "成员观测", eyebrow = "MEMBERS") {
        if (loading && members.isEmpty()) {
            CircularProgressIndicator(modifier = Modifier.width(24.dp).height(24.dp), strokeWidth = 2.dp)
        } else if (members.isEmpty()) {
            AdminEmpty("当前还没有可查看的成员。")
        } else {
            members.forEach { member ->
                val selected = member.id == selectedMemberId
                OutlinedButton(
                    onClick = { onSelectMember(member.id) },
                    modifier = Modifier.fillMaxWidth(),
                    border = BorderStroke(
                        1.dp,
                        if (selected) MaterialTheme.colorScheme.primary else MaterialTheme.colorScheme.outline.copy(alpha = .48f),
                    ),
                    colors = ButtonDefaults.outlinedButtonColors(
                        containerColor = if (selected) MaterialTheme.colorScheme.primary.copy(alpha = .12f) else Color.Transparent,
                    ),
                    contentPadding = PaddingValues(horizontal = 14.dp, vertical = 12.dp),
                ) {
                    Row(
                        modifier = Modifier.fillMaxWidth(),
                        verticalAlignment = Alignment.CenterVertically,
                        horizontalArrangement = Arrangement.SpaceBetween,
                    ) {
                        Column(modifier = Modifier.weight(1f), horizontalAlignment = Alignment.Start) {
                            Text(member.username, style = MaterialTheme.typography.bodyMedium)
                            Text(
                                text = "${member.role} · ${member.workEvents} signals · ${member.logs} logs",
                                color = MaterialTheme.colorScheme.onSurfaceVariant,
                                style = MaterialTheme.typography.labelSmall,
                            )
                        }
                        Text(formatTokens(member.totalTokens), color = MaterialTheme.colorScheme.primary, style = MaterialTheme.typography.labelLarge)
                    }
                }
                Spacer(Modifier.height(8.dp))
            }
        }
    }
}

@Composable
private fun AdminMemberActivityCard(activity: AdminMemberActivity?, loading: Boolean) {
    AdminSection(
        title = activity?.user?.username?.let { "成员活动 · $it" } ?: "成员活动",
        eyebrow = "SELECTED MEMBER",
    ) {
        if (loading && activity == null) {
            CircularProgressIndicator(modifier = Modifier.width(24.dp).height(24.dp), strokeWidth = 2.dp)
        } else if (activity == null) {
            AdminEmpty("从上面的成员列表选择一个账号查看明细。")
        } else {
            ActivityCounts(activity)
            ActivityRecords(activity.records)
            ActivityEvents(activity.events)
            ActivityLogs(activity.logs)
        }
    }
}

@Composable
private fun ActivityCounts(activity: AdminMemberActivity) {
    Text(
        text = "${activity.user.role} · ${activity.user.createdAt}",
        color = MaterialTheme.colorScheme.onSurfaceVariant,
        style = MaterialTheme.typography.labelSmall,
    )
    Spacer(Modifier.height(12.dp))
    Text(
        text = "records ${activity.records.size} · events ${activity.events.size} · logs ${activity.logs.size}",
        color = MaterialTheme.colorScheme.primary,
        style = MaterialTheme.typography.labelMedium,
    )
}

@Composable
private fun ActivityRecords(records: List<UsageRecord>) {
    ActivityHeading("Token records")
    if (records.isEmpty()) AdminEmpty("没有 token 记录。")
    records.take(5).forEach { record ->
        ActivityLine(
            title = record.model,
            detail = "${record.timestamp} · ${formatTokens(record.totalTokens)} · ${record.source.ifBlank { "manual" }}",
        )
    }
}

@Composable
private fun ActivityEvents(events: List<WorkEvent>) {
    ActivityHeading("Work signals")
    if (events.isEmpty()) AdminEmpty("没有工作信号。")
    events.take(5).forEach { event ->
        ActivityLine(
            title = "${event.direction} · ${event.outcome}",
            detail = listOf(event.createdAt, event.project, event.taskType, event.note)
                .filter(String::isNotBlank)
                .joinToString(" · "),
        )
    }
}

@Composable
private fun ActivityLogs(logs: List<AppLog>) {
    ActivityHeading("Diagnostic logs")
    if (logs.isEmpty()) AdminEmpty("没有诊断日志。")
    logs.take(5).forEach { log ->
        ActivityLine(
            title = "${log.level} · ${log.eventType}",
            detail = "${log.createdAt} · ${log.message}",
        )
    }
}

@Composable
private fun ActivityHeading(title: String) {
    Spacer(Modifier.height(16.dp))
    Text(title, color = MaterialTheme.colorScheme.primary, style = MaterialTheme.typography.labelLarge)
    Spacer(Modifier.height(8.dp))
}

@Composable
private fun ActivityLine(title: String, detail: String) {
    Column(modifier = Modifier.padding(vertical = 5.dp)) {
        Text(title, style = MaterialTheme.typography.bodyMedium)
        Text(detail.ifBlank { "无可显示的详情" }, color = MaterialTheme.colorScheme.onSurfaceVariant, style = MaterialTheme.typography.labelSmall, lineHeight = 18.sp)
    }
}

@Composable
private fun AdminSection(title: String, eyebrow: String, content: @Composable () -> Unit) {
    Card(
        colors = CardDefaults.cardColors(containerColor = Color(0xD9171921)),
        shape = androidx.compose.foundation.shape.RoundedCornerShape(18.dp),
    ) {
        Column(modifier = Modifier.padding(18.dp)) {
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically,
            ) {
                Text(title, style = MaterialTheme.typography.titleMedium)
                Text(eyebrow, color = MaterialTheme.colorScheme.onSurfaceVariant, style = MaterialTheme.typography.labelSmall, letterSpacing = 1.1.sp)
            }
            Spacer(Modifier.height(18.dp))
            content()
        }
    }
}

@Composable
private fun AdminEmpty(text: String) {
    Text(text, color = MaterialTheme.colorScheme.onSurfaceVariant, style = MaterialTheme.typography.bodySmall, lineHeight = 20.sp)
}

@Composable
private fun AdminBanner(message: String, isError: Boolean) {
    val color = if (isError) MaterialTheme.colorScheme.error else MaterialTheme.colorScheme.primary
    Card(
        colors = CardDefaults.cardColors(containerColor = color.copy(alpha = .14f)),
        shape = androidx.compose.foundation.shape.RoundedCornerShape(14.dp),
    ) {
        Text(message, modifier = Modifier.padding(horizontal = 16.dp, vertical = 13.dp), color = color, style = MaterialTheme.typography.bodySmall)
    }
}

private fun formatTokens(value: Long?): String {
    if (value == null) return "—"
    return when {
        value >= 1_000_000 -> String.format(java.util.Locale.US, "%.1fM", value / 1_000_000.0)
        value >= 1_000 -> String.format(java.util.Locale.US, "%.1fK", value / 1_000.0)
        else -> value.toString()
    }
}
