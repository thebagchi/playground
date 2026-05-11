# Dummy Kubernetes Operator

This is a simple, minimal Kubernetes operator for demonstration purposes. It is based on the [kubernetes/sample-controller](https://github.com/kubernetes/sample-controller) reference implementation.

## Overview

The Dummy Operator demonstrates the basic components needed to build a Kubernetes controller:

- **Custom Resource Definition (CRD)**: Defines a new resource type called `DummyResource`
- **Controller Logic**: Watches for changes to `DummyResource` instances and performs reconciliation
- **Client-go Integration**: Uses the Kubernetes client-go library for API interaction

## Project Structure

```
.
├── main.go                          # Entry point for the operator
├── controller.go                    # Main controller logic
├── go.mod                           # Go module definition
├── pkg/
│   └── apis/
│       └── dummycontroller/
│           └── v1alpha1/
│               ├── types.go         # API type definitions
│               ├── groupversion.go  # API group version information
│               └── register.go      # Type registration
└── artifacts/
    └── examples/
        ├── crd.yaml                 # CustomResourceDefinition manifest
        └── example-dummy.yaml       # Example DummyResource instance
```

## Components

### DummyResource Custom Resource

The `DummyResource` is a simple custom resource with the following properties:

**Spec:**
- `replicas`: Number of replicas (1-10)
- `message`: A dummy message field
- `image`: Container image to use

**Status:**
- `observedGeneration`: Generation of the most recently observed resource
- `phase`: Current phase of the resource
- `message`: Status message
- `readyReplicas`: Number of ready replicas

### Controller

The `DummyController` implements the core reconciliation logic:
- Watches for `DummyResource` create/update/delete events
- Processes events from a work queue
- Performs the desired state reconciliation (simplified for demonstration)

## Prerequisites

- Go 1.21 or later
- kubectl
- A working Kubernetes cluster (or Minikube/Docker Desktop for local testing)
- kubeconfig properly configured

## Building

```bash
go build -o dummy-operator .
```

## Running Locally

### 1. Create the Custom Resource Definition

```bash
kubectl apply -f artifacts/examples/crd.yaml
```

### 2. Run the operator

```bash
# If using kubeconfig at default location
./dummy-operator

# Or specify custom kubeconfig
./dummy-operator --kubeconfig=$HOME/.kube/config

# Specify number of worker threads
./dummy-operator --workers=4
```

### 3. Create a DummyResource instance

```bash
kubectl apply -f artifacts/examples/example-dummy.yaml
```

### 4. Verify the resource was created

```bash
kubectl get dummyresources
kubectl get dummy  # using shortName
kubectl describe dummyresource example-dummy
```

### 5. Check the operator logs

```bash
# The operator will log to stdout/stderr
# Look for messages about processing the DummyResource
```

## Extending the Operator

To extend this dummy operator:

1. **Add more fields to DummyResourceSpec/Status** in `pkg/apis/dummycontroller/v1alpha1/types.go`

2. **Implement reconciliation logic** in the `processNextWorkItem()` function in `controller.go`

3. **Add event handlers** in the controller for more complex use cases

4. **Add RBAC permissions** via ClusterRole and ClusterRoleBinding manifests

5. **Containerize** the operator using a Dockerfile for deployment to a cluster

## Common Operations

### List all DummyResources

```bash
kubectl get dummyresources
kubectl get dummy -A  # across all namespaces
```

### Get detailed information

```bash
kubectl describe dummyresource example-dummy
```

### Edit a DummyResource

```bash
kubectl edit dummyresource example-dummy
```

### Delete a DummyResource

```bash
kubectl delete dummyresource example-dummy
```

### Watch for changes

```bash
kubectl get dummyresources --watch
```

## Cleanup

Remove the CRD (this will also delete all DummyResource instances):

```bash
kubectl delete crd dummyresources.dummycontroller.k8s.io
```

## Related Resources

- [Kubernetes Custom Resources](https://kubernetes.io/docs/concepts/extend-kubernetes/api-extension/custom-resources/)
- [CustomResourceDefinitions](https://kubernetes.io/docs/tasks/extend-kubernetes/custom-resources/custom-resource-definitions/)
- [Kubernetes Client Libraries](https://kubernetes.io/docs/reference/using-api/client-libraries/)
- [sample-controller Reference](https://github.com/kubernetes/sample-controller)

## License

Apache License 2.0 - See LICENSE file for details
