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

package main

import (
	"context"
	"fmt"
	"time"

	utilruntime "k8s.io/apimachinery/pkg/util/runtime"
	"k8s.io/apimachinery/pkg/util/wait"
	"k8s.io/client-go/kubernetes"
	"k8s.io/client-go/tools/cache"
	"k8s.io/client-go/util/workqueue"
	"k8s.io/klog/v2"
)

// DummyController is a simple controller for DummyResource
type DummyController struct {
	kubeClientset kubernetes.Interface
	indexer       cache.Indexer
	queue         workqueue.RateLimitingInterface
	informer      cache.Controller
}

// NewDummyController creates a new DummyController
func NewDummyController(kubeClientset kubernetes.Interface) *DummyController {
	klog.V(4).Info("Creating event broadcaster")

	controller := &DummyController{
		kubeClientset: kubeClientset,
		queue:         workqueue.NewNamedRateLimitingQueue(workqueue.DefaultControllerRateLimiter(), "dummy"),
	}

	return controller
}

// Run starts the controller
func (c *DummyController) Run(ctx context.Context, workers int) error {
	defer utilruntime.HandleCrash()
	defer c.queue.ShutDown()

	klog.Info("Starting Dummy controller")

	for i := 0; i < workers; i++ {
		go wait.Until(func() { c.runWorker(ctx) }, time.Second, ctx.Done())
	}

	klog.Info("Started workers")
	<-ctx.Done()
	klog.Info("Shutting down workers")

	return nil
}

// runWorker is a long-running function that will continually call the
// processNextWorkItem function in order to read and process a message on the
// workqueue.
func (c *DummyController) runWorker(ctx context.Context) {
	for c.processNextWorkItem() {
	}
}

// processNextWorkItem will read a single work item off the workqueue and
// attempt to process it, by calling the syncHandler.
func (c *DummyController) processNextWorkItem() bool {
	obj, shutdown := c.queue.Get()

	if shutdown {
		return false
	}

	// We call Done here so the workqueue knows we have finished
	// processing this item. We also must remember to call Forget if we
	// do not want this work item being re-queued. For example, we do
	// not call Forget if a transient error is returned, instead the
	// item is put back on the workqueue and attempted again after a
	// back-off period.
	defer c.queue.Done(obj)

	key := obj.(string)
	namespace, name, err := cache.SplitMetaNamespaceKey(key)
	if err != nil {
		klog.ErrorS(err, "error splitting meta namespace key", "key", key)
		return true
	}

	klog.V(4).InfoS("Processing DummyResource", "namespace", namespace, "name", name)

	// Simulate processing the DummyResource
	_ = fmt.Sprintf("Processing DummyResource %s/%s", namespace, name)

	klog.V(4).InfoS("Successfully synced DummyResource", "namespace", namespace, "name", name)
	c.queue.Forget(obj)
	return true
}

// addDummyResource takes a DummyResource resource and converts it into a namespace/name
// string which is then put onto the work queue. This method should *not* be
// passed resources of any type other than DummyResource.
func (c *DummyController) addDummyResource(obj interface{}) {
	key, err := cache.MetaNamespaceKeyFunc(obj)
	if err != nil {
		utilruntime.HandleError(err)
		return
	}
	c.queue.Add(key)
}

// updateDummyResource and other handlers...
func (c *DummyController) updateDummyResource(old interface{}, new interface{}) {
	c.addDummyResource(new)
}

func (c *DummyController) deleteDummyResource(obj interface{}) {
	key, err := cache.DeletionHandlingMetaNamespaceKeyFunc(obj)
	if err != nil {
		utilruntime.HandleError(err)
		return
	}
	c.queue.Add(key)
}
