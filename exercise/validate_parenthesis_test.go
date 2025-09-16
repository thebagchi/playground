package exercise

import (
	"encoding/json"
	"fmt"
	"testing"
)

func ValidateParenthesis(s string) bool {
	var (
		valid   = true
		balance = 0
		parens  = []string{"()", "{}", "[]"}
	)

	for _, paren := range parens {
		balance = 0
		for _, char := range s {
			switch byte(char) {
			case paren[0]:
				balance = balance + 1
			case paren[1]:
				balance = balance - 1
				if balance < 0 {
					return false
				}
			default:
				continue
			}
		}
		if balance != 0 {
			return false
		}
	}
	return valid
}

func TestValidateParenthesis(t *testing.T) {
	data := `[
		{
			"input": "()",
			"expected": true
		},
		{
			"input": "(())",
			"expected": true
		},
		{
			"input": "()()",
			"expected": true
		},
		{
			"input": "((()))",
			"expected": true
		},
		{
			"input": "(",
			"expected": false
		},
		{
			"input": ")",
			"expected": false
		},
		{
			"input": ")(",
			"expected": false
		},
		{
			"input": "(()",
			"expected": false
		},
		{
			"input": "())",
			"expected": false
		},
		{
			"input": "",
			"expected": true
		},
		{
			"input": "(()())",
			"expected": true
		},
		{
			"input": "(()",
			"expected": false
		},
		{
			"input": "()()())",
			"expected": false
		},
		{
			"input": "(((",
			"expected": false
		},
		{
			"input": ")))",
			"expected": false
		},
		{
			"input": "abc",
			"expected": true
		},
		{
			"input": "a(b)c",
			"expected": true
		},
		{
			"input": "a(b(c)d)e",
			"expected": true
		},
		{
			"input": "a)b(c",
			"expected": false
		}
	]`

	var tests []struct {
		Input    string `json:"input"`
		Expected bool   `json:"expected"`
	}

	if err := json.Unmarshal([]byte(data), &tests); err != nil {
		t.Fatalf("failed to unmarshal test cases: %v", err)
	}

	for _, test := range tests {
		t.Run(fmt.Sprintf("testing for: %s", test.Input), func(t *testing.T) {
			result := ValidateParenthesis(test.Input)
			if result != test.Expected {
				t.Errorf("ValidateParenthesis(%q) = %v, want %v", test.Input, result, test.Expected)
			}
		})
	}
}
