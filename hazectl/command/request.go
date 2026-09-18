// cmd/instance.go
package cmd

import (
	"flag"
	"fmt"
	"hazectl/clients"
	"hazectl/msgpackrpc"
	"os"
)

func InstanceCommand() *Command {
    return &Command{
        Name: "instance",
        Subcommands: map[string]*Command{
            "stop": instanceStopCommand(),
        },
    }
}

func instanceStopCommand() *Command {
    return &Command{
        Name: "stop",
        Run: func(fs *flag.FlagSet, args []string) {
            force := fs.Bool("force", false, "força o encerramento")
            fs.Parse(args)

            id := fs.Arg(0)
            if id == "" {
                fmt.Fprintln(os.Stderr, "error: instance stop requer um id")
                os.Exit(1)
            }

            req := &msgpackrpc.Request{
                Func:   "instance/stop",
                Params: []interface{}{force},
            }

            resp, err := clients.SendRequest("127.0.0.1", lookupPort(id), req)
            if err != nil {
                fmt.Fprintln(os.Stderr, "error:", err)
                os.Exit(1)
            }

            fmt.Println(resp)
        },
    }
}
