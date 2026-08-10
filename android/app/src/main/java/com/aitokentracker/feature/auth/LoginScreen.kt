/**
 * Author: AI Token Tracker Engineering Team
 * Maintainer: Project Owner
 * Purpose: Accessible sign-in surface for the shared Windows account.
 * Module: Android feature / authentication presentation.
 *
 * This screen owns only transient form values. Credentials are passed directly
 * to the ViewModel and are never written to preferences, logs, or analytics.
 */
package com.aitokentracker.feature.auth

import androidx.compose.foundation.BorderStroke
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.BoxWithConstraints
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.imePadding
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.safeDrawingPadding
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.Button
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.Card
import androidx.compose.material3.CardDefaults
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.OutlinedTextFieldDefaults
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.setValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.text.input.PasswordVisualTransformation
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.aitokentracker.ui.SignalOrbit

/** Render the sign-in flow with a wide-screen editorial composition. */
@Composable
internal fun LoginScreen(
    initialEndpoint: String,
    onLogin: (String, String, String) -> Unit,
    error: String? = null,
    submitting: Boolean = false,
) {
    var endpoint by rememberSaveable { mutableStateOf(initialEndpoint) }
    var username by rememberSaveable { mutableStateOf("") }
    var password by rememberSaveable { mutableStateOf("") }
    val formEnabled = !submitting && endpoint.isNotBlank() && username.isNotBlank() && password.isNotEmpty()

    BoxWithConstraints(
        modifier = Modifier
            .fillMaxSize()
            .safeDrawingPadding()
            .imePadding()
            .verticalScroll(rememberScrollState())
            .padding(horizontal = 24.dp, vertical = 24.dp),
    ) {
        val wideLayout = maxWidth >= 760.dp
        if (wideLayout) {
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(vertical = 32.dp),
                horizontalArrangement = Arrangement.spacedBy(56.dp),
                verticalAlignment = Alignment.CenterVertically,
            ) {
                LoginStory(modifier = Modifier.weight(1f))
                LoginCard(
                    endpoint = endpoint,
                    username = username,
                    password = password,
                    error = error,
                    submitting = submitting,
                    enabled = formEnabled,
                    onEndpointChange = { endpoint = it },
                    onUsernameChange = { username = it },
                    onPasswordChange = { password = it },
                    onLogin = { onLogin(endpoint, username, password) },
                    modifier = Modifier.width(390.dp),
                )
            }
        } else {
            Column(
                modifier = Modifier.fillMaxWidth(),
                verticalArrangement = Arrangement.spacedBy(30.dp),
            ) {
                LoginStory(modifier = Modifier.fillMaxWidth())
                LoginCard(
                    endpoint = endpoint,
                    username = username,
                    password = password,
                    error = error,
                    submitting = submitting,
                    enabled = formEnabled,
                    onEndpointChange = { endpoint = it },
                    onUsernameChange = { username = it },
                    onPasswordChange = { password = it },
                    onLogin = { onLogin(endpoint, username, password) },
                    modifier = Modifier.fillMaxWidth(),
                )
            }
        }
    }
}

@Composable
private fun LoginStory(modifier: Modifier) {
    Column(modifier = modifier) {
        Text(
            text = "AI TOKEN / CONTROL PLANE",
            color = MaterialTheme.colorScheme.primary,
            style = MaterialTheme.typography.labelMedium,
            letterSpacing = 1.8.sp,
        )
        Spacer(Modifier.height(18.dp))
        Text(
            text = "SEE THE",
            style = MaterialTheme.typography.displayLarge,
            fontWeight = FontWeight.Black,
            letterSpacing = (-3).sp,
            color = MaterialTheme.colorScheme.onBackground,
        )
        Text(
            text = "SIGNAL.",
            style = MaterialTheme.typography.displayLarge,
            fontWeight = FontWeight.Black,
            letterSpacing = (-3).sp,
            color = MaterialTheme.colorScheme.primary,
        )
        Spacer(Modifier.height(16.dp))
        Text(
            text = "一个账号，连接 Windows 观测台与移动端。记录 token、方向、效率和错误信号。",
            color = MaterialTheme.colorScheme.onSurfaceVariant,
            style = MaterialTheme.typography.bodyLarge,
            lineHeight = 28.sp,
        )
        Spacer(Modifier.height(12.dp))
        SignalOrbit(modifier = Modifier.fillMaxWidth())
        Text(
            text = "LOCAL-FIRST · V1 API · ENCRYPTED SESSION",
            color = MaterialTheme.colorScheme.onSurfaceVariant.copy(alpha = .72f),
            style = MaterialTheme.typography.labelSmall,
            letterSpacing = 1.15.sp,
        )
    }
}

