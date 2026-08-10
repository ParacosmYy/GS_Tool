/**
 * Author: AI Token Tracker Engineering Team
 * Maintainer: Project Owner
 * Purpose: Host the Compose application and install the edge-to-edge policy.
 * Module: Android application / lifecycle entry point.
 */
package com.aitokentracker

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.enableEdgeToEdge
import androidx.activity.compose.setContent
import com.aitokentracker.ui.TokenTrackerApp
import com.aitokentracker.ui.theme.TokenTrackerTheme

/**
 * Author: AI Token Tracker Engineering Team
 * Maintainer: Project Owner
 * Purpose: Host the Compose navigation surface without owning application state.
 */
class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        // Keep the visual field behind system bars; screens own safe drawing
        // and IME insets so text inputs remain reachable on small devices.
        enableEdgeToEdge()
        setContent {
            TokenTrackerTheme {
                TokenTrackerApp()
            }
        }
    }
}
