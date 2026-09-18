package cmd

import (
	"flag"

)

type Command struct {
	Name        string
	Subcommands map[string]*Command
	Run         func(flags *flag.FlagSet, args []string)
}
