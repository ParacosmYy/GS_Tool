# Project-local Android toolchain boundary

This directory contains project-local Android toolchain components that were
downloaded only after the project owner requested an in-project environment.

- SDK root: `.toolchain/android-sdk/`
- Gradle distribution and dependency cache: `.gradle/user-home/`
- Android user metadata/cache: `.gradle/android-user/`
- JDK 17: `.toolchain/jdk-17/`
- Gradle 9.5.0 distribution: `.toolchain/gradle-9.5.0/`
- Android command-line tools: `.toolchain/android-sdk/cmdline-tools/latest/`

Archives are kept under `.toolchain/downloads/` for hash inspection. They are
ignored by Git and must not be uploaded as repository artifacts. The checked-in
`provision-toolchain.ps1` records version, source URL, and SHA-256 values and
can regenerate these paths on a clean Windows checkout.

Android SDK package installation is a separate explicit action because Google
SDK licenses require the user to accept them. The provisioning script never
pipes automatic acceptance into `sdkmanager`; run its license command in an
interactive shell and then install the requested API 37 packages only after
that user-owned decision.
