/**
 * Author: AI Token Tracker Engineering Team
 * Maintainer: Project Owner
 * Purpose: Connected mobile observatory for personal token and work signals.
 * Module: Android feature / dashboard presentation.
 *
 * The screen deliberately renders server projections only. It does not know
 * about HTTP, refresh tokens, SQLite, or provider credentials.
 */
package com.aitokentracker.feature.dashboard

import androidx.compose.foundation.Canvas
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Arrangement
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
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.Path
import androidx.compose.ui.graphics.StrokeCap
import androidx.compose.ui.graphics.drawscope.Stroke
import androidx.compose.ui.unit.LayoutDirection
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.aitokentracker.data.AppLog
import com.aitokentracker.data.DailyUsage
import com.aitokentracker.data.ModelUsage
import com.aitokentracker.data.ProviderCallResult
import com.aitokentracker.data.ProviderModels
import com.aitokentracker.data.UsageRecord
import com.aitokentracker.data.UsageSummary
import com.aitokentracker.data.UsageRecordDraft
import com.aitokentracker.data.UserProfile
import com.aitokentracker.data.WorkEvent
import com.aitokentracker.data.WorkEventDraft
import com.aitokentracker.ui.SignalOrbit
import java.util.Locale

/** Render a read-only summary while refresh and logout remain ViewModel-owned. */
@Composable
internal fun DashboardScreen(
    user: UserProfile,
    summary: UsageSummary?,
    workEvents: List<WorkEvent> = emptyList(),
    appLogs: List<AppLog> = emptyList(),
    usageRecords: List<UsageRecord> = emptyList(),
    providerModels: ProviderModels? = null,
    providerBusy: Boolean = false,
    providerMessage: String? = null,
    providerResult: ProviderCallResult? = null,
    message: String? = null,
    actionMessage: String? = null,
    onRefresh: () -> Unit,
    onLogout: () -> Unit,
    onSubmitUsageRecord: (UsageRecordDraft) -> Unit,
    onSubmitWorkEvent: (WorkEventDraft) -> Unit,
    onDetectProviderModels: (String, String, String) -> Unit,
    onProxyProviderChat: (String, String, String, String, String, String) -> Unit,
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
        item {
            DashboardHeader(
                user = user,
                onRefresh = onRefresh,
                onLogout = onLogout,
            )
        }
        item {
            DashboardHero(user = user, summary = summary)
        }
        if (message != null) {
            item { StatusBanner(message) }
        }
        if (actionMessage != null) {
            item { ActionBanner(actionMessage) }
        }
        item {
            ProviderCaptureCard(
                models = providerModels,
                busy = providerBusy,
                message = providerMessage,
                result = providerResult,
                onDetectModels = onDetectProviderModels,
                onProxyChat = onProxyProviderChat,
            )
        }
        item { UsageCaptureCard(onSubmit = onSubmitUsageRecord) }
        item { SignalCaptureCard(onSubmit = onSubmitWorkEvent) }
        item { MetricGrid(summary) }
        item { TrendCard(summary?.trend.orEmpty()) }
        item { ModelsCard(summary?.byModel.orEmpty()) }
        item { ActivityHistoryCard(events = workEvents, logs = appLogs) }
        item { RecentRecordsCard(summary, usageRecords) }
        item {
            Text(
                text = "移动端通过 /api/v1 读取个人汇总与分页记录；工作事件、日志与导出沿用同一账号边界。",
                modifier = Modifier.padding(horizontal = 4.dp, vertical = 8.dp),
                color = MaterialTheme.colorScheme.onSurfaceVariant.copy(alpha = .72f),
                style = MaterialTheme.typography.labelSmall,
                lineHeight = 18.sp,
            )
        }
    }
}

