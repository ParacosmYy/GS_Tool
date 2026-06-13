package main

import (
	"context"
	"encoding/json"
	"errors"
	"flag"
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
	"strings"
	"time"
)

const (
	defaultMaxRounds  = 20
	defaultMaxMinutes = 30
)

type Config struct {
	Workdir    string   `json:"workdir"`
	MaxRounds  int      `json:"max_rounds"`
	MaxMinutes int      `json:"max_minutes"`
	Execute    []string `json:"execute"`
	Check      []string `json:"check"`
	Fix        []string `json:"fix"`
}

type commandResult struct {
	Command string
	Output  string
	Err     error
}

func main() {
	configPath := flag.String("config", "", "JSON config path for the GO execution loop")
	dryRun := flag.Bool("dry-run", false, "print planned commands without running them")
	flag.Parse()

	if strings.TrimSpace(*configPath) == "" {
		exitWithError(errors.New("missing -config"))
	}

	cfg, err := loadConfig(*configPath)
	if err != nil {
		exitWithError(err)
	}

	if *dryRun {
		printPlan(cfg)
		return
	}

	if err := runLoop(cfg); err != nil {
		exitWithError(err)
	}
}

func loadConfig(path string) (Config, error) {
	data, err := os.ReadFile(path)
	if err != nil {
		return Config{}, fmt.Errorf("read config: %w", err)
	}

	var cfg Config
	if err := json.Unmarshal(data, &cfg); err != nil {
		return Config{}, fmt.Errorf("parse config: %w", err)
	}

	if cfg.MaxRounds <= 0 {
		cfg.MaxRounds = defaultMaxRounds
	}
	if cfg.MaxRounds > defaultMaxRounds {
		return Config{}, fmt.Errorf("max_rounds cannot exceed %d", defaultMaxRounds)
	}
	if cfg.MaxMinutes <= 0 {
		cfg.MaxMinutes = defaultMaxMinutes
	}
	if cfg.MaxMinutes > defaultMaxMinutes {
		return Config{}, fmt.Errorf("max_minutes cannot exceed %d", defaultMaxMinutes)
	}
	if len(cfg.Execute) == 0 && len(cfg.Check) == 0 {
		return Config{}, errors.New("config must contain execute or check commands")
	}
	if strings.TrimSpace(cfg.Workdir) == "" {
		cfg.Workdir = "."
	}

	abs, err := filepath.Abs(cfg.Workdir)
	if err != nil {
		return Config{}, fmt.Errorf("resolve workdir: %w", err)
	}
	cfg.Workdir = abs
	return cfg, nil
}

func runLoop(cfg Config) error {
	deadline := time.Now().Add(time.Duration(cfg.MaxMinutes) * time.Minute)
	var lastCheckFailure commandResult

	for round := 1; round <= cfg.MaxRounds; round++ {
		if time.Now().After(deadline) {
			return fmt.Errorf("safety brake: exceeded %d minutes", cfg.MaxMinutes)
		}

		fmt.Printf("GO round %d/%d\n", round, cfg.MaxRounds)

		if failed := runCommands("execute", cfg.Workdir, cfg.Execute, deadline); failed.Err != nil {
			return fmt.Errorf("execute failed: %s\n%s", failed.Command, failed.Output)
		}

		if failed := runCommands("check", cfg.Workdir, cfg.Check, deadline); failed.Err == nil {
			fmt.Println("GO loop passed")
			return nil
		} else {
			lastCheckFailure = failed
			fmt.Printf("check failed: %s\n%s\n", failed.Command, failed.Output)
		}

		if len(cfg.Fix) == 0 {
			return errors.New("check failed and no fix commands are configured")
		}
		if failed := runCommands("fix", cfg.Workdir, cfg.Fix, deadline); failed.Err != nil {
			return fmt.Errorf("fix failed: %s\n%s", failed.Command, failed.Output)
		}
	}

	if lastCheckFailure.Err != nil {
		return fmt.Errorf("safety brake: exceeded %d rounds; last check failed: %s\n%s",
			cfg.MaxRounds,
			lastCheckFailure.Command,
			lastCheckFailure.Output)
	}
	return fmt.Errorf("safety brake: exceeded %d rounds", cfg.MaxRounds)
}

func runCommands(stage string, workdir string, commands []string, deadline time.Time) commandResult {
	for _, command := range commands {
		command = strings.TrimSpace(command)
		if command == "" {
			continue
		}

		remaining := time.Until(deadline)
		if remaining <= 0 {
			return commandResult{Command: command, Err: context.DeadlineExceeded}
		}

		fmt.Printf("[%s] %s\n", stage, command)
		ctx, cancel := context.WithTimeout(context.Background(), remaining)
		output, err := shell(ctx, workdir, command)
		cancel()
		if err != nil {
			return commandResult{Command: command, Output: output, Err: err}
		}
		if strings.TrimSpace(output) != "" {
			fmt.Print(output)
		}
	}

	return commandResult{}
}

func shell(ctx context.Context, workdir string, command string) (string, error) {
	var cmd *exec.Cmd
	if runtime.GOOS == "windows" {
		cmd = exec.CommandContext(ctx, "powershell.exe", "-NoProfile", "-ExecutionPolicy", "Bypass", "-Command", command)
	} else {
		cmd = exec.CommandContext(ctx, "sh", "-c", command)
	}
	cmd.Dir = workdir
	data, err := cmd.CombinedOutput()
	return string(data), err
}

func printPlan(cfg Config) {
	fmt.Printf("workdir: %s\n", cfg.Workdir)
	fmt.Printf("max_rounds: %d\n", cfg.MaxRounds)
	fmt.Printf("max_minutes: %d\n", cfg.MaxMinutes)
	printCommands("execute", cfg.Execute)
	printCommands("check", cfg.Check)
	printCommands("fix", cfg.Fix)
}

func printCommands(stage string, commands []string) {
	if len(commands) == 0 {
		fmt.Printf("%s: <none>\n", stage)
		return
	}
	for _, command := range commands {
		fmt.Printf("%s: %s\n", stage, command)
	}
}

func exitWithError(err error) {
	fmt.Fprintln(os.Stderr, err)
	os.Exit(1)
}
