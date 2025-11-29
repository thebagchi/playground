package rfc6902

import (
	"testing"

	"google.golang.org/protobuf/types/known/structpb"
)

func TestLen(t *testing.T) {
	tests := []struct {
		name     string
		testFunc func(t *testing.T)
	}{
		{
			name: "NIL",
			testFunc: func(t *testing.T) {
				if Len((*structpb.Value)(nil)) != -1 {
					t.Errorf("Len(%v) = %d; want %d", (*structpb.Value)(nil), Len((*structpb.Value)(nil)), -1)
				}
			},
		},
		{
			name: "NULL",
			testFunc: func(t *testing.T) {
				value := MakeValue("null")
				if Len(value) != -1 {
					t.Errorf("Len(%v) = %d; want %d", value, Len(value), -1)
				}
			},
		},
		{
			name: "NUMBER",
			testFunc: func(t *testing.T) {
				value := MakeValue("42.0")
				if Len(value) != -1 {
					t.Errorf("Len(%v) = %d; want %d", value, Len(value), -1)
				}
			},
		},
		{
			name: "STRING_EMPTY",
			testFunc: func(t *testing.T) {
				value := MakeValue("\"\"")
				if Len(value) != 0 {
					t.Errorf("Len(%v) = %d; want %d", value, Len(value), 0)
				}
			},
		},
		{
			name: "STRING",
			testFunc: func(t *testing.T) {
				value := MakeValue("\"hello\"")
				if Len(value) != 5 {
					t.Errorf("Len(%v) = %d; want %d", value, Len(value), 5)
				}
			},
		},
		{
			name: "BOOL",
			testFunc: func(t *testing.T) {
				value := MakeValue("true")
				if Len(value) != -1 {
					t.Errorf("Len(%v) = %d; want %d", value, Len(value), -1)
				}
			},
		},
		{
			name: "STRUCT_EMPTY",
			testFunc: func(t *testing.T) {
				value := MakeValue("{}")
				if Len(value) != 0 {
					t.Errorf("Len(%v) = %d; want %d", value, Len(value), 0)
				}
			},
		},
		{
			name: "STRUCT_WITH_FIELDS",
			testFunc: func(t *testing.T) {
				value := MakeValue("{\"a\":1,\"b\":2}")
				if Len(value) != 2 {
					t.Errorf("Len(%v) = %d; want %d", value, Len(value), 2)
				}
			},
		},
		{
			name: "LIST_EMPTY",
			testFunc: func(t *testing.T) {
				value := MakeValue("[]")
				if Len(value) != 0 {
					t.Errorf("Len(%v) = %d; want %d", value, Len(value), 0)
				}
			},
		},
		{
			name: "LIST_WITH_ITEMS",
			testFunc: func(t *testing.T) {
				value := MakeValue("[1,2,3]")
				if Len(value) != 3 {
					t.Errorf("Len(%v) = %d; want %d", value, Len(value), 3)
				}
			},
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, tt.testFunc)
	}
}
