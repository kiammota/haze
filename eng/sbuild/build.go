package main

import (
	"flag"
	"fmt"
	"os"
	"os/exec"
	"runtime"
)

func run(name string, args ...string) error {
	cmd := exec.Command(name, args...)
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	cmd.Stdin = os.Stdin

	return cmd.Run()
}

func main() {
	rebuild := flag.Bool("rebuild", false, "rebuild everything")
	flag.Parse()

	platform := runtime.GOOS

	fmt.Printf("Building Haze for %s...\n", platform)

	if *rebuild {
		fmt.Println("Rebuilding everything...")

		if err := os.RemoveAll("b"); err != nil {
			fmt.Fprintf(os.Stderr, "failed to remove build directory: %v\n", err)
			os.Exit(1)
		}
	}

	if err := run(
		"cmake",
		"-S", ".",
		"-B", "b",
	); err != nil {
		fmt.Fprintf(os.Stderr, "cmake configure failed: %v\n", err)
		os.Exit(1)
	}

	buildArgs := []string{
		"--build",
		"b",
		"--parallel",
	}

	if platform == "windows" {
		buildArgs = append(buildArgs, "--config", "Debug")
	}

	if err := run("cmake", buildArgs...); err != nil {
		fmt.Fprintf(os.Stderr, "cmake build failed: %v\n", err)
		os.Exit(1)
	}

	fmt.Println("Haze built successfully.")
}
