package rfc6902

import (
	"testing"
)

func TestTest(t *testing.T) {
	tests := []struct {
		name     string
		testFunc func(t *testing.T)
	}{
		{
			name: "TEST_STRUCT_FIELD_MATCH",
			testFunc: func(t *testing.T) {
				if Test(MakeValue(`{
					"a": [
						{"b": "value1", "c": "value2"},
						{"d": "value3"}
					],
					"numberField": 1.0
				}`), "a", []any{map[string]any{"b": "value1", "c": "value2"}, map[string]any{"d": "value3"}}) != true {
					t.Errorf("Test(%q, %v) = %v; want %v", "a", []any{map[string]any{"b": "value1", "c": "value2"}, map[string]any{"d": "value3"}}, false, true)
				}
			},
		},
		{
			name: "TEST_STRUCT_FIELD_NO_MATCH",
			testFunc: func(t *testing.T) {
				if Test(MakeValue(`{
					"a": [
						{"b": "value1", "c": "value2"},
						{"d": "value3"}
					],
					"numberField": 1.0
				}`), "a", "different") != false {
					t.Errorf("Test(%q, %v) = %v; want %v", "a", "different", true, false)
				}
			},
		},
		{
			name: "TEST_LIST_ELEMENT_MATCH",
			testFunc: func(t *testing.T) {
				if Test(MakeValue(`{
					"a": [
						{"b": "value1", "c": "value2"},
						{"d": "value3"}
					],
					"numberField": 1.0
				}`), "a/0/b", "value1") != true {
					t.Errorf("Test(%q, %v) = %v; want %v", "a/0/b", "value1", false, true)
				}
			},
		},
		{
			name: "TEST_LIST_ELEMENT_NO_MATCH",
			testFunc: func(t *testing.T) {
				if Test(MakeValue(`{
					"a": [
						{"b": "value1", "c": "value2"},
						{"d": "value3"}
					],
					"numberField": 1.0
				}`), "a/0/b", "different") != false {
					t.Errorf("Test(%q, %v) = %v; want %v", "a/0/b", "different", true, false)
				}
			},
		},
		{
			name: "TEST_NESTED_FIELD_MATCH",
			testFunc: func(t *testing.T) {
				if Test(MakeValue(`{
					"a": [
						{"b": "value1", "c": "value2"},
						{"d": "value3"}
					],
					"numberField": 1.0
				}`), "a/1/d", "value3") != true {
					t.Errorf("Test(%q, %v) = %v; want %v", "a/1/d", "value3", false, true)
				}
			},
		},
		{
			name: "TEST_NUMBER_FIELD_MATCH",
			testFunc: func(t *testing.T) {
				if Test(MakeValue(`{
					"a": [
						{"b": "value1", "c": "value2"},
						{"d": "value3"}
					],
					"numberField": 1.0
				}`), "numberField", 1.0) != true {
					t.Errorf("Test(%q, %v) = %v; want %v", "numberField", 1.0, false, true)
				}
			},
		},
		{
			name: "TEST_NUMBER_FIELD_NO_MATCH",
			testFunc: func(t *testing.T) {
				if Test(MakeValue(`{
					"a": [
						{"b": "value1", "c": "value2"},
						{"d": "value3"}
					],
					"numberField": 1.0
				}`), "numberField", 2.0) != false {
					t.Errorf("Test(%q, %v) = %v; want %v", "numberField", 2.0, true, false)
				}
			},
		},
		{
			name: "TEST_NONEXISTENT_PATH",
			testFunc: func(t *testing.T) {
				if Test(MakeValue(`{
					"a": [
						{"b": "value1", "c": "value2"},
						{"d": "value3"}
					],
					"numberField": 1.0
				}`), "nonexistent", "value") != false {
					t.Errorf("Test(%q, %v) = %v; want %v", "nonexistent", "value", true, false)
				}
			},
		},
		{
			name: "TEST_INVALID_INDEX",
			testFunc: func(t *testing.T) {
				if Test(MakeValue(`{
					"a": [
						{"b": "value1", "c": "value2"},
						{"d": "value3"}
					],
					"numberField": 1.0
				}`), "a/10", "value") != false {
					t.Errorf("Test(%q, %v) = %v; want %v", "a/10", "value", true, false)
				}
			},
		},
		{
			name: "TEST_NIL_VALUE",
			testFunc: func(t *testing.T) {
				if Test(MakeValue(`{
					"a": [
						{"b": "value1", "c": "value2"},
						{"d": "value3"}
					],
					"numberField": 1.0
				}`), "a", (any)(nil)) != false {
					t.Errorf("Test(%q, %v) = %v; want %v", "a", (any)(nil), true, false)
				}
			},
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, tt.testFunc)
	}
}
