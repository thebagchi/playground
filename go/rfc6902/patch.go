package rfc6902

import (
	"encoding/base64"
	"encoding/json"
	"fmt"
	"strconv"
	"strings"

	"google.golang.org/protobuf/proto"
	"google.golang.org/protobuf/types/known/structpb"
)

const (
	KIND_NULL   = "NULL"
	KIND_NUMBER = "NUMBER"
	KIND_STRING = "STRING"
	KIND_BOOL   = "BOOL"
	KIND_STRUCT = "STRUCT"
	KIND_LIST   = "LIST"
)

func Kind(value *structpb.Value) string {
	if value == nil {
		return ""
	}
	switch value.Kind.(type) {
	case *structpb.Value_NullValue:
		return KIND_NULL
	case *structpb.Value_NumberValue:
		return KIND_NUMBER
	case *structpb.Value_StringValue:
		return KIND_STRING
	case *structpb.Value_BoolValue:
		return KIND_BOOL
	case *structpb.Value_StructValue:
		return KIND_STRUCT
	case *structpb.Value_ListValue:
		return KIND_LIST
	default:
		return ""
	}
}

// MakeValue creates a structpb.Value from a JSON string.
// Panics if the JSON is invalid.
func MakeValue(content string) *structpb.Value {
	val := new(structpb.Value)
	if err := val.UnmarshalJSON([]byte(content)); err != nil {
		panic("Failed to unmarshal JSON: " + err.Error())
	}
	return val
}

func IsNull(value *structpb.Value) bool {
	if value == nil {
		return false
	}
	_, ok := value.Kind.(*structpb.Value_NullValue)
	return ok
}

func IsNumber(value *structpb.Value) bool {
	if value == nil {
		return false
	}
	_, ok := value.Kind.(*structpb.Value_NumberValue)
	return ok
}

func IsString(value *structpb.Value) bool {
	if value == nil {
		return false
	}
	_, ok := value.Kind.(*structpb.Value_StringValue)
	return ok
}

func IsBool(value *structpb.Value) bool {
	if value == nil {
		return false
	}
	_, ok := value.Kind.(*structpb.Value_BoolValue)
	return ok
}

func IsDict(value *structpb.Value) bool {
	if value == nil {
		return false
	}
	_, ok := value.Kind.(*structpb.Value_StructValue)
	return ok
}

func IsList(value *structpb.Value) bool {
	if value == nil {
		return false
	}
	_, ok := value.Kind.(*structpb.Value_ListValue)
	return ok
}

func Delete(value *structpb.Value, path string) *structpb.Value {
	if value == nil || path == "" {
		return nil
	}

	// Remove leading slash if present
	if path[0] == '/' {
		path = path[1:]
	}

	// Split path by "/"
	parts := strings.Split(path, "/")
	if len(parts) == 0 {
		return nil
	}

	// Handle single part (delete from root)
	if len(parts) == 1 {
		last := parts[0]
		if last == "" {
			return nil
		}
		return remove(value, last)
	}

	// For multiple parts, extract parent and delete from it
	var (
		first  = strings.Join(parts[:len(parts)-1], "/")
		parent = Extract(value, first)
	)
	if parent == nil {
		return nil
	}

	last := parts[len(parts)-1]
	if last == "" {
		return nil
	}

	return remove(parent, last)
}

func Add(value *structpb.Value, path string, with any) error {
	if value == nil || path == "" {
		return fmt.Errorf("invalid arguments: value and path cannot be nil/empty")
	}

	// Check if with is already a *structpb.Value
	var data *structpb.Value
	if pbVal, ok := with.(*structpb.Value); ok {
		data = pbVal
	} else {
		// Convert newValue to *structpb.Value
		var err error
		data, err = structpb.NewValue(with)
		if err != nil {
			return fmt.Errorf("failed to convert value: %w", err)
		}
	}
	if data == nil {
		return fmt.Errorf("converted value is nil")
	}

	// Remove leading slash if present
	if path[0] == '/' {
		path = path[1:]
	}

	// Split path by "/"
	parts := strings.Split(path, "/")
	if len(parts) == 0 {
		return fmt.Errorf("invalid path: empty after splitting")
	}

	// Handle single part (add to root)
	if len(parts) == 1 {
		last := parts[0]
		if last == "" {
			return fmt.Errorf("invalid path: empty part")
		}
		return set(value, last, data)
	}

	// For multiple parts, extract parent and add to it
	var (
		first  = strings.Join(parts[:len(parts)-1], "/")
		parent = Extract(value, first)
	)
	if parent == nil {
		return fmt.Errorf("parent path not found: %s", first)
	}

	last := parts[len(parts)-1]
	if last == "" {
		return fmt.Errorf("invalid path: empty final part")
	}

	return set(parent, last, data)
}

