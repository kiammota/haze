package cmd

import (
	"hazectl/clients"
	"net"
	"os"

)

func Clean(instancePath string) error {
	entries, err := os.ReadDir(instancePath)

	if err != nil {
		return err
	}
	for i := 0; i < len(entries); i++ {
		if !entries[i].IsDir() {
			filePort := entries[i].Name()
			conn, err := net.Dial("tcp", ":"+filePort)
			if err != nil {
				return err
			}
			client := clients.NewClient(conn)
			_, err = client.Call("test/ping", nil)
			println("testing: ", filePort)
			if err != nil {
				println("err: ", err.Error())
				println("inactive! deleting...")
				err = os.Remove(filePort)
				if err != nil {
					return err
				}
			}
		}
	}

	return nil

}
