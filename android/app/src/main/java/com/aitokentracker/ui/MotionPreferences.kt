/**
 * Author: AI Token Tracker Engineering Team
 * Maintainer: Project Owner
 * Purpose: Resolve the Android system animation preference for decorative UI.
 * Module: Android UI / accessibility and motion policy.
 *
 * The project keeps business state independent from platform motion settings.
 * This boundary only decides whether optional visual loops may run; it never
 * hides content, changes navigation, or changes network behavior.
 */
package com.aitokentracker.ui

import android.content.Context
import android.provider.Settings
import androidx.compose.runtime.Composable
import androidx.compose.runtime.remember
import androidx.compose.ui.platform.LocalContext

/** Read one system animation scale without making settings access fatal. */
private fun readAnimationScale(context: Context, key: String): Float = runCatching {
    Settings.Global.getFloat(context.contentResolver, key, 1f)
}.getOrDefault(1f)

/**
 * Return true when Android's system setting disables animation.
 *
 * The value is sampled at composition time. A process restart or a screen
 * recreation picks up a setting changed while the app was backgrounded; the
 * visual policy deliberately has no observer or polling loop.
 */
@Composable
internal fun rememberReducedMotion(): Boolean {
    val context = LocalContext.current
    return remember(context) {
        readAnimationScale(context, Settings.Global.ANIMATOR_DURATION_SCALE) <= 0f ||
            readAnimationScale(context, Settings.Global.TRANSITION_ANIMATION_SCALE) <= 0f
    }
}

