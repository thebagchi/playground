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
	"flag"
	"fmt"
	"os"
	"path/filepath"

	"k8s.io/client-go/kubernetes"
	"k8s.io/client-go/tools/clientcmd"
	"k8s.io/klog/v2"
)

var (
	masterURL  string
	kubeconfig string
	workers    int
)

func main() {
	klog.InitFlags(nil)
	flag.StringVar(&kubeconfig, "kubeconfig", "", "Path to a kubeconfig. Only required if out-of-cluster.")
	flag.StringVar(&masterURL, "master", "", "The address of the Kubernetes API server. Overrides any value in kubeconfig. Only required if out-of-cluster.")
	flag.IntVar(&workers, "workers", 2, "Number of worker threads.")
	flag.Parse()

	if len(os.Args) > 1 && os.Args[1] == "--help" {
		fmt.Fprintf(os.Stderr, "Usage: dummy-operator [flags]\n")
		flag.PrintDefaults()
		os.Exit(0)
	}

	klog.V(2).InfoS("Starting Dummy Operator", "version", "v1.0.0")

	// Set up signals so we handle the first shutdown signal gracefully
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	// Create kubeconfig if not provided (try to use in-cluster config)
	if kubeconfig == "" {
		// Try to use in-cluster config
		home, err := os.UserHomeDir()
		if err != nil {
			klog.V(4).InfoS("Could not get home directory", "error", err)
		} else {
			kubeconfig = filepath.Join(home, ".kube", "config")
		}
	}

	// Build config from kubeconfig
	cfg, err := clientcmd.BuildConfigFromFlags(masterURL, kubeconfig)
	if err != nil {
		klog.ErrorS(err, "Error building kubeconfig")
		os.Exit(1)
	}

	// Create kubernetes clientset
	kubeClientset, err := kubernetes.NewForConfig(cfg)
	if err != nil {
		klog.ErrorS(err, "Error creating kubernetes client")
		os.Exit(1)
	}

	// Create controller
	controller := NewDummyController(kubeClientset)

	// Run the controller
	klog.Info("Starting controller workers")
	if err := controller.Run(ctx, workers); err != nil {
		klog.ErrorS(err, "Error running controller")
		os.Exit(1)
	}
}
