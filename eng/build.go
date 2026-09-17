package main

import (
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"runtime"
)

const (
	UNDEFINED = 0
	WINDOWS   = 1
	LINUX     = 2
	MACOS     = 3
)

var Platform int

func main() {
	switch runtime.GOOS {
	case "windows":
		Platform = WINDOWS
	case "linux":
		Platform = LINUX
	case "darwin":
		Platform = MACOS
	default:
		Platform = UNDEFINED
	}

	root, err := findRoot()
	if err != nil {
		fmt.Fprintln(os.Stderr, err)
		os.Exit(1)
	}

	buildDir := filepath.Join(root, "build")
	cpuCount := runtime.NumCPU()

	fmt.Println("==> Configuring Haze...")

	if err := run("cmake", "-S", root, "-B", buildDir); err != nil {
		os.Exit(1)
	}

	fmt.Println()
	fmt.Printf("Found: %d CPU logical processors\n", cpuCount)
	fmt.Println()
	fmt.Println("==> Building Haze...")
	fmt.Printf(
		"    Compiling in parallel using %d CPU cores/logical processors...\n",
		cpuCount,
	)

	if err := run(
		"cmake",
		"--build",
		buildDir,
		"--parallel",
		fmt.Sprint(cpuCount),
	); err != nil {
		os.Exit(1)
	}

	executable := filepath.Join(buildDir, "haze")

	if Platform == WINDOWS {
		executable += ".exe"
	}

	if _, err := os.Stat(executable); err != nil {
		fmt.Fprintf(
			os.Stderr,
			"Build completed, but haze was not found at: %s\n",
			executable,
		)
		os.Exit(1)
	}

	fmt.Println()
	fmt.Println("==> Build successful")
	fmt.Printf("    Found: %s\n", executable)
}

func findRoot() (string, error) {
	dir, err := os.Getwd()
	if err != nil {
		return "", err
	}

	for {
		cmake := filepath.Join(dir, "CMakeLists.txt")

		if _, err := os.Stat(cmake); err == nil {
			return dir, nil
		}

		parent := filepath.Dir(dir)

		if parent == dir {
			return "", fmt.Errorf("CMakeLists.txt not found")
		}

		dir = parent
	}
}

func run(name string, args ...string) error {
	cmd := exec.Command(name, args...)
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	cmd.Stdin = os.Stdin

	return cmd.Run()
}
