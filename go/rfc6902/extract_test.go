package rfc6902

import (
	"testing"

	"google.golang.org/protobuf/types/known/structpb"
)

func TestExtract(t *testing.T) {
	tests := []struct {
		name     string
		testFunc func(t *testing.T)
	}{
		{
			name: "EXTRACT_ROOT",
			testFunc: func(t *testing.T) {
				value := MakeValue(`{
					"a": [
						{"b": "value1"},
						{"b": "value2"}
					]
				}`)
				if !cmp(Extract(value, ""), (*structpb.Value)(nil)) {
					t.Errorf("Extract(%v, %q) = %v; want %v", value, "", Extract(value, ""), (*structpb.Value)(nil))
				}
			},
		},
		{
			name: "EXTRACT_A",
			testFunc: func(t *testing.T) {
				value := MakeValue(`{
					"a": [
						{"b": "value1"},
						{"b": "value2"}
					]
				}`)
				expected := value.GetStructValue().GetFields()["a"]
				if !cmp(Extract(value, "a"), expected) {
					t.Errorf("Extract(%v, %q) = %v; want %v", value, "a", Extract(value, "a"), expected)
				}
			},
		},
		{
			name: "EXTRACT_A_0",
			testFunc: func(t *testing.T) {
				value := MakeValue(`{
					"a": [
						{"b": "value1"},
						{"b": "value2"}
					]
				}`)
				expected := value.GetStructValue().GetFields()["a"].GetListValue().GetValues()[0]
				if !cmp(Extract(value, "a/0"), expected) {
					t.Errorf("Extract(%v, %q) = %v; want %v", value, "a/0", Extract(value, "a/0"), expected)
				}
			},
		},
		{
			name: "EXTRACT_A_0_B",
			testFunc: func(t *testing.T) {
				value := MakeValue(`{
					"a": [
						{"b": "value1"},
						{"b": "value2"}
					]
				}`)
				if !cmp(Extract(value, "a/0/b"), MakeValue("\"value1\"")) {
					t.Errorf("Extract(%v, %q) = %v; want %v", value, "a/0/b", Extract(value, "a/0/b"), MakeValue("\"value1\""))
				}
			},
		},
		{
			name: "EXTRACT_A_1_B",
			testFunc: func(t *testing.T) {
				value := MakeValue(`{
					"a": [
						{"b": "value1"},
						{"b": "value2"}
					]
				}`)
				if !cmp(Extract(value, "a/1/b"), MakeValue("\"value2\"")) {
					t.Errorf("Extract(%v, %q) = %v; want %v", value, "a/1/b", Extract(value, "a/1/b"), MakeValue("\"value2\""))
				}
			},
		},
		{
			name: "EXTRACT_WITH_LEADING_SLASH",
			testFunc: func(t *testing.T) {
				value := MakeValue(`{
					"a": [
						{"b": "value1"},
						{"b": "value2"}
					]
				}`)
				if !cmp(Extract(value, "/a/0/b"), MakeValue("\"value1\"")) {
					t.Errorf("Extract(%v, %q) = %v; want %v", value, "/a/0/b", Extract(value, "/a/0/b"), MakeValue("\"value1\""))
				}
			},
		},
		{
			name: "EXTRACT_INVALID_INDEX",
			testFunc: func(t *testing.T) {
				value := MakeValue(`{
					"a": [
						{"b": "value1"},
						{"b": "value2"}
					]
				}`)
				if !cmp(Extract(value, "a/2"), (*structpb.Value)(nil)) {
					t.Errorf("Extract(%v, %q) = %v; want %v", value, "a/2", Extract(value, "a/2"), (*structpb.Value)(nil))
				}
			},
		},
		{
			name: "EXTRACT_NON_NUMERIC_INDEX",
			testFunc: func(t *testing.T) {
				value := MakeValue(`{
					"a": [
						{"b": "value1"},
						{"b": "value2"}
					]
				}`)
				if !cmp(Extract(value, "a/abc"), (*structpb.Value)(nil)) {
					t.Errorf("Extract(%v, %q) = %v; want %v", value, "a/abc", Extract(value, "a/abc"), (*structpb.Value)(nil))
				}
			},
		},
		{
			name: "EXTRACT_NONEXISTENT",
			testFunc: func(t *testing.T) {
				value := MakeValue(`{
					"a": [
						{"b": "value1"},
						{"b": "value2"}
					]
				}`)
				if !cmp(Extract(value, "a/0/c"), (*structpb.Value)(nil)) {
					t.Errorf("Extract(%v, %q) = %v; want %v", value, "a/0/c", Extract(value, "a/0/c"), (*structpb.Value)(nil))
				}
			},
		},
		{
			name: "EXTRACT_NIL_VALUE",
			testFunc: func(t *testing.T) {
				value := (*structpb.Value)(nil)
				if !cmp(Extract(value, "a"), (*structpb.Value)(nil)) {
					t.Errorf("Extract(%v, %q) = %v; want %v", value, "a", Extract(value, "a"), (*structpb.Value)(nil))
				}
			},
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, tt.testFunc)
	}
}
