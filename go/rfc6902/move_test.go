package rfc6902

import (
	"testing"
)

func TestMove(t *testing.T) {
	tests := []struct {
		name     string
		testFunc func(t *testing.T)
	}{
		{
			name: "MOVE_STRUCT_FIELD",
			testFunc: func(t *testing.T) {
				value := MakeValue(`{
					"a": [
						{"b": "value1", "c": "value2"},
						{"d": "value3"}
					],
					"numberField": 1.0
				}`)
				if err := Move(value, "numberField", "movedField"); err != nil {
					t.Errorf("Move(%q, %q) returned unexpected error: %v", "numberField", "movedField", err)
					return
				}
				for path, expected := range map[string]any{
					"movedField": 1.0,
				} {
					if !Test(value, path, expected) {
						t.Errorf("Post-move verification failed for path %q: expected %v", path, expected)
					}
				}
				for _, path := range []string{"numberField"} {
					if Test(value, path, nil) {
						t.Errorf("Post-move verification failed: path %q should not exist", path)
					}
				}
			},
		},
		{
			name: "MOVE_LIST_ELEMENT",
			testFunc: func(t *testing.T) {
				value := MakeValue(`{
					"a": [
						{"b": "value1", "c": "value2"},
						{"d": "value3"}
					],
					"numberField": 1.0
				}`)
				if err := Move(value, "a/0", "a/1"); err != nil {
					t.Errorf("Move(%q, %q) returned unexpected error: %v", "a/0", "a/1", err)
					return
				}
				for path, expected := range map[string]any{
					"a/0/d": "value3",
					"a/1/b": "value1",
					"a/1/c": "value2",
				} {
					if !Test(value, path, expected) {
						t.Errorf("Post-move verification failed for path %q: expected %v", path, expected)
					}
				}
				for _, path := range []string{} {
					if Test(value, path, nil) {
						t.Errorf("Post-move verification failed: path %q should not exist", path)
					}
				}
			},
		},
		{
			name: "MOVE_NESTED_FIELD",
			testFunc: func(t *testing.T) {
				value := MakeValue(`{
					"a": [
						{"b": "value1", "c": "value2"},
						{"d": "value3"}
					],
					"numberField": 1.0
				}`)
				if err := Move(value, "a/1/d", "a/0/newField"); err != nil {
					t.Errorf("Move(%q, %q) returned unexpected error: %v", "a/1/d", "a/0/newField", err)
					return
				}
				for path, expected := range map[string]any{
					"a/0/newField": "value3",
				} {
					if !Test(value, path, expected) {
						t.Errorf("Post-move verification failed for path %q: expected %v", path, expected)
					}
				}
				for _, path := range []string{"a/1/d"} {
					if Test(value, path, nil) {
						t.Errorf("Post-move verification failed: path %q should not exist", path)
					}
				}
			},
		},
		{
			name: "MOVE_TO_NEW_ARRAY_POSITION",
			testFunc: func(t *testing.T) {
				value := MakeValue(`{
					"a": [
						{"b": "value1", "c": "value2"},
						{"d": "value3"}
					],
					"numberField": 1.0
				}`)
				if err := Move(value, "a/0", "a/1"); err != nil {
					t.Errorf("Move(%q, %q) returned unexpected error: %v", "a/0", "a/1", err)
					return
				}
				for path, expected := range map[string]any{
					"a/0/d": "value3",
					"a/1/b": "value1",
					"a/1/c": "value2",
				} {
					if !Test(value, path, expected) {
						t.Errorf("Post-move verification failed for path %q: expected %v", path, expected)
					}
				}
				for _, path := range []string{} {
					if Test(value, path, nil) {
						t.Errorf("Post-move verification failed: path %q should not exist", path)
					}
				}
			},
		},
		{
			name: "MOVE_FROM_NONEXISTENT",
			testFunc: func(t *testing.T) {
				value := MakeValue(`{
					"a": [
						{"b": "value1", "c": "value2"},
						{"d": "value3"}
					],
					"numberField": 1.0
				}`)
				if err := Move(value, "nonexistent", "somewhere"); err == nil {
					t.Errorf("Move(%q, %q) expected error but got none", "nonexistent", "somewhere")
				}
			},
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, tt.testFunc)
	}
}
