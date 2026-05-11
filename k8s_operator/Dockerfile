# Build stage
FROM golang:1.21-alpine AS builder

WORKDIR /app

# Copy go mod files
COPY go.mod go.sum ./

# Download dependencies
RUN go mod download

# Copy source code
COPY . .

# Build the operator
RUN CGO_ENABLED=0 GOOS=linux go build -a -installsuffix cgo -o dummy-operator .

# Final stage
FROM alpine:latest

RUN apk --no-cache add ca-certificates

WORKDIR /root/

# Copy binary from builder
COPY --from=builder /app/dummy-operator .

ENTRYPOINT ["./dummy-operator"]
