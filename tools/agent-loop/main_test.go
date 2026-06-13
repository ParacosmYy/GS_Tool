package main

import (
	"os"
	"path/filepath"
	"runtime"
	"strings"
	"testing"
)

func TestLoadConfigAppliesDefaults(t *testing.T) {
	dir := t.TempDir()
	configPath := writeConfig(t, dir, `{
		"workdir": ".",
		"check": ["echo ok"]
	}`)

	cfg, err := loadConfig(configPath)
	if err != nil {
		t.Fatalf("loadConfig returned error: %v", err)
	}
	if cfg.MaxRounds != defaultMaxRounds {
		t.Fatalf("MaxRounds = %d, want %d", cfg.MaxRounds, defaultMaxRounds)
	}
	if cfg.MaxMinutes != defaultMaxMinutes {
		t.Fatalf("MaxMinutes = %d, want %d", cfg.MaxMinutes, defaultMaxMinutes)
	}
	if !filepath.IsAbs(cfg.Workdir) {
		t.Fatalf("Workdir is not absolute: %q", cfg.Workdir)
	}
}

func TestLoadConfigRejectsRoundLimitAboveSafetyBrake(t *testing.T) {
	dir := t.TempDir()
	configPath := writeConfig(t, dir, `{
		"max_rounds": 21,
		"check": ["echo ok"]
	}`)

	_, err := loadConfig(configPath)
	if err == nil || !strings.Contains(err.Error(), "max_rounds") {
		t.Fatalf("loadConfig error = %v, want max_rounds safety error", err)
	}
}

func TestLoadConfigRejectsMinuteLimitAboveSafetyBrake(t *testing.T) {
	dir := t.TempDir()
	configPath := writeConfig(t, dir, `{
		"max_minutes": 31,
		"check": ["echo ok"]
	}`)

	_, err := loadConfig(configPath)
	if err == nil || !strings.Contains(err.Error(), "max_minutes") {
		t.Fatalf("loadConfig error = %v, want max_minutes safety error", err)
	}
}

func TestLoadConfigRejectsMissingWorkCommands(t *testing.T) {
	dir := t.TempDir()
	configPath := writeConfig(t, dir, `{
		"fix": ["echo fix"]
	}`)

	_, err := loadConfig(configPath)
	if err == nil || !strings.Contains(err.Error(), "execute or check") {
		t.Fatalf("loadConfig error = %v, want missing command error", err)
	}
}

func TestLoadConfigKeepsConfiguredCommands(t *testing.T) {
	dir := t.TempDir()
	configPath := writeConfig(t, dir, `{
		"max_rounds": 3,
		"max_minutes": 5,
		"execute": ["echo execute"],
		"check": ["echo check"],
		"fix": ["echo fix"]
	}`)

	cfg, err := loadConfig(configPath)
	if err != nil {
		t.Fatalf("loadConfig returned error: %v", err)
	}
	if cfg.MaxRounds != 3 || cfg.MaxMinutes != 5 {
		t.Fatalf("limits = %d/%d, want 3/5", cfg.MaxRounds, cfg.MaxMinutes)
	}
	if cfg.Execute[0] != "echo execute" || cfg.Check[0] != "echo check" || cfg.Fix[0] != "echo fix" {
		t.Fatalf("commands were not preserved: %#v", cfg)
	}
}

func TestRunLoopPassesWhenCheckSucceeds(t *testing.T) {
	dir := t.TempDir()
	cfg := Config{
		Workdir:    dir,
		MaxRounds:  1,
		MaxMinutes: 1,
		Check:      []string{successCommand()},
	}

	if err := runLoop(cfg); err != nil {
		t.Fatalf("runLoop returned error: %v", err)
	}
}

func TestRunLoopFailsWhenCheckFailsWithoutFix(t *testing.T) {
	dir := t.TempDir()
	cfg := Config{
		Workdir:    dir,
		MaxRounds:  1,
		MaxMinutes: 1,
		Check:      []string{failureCommand()},
	}

	err := runLoop(cfg)
	if err == nil || !strings.Contains(err.Error(), "no fix") {
		t.Fatalf("runLoop error = %v, want missing fix error", err)
	}
}

func TestRunLoopPassesAfterFixCommand(t *testing.T) {
	dir := t.TempDir()
	cfg := Config{
		Workdir:    dir,
		MaxRounds:  3,
		MaxMinutes: 1,
		Check:      []string{flagCheckCommand()},
		Fix:        []string{flagFixCommand()},
	}

	if err := runLoop(cfg); err != nil {
		t.Fatalf("runLoop returned error: %v", err)
	}
	if _, err := os.Stat(filepath.Join(dir, "ok.flag")); err != nil {
		t.Fatalf("fix command did not create marker: %v", err)
	}
}

func TestRunLoopRoundSafetyBrakeReportsLastCheckFailure(t *testing.T) {
	dir := t.TempDir()
	cfg := Config{
		Workdir:    dir,
		MaxRounds:  2,
		MaxMinutes: 1,
		Check:      []string{failureWithOutputCommand("still failing")},
		Fix:        []string{successCommand()},
	}

	err := runLoop(cfg)
	if err == nil {
		t.Fatal("runLoop returned nil, want round safety brake error")
	}
	if !strings.Contains(err.Error(), "exceeded 2 rounds") {
		t.Fatalf("runLoop error = %v, want round safety brake", err)
	}
	if !strings.Contains(err.Error(), "still failing") {
		t.Fatalf("runLoop error = %v, want last check output", err)
	}
}

func writeConfig(t *testing.T, dir string, body string) string {
	t.Helper()

	path := filepath.Join(dir, "agent-loop.json")
	if err := os.WriteFile(path, []byte(body), 0o600); err != nil {
		t.Fatalf("write config: %v", err)
	}
	return path
}

func successCommand() string {
	if runtime.GOOS == "windows" {
		return "Write-Output ok"
	}
	return "echo ok"
}

func failureCommand() string {
	if runtime.GOOS == "windows" {
		return "exit 1"
	}
	return "exit 1"
}

func failureWithOutputCommand(message string) string {
	if runtime.GOOS == "windows" {
		return "Write-Output '" + message + "'; exit 1"
	}
	return "echo '" + message + "'; exit 1"
}

func flagCheckCommand() string {
	if runtime.GOOS == "windows" {
		return "if (Test-Path .\\ok.flag) { exit 0 } else { exit 1 }"
	}
	return "test -f ok.flag"
}

func flagFixCommand() string {
	if runtime.GOOS == "windows" {
		return "Set-Content -Path .\\ok.flag -Value ok"
	}
	return "touch ok.flag"
}
