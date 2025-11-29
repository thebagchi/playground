package rfc6902

import (
	"testing"
)

func TestCopy(t *testing.T) {
	tests := []struct {
		name     string
		testFunc func(t *testing.T)
	}{
		{
			name: "COPY_STRUCT_FIELD",
			testFunc: func(t *testing.T) {
				// Create a fresh test value for each test
				value := MakeValue(`{
					"a": [
						{"b": "value1", "c": "value2"},
						{"d": "value3"}
					],
					"numberField": 1.0
				}`)

				err := Copy(value, "numberField", "copiedField")
				if err != nil {
					t.Errorf("Copy(%q, %q) returned unexpected error: %v", "numberField", "copiedField", err)
					return
				}

				// Verify post-copy state using Test function
				for path, expected := range map[string]any{
					"copiedField": 1.0,
					"numberField": 1.0,
				} {
					if !Test(value, path, expected) {
						t.Errorf("Post-copy verification failed for path %q: expected true, got false", path)
					}
				}
			},
		},
		{
			name: "COPY_LIST_ELEMENT",
			testFunc: func(t *testing.T) {
				// Create a fresh test value for each test
				value := MakeValue(`{
					"a": [
						{"b": "value1", "c": "value2"},
						{"d": "value3"}
					],
					"numberField": 1.0
				}`)

				err := Copy(value, "a/0", "a/2")
				if err != nil {
					t.Errorf("Copy(%q, %q) returned unexpected error: %v", "a/0", "a/2", err)
					return
				}

				// Verify post-copy state using Test function
				for path, expected := range map[string]any{
					"a/0/b": "value1",
					"a/0/c": "value2",
					"a/1/d": "value3",
					"a/2/b": "value1",
					"a/2/c": "value2",
				} {
					if !Test(value, path, expected) {
						t.Errorf("Post-copy verification failed for path %q: expected true, got false", path)
					}
				}
			},
		},
		{
			name: "COPY_NESTED_FIELD",
			testFunc: func(t *testing.T) {
				// Create a fresh test value for each test
				value := MakeValue(`{
					"a": [
						{"b": "value1", "c": "value2"},
						{"d": "value3"}
					],
					"numberField": 1.0
				}`)

				err := Copy(value, "a/1/d", "a/0/newField")
				if err != nil {
					t.Errorf("Copy(%q, %q) returned unexpected error: %v", "a/1/d", "a/0/newField", err)
					return
				}

				// Verify post-copy state using Test function
				for path, expected := range map[string]any{
					"a/0/newField": "value3",
					"a/1/d":        "value3",
				} {
					if !Test(value, path, expected) {
						t.Errorf("Post-copy verification failed for path %q: expected true, got false", path)
					}
				}
			},
		},
		{
			name: "COPY_FROM_NONEXISTENT",
			testFunc: func(t *testing.T) {
				// Create a fresh test value for each test
				value := MakeValue(`{
					"a": [
						{"b": "value1", "c": "value2"},
						{"d": "value3"}
					],
					"numberField": 1.0
				}`)

				err := Copy(value, "nonexistent", "somewhere")
				if err == nil {
					t.Errorf("Copy(%q, %q) expected error but got none", "nonexistent", "somewhere")
				}
			},
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, tt.testFunc)
	}
}
