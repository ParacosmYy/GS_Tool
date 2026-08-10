/**
 * Author: AI Token Tracker Engineering Team
 * Maintainer: Project Owner
 * Purpose: Compose composition root for the connected Android client.
 * Module: Android UI / application shell.
 *
 * The composition root owns only screen selection and visual layering. Network
 * calls, session rotation, and persistence remain behind TrackerViewModel.
 */
package com.aitokentracker.ui

import androidx.compose.animation.core.RepeatMode
import androidx.compose.animation.core.animateFloat
import androidx.compose.animation.core.infiniteRepeatable
import androidx.compose.animation.core.rememberInfiniteTransition
import androidx.compose.animation.core.tween
import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.safeDrawingPadding
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.graphicsLayer
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.unit.dp
import androidx.compose.ui.platform.LocalContext
import androidx.lifecycle.viewmodel.compose.viewModel
import com.aitokentracker.R
import com.aitokentracker.data.UsageSummary
import com.aitokentracker.data.UserProfile
import com.aitokentracker.feature.auth.TrackerUiState
import com.aitokentracker.feature.auth.TrackerViewModel
import com.aitokentracker.feature.auth.TrackerViewModelFactory
import com.aitokentracker.feature.auth.LoginScreen
import com.aitokentracker.feature.admin.AdminScreen
import com.aitokentracker.feature.dashboard.DashboardScreen

/** Render the current authenticated route from one immutable state source. */
@Composable
fun TokenTrackerApp() {
    val context = LocalContext.current
    val viewModel: TrackerViewModel = viewModel(
        factory = TrackerViewModelFactory(context),
    )

    AppBackdrop {
        when (val state = viewModel.state) {
            TrackerUiState.Booting -> LoadingSurface("正在恢复安全会话")
            TrackerUiState.SignedOut -> LoginScreen(
                initialEndpoint = viewModel.endpoint,
                onLogin = viewModel::login,
            )
            TrackerUiState.SigningIn -> LoginScreen(
                initialEndpoint = viewModel.endpoint,
                onLogin = viewModel::login,
                submitting = true,
            )
            TrackerUiState.SigningOut -> LoadingSurface("正在安全退出")
            is TrackerUiState.Refreshing -> AuthenticatedSurface(
                viewModel = viewModel,
                user = state.user,
                summary = state.summary,
                message = "正在同步团队观测数据…",
                actionMessage = viewModel.actionMessage,
            )
            is TrackerUiState.Ready -> AuthenticatedSurface(
                viewModel = viewModel,
                user = state.user,
                summary = state.summary,
                actionMessage = viewModel.actionMessage,
            )
            is TrackerUiState.Failure -> if (state.user == null) {
                LoginScreen(
                    initialEndpoint = viewModel.endpoint,
                    onLogin = viewModel::login,
                    error = state.message,
                )
            } else {
                AuthenticatedSurface(
                    viewModel = viewModel,
                    user = state.user,
                    summary = state.summary,
                    message = state.message,
                    actionMessage = viewModel.actionMessage,
                )
            }
        }
    }
}

/** Route an authenticated account to the least-privilege surface for its role. */
@Composable
private fun AuthenticatedSurface(
    viewModel: TrackerViewModel,
    user: UserProfile,
    summary: UsageSummary?,
    message: String? = null,
    actionMessage: String? = null,
) {
    if (isAdmin(user)) {
        AdminScreen(
            user = user,
            overview = viewModel.adminOverview,
            members = viewModel.adminMembers,
            selectedMemberId = viewModel.selectedAdminMemberId,
            selectedActivity = viewModel.adminMemberActivity,
            loading = viewModel.adminLoading,
            error = viewModel.adminError,
            message = message,
            onRefresh = viewModel::refresh,
            onLogout = viewModel::logout,
            onSelectMember = viewModel::selectAdminMember,
        )
    } else {
        DashboardScreen(
            user = user,
            summary = summary,
            workEvents = viewModel.workEvents,
            appLogs = viewModel.appLogs,
            usageRecords = viewModel.usageRecords,
            providerModels = viewModel.providerModels,
            providerBusy = viewModel.providerBusy,
            providerMessage = viewModel.providerMessage,
            providerResult = viewModel.providerResult,
            message = message,
            actionMessage = actionMessage,
            onRefresh = viewModel::refresh,
            onLogout = viewModel::logout,
            onSubmitUsageRecord = viewModel::recordUsage,
            onSubmitWorkEvent = viewModel::recordWorkEvent,
            onDetectProviderModels = viewModel::detectProviderModels,
            onProxyProviderChat = viewModel::proxyProviderChat,
        )
    }
}

