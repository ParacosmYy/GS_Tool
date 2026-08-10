/**
 * Author: AI Token Tracker Engineering Team
 * Maintainer: Project Owner
 * Purpose: Render bounded work-event and diagnostic-log history projections.
 * Module: Android feature / dashboard history.
 *
 * This presentation module intentionally renders sanitized server DTOs and
 * never formats or interprets provider credentials or raw model payloads.
 */
package com.aitokentracker.feature.dashboard

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.aitokentracker.data.AppLog
import com.aitokentracker.data.WorkEvent

/** Show the latest structured signals returned by the Windows service. */
@Composable
internal fun ActivityHistoryCard(
    events: List<WorkEvent>,
    logs: List<AppLog>,
) {
    SectionCard(title = "活动历史", eyebrow = "EVENTS / LOGS") {
        Text(
            text = "工作信号",
            color = MaterialTheme.colorScheme.primary,
            style = MaterialTheme.typography.labelLarge,
        )
        if (events.isEmpty()) {
            EmptyCopy("还没有工作信号。")
        } else {
            events.take(5).forEach { event ->
                WorkEventRow(event)
                Spacer(Modifier.height(10.dp))
            }
        }
        Spacer(Modifier.height(8.dp))
        Text(
            text = "诊断日志",
            color = MaterialTheme.colorScheme.primary,
            style = MaterialTheme.typography.labelLarge,
        )
        if (logs.isEmpty()) {
            EmptyCopy("还没有诊断日志。")
        } else {
            logs.take(5).forEach { log ->
                LogRow(log)
                Spacer(Modifier.height(10.dp))
            }
        }
    }
}

@Composable
private fun WorkEventRow(event: WorkEvent) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .padding(top = 8.dp),
        horizontalArrangement = Arrangement.SpaceBetween,
        verticalAlignment = Alignment.Top,
    ) {
        Column(modifier = Modifier.weight(1f)) {
            Text(
                text = "${event.direction} · ${event.outcome}",
                style = MaterialTheme.typography.bodyMedium,
            )
            Text(
                text = listOf(event.project, event.taskType, event.note)
                    .filter(String::isNotBlank)
                    .joinToString(" · ")
                    .ifBlank { "无备注" },
                color = MaterialTheme.colorScheme.onSurfaceVariant,
                style = MaterialTheme.typography.labelSmall,
                lineHeight = 18.sp,
            )
        }
        Text(
            text = event.createdAt,
            color = MaterialTheme.colorScheme.onSurfaceVariant,
            style = MaterialTheme.typography.labelSmall,
        )
    }
}

@Composable
private fun LogRow(log: AppLog) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .padding(top = 8.dp),
        horizontalArrangement = Arrangement.SpaceBetween,
        verticalAlignment = Alignment.Top,
    ) {
        Column(modifier = Modifier.weight(1f)) {
            Text(
                text = "${log.level} · ${log.eventType}",
                style = MaterialTheme.typography.bodyMedium,
            )
            Text(
                text = log.message,
                color = MaterialTheme.colorScheme.onSurfaceVariant,
                style = MaterialTheme.typography.labelSmall,
                lineHeight = 18.sp,
            )
        }
        Text(
            text = log.createdAt,
            color = MaterialTheme.colorScheme.onSurfaceVariant,
            style = MaterialTheme.typography.labelSmall,
        )
    }
}