func Replace(value *structpb.Value, path string, with any) error {
	if value == nil || path == "" {
		return fmt.Errorf("invalid arguments: value and path cannot be nil/empty")
	}

	// Convert newValue to *structpb.Value
	data, err := structpb.NewValue(with)
	if err != nil {
		return fmt.Errorf("failed to convert value: %w", err)
	}
	if data == nil {
		return fmt.Errorf("converted value is nil")
	}

	// Remove leading slash if present
	if path[0] == '/' {
		path = path[1:]
	}

	// Split path by "/"
	parts := strings.Split(path, "/")
	if len(parts) == 0 {
		return fmt.Errorf("invalid path: empty after splitting")
	}

	// Handle single part (replace at root)
	if len(parts) == 1 {
		last := parts[0]
		if last == "" {
			return fmt.Errorf("invalid path: empty part")
		}
		return replace(value, last, data)
	}

	// For multiple parts, extract parent and replace in it
	var (
		first  = strings.Join(parts[:len(parts)-1], "/")
		parent = Extract(value, first)
	)
	if parent == nil {
		return fmt.Errorf("parent path not found: %s", first)
	}

	last := parts[len(parts)-1]
	if last == "" {
		return fmt.Errorf("invalid path: empty final part")
	}

	return replace(parent, last, data)
}

func Move(value *structpb.Value, from, to string) error {
	if value == nil || from == "" || to == "" {
		return fmt.Errorf("invalid arguments: value, from, and to cannot be nil/empty")
	}

	// Extract and remove the value from the 'from' path
	temp := Delete(value, from)
	if temp == nil {
		return fmt.Errorf("source path not found: %s", from)
	}

	// Add the moved value to the 'to' path
	return Add(value, to, temp)
}

func Copy(value *structpb.Value, from, to string) error {
	if value == nil || from == "" || to == "" {
		return fmt.Errorf("invalid arguments: value, from, and to cannot be nil/empty")
	}

	// Extract the value from the 'from' path (without removing it)
	temp := Extract(value, from)
	if temp == nil {
		return fmt.Errorf("source path not found: %s", from)
	}

	// Add the copied value to the 'to' path
	return Add(value, to, temp)
}

// remove deletes a field/element from the given value and returns the deleted value
func remove(value *structpb.Value, key string) *structpb.Value {
	switch v := value.Kind.(type) {
	case *structpb.Value_StructValue:
		if v.StructValue.Fields == nil {
			return nil
		}
		previous, exists := v.StructValue.Fields[key]
		if !exists {
			return nil
		}
		delete(v.StructValue.Fields, key)
		return previous
	case *structpb.Value_ListValue:
		index, err := strconv.Atoi(key)
		if err != nil {
			return nil // invalid index
		}
		if v.ListValue.Values == nil || index < 0 || index >= len(v.ListValue.Values) {
			return nil
		}
		// Capture the old value before removing
		previous := v.ListValue.Values[index]
		// Remove element at index by slicing
		v.ListValue.Values = append(v.ListValue.Values[:index], v.ListValue.Values[index+1:]...)
		return previous
	}

	return nil // not a struct or list
}

// set adds/sets a field/element in the given value and returns the old value
func set(value *structpb.Value, key string, updated *structpb.Value) error {
	switch v := value.Kind.(type) {
	case *structpb.Value_StructValue:
		if v.StructValue.Fields == nil {
			// create fields map if it doesn't exist
			v.StructValue.Fields = make(map[string]*structpb.Value)
		}
		v.StructValue.Fields[key] = updated
		return nil
	case *structpb.Value_ListValue:
		index, err := strconv.Atoi(key)
		if err != nil {
			return fmt.Errorf("invalid list index '%s': %w", key, err)
		}
		if v.ListValue.Values == nil {
			v.ListValue.Values = make([]*structpb.Value, 0)
		}

		if index < 0 || index > len(v.ListValue.Values) {
			return fmt.Errorf("array index out of bounds: %d (valid range: 0-%d)", index, len(v.ListValue.Values))
		}

		// Insert at index, shifting elements to the right (RFC 6902 compliant)
		v.ListValue.Values = append(v.ListValue.Values[:index], append([]*structpb.Value{updated}, v.ListValue.Values[index:]...)...)
		return nil
	}

	return fmt.Errorf("value is not a struct or list, cannot set field/element")
}

