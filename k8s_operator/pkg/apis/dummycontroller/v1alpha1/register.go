/*
Copyright 2024 The Kubernetes Authors.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

package v1alpha1

import (
	metav1 "k8s.io/apimachinery/pkg/apis/meta/v1"
	"k8s.io/apimachinery/pkg/runtime"
	"k8s.io/apimachinery/pkg/runtime/schema"
)

// GroupVersion specifies the API group and version used to route the request
var GroupVersion = SchemeGroupVersion

// Kind takes an unqualified kind and returns back a Group qualified GroupKind
func Kind(kind string) schema.GroupKind {
	return GroupVersion.WithKind(kind).GroupKind()
}

// DeepCopyObject implements runtime.Object for DummyResource
func (m *DummyResource) DeepCopyObject() runtime.Object {
	if m == nil {
		return nil
	}
	out := new(DummyResource)
	m.DeepCopyInto(out)
	return out
}

// DeepCopyObject implements runtime.Object for DummyResourceList
func (m *DummyResourceList) DeepCopyObject() runtime.Object {
	if m == nil {
		return nil
	}
	out := new(DummyResourceList)
	m.DeepCopyInto(out)
	return out
}

func init() {
	// We register our types here. The registration of the
	// generated functions takes place in the generated files.
	SchemeBuilder.Register(addKnownTypes)
}

// addKnownTypes adds types to API group used in tests here. This function is used by the
// scheme to know about all types managed by this API group.
func addKnownTypes(scheme *runtime.Scheme) error {
	scheme.AddKnownTypes(SchemeGroupVersion,
		&DummyResource{},
		&DummyResourceList{},
	)
	metav1.AddToGroupVersion(scheme, SchemeGroupVersion)
	return nil
}
