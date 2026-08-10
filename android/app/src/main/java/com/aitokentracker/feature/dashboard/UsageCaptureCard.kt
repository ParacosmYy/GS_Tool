/**
 * Author: AI Token Tracker Engineering Team
 * Maintainer: Project Owner
 * Purpose: Capture a bounded token usage record from the Android client.
 * Module: Android feature / dashboard input.
 *
 * This form emits only the versioned UsageRecordDraft contract. Provider keys,
 * prompts, raw responses, and user identity are intentionally out of scope.
 */
package com.aitokentracker.feature.dashboard

import androidx.compose.foundation.layout.Arrangement
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
import androidx.compose.material3.MaterialTheme
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
import com.aitokentracker.data.UsageRecordDraft

/** Small dashboard card that opens the token usage form. */
@Composable
internal fun UsageCaptureCard(onSubmit: (UsageRecordDraft) -> Unit) {
    var dialogVisible by rememberSaveable { mutableStateOf(false) }
    Card(
        colors = CardDefaults.cardColors(containerColor = Color(0xD9171921)),
        shape = androidx.compose.foundation.shape.RoundedCornerShape(18.dp),
    ) {
        Column(
            modifier = Modifier.padding(18.dp),
            verticalArrangement = Arrangement.spacedBy(10.dp),
        ) {
            Text("记录 token 用量", style = MaterialTheme.typography.titleMedium)
            Text(
                "填写模型和输入/输出 token；时间由 Windows 中心服务按本地时间补齐。",
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
                Text("新增一条 token 记录")
            }
        }
    }
    if (dialogVisible) {
        UsageRecordDialog(
            onDismiss = { dialogVisible = false },
            onSubmit = {
                dialogVisible = false
                onSubmit(it)
            },
        )
    }
}

@Composable
private fun UsageRecordDialog(
    onDismiss: () -> Unit,
    onSubmit: (UsageRecordDraft) -> Unit,
) {
    var model by rememberSaveable { mutableStateOf("") }
    var inputTokens by rememberSaveable { mutableStateOf("") }
    var outputTokens by rememberSaveable { mutableStateOf("") }
    var note by rememberSaveable { mutableStateOf("") }
    var validationError by rememberSaveable { mutableStateOf<String?>(null) }

    AlertDialog(
        onDismissRequest = onDismiss,
        title = { Text("记录 token 用量") },
        text = {
            Column(
                modifier = Modifier
                    .fillMaxWidth()
                    .verticalScroll(rememberScrollState()),
                verticalArrangement = Arrangement.spacedBy(12.dp),
            ) {
                OutlinedTextField(
                    value = model,
                    onValueChange = { model = it.take(200) },
                    modifier = Modifier.fillMaxWidth(),
                    singleLine = true,
                    label = { Text("模型名称") },
                    placeholder = { Text("例如 kimi-code") },
                )
                OutlinedTextField(
                    value = inputTokens,
                    onValueChange = { inputTokens = it.filter(Char::isDigit).take(18) },
                    modifier = Modifier.fillMaxWidth(),
                    singleLine = true,
                    label = { Text("输入 token") },
                )
                OutlinedTextField(
                    value = outputTokens,
                    onValueChange = { outputTokens = it.filter(Char::isDigit).take(18) },
                    modifier = Modifier.fillMaxWidth(),
                    singleLine = true,
                    label = { Text("输出 token") },
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
                val modelValue = model.trim()
                val inputValue = inputTokens.toLongOrNull()
                val outputValue = outputTokens.toLongOrNull()
                validationError = when {
                    modelValue.isBlank() -> "请填写模型名称"
                    inputValue == null -> "输入 token 必须是非负整数"
                    outputValue == null -> "输出 token 必须是非负整数"
                    else -> null
                }
                if (validationError != null) return@TextButton
                onSubmit(
                    UsageRecordDraft(
                        model = modelValue,
                        inputTokens = inputValue ?: 0,
                        outputTokens = outputValue ?: 0,
                        note = note.trim(),
                    ),
                )
            }) { Text("写入") }
        },
    )
}
