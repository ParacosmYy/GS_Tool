/**
 * Author: AI Token Tracker Engineering Team
 * Maintainer: Project Owner
 * Purpose: Capture a bounded work-direction signal from the Android client.
 * Module: Android feature / dashboard input.
 *
 * The form intentionally excludes prompts, provider keys, and raw model
 * payloads. It emits the public WorkEventDraft contract only.
 */
package com.aitokentracker.feature.dashboard

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.AlertDialog
import androidx.compose.material3.Button
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.Card
import androidx.compose.material3.CardDefaults
import androidx.compose.material3.DropdownMenu
import androidx.compose.material3.DropdownMenuItem
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.aitokentracker.data.WorkEventDraft

private val directionOptions = listOf(
    "coding" to "编码",
    "debugging" to "调试",
    "research" to "研究",
    "writing" to "写作",
    "planning" to "规划",
    "review" to "评审",
    "other" to "其他",
)

private val outcomeOptions = listOf(
    "success" to "成功",
    "partial" to "部分完成",
    "failure" to "失败",
)

/** Small dashboard card that opens the structured event form. */
@Composable
internal fun SignalCaptureCard(onSubmit: (WorkEventDraft) -> Unit) {
    var dialogVisible by rememberSaveable { mutableStateOf(false) }
    Card(
        colors = CardDefaults.cardColors(containerColor = Color(0xD9171921)),
        shape = androidx.compose.foundation.shape.RoundedCornerShape(18.dp),
    ) {
        Column(
            modifier = Modifier.padding(18.dp),
            verticalArrangement = Arrangement.spacedBy(10.dp),
        ) {
            Text("记录工作信号", style = MaterialTheme.typography.titleMedium)
            Text(
                "同步方向、结果和效率评分；不上传 prompt、Key 或原始模型响应。",
                color = MaterialTheme.colorScheme.onSurfaceVariant,
                style = MaterialTheme.typography.bodySmall,
                lineHeight = 20.sp,
            )
            Button(
                onClick = { dialogVisible = true },
                modifier = Modifier.fillMaxWidth(),
                colors = ButtonDefaults.buttonColors(
                    containerColor = Color(0xFFD9FF78),
                    contentColor = Color(0xFF11150B),
                ),
            ) {
                Text("新增一条工作信号")
            }
        }
    }
    if (dialogVisible) {
        WorkEventDialog(
            onDismiss = { dialogVisible = false },
            onSubmit = {
                dialogVisible = false
                onSubmit(it)
            },
        )
    }
}

@Composable
private fun WorkEventDialog(
    onDismiss: () -> Unit,
    onSubmit: (WorkEventDraft) -> Unit,
) {
    var direction by rememberSaveable { mutableStateOf("coding") }
    var outcome by rememberSaveable { mutableStateOf("success") }
    var efficiencyText by rememberSaveable { mutableStateOf("") }
    var note by rememberSaveable { mutableStateOf("") }
    var validationError by rememberSaveable { mutableStateOf<String?>(null) }

    AlertDialog(
        onDismissRequest = onDismiss,
        title = { Text("记录工作信号") },
        text = {
            Column(
                modifier = Modifier
                    .fillMaxWidth()
                    .verticalScroll(rememberScrollState()),
                verticalArrangement = Arrangement.spacedBy(12.dp),
            ) {
                SelectionMenu(
                    label = "方向",
                    options = directionOptions,
                    selected = direction,
                    onSelected = { direction = it },
                )
                SelectionMenu(
                    label = "结果",
                    options = outcomeOptions,
                    selected = outcome,
                    onSelected = { outcome = it },
                )
                OutlinedTextField(
                    value = efficiencyText,
                    onValueChange = { efficiencyText = it.filter(Char::isDigit).take(3) },
                    modifier = Modifier.fillMaxWidth(),
                    singleLine = true,
                    label = { Text("效率评分（0-100，可选）") },
                )
                OutlinedTextField(
                    value = note,
                    onValueChange = { note = it.take(1000) },
                    modifier = Modifier.fillMaxWidth(),
                    minLines = 3,
                    label = { Text("备注（可选）") },
                )
                if (validationError != null) {
                    Text(
                        text = validationError.orEmpty(),
                        color = MaterialTheme.colorScheme.error,
                        style = MaterialTheme.typography.bodySmall,
                    )
                }
            }
        },
        dismissButton = { TextButton(onClick = onDismiss) { Text("取消") } },
        confirmButton = {
            TextButton(onClick = {
                val score = efficiencyText.toIntOrNull()
                if (efficiencyText.isNotBlank() && (score == null || score !in 0..100)) {
                    validationError = "效率评分必须在 0 到 100 之间"
                    return@TextButton
                }
                onSubmit(
                    WorkEventDraft(
                        direction = direction,
                        outcome = outcome,
                        efficiencyScore = score,
                        resultCode = if (outcome == "success") "OK" else null,
                        note = note.trim(),
                    ),
                )
            }) { Text("写入") }
        },
    )
}

@Composable
private fun SelectionMenu(
    label: String,
    options: List<Pair<String, String>>,
    selected: String,
    onSelected: (String) -> Unit,
) {
    var expanded by rememberSaveable { mutableStateOf(false) }
    val selectedLabel = options.firstOrNull { it.first == selected }?.second ?: selected
    Box {
        OutlinedButton(
            onClick = { expanded = true },
            modifier = Modifier.fillMaxWidth(),
        ) {
            Text("$label：$selectedLabel")
        }
        DropdownMenu(
            expanded = expanded,
            onDismissRequest = { expanded = false },
        ) {
            options.forEach { (value, display) ->
                DropdownMenuItem(
                    text = { Text(display) },
                    onClick = {
                        onSelected(value)
                        expanded = false
                    },
                )
            }
        }
    }
}
