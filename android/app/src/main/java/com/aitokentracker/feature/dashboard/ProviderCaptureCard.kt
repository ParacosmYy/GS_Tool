/**
 * Author: AI Token Tracker Engineering Team
 * Maintainer: Project Owner
 * Purpose: Provide the Android automatic provider detection and usage flow.
 * Module: Android feature / dashboard provider capture.
 *
 * The API key intentionally lives in a plain remember state instead of
 * rememberSaveable. It is passed to the ViewModel for one request only and is
 * never included in a domain model, saved-state bundle, log, or record note.
 */
package com.aitokentracker.feature.dashboard

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.BoxWithConstraints
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.weight
import androidx.compose.material3.Button
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.Card
import androidx.compose.material3.CardDefaults
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.text.input.PasswordVisualTransformation
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.aitokentracker.data.ProviderCallResult
import com.aitokentracker.data.ProviderModels

private const val AUTO_PROVIDER = "auto"

/** Automatic provider card: discover models, call once, and archive usage. */
@Composable
internal fun ProviderCaptureCard(
    models: ProviderModels?,
    busy: Boolean,
    message: String?,
    result: ProviderCallResult?,
    onDetectModels: (String, String, String) -> Unit,
    onProxyChat: (String, String, String, String, String, String) -> Unit,
) {
    var baseUrl by rememberSaveable { mutableStateOf("") }
    var apiKey by remember { mutableStateOf("") }
    var model by rememberSaveable { mutableStateOf("") }
    // Prompts can contain source code or private context; keep them out of
    // Android saved state just like the provider key.
    var prompt by remember { mutableStateOf("") }
    var note by rememberSaveable { mutableStateOf("") }

    LaunchedEffect(models?.models) {
        val firstModel = models?.models?.firstOrNull()
        if (model.isBlank() && firstModel != null) model = firstModel
    }

    Card(
        colors = CardDefaults.cardColors(containerColor = Color(0xE31A1C26)),
        shape = androidx.compose.foundation.shape.RoundedCornerShape(18.dp),
    ) {
        Column(
            modifier = Modifier.padding(18.dp),
            verticalArrangement = Arrangement.spacedBy(10.dp),
        ) {
            Text("自动采集", style = MaterialTheme.typography.titleMedium)
            Text(
                "填一次上游 Base URL 和 Key，检测模型后直接调用；服务端从真实 usage 自动记录 token。",
                color = MaterialTheme.colorScheme.onSurfaceVariant,
                style = MaterialTheme.typography.bodySmall,
                lineHeight = 20.sp,
            )
            BoxWithConstraints(modifier = Modifier.fillMaxWidth()) {
                if (maxWidth >= 620.dp) {
                    Row(
                        modifier = Modifier.fillMaxWidth(),
                        horizontalArrangement = Arrangement.spacedBy(16.dp),
                    ) {
                        ProviderForm(
                            modifier = Modifier.weight(1f),
                            baseUrl = baseUrl,
                            apiKey = apiKey,
                            model = model,
                            prompt = prompt,
                            note = note,
                            models = models,
                            busy = busy,
                            onBaseUrlChanged = { baseUrl = it },
                            onApiKeyChanged = { apiKey = it },
                            onModelChanged = { model = it },
                            onPromptChanged = { prompt = it },
                            onNoteChanged = { note = it },
                            onDetect = { onDetectModels(AUTO_PROVIDER, baseUrl, apiKey) },
                            onCall = {
                                onProxyChat(AUTO_PROVIDER, baseUrl, apiKey, model, prompt, note)
                            },
                        )
                        ProviderResultPanel(
                            modifier = Modifier.weight(1f),
                            message = message,
                            result = result,
                        )
                    }
                } else {
                    Column(verticalArrangement = Arrangement.spacedBy(10.dp)) {
                        ProviderForm(
                            modifier = Modifier.fillMaxWidth(),
                            baseUrl = baseUrl,
                            apiKey = apiKey,
                            model = model,
                            prompt = prompt,
                            note = note,
                            models = models,
                            busy = busy,
                            onBaseUrlChanged = { baseUrl = it },
                            onApiKeyChanged = { apiKey = it },
                            onModelChanged = { model = it },
                            onPromptChanged = { prompt = it },
                            onNoteChanged = { note = it },
                            onDetect = { onDetectModels(AUTO_PROVIDER, baseUrl, apiKey) },
                            onCall = {
                                onProxyChat(AUTO_PROVIDER, baseUrl, apiKey, model, prompt, note)
                            },
                        )
                        ProviderResultPanel(
                            modifier = Modifier.fillMaxWidth(),
                            message = message,
                            result = result,
                        )
                    }
                }
            }
        }
    }
}