// replace replaces a field/element in the given value (RFC 6902 compliant)
func replace(value *structpb.Value, key string, updated *structpb.Value) error {
	switch v := value.Kind.(type) {
	case *structpb.Value_StructValue:
		if v.StructValue.Fields == nil {
			return fmt.Errorf("cannot replace field '%s': object has no fields", key)
		}
		if _, exists := v.StructValue.Fields[key]; !exists {
			return fmt.Errorf("cannot replace field '%s': field does not exist", key)
		}
		v.StructValue.Fields[key] = updated
		return nil
	case *structpb.Value_ListValue:
		index, err := strconv.Atoi(key)
		if err != nil {
			return fmt.Errorf("invalid list index '%s': %w", key, err)
		}
		if v.ListValue.Values == nil || index < 0 || index >= len(v.ListValue.Values) {
			return fmt.Errorf("array index out of bounds: %d (valid range: 0-%d)", index, len(v.ListValue.Values)-1)
		}

		// Replace at index without shifting elements
		v.ListValue.Values[index] = updated
		return nil
	}

	return fmt.Errorf("value is not a struct or list, cannot replace field/element")
}

func Len(value *structpb.Value) int {
	if value == nil {
		return -1
	}
	switch v := value.Kind.(type) {
	case *structpb.Value_NullValue:
		return -1
	case *structpb.Value_NumberValue:
		return -1
	case *structpb.Value_StringValue:
		if nil != v {
			return len(v.StringValue)
		}
		return 0
	case *structpb.Value_BoolValue:
		return -1
	case *structpb.Value_StructValue:
		if nil != v {
			if nil != v.StructValue {
				if nil != v.StructValue.Fields {
					return len(v.StructValue.Fields)
				}
			}
		}
		return 0
	case *structpb.Value_ListValue:
		if nil != v {
			if nil != v.ListValue {
				if nil != v.ListValue.Values {
					return len(v.ListValue.Values)
				}
			}
		}
		return 0
	default:
		return -1
	}
}

func Extract(value *structpb.Value, path string) *structpb.Value {
	if value == nil || path == "" {
		return nil
	}

	// Remove leading slash if present
	if path[0] == '/' {
		path = path[1:]
	}

	// Split path by "/"
	var (
		parts   = strings.Split(path, "/")
		current = value
	)

	for _, part := range parts {
		if part == "" {
			continue // skip empty parts from leading/trailing slashes
		}

		if sv, ok := current.Kind.(*structpb.Value_StructValue); ok {
			// Navigate struct field
			if sv.StructValue.Fields == nil {
				return nil
			}
			current = sv.StructValue.Fields[part]
			if current == nil {
				return nil
			}
		} else if lv, ok := current.Kind.(*structpb.Value_ListValue); ok {
			// Navigate list index
			index, err := strconv.Atoi(part)
			if err != nil {
				return nil // invalid index
			}
			if lv.ListValue.Values == nil || index < 0 || index >= len(lv.ListValue.Values) {
				return nil
			}
			current = lv.ListValue.Values[index]
		} else {
			return nil // not a struct or list, can't navigate further
		}
	}

	return current
}

// Test checks if the value at the given path matches the expected value (RFC 6902 compliant).
// This function uses direct type comparison for optimal performance (~887 ns/op vs ~6351 ns/op for test).
func Test(value *structpb.Value, path string, ev any) bool {
	if value == nil {
		return false
	}

	// Extract the value at the path
	av := Extract(value, path)
	if av == nil {
		return false
	}

	// Compare the extracted value with expected using cmp
	return cmp(av, ev)
}

// test checks if the value at the given path matches the expected value using equals.
// This function converts the expected value to structpb.Value and uses proto.Equal for comparison.
func test(value *structpb.Value, path string, expected any) bool {
	if value == nil {
		return false
	}

	// Extract the value at the path
	av := Extract(value, path)
	if av == nil {
		return false
	}

	// Convert expected value to structpb.Value
	ev, err := structpb.NewValue(expected)
	if err != nil || ev == nil {
		return false
	}

	// Compare the extracted value with expected using equals
	return equals(av, ev)
}

// equals compares two structpb.Value objects using proto.Equal for deep equality
func equals(a, b *structpb.Value) bool {
	return proto.Equal(a, b)
}

// match compares two structpb.Value objects by switching on their types and validating
func match(a, b *structpb.Value) bool {
	if a == nil && b == nil {
		return true
	}
	if a == nil || b == nil {
		return false
	}

	// Switch on the type of the first value and validate both have the same type
	switch ta := a.Kind.(type) {
	case *structpb.Value_NullValue:
		_, ok := b.Kind.(*structpb.Value_NullValue)
		return ok
	case *structpb.Value_BoolValue:
		if tb, ok := b.Kind.(*structpb.Value_BoolValue); ok {
			return ta.BoolValue == tb.BoolValue
		}
		return false
	case *structpb.Value_NumberValue:
		if tb, ok := b.Kind.(*structpb.Value_NumberValue); ok {
			return ta.NumberValue == tb.NumberValue
		}
		return false
	case *structpb.Value_StringValue:
		if tb, ok := b.Kind.(*structpb.Value_StringValue); ok {
			return ta.StringValue == tb.StringValue
		}
		return false
	case *structpb.Value_StructValue:
		if tb, ok := b.Kind.(*structpb.Value_StructValue); ok {
			if len(ta.StructValue.Fields) != len(tb.StructValue.Fields) {
				return false
			}
			for k, vA := range ta.StructValue.Fields {
				if vB, ok := tb.StructValue.Fields[k]; ok {
					if !match(vA, vB) {
						return false
					}
				} else {
					return false
				}
			}
			return true
		}
		return false
	case *structpb.Value_ListValue:
		if tb, ok := b.Kind.(*structpb.Value_ListValue); ok {
			if len(ta.ListValue.Values) != len(tb.ListValue.Values) {
				return false
			}
			for i, vA := range ta.ListValue.Values {
				if !match(vA, tb.ListValue.Values[i]) {
					return false
				}
			}
			return true
		}
		return false
	default:
		return false
	}
}

