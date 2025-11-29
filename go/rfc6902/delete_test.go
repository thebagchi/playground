package rfc6902

import (
	"testing"

	"google.golang.org/protobuf/types/known/structpb"
)

func TestDelete(t *testing.T) {
	tests := []struct {
		name     string
		testFunc func(t *testing.T)
	}{
		{
			name: "DELETE_STRUCT_FIELD",
			testFunc: func(t *testing.T) {
				value := MakeValue(`{
					"a": [
						{"b": "value1", "c": "value2"},
						{"d": "value3"}
					]
				}`)
				if !cmp(Delete(value, "a"), MakeValue(`[{"b": "value1", "c": "value2"}, {"d": "value3"}]`)) {
					t.Errorf("Delete(%q) = %v; want %v", "a", Delete(value, "a"), MakeValue(`[{"b": "value1", "c": "value2"}, {"d": "value3"}]`))
				}
				for path, expected := range map[string]any{} {
					if !Test(value, path, expected) {
						t.Errorf("Post-delete verification failed for path %q: expected %v", path, expected)
					}
				}
				for _, path := range []string{"a"} {
					if Test(value, path, nil) {
						t.Errorf("Post-delete verification failed: path %q should not exist", path)
					}
				}
			},
		},
		{
			name: "DELETE_LIST_ELEMENT",
			testFunc: func(t *testing.T) {
				value := MakeValue(`{
					"a": [
						{"b": "value1", "c": "value2"},
						{"d": "value3"}
					]
				}`)
				if !cmp(Delete(value, "a/0"), MakeValue(`{"b": "value1", "c": "value2"}`)) {
					t.Errorf("Delete(%q) = %v; want %v", "a/0", Delete(value, "a/0"), MakeValue(`{"b": "value1", "c": "value2"}`))
				}
				for path, expected := range map[string]any{
					"a": []any{map[string]any{"d": "value3"}},
				} {
					if !Test(value, path, expected) {
						t.Errorf("Post-delete verification failed for path %q: expected %v", path, expected)
					}
				}
			},
		},
		{
			name: "DELETE_NESTED_FIELD",
			testFunc: func(t *testing.T) {
				value := MakeValue(`{
					"a": [
						{"b": "value1", "c": "value2"},
						{"d": "value3"}
					]
				}`)
				if !cmp(Delete(value, "a/1/d"), MakeValue("\"value3\"")) {
					t.Errorf("Delete(%q) = %v; want %v", "a/1/d", Delete(value, "a/1/d"), MakeValue("\"value3\""))
				}
				for path, expected := range map[string]any{
					"a/1": map[string]any{},
				} {
					if !Test(value, path, expected) {
						t.Errorf("Post-delete verification failed for path %q: expected %v", path, expected)
					}
				}
				for _, path := range []string{"a/1/d"} {
					if Test(value, path, nil) {
						t.Errorf("Post-delete verification failed: path %q should not exist", path)
					}
				}
			},
		},
		{
			name: "DELETE_NONEXISTENT",
			testFunc: func(t *testing.T) {
				value := MakeValue(`{
					"a": [
						{"b": "value1", "c": "value2"},
						{"d": "value3"}
					]
				}`)
				if !cmp(Delete(value, "nonexistent"), (*structpb.Value)(nil)) {
					t.Errorf("Delete(%q) = %v; want %v", "nonexistent", Delete(value, "nonexistent"), (*structpb.Value)(nil))
				}
				for path, expected := range map[string]any{} {
					if !Test(value, path, expected) {
						t.Errorf("Post-delete verification failed for path %q: expected %v", path, expected)
					}
				}
			},
		},
		{
			name: "DELETE_INVALID_INDEX",
			testFunc: func(t *testing.T) {
				value := MakeValue(`{
					"a": [
						{"b": "value1", "c": "value2"},
						{"d": "value3"}
					]
				}`)
				if !cmp(Delete(value, "a/10"), (*structpb.Value)(nil)) {
					t.Errorf("Delete(%q) = %v; want %v", "a/10", Delete(value, "a/10"), (*structpb.Value)(nil))
				}
				for path, expected := range map[string]any{} {
					if !Test(value, path, expected) {
						t.Errorf("Post-delete verification failed for path %q: expected %v", path, expected)
					}
				}
			},
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, tt.testFunc)
	}
}