@Composable
private fun LoginCard(
    endpoint: String,
    username: String,
    password: String,
    error: String?,
    submitting: Boolean,
    enabled: Boolean,
    onEndpointChange: (String) -> Unit,
    onUsernameChange: (String) -> Unit,
    onPasswordChange: (String) -> Unit,
    onLogin: () -> Unit,
    modifier: Modifier,
) {
    Card(
        modifier = modifier,
        colors = CardDefaults.cardColors(containerColor = Color(0xE8171921)),
        border = BorderStroke(1.dp, MaterialTheme.colorScheme.outline.copy(alpha = .5f)),
        shape = androidx.compose.foundation.shape.RoundedCornerShape(22.dp),
    ) {
        Column(
            modifier = Modifier.padding(24.dp),
            verticalArrangement = Arrangement.spacedBy(14.dp),
        ) {
            Text(
                text = "进入观测台",
                style = MaterialTheme.typography.headlineSmall,
                fontWeight = FontWeight.Bold,
            )
            Text(
                text = "使用 Windows 服务中的同一账号。",
                color = MaterialTheme.colorScheme.onSurfaceVariant,
                style = MaterialTheme.typography.bodyMedium,
            )
            OutlinedTextField(
                value = endpoint,
                onValueChange = onEndpointChange,
                modifier = Modifier.fillMaxWidth(),
                singleLine = true,
                label = { Text("Windows 服务地址") },
                supportingText = { Text("模拟器默认 http://10.0.2.2:5000/api/v1") },
                keyboardOptions = androidx.compose.foundation.text.KeyboardOptions(
                    keyboardType = KeyboardType.Uri,
                ),
                colors = loginFieldColors(),
            )
            OutlinedTextField(
                value = username,
                onValueChange = onUsernameChange,
                modifier = Modifier.fillMaxWidth(),
                singleLine = true,
                label = { Text("用户名") },
                colors = loginFieldColors(),
            )
            OutlinedTextField(
                value = password,
                onValueChange = onPasswordChange,
                modifier = Modifier.fillMaxWidth(),
                singleLine = true,
                label = { Text("密码") },
                visualTransformation = PasswordVisualTransformation(),
                keyboardOptions = androidx.compose.foundation.text.KeyboardOptions(
                    keyboardType = KeyboardType.Password,
                ),
                colors = loginFieldColors(),
            )
            if (error != null) {
                Text(
                    text = error,
                    color = MaterialTheme.colorScheme.error,
                    style = MaterialTheme.typography.bodySmall,
                )
            }
            Button(
                onClick = onLogin,
                enabled = enabled,
                modifier = Modifier
                    .fillMaxWidth()
                    .height(52.dp),
                colors = ButtonDefaults.buttonColors(
                    containerColor = Color(0xFFD9FF78),
                    contentColor = Color(0xFF11150B),
                ),
            ) {
                if (submitting) {
                    CircularProgressIndicator(
                        modifier = Modifier.height(20.dp),
                        color = Color(0xFF11150B),
                        strokeWidth = 2.dp,
                    )
                    Spacer(Modifier.width(10.dp))
                }
                Text(if (submitting) "正在连接" else "进入观测台")
            }
            Text(
                text = "数据保存在你的 Windows 中心服务；移动端仅保存加密会话。",
                color = MaterialTheme.colorScheme.onSurfaceVariant.copy(alpha = .78f),
                style = MaterialTheme.typography.labelSmall,
                lineHeight = 18.sp,
            )
        }
    }
}

@Composable
private fun loginFieldColors() = OutlinedTextFieldDefaults.colors(
    focusedBorderColor = MaterialTheme.colorScheme.primary,
    unfocusedBorderColor = MaterialTheme.colorScheme.outline.copy(alpha = .65f),
    focusedLabelColor = MaterialTheme.colorScheme.primary,
    unfocusedLabelColor = MaterialTheme.colorScheme.onSurfaceVariant,
    cursorColor = MaterialTheme.colorScheme.primary,
)
