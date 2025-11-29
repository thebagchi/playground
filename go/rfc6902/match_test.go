package rfc6902

import (
	"testing"

	"google.golang.org/protobuf/types/known/structpb"
)

func TestMatching(t *testing.T) {
	tests := []struct {
		name     string
		testFunc func(t *testing.T)
	}{
		{
			name: "BOTH_NIL",
			testFunc: func(t *testing.T) {
				a := (*structpb.Value)(nil)
				b := (*structpb.Value)(nil)
				if equals(a, b) != match(a, b) {
					t.Errorf("equals(%v, %v) = %v, but match(%v, %v) = %v",
						a, b, equals(a, b), a, b, match(a, b))
				}
			},
		},
		{
			name: "A_NIL_B_NOT",
			testFunc: func(t *testing.T) {
				a := (*structpb.Value)(nil)
				b := structpb.NewStringValue("test")
				if equals(a, b) != match(a, b) {
					t.Errorf("equals(%v, %v) = %v, but match(%v, %v) = %v",
						a, b, equals(a, b), a, b, match(a, b))
				}
			},
		},
		{
			name: "A_NOT_B_NIL",
			testFunc: func(t *testing.T) {
				a := structpb.NewStringValue("test")
				b := (*structpb.Value)(nil)
				if equals(a, b) != match(a, b) {
					t.Errorf("equals(%v, %v) = %v, but match(%v, %v) = %v",
						a, b, equals(a, b), a, b, match(a, b))
				}
			},
		},
		{
			name: "NULL_VALUES",
			testFunc: func(t *testing.T) {
				a := structpb.NewNullValue()
				b := structpb.NewNullValue()
				if equals(a, b) != match(a, b) {
					t.Errorf("equals(%v, %v) = %v, but match(%v, %v) = %v",
						a, b, equals(a, b), a, b, match(a, b))
				}
			},
		},
		{
			name: "BOOL_SAME",
			testFunc: func(t *testing.T) {
				a := structpb.NewBoolValue(true)
				b := structpb.NewBoolValue(true)
				if equals(a, b) != match(a, b) {
					t.Errorf("equals(%v, %v) = %v, but match(%v, %v) = %v",
						a, b, equals(a, b), a, b, match(a, b))
				}
			},
		},
		{
			name: "BOOL_DIFFERENT",
			testFunc: func(t *testing.T) {
				a := structpb.NewBoolValue(true)
				b := structpb.NewBoolValue(false)
				if equals(a, b) != match(a, b) {
					t.Errorf("equals(%v, %v) = %v, but match(%v, %v) = %v",
						a, b, equals(a, b), a, b, match(a, b))
				}
			},
		},
		{
			name: "NUMBER_SAME",
			testFunc: func(t *testing.T) {
				a := structpb.NewNumberValue(42.5)
				b := structpb.NewNumberValue(42.5)
				if equals(a, b) != match(a, b) {
					t.Errorf("equals(%v, %v) = %v, but match(%v, %v) = %v",
						a, b, equals(a, b), a, b, match(a, b))
				}
			},
		},
		{
			name: "NUMBER_DIFFERENT",
			testFunc: func(t *testing.T) {
				a := structpb.NewNumberValue(42.5)
				b := structpb.NewNumberValue(43.5)
				if equals(a, b) != match(a, b) {
					t.Errorf("equals(%v, %v) = %v, but match(%v, %v) = %v",
						a, b, equals(a, b), a, b, match(a, b))
				}
			},
		},
		{
			name: "STRING_SAME",
			testFunc: func(t *testing.T) {
				a := structpb.NewStringValue("hello")
				b := structpb.NewStringValue("hello")
				if equals(a, b) != match(a, b) {
					t.Errorf("equals(%v, %v) = %v, but match(%v, %v) = %v",
						a, b, equals(a, b), a, b, match(a, b))
				}
			},
		},
		{
			name: "STRING_DIFFERENT",
			testFunc: func(t *testing.T) {
				a := structpb.NewStringValue("hello")
				b := structpb.NewStringValue("world")
				if equals(a, b) != match(a, b) {
					t.Errorf("equals(%v, %v) = %v, but match(%v, %v) = %v",
						a, b, equals(a, b), a, b, match(a, b))
				}
			},
		},
		{
			name: "EMPTY_STRUCTS",
			testFunc: func(t *testing.T) {
				a := MakeValue("{}")
				b := MakeValue("{}")
				if equals(a, b) != match(a, b) {
					t.Errorf("equals(%v, %v) = %v, but match(%v, %v) = %v",
						a, b, equals(a, b), a, b, match(a, b))
				}
			},
		},
		{
			name: "SAME_STRUCTS",
			testFunc: func(t *testing.T) {
				a := MakeValue(`{"key": "value"}`)
				b := MakeValue(`{"key": "value"}`)
				if equals(a, b) != match(a, b) {
					t.Errorf("equals(%v, %v) = %v, but match(%v, %v) = %v",
						a, b, equals(a, b), a, b, match(a, b))
				}
			},
		},
		{
			name: "DIFFERENT_STRUCTS",
			testFunc: func(t *testing.T) {
				a := MakeValue(`{"key": "value1"}`)
				b := MakeValue(`{"key": "value2"}`)
				if equals(a, b) != match(a, b) {
					t.Errorf("equals(%v, %v) = %v, but match(%v, %v) = %v",
						a, b, equals(a, b), a, b, match(a, b))
				}
			},
		},
		{
			name: "EMPTY_LISTS",
			testFunc: func(t *testing.T) {
				a := MakeValue("[]")
				b := MakeValue("[]")
				if equals(a, b) != match(a, b) {
					t.Errorf("equals(%v, %v) = %v, but match(%v, %v) = %v",
						a, b, equals(a, b), a, b, match(a, b))
				}
			},
		},
		{
			name: "SAME_LISTS",
			testFunc: func(t *testing.T) {
				a := MakeValue(`["a", "b", "c"]`)
				b := MakeValue(`["a", "b", "c"]`)
				if equals(a, b) != match(a, b) {
					t.Errorf("equals(%v, %v) = %v, but match(%v, %v) = %v",
						a, b, equals(a, b), a, b, match(a, b))
				}
			},
		},
		{
			name: "DIFFERENT_LISTS",
			testFunc: func(t *testing.T) {
				a := MakeValue(`["a", "b", "c"]`)
				b := MakeValue(`["x", "y", "z"]`)
				if equals(a, b) != match(a, b) {
					t.Errorf("equals(%v, %v) = %v, but match(%v, %v) = %v",
						a, b, equals(a, b), a, b, match(a, b))
				}
			},
		},
		{
			name: "NESTED_STRUCTS_SAME",
			testFunc: func(t *testing.T) {
				a := MakeValue(`{"obj": {"nested": "value"}}`)
				b := MakeValue(`{"obj": {"nested": "value"}}`)
				if equals(a, b) != match(a, b) {
					t.Errorf("equals(%v, %v) = %v, but match(%v, %v) = %v",
						a, b, equals(a, b), a, b, match(a, b))
				}
			},
		},
		{
			name: "NESTED_STRUCTS_DIFFERENT",
			testFunc: func(t *testing.T) {
				a := MakeValue(`{"obj": {"nested": "value1"}}`)
				b := MakeValue(`{"obj": {"nested": "value2"}}`)
				if equals(a, b) != match(a, b) {
					t.Errorf("equals(%v, %v) = %v, but match(%v, %v) = %v",
						a, b, equals(a, b), a, b, match(a, b))
				}
			},
		},
		{
			name: "DIFFERENT_TYPES",
			testFunc: func(t *testing.T) {
				a := structpb.NewStringValue("test")
				b := structpb.NewNumberValue(42)
				if equals(a, b) != match(a, b) {
					t.Errorf("equals(%v, %v) = %v, but match(%v, %v) = %v",
						a, b, equals(a, b), a, b, match(a, b))
				}
			},
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, tt.testFunc)
	}
}
