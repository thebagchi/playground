package tools

import (
	_ "github.com/mmcloughlin/avo"
)

//go:generate go run using_avo.go -out using_avo/add_amd64.s -stubs using_avo/stub.go -pkg main
//go:generate venv/bin/python -m peachpy.x86_64 -mabi=goasm -S -o using_peachpy/add_amd64.s using_peachpy.py
