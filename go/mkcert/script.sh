#!/bin/bash

# Generate a 2048-bit RSA private key for the CA
openssl genrsa -out ca.key 2048

# Generate a self-signed X.509 certificate valid for 1025 days using the CA private key
openssl req -x509 -new -nodes -key ca.key -sha256 -days 1025 -out ca.crt

# Generate a 2048-bit RSA private key for the device
openssl genrsa -out dev.key 2048

# Generate a self-signed X.509 certificate for the device valid for 365 days
openssl req -x509 -new -nodes -key dev.key -sha256 -days 365 -out dev.crt

# Combine the device private key and self-signed certificate into a PEM file
cat dev.key dev.crt > dev.pem

# Extract the public key from the private key and save it to a .pub file
openssl rsa -pubout -in dev.key -out dev.pub

# Generate a certificate signing request (CSR) using the private key
openssl req -new -key dev.key -out dev.csr

# Sign the certificate signing request with the CA to create a signed certificate valid for 525 days
openssl x509 -req -in dev.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out dev_ca.crt -days 525 -sha256

# Concatenate the CA certificate and CA key into a separate PEM file
cat ca.crt ca.key > ca.pem

# Concatenate the CA certificate, CA key, and signed device certificate into a single PEM file
cat ca.crt ca.key dev_ca.crt > dev_ca.pem

# Create a server-ready PEM file for CA-signed certificate (key + cert + CA chain)
cat dev.key dev_ca.crt ca.crt > dev_ca_server.pem

echo "Certificate generation complete. File usage guide:"
echo "--------------------------------------------------"
echo "ca.key: CA private key (keep secure, not for distribution)"
echo "ca.crt: CA certificate (distribute to clients for trust)"
echo "ca.pem: CA cert + key (for CA operations, keep secure)"
echo "dev.key: Device private key (keep secure on device/server)"
echo "dev.pub: Device public key (for verification)"
echo "dev.crt: Self-signed device certificate"
echo "dev.pem: Device key + self-signed cert (for self-signed server)"
echo "dev.csr: Certificate signing request (temporary, can be deleted)"
echo "dev_ca.crt: CA-signed device certificate"
echo "dev_ca.pem: CA cert + CA key + signed cert (for CA management, e.g., signing more certs or CA operations - keep secure)"
echo "dev_ca_server.pem: Server PEM for CA-signed cert (key + signed cert + CA chain)"
echo ""
echo "Usage recommendations:"
echo "- Self-signed server: Use dev.pem"
echo "- CA-signed server: Use dev_ca_server.pem"
echo "- Client trust: Distribute ca.crt"