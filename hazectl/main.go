package main

import (
	"fmt"
	"hazectl/cli"
	"hazectl/hazefs"
	"os"
	"strings"
	"unsafe"

	"golang.org/x/sys/windows"
)

func IsLaunchedFromExplorer() bool {
	snapshot, err := windows.CreateToolhelp32Snapshot(
		windows.TH32CS_SNAPPROCESS,
		0,
	)
	if err != nil {
		return false
	}
	defer windows.CloseHandle(snapshot)

	var current windows.ProcessEntry32
	current.Size = uint32(unsafe.Sizeof(current))

	if err := windows.Process32First(snapshot, &current); err != nil {
		return false
	}

	var parentPID uint32

	for {
		if current.ProcessID == uint32(windows.GetCurrentProcessId()) {
			parentPID = current.ParentProcessID
			break
		}

		if err := windows.Process32Next(snapshot, &current); err != nil {
			break
		}
	}

	if parentPID == 0 {
		return false
	}

	if err := windows.Process32First(snapshot, &current); err != nil {
		return false
	}

	for {
		if current.ProcessID == parentPID {
			name := windows.UTF16ToString(current.ExeFile[:])
			return strings.EqualFold(name, "explorer.exe")
		}

		if err := windows.Process32Next(snapshot, &current); err != nil {
			break
		}
	}

	return false
}

func main() {

	if IsLaunchedFromExplorer() {
		cli.Print("You need a terminal to run this.")
		fmt.Println()
		fmt.Print("Press Enter to exit...")
		fmt.Scanln()
		return
	}
	hazefs.PathsInstance.InitPaths()

	args := os.Args[1:]

	if len(args) == 0 {
		cli.Help()
		return
	}

	switch args[0] {
	case "-v", "--version":
		cli.Version()
		return

	case "-h", "--help":
		cli.Help()
		return
	}

	cli.TraitArgs(args)
}