@Composable
private fun ProviderForm(
    modifier: Modifier,
    baseUrl: String,
    apiKey: String,
    model: String,
    prompt: String,
    note: String,
    models: ProviderModels?,
    busy: Boolean,
    onBaseUrlChanged: (String) -> Unit,
    onApiKeyChanged: (String) -> Unit,
    onModelChanged: (String) -> Unit,
    onPromptChanged: (String) -> Unit,
    onNoteChanged: (String) -> Unit,
    onDetect: () -> Unit,
    onCall: () -> Unit,
) {
    Column(
        modifier = modifier,
        verticalArrangement = Arrangement.spacedBy(10.dp),
    ) {
        OutlinedTextField(
            value = baseUrl,
            onValueChange = { onBaseUrlChanged(it.take(500)) },
            modifier = Modifier.fillMaxWidth(),
            singleLine = true,
            label = { Text("上游 Base URL") },
            placeholder = { Text("https://api.example.com/v1") },
            keyboardOptions = KeyboardOptions(keyboardType = KeyboardType.Uri),
        )
        OutlinedTextField(
            value = apiKey,
            onValueChange = { onApiKeyChanged(it.take(500)) },
            modifier = Modifier.fillMaxWidth(),
            singleLine = true,
            label = { Text("API Key（仅本次内存使用）") },
            visualTransformation = PasswordVisualTransformation(),
        )
        OutlinedTextField(
            value = model,
            onValueChange = { onModelChanged(it.take(200)) },
            modifier = Modifier.fillMaxWidth(),
            singleLine = true,
            label = { Text("模型") },
            placeholder = { Text("先检测，或手动填写模型 ID") },
        )
        if (!models?.models.isNullOrEmpty()) {
            Text(
                "检测到的模型（点击选择）",
                color = MaterialTheme.colorScheme.onSurfaceVariant,
                style = MaterialTheme.typography.labelSmall,
            )
            models?.models.orEmpty().take(8).forEach { detectedModel ->
                TextButton(
                    onClick = { onModelChanged(detectedModel) },
                    modifier = Modifier.fillMaxWidth(),
                ) {
                    Text(detectedModel, modifier = Modifier.fillMaxWidth())
                }
            }
        }
        OutlinedTextField(
            value = prompt,
            onValueChange = { onPromptChanged(it.take(8_000)) },
            modifier = Modifier.fillMaxWidth(),
            minLines = 4,
            label = { Text("提示词") },
        )
        OutlinedTextField(
            value = note,
            onValueChange = { onNoteChanged(it.take(1_000)) },
            modifier = Modifier.fillMaxWidth(),
            singleLine = true,
            label = { Text("记录备注（可选）") },
        )
        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.spacedBy(10.dp),
        ) {
            OutlinedButton(
                onClick = onDetect,
                enabled = !busy,
                modifier = Modifier.weight(1f),
                colors = ButtonDefaults.outlinedButtonColors(),
            ) {
                Text(if (busy) "处理中…" else "检测模型")
            }
            Button(
                onClick = onCall,
                enabled = !busy,
                modifier = Modifier.weight(1f),
                colors = ButtonDefaults.buttonColors(
                    containerColor = Color(0xFFD9FF78),
                    contentColor = Color(0xFF11150B),
                ),
            ) {
                Text("调用并记账")
            }
        }
        Text(
            "Key 不写入 SQLite、日志或 Android saved state；只在本次请求链路中存在。",
            color = MaterialTheme.colorScheme.onSurfaceVariant.copy(alpha = .78f),
            style = MaterialTheme.typography.labelSmall,
            lineHeight = 18.sp,
        )
    }
}

@Composable
private fun ProviderResultPanel(
    modifier: Modifier,
    message: String?,
    result: ProviderCallResult?,
) {
    Surface(
        modifier = modifier,
        color = Color.White.copy(alpha = .045f),
        shape = androidx.compose.foundation.shape.RoundedCornerShape(14.dp),
    ) {
        Column(
            modifier = Modifier.padding(14.dp),
            verticalArrangement = Arrangement.spacedBy(10.dp),
        ) {
            Text("采集结果", style = MaterialTheme.typography.titleSmall)
            if (message != null) {
                Text(
                    message,
                    color = MaterialTheme.colorScheme.primary,
                    style = MaterialTheme.typography.bodySmall,
                    lineHeight = 20.sp,
                )
            }
            if (result == null) {
                Text(
                    "调用成功后，这里只显示助手文本和 usage 摘要。",
                    color = MaterialTheme.colorScheme.onSurfaceVariant,
                    style = MaterialTheme.typography.bodySmall,
                    lineHeight = 20.sp,
                )
                return@Column
            }
            if (result.usageSummary.isNotBlank()) {
                Text(
                    result.usageSummary,
                    color = MaterialTheme.colorScheme.secondary,
                    style = MaterialTheme.typography.labelLarge,
                )
            }
            if (result.assistantText.isNotBlank()) {
                Text(
                    result.assistantText,
                    color = MaterialTheme.colorScheme.onSurface,
                    style = MaterialTheme.typography.bodySmall,
                    maxLines = 8,
                    overflow = TextOverflow.Ellipsis,
                    lineHeight = 19.sp,
                )
            } else {
                Text(
                    "上游没有返回可显示的文本内容。",
                    color = MaterialTheme.colorScheme.onSurfaceVariant,
                    style = MaterialTheme.typography.bodySmall,
                )
            }
            result.warning?.let { warning ->
                Spacer(Modifier.height(2.dp))
                Text(
                    warning,
                    color = MaterialTheme.colorScheme.error,
                    style = MaterialTheme.typography.labelSmall,
                    lineHeight = 18.sp,
                )
            }
        }
    }
}
