/**
 * Author: AI Token Tracker Engineering Team
 * Maintainer: Project Owner
 * Purpose: Define the shared high-contrast Observatory Material color system.
 * Module: Android UI / theme boundary.
 */
package com.aitokentracker.ui.theme

import androidx.compose.foundation.isSystemInDarkTheme
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.darkColorScheme
import androidx.compose.runtime.Composable
import androidx.compose.ui.graphics.Color

/**
 * Author: AI Token Tracker Engineering Team
 * Maintainer: Project Owner
 * Purpose: Keep mobile surfaces aligned with the web observatory tokens.
 */
private val ObservatoryColors = darkColorScheme(
    background = Color(0xFF0B0C10),
    surface = Color(0xFF15161C),
    surfaceVariant = Color(0xFF262731),
    primary = Color(0xFFC9C6FF),
    onPrimary = Color(0xFF2B2851),
    onBackground = Color(0xFFF5F2FA),
    onSurface = Color(0xFFF5F2FA),
    onSurfaceVariant = Color(0xFFC9C7D1),
)

@Composable
fun TokenTrackerTheme(content: @Composable () -> Unit) {
    MaterialTheme(
        colorScheme = if (isSystemInDarkTheme()) ObservatoryColors else ObservatoryColors,
        content = content,
    )
}
