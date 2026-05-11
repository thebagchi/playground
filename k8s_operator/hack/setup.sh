#!/bin/bash

# Script to quickly set up and deploy the dummy operator

set -e

NAMESPACE=${1:-default}

echo "=== Dummy Operator Setup ==="
echo "Namespace: $NAMESPACE"

# Create namespace if needed
kubectl get namespace "$NAMESPACE" &>/dev/null || kubectl create namespace "$NAMESPACE"

# Deploy CRD
echo "Deploying CustomResourceDefinition..."
kubectl apply -f artifacts/examples/crd.yaml

# Build operator
echo "Building operator..."
go build -o dummy-operator .

echo "=== Setup Complete ==="
echo ""
echo "To run the operator:"
echo "  ./dummy-operator --kubeconfig=$HOME/.kube/config"
echo ""
echo "To create an example resource:"
echo "  kubectl apply -f artifacts/examples/example-dummy.yaml"
echo ""
echo "To list resources:"
echo "  kubectl get dummyresources"
