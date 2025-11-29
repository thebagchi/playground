package rfc6902

import (
	"testing"
)

func TestAdd(t *testing.T) {
	tests := []struct {
		name     string
		testFunc func(t *testing.T)
	}{
		{
			name: "ADD_NEW_STRUCT_FIELD",
			testFunc: func(t *testing.T) {
				value := MakeValue(`{
					"a": [
						{"b": "value1", "c": "value2"},
						{"d": "value3"}
					]
				}`)
				err := Add(value, "newField", "newValue")
				if err != nil {
					t.Errorf("Add(%q, %v) returned unexpected error: %v", "newField", "newValue", err)
					return
				}
				for path, expected := range map[string]any{
					"newField": "newValue",
				} {
					if !Test(value, path, expected) {
						t.Errorf("Post-add verification failed for path %q: expected %v", path, expected)
					}
				}
			},
		},
		{
			name: "REPLACE_STRUCT_FIELD",
			testFunc: func(t *testing.T) {
				value := MakeValue(`{
					"a": [
						{"b": "value1", "c": "value2"},
						{"d": "value3"}
					]
				}`)
				err := Add(value, "a", "replaced")
				if err != nil {
					t.Errorf("Add(%q, %v) returned unexpected error: %v", "a", "replaced", err)
					return
				}
				for path, expected := range map[string]any{
					"a": "replaced",
				} {
					if !Test(value, path, expected) {
						t.Errorf("Post-add verification failed for path %q: expected %v", path, expected)
					}
				}
			},
		},
		{
			name: "ADD_LIST_ELEMENT",
			testFunc: func(t *testing.T) {
				value := MakeValue(`{
					"a": [
						{"b": "value1", "c": "value2"},
						{"d": "value3"}
					]
				}`)
				err := Add(value, "a/2", "newElement")
				if err != nil {
					t.Errorf("Add(%q, %v) returned unexpected error: %v", "a/2", "newElement", err)
					return
				}
				for path, expected := range map[string]any{
					"a/2": "newElement",
				} {
					if !Test(value, path, expected) {
						t.Errorf("Post-add verification failed for path %q: expected %v", path, expected)
					}
				}
			},
		},
		{
			name: "INSERT_LIST_ELEMENT_AT_START",
			testFunc: func(t *testing.T) {
				value := MakeValue(`{
					"a": [
						{"b": "value1", "c": "value2"},
						{"d": "value3"}
					]
				}`)
				err := Add(value, "a/0", "insertedElement")
				if err != nil {
					t.Errorf("Add(%q, %v) returned unexpected error: %v", "a/0", "insertedElement", err)
					return
				}
				for path, expected := range map[string]any{
					"a/0":   "insertedElement",
					"a/1/b": "value1",
					"a/2/d": "value3",
				} {
					if !Test(value, path, expected) {
						t.Errorf("Post-add verification failed for path %q: expected %v", path, expected)
					}
				}
			},
		},
		{
			name: "ADD_NESTED_FIELD",
			testFunc: func(t *testing.T) {
				value := MakeValue(`{
					"a": [
						{"b": "value1", "c": "value2"},
						{"d": "value3"}
					]
				}`)
				err := Add(value, "a/1/e", "nestedValue")
				if err != nil {
					t.Errorf("Add(%q, %v) returned unexpected error: %v", "a/1/e", "nestedValue", err)
					return
				}
				for path, expected := range map[string]any{
					"a/1/e": "nestedValue",
					"a/1/d": "value3",
				} {
					if !Test(value, path, expected) {
						t.Errorf("Post-add verification failed for path %q: expected %v", path, expected)
					}
				}
			},
		},
		{
			name: "ADD_NUMBER",
			testFunc: func(t *testing.T) {
				value := MakeValue(`{
					"a": [
						{"b": "value1", "c": "value2"},
						{"d": "value3"}
					]
				}`)
				err := Add(value, "numberField", 42.5)
				if err != nil {
					t.Errorf("Add(%q, %v) returned unexpected error: %v", "numberField", 42.5, err)
					return
				}
				for path, expected := range map[string]any{
					"numberField": 42.5,
				} {
					if !Test(value, path, expected) {
						t.Errorf("Post-add verification failed for path %q: expected %v", path, expected)
					}
				}
			},
		},
		{
			name: "ADD_BOOL",
			testFunc: func(t *testing.T) {
				value := MakeValue(`{
					"a": [
						{"b": "value1", "c": "value2"},
						{"d": "value3"}
					]
				}`)
				err := Add(value, "boolField", true)
				if err != nil {
					t.Errorf("Add(%q, %v) returned unexpected error: %v", "boolField", true, err)
					return
				}
				for path, expected := range map[string]any{
					"boolField": true,
				} {
					if !Test(value, path, expected) {
						t.Errorf("Post-add verification failed for path %q: expected %v", path, expected)
					}
				}
			},
		},
		{
			name: "ADD_ARRAY",
			testFunc: func(t *testing.T) {
				value := MakeValue(`{
					"a": [
						{"b": "value1", "c": "value2"},
						{"d": "value3"}
					]
				}`)
				err := Add(value, "arrayField", []any{"item1", 2, true})
				if err != nil {
					t.Errorf("Add(%q, %v) returned unexpected error: %v", "arrayField", []any{"item1", 2, true}, err)
					return
				}
				for path, expected := range map[string]any{
					"arrayField/0": "item1",
					"arrayField/1": 2,
					"arrayField/2": true,
				} {
					if !Test(value, path, expected) {
						t.Errorf("Post-add verification failed for path %q: expected %v", path, expected)
					}
				}
			},
		},
		{
			name: "ADD_OBJECT",
			testFunc: func(t *testing.T) {
				value := MakeValue(`{
					"a": [
						{"b": "value1", "c": "value2"},
						{"d": "value3"}
					]
				}`)
				err := Add(value, "objectField", map[string]any{
					"nestedString": "value",
					"nestedNumber": 123,
				})
				if err != nil {
					t.Errorf("Add(%q, %v) returned unexpected error: %v", "objectField", map[string]any{
						"nestedString": "value",
						"nestedNumber": 123,
					}, err)
					return
				}
				for path, expected := range map[string]any{
					"objectField/nestedString": "value",
					"objectField/nestedNumber": 123,
				} {
					if !Test(value, path, expected) {
						t.Errorf("Post-add verification failed for path %q: expected %v", path, expected)
					}
				}
			},
		},
		{
			name: "ADD_LIST_ELEMENT_OUT_OF_BOUNDS",
			testFunc: func(t *testing.T) {
				value := MakeValue(`{
					"a": [
						{"b": "value1", "c": "value2"},
						{"d": "value3"}
					]
				}`)
				err := Add(value, "a/5", "shouldFail")
				if err == nil {
					t.Errorf("Add(%q, %v) expected error but got none", "a/5", "shouldFail")
				}
			},
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, tt.testFunc)
	}
}
