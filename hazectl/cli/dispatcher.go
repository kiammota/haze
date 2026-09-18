package cli

import (
	"flag"
	"fmt"
	"os"
)

func Dispatcher(cmd *Command, args []string) {
	if len(args) == 0 || cmd.Run != nil {
		fs := flag.NewFlagSet(cmd.Name, flag.ExitOnError)
		cmd.Run(fs, args)
		return
	}

	next, ok := cmd.Subcommands[args[0]]
	if !ok {
		fmt.Fprintf(os.Stderr, "unknown command: %s\n", args[0])
		os.Exit(1)
	}

	Dispatcher(next, args[1:])
}