@Composable
private fun DashboardHeader(
    user: UserProfile,
    onRefresh: () -> Unit,
    onLogout: () -> Unit,
) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .padding(bottom = 8.dp),
        verticalAlignment = Alignment.CenterVertically,
        horizontalArrangement = Arrangement.SpaceBetween,
    ) {
        Column {
            Text(
                text = "AI TOKEN / OBSERVATORY",
                color = MaterialTheme.colorScheme.primary,
                style = MaterialTheme.typography.labelMedium,
                letterSpacing = 1.6.sp,
            )
            Spacer(Modifier.height(4.dp))
            Text(
                text = user.username,
                style = MaterialTheme.typography.titleMedium,
            )
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
private fun DashboardHero(user: UserProfile, summary: UsageSummary?) {
    Card(
        colors = CardDefaults.cardColors(containerColor = Color(0xD91A1C26)),
        shape = androidx.compose.foundation.shape.RoundedCornerShape(24.dp),
    ) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(start = 22.dp, top = 22.dp, bottom = 8.dp),
            verticalAlignment = Alignment.CenterVertically,
        ) {
            Column(modifier = Modifier.weight(1f)) {
                Text(
                    text = "LIVE SIGNAL",
                    color = MaterialTheme.colorScheme.primary,
                    style = MaterialTheme.typography.labelMedium,
                    letterSpacing = 1.6.sp,
                )
                Spacer(Modifier.height(12.dp))
                Text(
                    text = "今天，\n看见流量。",
                    style = MaterialTheme.typography.headlineLarge,
                    lineHeight = 40.sp,
                )
                Spacer(Modifier.height(10.dp))
                Text(
                    text = "${summary?.period ?: "day"} / ${user.role.uppercase(Locale.US)}",
                    color = MaterialTheme.colorScheme.onSurfaceVariant,
                    style = MaterialTheme.typography.bodySmall,
                )
            }
            SignalOrbit(modifier = Modifier.width(176.dp))
        }
    }
}

@Composable
private fun StatusBanner(message: String) {
    Card(
        colors = CardDefaults.cardColors(
            containerColor = MaterialTheme.colorScheme.error.copy(alpha = .14f),
        ),
        shape = androidx.compose.foundation.shape.RoundedCornerShape(14.dp),
    ) {
        Text(
            text = message,
            modifier = Modifier.padding(horizontal = 16.dp, vertical = 13.dp),
            color = MaterialTheme.colorScheme.error,
            style = MaterialTheme.typography.bodySmall,
        )
    }
}

@Composable
private fun ActionBanner(message: String) {
    Card(
        colors = CardDefaults.cardColors(
            containerColor = MaterialTheme.colorScheme.primary.copy(alpha = .14f),
        ),
        shape = androidx.compose.foundation.shape.RoundedCornerShape(14.dp),
    ) {
        Text(
            text = message,
            modifier = Modifier.padding(horizontal = 16.dp, vertical = 13.dp),
            color = MaterialTheme.colorScheme.primary,
            style = MaterialTheme.typography.bodySmall,
        )
    }
}

@Composable
private fun MetricGrid(summary: UsageSummary?) {
    val totals = summary?.totals
    Column(verticalArrangement = Arrangement.spacedBy(10.dp)) {
        Row(horizontalArrangement = Arrangement.spacedBy(10.dp)) {
            MetricCard("总 token", formatTokens(totals?.totalTokens), Modifier.weight(1f))
            MetricCard("调用次数", formatTokens(totals?.calls), Modifier.weight(1f))
        }
        Row(horizontalArrangement = Arrangement.spacedBy(10.dp)) {
            MetricCard("输入", formatTokens(totals?.inputTokens), Modifier.weight(1f))
            MetricCard("输出", formatTokens(totals?.outputTokens), Modifier.weight(1f))
        }
    }
}

@Composable
private fun MetricCard(label: String, value: String, modifier: Modifier) {
    Card(
        modifier = modifier,
        colors = CardDefaults.cardColors(containerColor = Color(0xD9171921)),
        shape = androidx.compose.foundation.shape.RoundedCornerShape(16.dp),
    ) {
        Column(modifier = Modifier.padding(16.dp)) {
            Text(
                text = label,
                color = MaterialTheme.colorScheme.onSurfaceVariant,
                style = MaterialTheme.typography.labelMedium,
            )
            Spacer(Modifier.height(8.dp))
            Text(
                text = value,
                color = MaterialTheme.colorScheme.onBackground,
                style = MaterialTheme.typography.headlineSmall,
            )
        }
    }
}

@Composable
private fun TrendCard(trend: List<DailyUsage>) {
    SectionCard(title = "最近趋势", eyebrow = "LOCAL DAY") {
        if (trend.isEmpty()) {
            EmptyCopy("还没有 token 记录；在 Windows 端完成第一次调用后，这里会出现趋势。")
        } else {
            TrendGraph(trend)
            Spacer(Modifier.height(8.dp))
            Text(
                text = "${trend.first().day} → ${trend.last().day}",
                color = MaterialTheme.colorScheme.onSurfaceVariant,
                style = MaterialTheme.typography.labelSmall,
            )
        }
    }
}

