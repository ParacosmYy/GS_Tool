/**
 * Author: AI Token Tracker Engineering Team
 * Maintainer: Project Owner
 * Purpose: Bounded decorative signal orbit for the Android Observatory screen.
 * Module: Android UI / motion primitive.
 */
package com.aitokentracker.ui

import androidx.compose.animation.core.LinearEasing
import androidx.compose.animation.core.RepeatMode
import androidx.compose.animation.core.animateFloat
import androidx.compose.animation.core.infiniteRepeatable
import androidx.compose.animation.core.rememberInfiniteTransition
import androidx.compose.animation.core.tween
import androidx.compose.foundation.Canvas
import androidx.compose.foundation.layout.height
import androidx.compose.material3.MaterialTheme
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.geometry.Size
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.drawscope.Stroke
import androidx.compose.ui.graphics.drawscope.withTransform
import androidx.compose.ui.unit.dp

/**
 * Author: AI Token Tracker Engineering Team
 * Maintainer: Project Owner
 * Purpose: Low-frequency mobile orbit animation that remains decorative and bounded.
 */
@Composable
fun SignalOrbit(modifier: Modifier = Modifier) {
    val transition = rememberInfiniteTransition(label = "signal-orbit")
    val rotation = transition.animateFloat(
        initialValue = 0f,
        targetValue = 360f,
        animationSpec = infiniteRepeatable(
            animation = tween(18000, easing = LinearEasing),
            repeatMode = RepeatMode.Restart,
        ),
        label = "orbit-rotation",
    )
    val signal = MaterialTheme.colorScheme.primary
    Canvas(modifier = modifier.height(170.dp)) {
        val center = Offset(size.width / 2f, size.height / 2f)
        val outer = Size(size.width * .82f, size.height * .42f)
        val inner = Size(size.width * .56f, size.height * .72f)
        withTransform({ rotate(rotation.value, pivot = center) }) {
            drawOval(
                color = signal.copy(alpha = .28f),
                topLeft = Offset(center.x - outer.width / 2f, center.y - outer.height / 2f),
                size = outer,
                style = Stroke(width = 1.dp.toPx()),
            )
        }
        withTransform({ rotate(-rotation.value * .72f, pivot = center) }) {
            drawOval(
                color = Color.White.copy(alpha = .16f),
                topLeft = Offset(center.x - inner.width / 2f, center.y - inner.height / 2f),
                size = inner,
                style = Stroke(width = 1.dp.toPx()),
            )
        }
        drawCircle(color = signal.copy(alpha = .86f), radius = 5.dp.toPx(), center = center)
        drawCircle(color = signal.copy(alpha = .16f), radius = 30.dp.toPx(), center = center)
    }
}
