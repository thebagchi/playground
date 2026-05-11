.PHONY: help build run test clean docker-build docker-push

OPERATOR_NAME := dummy-operator
DOCKER_REGISTRY ?= localhost:5000
DOCKER_IMAGE := $(DOCKER_REGISTRY)/$(OPERATOR_NAME)
DOCKER_TAG ?= latest

help:
	@echo "Available targets:"
	@echo "  build              - Build the operator binary"
	@echo "  run                - Run the operator locally"
	@echo "  test               - Run tests"
	@echo "  clean              - Remove binary and artifacts"
	@echo "  docker-build       - Build Docker image"
	@echo "  docker-push        - Push Docker image to registry"
	@echo "  deploy-crd         - Deploy the CRD to cluster"
	@echo "  deploy-operator    - Deploy the operator to cluster"
	@echo "  example-create     - Create example DummyResource"
	@echo "  example-delete     - Delete example DummyResource"
	@echo "  logs               - Tail operator logs"

build:
	go build -o $(OPERATOR_NAME) .

run: build
	./$(OPERATOR_NAME) --kubeconfig=$(HOME)/.kube/config

test:
	go test -v ./...

clean:
	rm -f $(OPERATOR_NAME)
	go clean

docker-build: 
	docker build -t $(DOCKER_IMAGE):$(DOCKER_TAG) .
	docker tag $(DOCKER_IMAGE):$(DOCKER_TAG) $(DOCKER_IMAGE):latest

docker-push: docker-build
	docker push $(DOCKER_IMAGE):$(DOCKER_TAG)
	docker push $(DOCKER_IMAGE):latest

deploy-crd:
	kubectl apply -f artifacts/examples/crd.yaml

deploy-operator: docker-build
	kubectl apply -f artifacts/deploy/

example-create: deploy-crd
	kubectl apply -f artifacts/examples/example-dummy.yaml

example-delete:
	kubectl delete -f artifacts/examples/example-dummy.yaml || true

logs:
	kubectl logs -f -l app=$(OPERATOR_NAME)

.DEFAULT_GOAL := help
