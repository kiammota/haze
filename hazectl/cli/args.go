package cli

import (
	"errors"
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
	case "ps":
		// TODO: Implement ps


	case "help":
		// hazepkg help
	}
}


