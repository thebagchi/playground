package rfc6902

import (
	"testing"
)

func TestReplace(t *testing.T) {
	tests := []struct {
		name     string
		testFunc func(t *testing.T)
	}{
		{
			name: "REPLACE_STRUCT_FIELD",
			testFunc: func(t *testing.T) {
				value := MakeValue(`{
					"a": [
						{"b": "value1", "c": "value2"},
						{"d": "value3"}
					],
					"numberField": 1.0
				}`)
				err := Replace(value, "a", "replaced")
				if err != nil {
					t.Errorf("Replace(%q, %v) returned unexpected error: %v", "a", "replaced", err)
					return
				}
				for path, expected := range map[string]any{
					"a": "replaced",
				} {
					if !Test(value, path, expected) {
						t.Errorf("Post-replace verification failed for path %q: expected %v", path, expected)
					}
				}
			},
		},
		{
			name: "REPLACE_LIST_ELEMENT",
			testFunc: func(t *testing.T) {
				value := MakeValue(`{
					"a": [
						{"b": "value1", "c": "value2"},
						{"d": "value3"}
					],
					"numberField": 1.0
				}`)
				err := Replace(value, "a/0", "replacedElement")
				if err != nil {
					t.Errorf("Replace(%q, %v) returned unexpected error: %v", "a/0", "replacedElement", err)
					return
				}
				for path, expected := range map[string]any{
					"a/0":   "replacedElement",
					"a/1/d": "value3",
				} {
					if !Test(value, path, expected) {
						t.Errorf("Post-replace verification failed for path %q: expected %v", path, expected)
					}
				}
			},
		},
		{
			name: "REPLACE_NESTED_FIELD",
			testFunc: func(t *testing.T) {
				value := MakeValue(`{
					"a": [
						{"b": "value1", "c": "value2"},
						{"d": "value3"}
					],
					"numberField": 1.0
				}`)
				err := Replace(value, "a/1/d", "replacedValue")
				if err != nil {
					t.Errorf("Replace(%q, %v) returned unexpected error: %v", "a/1/d", "replacedValue", err)
					return
				}
				for path, expected := range map[string]any{
					"a/1/d": "replacedValue",
				} {
					if !Test(value, path, expected) {
						t.Errorf("Post-replace verification failed for path %q: expected %v", path, expected)
					}
				}
			},
		},
		{
			name: "REPLACE_NONEXISTENT_FIELD",
			testFunc: func(t *testing.T) {
				value := MakeValue(`{
					"a": [
						{"b": "value1", "c": "value2"},
						{"d": "value3"}
					],
					"numberField": 1.0
				}`)
				err := Replace(value, "nonexistent", "value")
				if err == nil {
					t.Errorf("Replace(%q, %v) expected error but got none", "nonexistent", "value")
				}
			},
		},
		{
			name: "REPLACE_OUT_OF_BOUNDS",
			testFunc: func(t *testing.T) {
				value := MakeValue(`{
					"a": [
						{"b": "value1", "c": "value2"},
						{"d": "value3"}
					],
					"numberField": 1.0
				}`)
				err := Replace(value, "a/5", "value")
				if err == nil {
					t.Errorf("Replace(%q, %v) expected error but got none", "a/5", "value")
				}
			},
		},
		{
			name: "REPLACE_NUMBER",
			testFunc: func(t *testing.T) {
				value := MakeValue(`{
					"a": [
						{"b": "value1", "c": "value2"},
						{"d": "value3"}
					],
					"numberField": 1.0
				}`)
				err := Replace(value, "numberField", 42.5)
				if err != nil {
					t.Errorf("Replace(%q, %v) returned unexpected error: %v", "numberField", 42.5, err)
					return
				}
				for path, expected := range map[string]any{
					"numberField": 42.5,
				} {
					if !Test(value, path, expected) {
						t.Errorf("Post-replace verification failed for path %q: expected %v", path, expected)
					}
				}
			},
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, tt.testFunc)
	}
}