@Composable
private fun TrendGraph(trend: List<DailyUsage>) {
    val signal = MaterialTheme.colorScheme.primary
    Canvas(
        modifier = Modifier
            .fillMaxWidth()
            .height(150.dp),
    ) {
        val maximum = trend.maxOfOrNull { it.totalTokens }?.toFloat()?.coerceAtLeast(1f) ?: 1f
        val step = if (trend.size == 1) size.width else size.width / (trend.size - 1)
        val points = trend.mapIndexed { index, item ->
            androidx.compose.ui.geometry.Offset(
                x = index * step,
                y = size.height - (item.totalTokens / maximum) * (size.height - 18.dp.toPx()) - 9.dp.toPx(),
            )
        }
        drawLine(
            color = signal.copy(alpha = .16f),
            start = androidx.compose.ui.geometry.Offset(0f, size.height - 9.dp.toPx()),
            end = androidx.compose.ui.geometry.Offset(size.width, size.height - 9.dp.toPx()),
            strokeWidth = 1.dp.toPx(),
        )
        if (points.isNotEmpty()) {
            val path = Path().apply {
                moveTo(points.first().x, points.first().y)
                points.drop(1).forEach { lineTo(it.x, it.y) }
            }
            drawPath(
                path = path,
                color = signal,
                style = Stroke(width = 3.dp.toPx(), cap = StrokeCap.Round),
            )
            points.forEach { point ->
                drawCircle(color = signal.copy(alpha = .18f), radius = 10.dp.toPx(), center = point)
                drawCircle(color = signal, radius = 3.5.dp.toPx(), center = point)
            }
        }
    }
}

@Composable
private fun ModelsCard(models: List<ModelUsage>) {
    SectionCard(title = "模型占比", eyebrow = "BY MODEL") {
        if (models.isEmpty()) {
            EmptyCopy("暂时没有模型数据。")
        } else {
            val total = models.sumOf { it.totalTokens }.coerceAtLeast(1L)
            models.take(6).forEach { model ->
                ModelRow(model, model.totalTokens.toFloat() / total.toFloat())
                Spacer(Modifier.height(10.dp))
            }
        }
    }
}

@Composable
private fun ModelRow(model: ModelUsage, ratio: Float) {
    Column {
        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.SpaceBetween,
        ) {
            Text(model.model, style = MaterialTheme.typography.bodyMedium)
            Text(
                text = formatTokens(model.totalTokens),
                color = MaterialTheme.colorScheme.primary,
                style = MaterialTheme.typography.labelLarge,
            )
        }
        Spacer(Modifier.height(6.dp))
        androidx.compose.foundation.layout.Box(
            modifier = Modifier
                .fillMaxWidth()
                .height(6.dp)
                .background(Color.White.copy(alpha = .08f), androidx.compose.foundation.shape.RoundedCornerShape(99.dp)),
        ) {
            androidx.compose.foundation.layout.Box(
                modifier = Modifier
                    .fillMaxWidth(ratio.coerceIn(.08f, 1f))
                    .height(6.dp)
                    .background(MaterialTheme.colorScheme.primary, androidx.compose.foundation.shape.RoundedCornerShape(99.dp)),
            )
        }
    }
}

@Composable
private fun RecentRecordsCard(summary: UsageSummary?, usageRecords: List<UsageRecord>) {
    SectionCard(title = "最近记录", eyebrow = "LATEST CALLS") {
        val records = usageRecords.ifEmpty { summary?.records.orEmpty() }
        if (records.isEmpty()) {
            EmptyCopy("完成一次 token 记录后，模型、输入和输出会在这里显示。")
        } else {
            records.take(5).forEach { record ->
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.SpaceBetween,
                    verticalAlignment = Alignment.CenterVertically,
                ) {
                    Column(modifier = Modifier.weight(1f)) {
                        Text(record.model, style = MaterialTheme.typography.bodyMedium)
                        Text(
                            record.timestamp,
                            color = MaterialTheme.colorScheme.onSurfaceVariant,
                            style = MaterialTheme.typography.labelSmall,
                        )
                    }
                    Text(
                        formatTokens(record.totalTokens),
                        color = MaterialTheme.colorScheme.primary,
                        style = MaterialTheme.typography.labelLarge,
                    )
                }
                Spacer(Modifier.height(12.dp))
            }
        }
    }
}

private fun formatTokens(value: Long?): String {
    if (value == null) return "—"
    return when {
        value >= 1_000_000 -> String.format(Locale.US, "%.1fM", value / 1_000_000.0)
        value >= 1_000 -> String.format(Locale.US, "%.1fK", value / 1_000.0)
        else -> value.toString()
    }
}