// cmp compares a structpb.Value with any expected value by switching on expected types
func cmp(actual *structpb.Value, expected any) bool {
	switch ev := expected.(type) {
	case nil:
		if actual == nil {
			return true
		}
		_, ok := actual.Kind.(*structpb.Value_NullValue)
		return ok
	case bool:
		if temp, ok := actual.Kind.(*structpb.Value_BoolValue); ok {
			return temp.BoolValue == ev
		}
		return false
	case int:
		if temp, ok := actual.Kind.(*structpb.Value_NumberValue); ok {
			return temp.NumberValue == float64(ev)
		}
		return false
	case int8:
		if temp, ok := actual.Kind.(*structpb.Value_NumberValue); ok {
			return temp.NumberValue == float64(ev)
		}
		return false
	case int16:
		if temp, ok := actual.Kind.(*structpb.Value_NumberValue); ok {
			return temp.NumberValue == float64(ev)
		}
		return false
	case int32:
		if temp, ok := actual.Kind.(*structpb.Value_NumberValue); ok {
			return temp.NumberValue == float64(ev)
		}
		return false
	case int64:
		if temp, ok := actual.Kind.(*structpb.Value_NumberValue); ok {
			return temp.NumberValue == float64(ev)
		}
		return false
	case uint:
		if temp, ok := actual.Kind.(*structpb.Value_NumberValue); ok {
			return temp.NumberValue == float64(ev)
		}
		return false
	case uint8:
		if temp, ok := actual.Kind.(*structpb.Value_NumberValue); ok {
			return temp.NumberValue == float64(ev)
		}
		return false
	case uint16:
		if temp, ok := actual.Kind.(*structpb.Value_NumberValue); ok {
			return temp.NumberValue == float64(ev)
		}
		return false
	case uint32:
		if temp, ok := actual.Kind.(*structpb.Value_NumberValue); ok {
			return temp.NumberValue == float64(ev)
		}
		return false
	case uint64:
		if temp, ok := actual.Kind.(*structpb.Value_NumberValue); ok {
			return temp.NumberValue == float64(ev)
		}
		return false
	case float32:
		if temp, ok := actual.Kind.(*structpb.Value_NumberValue); ok {
			return temp.NumberValue == float64(ev)
		}
		return false
	case float64:
		if temp, ok := actual.Kind.(*structpb.Value_NumberValue); ok {
			return temp.NumberValue == ev
		}
		return false
	case json.Number:
		if temp, ok := actual.Kind.(*structpb.Value_NumberValue); ok {
			e, err := ev.Float64()
			if err != nil {
				return false
			}
			return temp.NumberValue == e
		}
		return false
	case string:
		if temp, ok := actual.Kind.(*structpb.Value_StringValue); ok {
			return temp.StringValue == ev
		}
		return false
	case []byte:
		if temp, ok := actual.Kind.(*structpb.Value_StringValue); ok {
			// []byte is base64-encoded as string in protobuf
			e := base64.StdEncoding.EncodeToString(ev)
			return temp.StringValue == e
		}
		return false
	case *structpb.Value:
		return proto.Equal(actual, ev)
	case map[string]any:
		if temp, ok := actual.Kind.(*structpb.Value_StructValue); ok {
			if len(temp.StructValue.Fields) != len(ev) {
				return false
			}
			for k, v := range ev {
				if value, ok := temp.StructValue.Fields[k]; ok {
					if !cmp(value, v) {
						return false
					}
				} else {
					return false
				}
			}
			return true
		}
		return false
	case []any:
		if temp, ok := actual.Kind.(*structpb.Value_ListValue); ok {
			if len(temp.ListValue.Values) != len(ev) {
				return false
			}
			for i, value := range ev {
				if !cmp(temp.ListValue.Values[i], value) {
					return false
				}
			}
			return true
		}
		return false
	default:
		return false
	}
}
