package rfc6902

import (
	"testing"

	"google.golang.org/protobuf/types/known/structpb"
)

func TestIsFunctions(t *testing.T) {
	tests := []struct {
		name     string
		testFunc func(t *testing.T)
	}{
		{
			name: "IS_NULL",
			testFunc: func(t *testing.T) {
				value := structpb.NewNullValue()
				if got := IsNull(value); got != true {
					t.Errorf("IsNull(%v) = %v; want %v", value, got, true)
				}
				if got := IsNumber(value); got != false {
					t.Errorf("IsNumber(%v) = %v; want %v", value, got, false)
				}
				if got := IsString(value); got != false {
					t.Errorf("IsString(%v) = %v; want %v", value, got, false)
				}
				if got := IsBool(value); got != false {
					t.Errorf("IsBool(%v) = %v; want %v", value, got, false)
				}
				if got := IsDict(value); got != false {
					t.Errorf("IsDict(%v) = %v; want %v", value, got, false)
				}
				if got := IsList(value); got != false {
					t.Errorf("IsList(%v) = %v; want %v", value, got, false)
				}
			},
		},
		{
			name: "IS_NUMBER",
			testFunc: func(t *testing.T) {
				value := structpb.NewNumberValue(42.0)
				if got := IsNull(value); got != false {
					t.Errorf("IsNull(%v) = %v; want %v", value, got, false)
				}
				if got := IsNumber(value); got != true {
					t.Errorf("IsNumber(%v) = %v; want %v", value, got, true)
				}
				if got := IsString(value); got != false {
					t.Errorf("IsString(%v) = %v; want %v", value, got, false)
				}
				if got := IsBool(value); got != false {
					t.Errorf("IsBool(%v) = %v; want %v", value, got, false)
				}
				if got := IsDict(value); got != false {
					t.Errorf("IsDict(%v) = %v; want %v", value, got, false)
				}
				if got := IsList(value); got != false {
					t.Errorf("IsList(%v) = %v; want %v", value, got, false)
				}
			},
		},
		{
			name: "IS_STRING",
			testFunc: func(t *testing.T) {
				value := structpb.NewStringValue("hello")
				if got := IsNull(value); got != false {
					t.Errorf("IsNull(%v) = %v; want %v", value, got, false)
				}
				if got := IsNumber(value); got != false {
					t.Errorf("IsNumber(%v) = %v; want %v", value, got, false)
				}
				if got := IsString(value); got != true {
					t.Errorf("IsString(%v) = %v; want %v", value, got, true)
				}
				if got := IsBool(value); got != false {
					t.Errorf("IsBool(%v) = %v; want %v", value, got, false)
				}
				if got := IsDict(value); got != false {
					t.Errorf("IsDict(%v) = %v; want %v", value, got, false)
				}
				if got := IsList(value); got != false {
					t.Errorf("IsList(%v) = %v; want %v", value, got, false)
				}
			},
		},
		{
			name: "IS_BOOL",
			testFunc: func(t *testing.T) {
				value := structpb.NewBoolValue(true)
				if got := IsNull(value); got != false {
					t.Errorf("IsNull(%v) = %v; want %v", value, got, false)
				}
				if got := IsNumber(value); got != false {
					t.Errorf("IsNumber(%v) = %v; want %v", value, got, false)
				}
				if got := IsString(value); got != false {
					t.Errorf("IsString(%v) = %v; want %v", value, got, false)
				}
				if got := IsBool(value); got != true {
					t.Errorf("IsBool(%v) = %v; want %v", value, got, true)
				}
				if got := IsDict(value); got != false {
					t.Errorf("IsDict(%v) = %v; want %v", value, got, false)
				}
				if got := IsList(value); got != false {
					t.Errorf("IsList(%v) = %v; want %v", value, got, false)
				}
			},
		},
		{
			name: "IS_DICT",
			testFunc: func(t *testing.T) {
				value := MakeValue("{}")
				if got := IsNull(value); got != false {
					t.Errorf("IsNull(%v) = %v; want %v", value, got, false)
				}
				if got := IsNumber(value); got != false {
					t.Errorf("IsNumber(%v) = %v; want %v", value, got, false)
				}
				if got := IsString(value); got != false {
					t.Errorf("IsString(%v) = %v; want %v", value, got, false)
				}
				if got := IsBool(value); got != false {
					t.Errorf("IsBool(%v) = %v; want %v", value, got, false)
				}
				if got := IsDict(value); got != true {
					t.Errorf("IsDict(%v) = %v; want %v", value, got, true)
				}
				if got := IsList(value); got != false {
					t.Errorf("IsList(%v) = %v; want %v", value, got, false)
				}
			},
		},
		{
			name: "IS_LIST",
			testFunc: func(t *testing.T) {
				value := MakeValue("[]")
				if got := IsNull(value); got != false {
					t.Errorf("IsNull(%v) = %v; want %v", value, got, false)
				}
				if got := IsNumber(value); got != false {
					t.Errorf("IsNumber(%v) = %v; want %v", value, got, false)
				}
				if got := IsString(value); got != false {
					t.Errorf("IsString(%v) = %v; want %v", value, got, false)
				}
				if got := IsBool(value); got != false {
					t.Errorf("IsBool(%v) = %v; want %v", value, got, false)
				}
				if got := IsDict(value); got != false {
					t.Errorf("IsDict(%v) = %v; want %v", value, got, false)
				}
				if got := IsList(value); got != true {
					t.Errorf("IsList(%v) = %v; want %v", value, got, true)
				}
			},
		},
		{
			name: "IS_NIL",
			testFunc: func(t *testing.T) {
				value := (*structpb.Value)(nil)
				if got := IsNull(value); got != false {
					t.Errorf("IsNull(%v) = %v; want %v", value, got, false)
				}
				if got := IsNumber(value); got != false {
					t.Errorf("IsNumber(%v) = %v; want %v", value, got, false)
				}
				if got := IsString(value); got != false {
					t.Errorf("IsString(%v) = %v; want %v", value, got, false)
				}
				if got := IsBool(value); got != false {
					t.Errorf("IsBool(%v) = %v; want %v", value, got, false)
				}
				if got := IsDict(value); got != false {
					t.Errorf("IsDict(%v) = %v; want %v", value, got, false)
				}
				if got := IsList(value); got != false {
					t.Errorf("IsList(%v) = %v; want %v", value, got, false)
				}
			},
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, tt.testFunc)
	}
}
