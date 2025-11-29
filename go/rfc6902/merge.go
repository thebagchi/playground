package rfc6902

import (
	"google.golang.org/protobuf/proto"
	"google.golang.org/protobuf/types/known/structpb"
)

// DeepCopy copies src into dst by merging
func DeepCopy[T proto.Message](dst, src T) {
	proto.Reset(dst)
	proto.Merge(dst, src)
}

// Implementation of RFC 7396 JSON Merge Patch

func Merge(value *structpb.Value, with any) bool {
	// Convert the patch to a structpb.Value
	patch, err := structpb.NewValue(with)
	if err != nil {
		return false
	}

	return MergeValue(value, patch)
}

// mergeValues merges patch into value (both are already structpb.Value)
func MergeValue(value, patch *structpb.Value) bool {
	// If patch is not an object, replace the entire value
	if patch.GetStructValue() == nil {
		DeepCopy(value, patch)
		return true
	}

	// If target is not an object, replace it with an empty object
	if value.GetStructValue() == nil {
		value.Kind = &structpb.Value_StructValue{
			StructValue: &structpb.Struct{
				Fields: make(map[string]*structpb.Value),
			},
		}
	}

	ts := value.GetStructValue()
	ps := patch.GetStructValue()

	// Apply merge patch
	for key, pv := range ps.Fields {
		if _, ok := pv.Kind.(*structpb.Value_NullValue); ok {
			// null value means remove
			delete(ts.Fields, key)
		} else {
			// Recursively merge
			if _, ok := ts.Fields[key]; !ok {
				ts.Fields[key] = structpb.NewNullValue()
			}
			if !MergeValue(ts.Fields[key], pv) {
				return false
			}
		}
	}
	return true
}
