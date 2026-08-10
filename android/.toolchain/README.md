# Project-local Android toolchain boundary

This directory is reserved for approved Android SDK packages only.

- SDK root: `.toolchain/android-sdk/`
- Gradle distribution and dependency cache: `.gradle/user-home/`
- Android user metadata/cache: `.gradle/android-user/`
- JDK 17 may remain an external language runtime as explicitly allowed by the project policy.

No SDK package is provisioned in the current checkout. Do not populate this
directory until the project owner approves the exact Android SDK installation.
