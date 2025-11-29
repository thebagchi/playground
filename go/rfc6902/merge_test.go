package rfc6902

import (
	"testing"

	"google.golang.org/protobuf/proto"
)

func TestMerge(t *testing.T) {
	tests := []struct {
		name     string
		testFunc func(t *testing.T)
	}{
		{
			name: "MERGE_NEW_FIELD",
			testFunc: func(t *testing.T) {
				value := MakeValue(`{"a": "b"}`)
				if !Merge(value, map[string]any{"c": "d"}) {
					t.Errorf("Merge() returned false")
					return
				}
				if !Test(value, "c", "d") {
					t.Errorf("Post-merge verification failed for path %q: expected %v", "c", "d")
				}
			},
		},
		{
			name: "REPLACE_FIELD",
			testFunc: func(t *testing.T) {
				value := MakeValue(`{"a": "b"}`)
				if !Merge(value, map[string]any{"a": "c"}) {
					t.Errorf("Merge() returned false")
					return
				}
				if !Test(value, "a", "c") {
					t.Errorf("Post-merge verification failed for path %q: expected %v", "a", "c")
				}
			},
		},
		{
			name: "REMOVE_FIELD_WITH_NULL",
			testFunc: func(t *testing.T) {
				value := MakeValue(`{"a": "b", "c": "d"}`)
				if !Merge(value, map[string]any{"a": nil}) {
					t.Errorf("Merge() returned false")
					return
				}
				if Test(value, "a", nil) {
					t.Errorf("Field %q should have been removed", "a")
				}
				if !Test(value, "c", "d") {
					t.Errorf("Post-merge verification failed for path %q: expected %v", "c", "d")
				}
			},
		},
		{
			name: "REPLACE_WITH_NON_OBJECT",
			testFunc: func(t *testing.T) {
				value := MakeValue(`{"a": "b"}`)
				if !Merge(value, "replacement") {
					t.Errorf("Merge() returned false")
					return
				}
				expected := MakeValue(`"replacement"`)
				if !proto.Equal(value, expected) {
					t.Errorf("Merge failed to replace with non-object")
				}
			},
		},
		{
			name: "NESTED_MERGE",
			testFunc: func(t *testing.T) {
				value := MakeValue(`{"a": {"b": "c"}}`)
				if !Merge(value, map[string]any{"a": map[string]any{"d": "e"}}) {
					t.Errorf("Merge() returned false")
					return
				}
				if !Test(value, "a/b", "c") {
					t.Errorf("Post-merge verification failed for path %q: expected %v", "a/b", "c")
				}
				if !Test(value, "a/d", "e") {
					t.Errorf("Post-merge verification failed for path %q: expected %v", "a/d", "e")
				}
			},
		},
		{
			name: "NESTED_REPLACE",
			testFunc: func(t *testing.T) {
				value := MakeValue(`{"a": {"b": "c"}}`)
				if !Merge(value, map[string]any{"a": map[string]any{"b": "d"}}) {
					t.Errorf("Merge() returned false")
					return
				}
				if !Test(value, "a/b", "d") {
					t.Errorf("Post-merge verification failed for path %q: expected %v", "a/b", "d")
				}
			},
		},
		{
			name: "NESTED_REMOVE",
			testFunc: func(t *testing.T) {
				value := MakeValue(`{"a": {"b": "c", "d": "e"}}`)
				if !Merge(value, map[string]any{"a": map[string]any{"b": nil}}) {
					t.Errorf("Merge() returned false")
					return
				}
				if Test(value, "a/b", nil) {
					t.Errorf("Field %q should have been removed", "a/b")
				}
				if !Test(value, "a/d", "e") {
					t.Errorf("Post-merge verification failed for path %q: expected %v", "a/d", "e")
				}
			},
		},
		{
			name: "CONVERT_PRIMITIVE_TO_OBJECT",
			testFunc: func(t *testing.T) {
				value := MakeValue(`"original"`)
				if !Merge(value, map[string]any{"c": "d"}) {
					t.Errorf("Merge() returned false")
					return
				}
				if !Test(value, "c", "d") {
					t.Errorf("Post-merge verification failed for path %q: expected %v", "c", "d")
				}
			},
		},
		{
			name: "MERGE_EMPTY_OBJECT",
			testFunc: func(t *testing.T) {
				value := MakeValue(`{"a": "b"}`)
				if !Merge(value, map[string]any{}) {
					t.Errorf("Merge() returned false")
					return
				}
				if !Test(value, "a", "b") {
					t.Errorf("Post-merge verification failed for path %q: expected %v", "a", "b")
				}
			},
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, tt.testFunc)
	}
}
