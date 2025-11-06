package tools

import (
	_ "github.com/mmcloughlin/avo"
)

//go:generate python3 -m venv venv
//go:generate venv/bin/pip install --upgrade pip
//go:generate venv/bin/pip install -r requirements.txt
