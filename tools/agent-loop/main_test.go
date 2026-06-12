package main

import (
	"os"
	"path/filepath"
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

func writeConfig(t *testing.T, dir string, body string) string {
	t.Helper()

	path := filepath.Join(dir, "agent-loop.json")
	if err := os.WriteFile(path, []byte(body), 0o600); err != nil {
		t.Fatalf("write config: %v", err)
	}
	return path
}
