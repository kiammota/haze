package cli

import (
	"errors"
	"fmt"
	"hazectl/cmd"
	"hazectl/msgpackrpc"
	"regexp"
	"slices"
	"strconv"
	"strings"
)
var reParams = regexp.MustCompile(`\[(.*?)\]`)

var CommandTable []string = []string{
	"ps",
	"instance",
	"request", "req",
	"plugin",	
} 

func parseParams(raw string) []any {
	match := reParams.FindStringSubmatch(raw)
	if len(match) < 2 || match[1] == "" {
		return nil
	}

	rawParams := strings.Split(match[1], ",")
	params := make([]any, 0, len(rawParams))

	for _, p := range rawParams {
		p = strings.TrimSpace(p)
		if p == "" {
			continue
		}

		if num, err := strconv.Atoi(p); err == nil {
			params = append(params, num)
			continue
		}

		if b, err := strconv.ParseBool(p); err == nil {
			params = append(params, b)
			continue
		}

		params = append(params, strings.Trim(p, `"'`))
	}

	return params
}


func ValidateCommand(cmd string) error {
	if slices.Contains(CommandTable, cmd) {
		return nil 
	}

	return errors.New("invalid command: " + cmd)
}


func TraitArgs(args []string) {
	if len(args) == 0 {
		return
	}

	err := ValidateCommand(args[0]) 
	if err != nil {
		PrintErr(err.Error())
		return
	}
	
	switch args[0] {
		case "ps": {

	}
	case "request", "req":
	if len(args) < 3 {
		PrintErr("request requires: <ip:port> <module/acessor>[<parameters>]")
		break
	}

	ip, portStr, found := strings.Cut(args[1], ":")
	if !found {
		PrintErr("invalid IP:Port format. Use ip:port")
		break
	}

	port, err := strconv.Atoi(portStr)
	if err != nil {
		PrintErr("port must be a valid number")
		break
	}

	rpc := args[2]
	function, _, _ := strings.Cut(rpc, "[")
	params := parseParams(rpc)

	req := &msgpackrpc.Request{}
	req.Init(1, function, params)

	resp, err := cmd.Request(ip, port, req)
	if err != nil {
		PrintErr(err.Error())
		break
	

	fmt.Println(resp.ToString())

		// Imprime o resultado formatado!
		fmt.Println(resp.ToString())


		req := &msgpackrpc.Request{}
		req.Init(1, function, params)

		// Executa a chamada
		resp, err = cmd.Request(ip, port, req)
		if err != nil {
			fmt.Println(err)
			break
		}

		// Imprime o resultado formatado!
		fmt.Println(resp.ToString())

	}





	case "help":
		// hazepkg help

	}
}