private fun isAdmin(user: UserProfile): Boolean = user.role.equals("admin", ignoreCase = true)

/** Shared visual field keeps the generated illustration behind readable UI. */
@Composable
private fun AppBackdrop(content: @Composable () -> Unit) {
    Box(
        modifier = Modifier
            .fillMaxSize()
            .background(androidx.compose.material3.MaterialTheme.colorScheme.background),
    ) {
        BrandBackdrop()
        Box(
            modifier = Modifier
                .fillMaxSize()
                .background(
                    Brush.horizontalGradient(
                        0f to androidx.compose.ui.graphics.Color(0xFF0B0C10).copy(alpha = .99f),
                        .48f to androidx.compose.ui.graphics.Color(0xFF0B0C10).copy(alpha = .89f),
                        .78f to androidx.compose.ui.graphics.Color(0xFF0B0C10).copy(alpha = .72f),
                        1f to androidx.compose.ui.graphics.Color(0xFF0B0C10).copy(alpha = .68f),
                    ),
                ),
        )
        Box(
            modifier = Modifier
                .fillMaxSize()
                .background(
                    Brush.verticalGradient(
                        0f to androidx.compose.ui.graphics.Color.Black.copy(alpha = .2f),
                        .5f to androidx.compose.ui.graphics.Color.Transparent,
                        1f to androidx.compose.ui.graphics.Color.Black.copy(alpha = .38f),
                    ),
                ),
        )
        content()
    }
}

/** Select a static or animated illustration without changing content semantics. */
@Composable
private fun BrandBackdrop() {
    if (rememberReducedMotion()) {
        StaticBrandBackdrop()
    } else {
        AnimatedBrandBackdrop()
    }
}

/** Render the same decorative scene without starting an infinite transition. */
@Composable
private fun StaticBrandBackdrop() {
    Image(
        painter = painterResource(R.drawable.embedded_rust_engineer_bg_v14),
        contentDescription = null,
        contentScale = ContentScale.Crop,
        modifier = Modifier
            .fillMaxSize()
            .alpha(.36f),
    )
}

/**
 * Own only the decorative image animation so dashboard recomposition does not
 * run once per frame. The low-amplitude drift gives the static illustration a
 * calm depth cue while remaining behind all interactive content.
 */
@Composable
private fun AnimatedBrandBackdrop() {
    val drift = rememberInfiniteTransition(label = "brand-backdrop")
    val driftX = drift.animateFloat(
        initialValue = -7f,
        targetValue = 7f,
        animationSpec = infiniteRepeatable(tween(14_000), RepeatMode.Reverse),
        label = "brand-backdrop-x",
    )
    val driftY = drift.animateFloat(
        initialValue = 3f,
        targetValue = -3f,
        animationSpec = infiniteRepeatable(tween(17_000), RepeatMode.Reverse),
        label = "brand-backdrop-y",
    )
    Image(
        painter = painterResource(R.drawable.embedded_rust_engineer_bg_v14),
        contentDescription = null,
        contentScale = ContentScale.Crop,
        modifier = Modifier
            .fillMaxSize()
            .graphicsLayer {
                translationX = driftX.value
                translationY = driftY.value
                scaleX = 1.04f
                scaleY = 1.04f
            }
            .alpha(.36f),
    )
}

/** Bounded boot state; it intentionally has no fake progress percentage. */
@Composable
private fun LoadingSurface(message: String) {
    androidx.compose.foundation.layout.Column(
        modifier = Modifier
            .fillMaxSize()
            .safeDrawingPadding()
            .padding(28.dp),
        verticalArrangement = androidx.compose.foundation.layout.Arrangement.Center,
    ) {
        androidx.compose.material3.CircularProgressIndicator()
        androidx.compose.foundation.layout.Spacer(Modifier.height(18.dp))
        androidx.compose.material3.Text(
            text = message,
            style = androidx.compose.material3.MaterialTheme.typography.bodyLarge,
            color = androidx.compose.material3.MaterialTheme.colorScheme.onSurfaceVariant,
        )
    }
}
