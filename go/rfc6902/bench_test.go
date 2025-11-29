package rfc6902

import (
	"testing"

	"google.golang.org/protobuf/types/known/structpb"
)

func BenchmarkEquals(b *testing.B) {
	// Test data for benchmarking - all created with MakeValue
	testCases := []*structpb.Value{
		nil,
		MakeValue("null"),
		MakeValue("true"),
		MakeValue("42.5"),
		MakeValue(`"hello world"`),
		MakeValue(`{"key": "value", "number": 42}`),
		MakeValue(`["a", "b", "c", 1, 2, 3]`),
		MakeValue(`{"nested": {"deep": {"value": true}}}`),
	}

	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		for _, val := range testCases {
			equals(val, val) // Compare value with itself
		}
	}
}

func BenchmarkCmp(b *testing.B) {
	// Test data for benchmarking - mix of types for cmp function
	testCases := []struct {
		actual   *structpb.Value
		expected any
	}{
		{nil, nil},
		{MakeValue("null"), nil},
		{MakeValue("true"), true},
		{MakeValue("42.5"), 42.5},
		{MakeValue(`"hello world"`), "hello world"},
		{MakeValue(`{"key": "value"}`), map[string]any{"key": "value"}},
		{MakeValue(`["a", "b", "c"]`), []any{"a", "b", "c"}},
		{MakeValue(`{"nested": {"value": true}}`), map[string]any{"nested": map[string]any{"value": true}}},
	}

	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		for _, tc := range testCases {
			cmp(tc.actual, tc.expected)
		}
	}
}

func BenchmarkMatch(b *testing.B) {
	// Test data for benchmarking (same as BenchmarkEquals)
	testCases := []*structpb.Value{
		nil,
		structpb.NewNullValue(),
		structpb.NewBoolValue(true),
		structpb.NewNumberValue(42.5),
		structpb.NewStringValue("hello world"),
		MakeValue(`{"key": "value", "number": 42}`),
		MakeValue(`["a", "b", "c", 1, 2, 3]`),
		MakeValue(`{"nested": {"deep": {"value": true}}}`),
	}

	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		for _, val := range testCases {
			match(val, val) // Compare value with itself
		}
	}
}

func BenchmarkTestLower(b *testing.B) {
	// Test data for benchmarking test (lowercase)
	testCases := []struct {
		value    *structpb.Value
		path     string
		expected any
	}{
		{MakeValue(`{"key": "value"}`), "key", "value"},
		{MakeValue(`{"number": 42}`), "number", 42},
		{MakeValue(`{"bool": true}`), "bool", true},
		{MakeValue(`{"array": [1, 2, 3]}`), "array/0", 1},
		{MakeValue(`{"nested": {"deep": "value"}}`), "nested/deep", "value"},
		{MakeValue(`["a", "b", "c"]`), "/1", "b"},
	}

	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		for _, tc := range testCases {
			test(tc.value, tc.path, tc.expected)
		}
	}
}

func BenchmarkTest(b *testing.B) {
	// Test data for benchmarking Test (original function)
	testCases := []struct {
		value    *structpb.Value
		path     string
		expected any
	}{
		{MakeValue(`{"key": "value"}`), "key", "value"},
		{MakeValue(`{"number": 42}`), "number", 42},
		{MakeValue(`{"bool": true}`), "bool", true},
		{MakeValue(`{"array": [1, 2, 3]}`), "array/0", 1},
		{MakeValue(`{"nested": {"deep": "value"}}`), "nested/deep", "value"},
		{MakeValue(`["a", "b", "c"]`), "/1", "b"},
	}

	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		for _, tc := range testCases {
			Test(tc.value, tc.path, tc.expected)
		}
	}
}
