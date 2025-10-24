package mkcert

import (
	"crypto/x509"
	"encoding/pem"
	"os"
	"strconv"
	"strings"
	"testing"
	"time"
)

const (
	CERT_PEM_HEADER = "-----BEGIN CERTIFICATE-----"
	KEY_PEM_HEADER  = "-----BEGIN RSA PRIVATE KEY-----"
)

// tempname creates a unique temporary filename with timestamp
func tempname(base, extn string) string {
	var buf strings.Builder
	buf.WriteString("/tmp/")
	buf.WriteString(base)
	buf.WriteString("_")
	buf.WriteString(time.Now().Format("20060102_150405_000"))
	buf.WriteString(extn)
	return buf.String()
}

func TestGenerateSelfSignedCert(t *testing.T) {
	host := "wayneenterprises.com"
	cert, key, err := MakeCertificate(host)
	if err != nil {
		t.Fatal("Failed to generate certificate:", err)
	}

	if len(cert) == 0 {
		t.Error("Certificate PEM is empty")
	}

	if len(key) == 0 {
		t.Error("Key PEM is empty")
	}

	// Basic validation that PEM blocks are properly formatted
	if !strings.HasPrefix(string(cert), CERT_PEM_HEADER) {
		t.Error("Certificate PEM does not start with correct header")
	}

	if !strings.HasPrefix(string(key), KEY_PEM_HEADER) {
		t.Error("Key PEM does not start with correct header")
	}
}

func TestGenerateSelfSignedTLSCert(t *testing.T) {
	host := "wayneenterprises.com"
	certs, err := MakeTLSCertificate(host)
	if err != nil {
		t.Fatal("Failed to generate TLS certificate:", err)
	}

	if len(certs) == 0 {
		t.Error("Certificate slice is empty")
	}

	// Verify the certificate can be used for TLS
	if len(certs[0].Certificate) == 0 {
		t.Error("TLS certificate has no certificate chain")
	}

	if certs[0].PrivateKey == nil {
		t.Error("TLS certificate has no private key")
	}
}

func TestGenerateSelfSignedCertWithOrganization(t *testing.T) {
	var (
		host = "wayneenterprises.com"
		org  = "Wayne Enterprises"
	)
	cert, key, err := MakeCertificate(host, WithOrganization(org))
	if err != nil {
		t.Fatal("Failed to generate certificate with organization:", err)
	}

	if len(cert) == 0 {
		t.Error("Certificate PEM is empty")
	}

	if len(key) == 0 {
		t.Error("Key PEM is empty")
	}

	// Parse the certificate to verify the organization
	block, _ := pem.Decode(cert)
	if block == nil {
		t.Fatal("Failed to decode certificate PEM")
	}

	certificate, err := x509.ParseCertificate(block.Bytes)
	if err != nil {
		t.Fatal("Failed to parse certificate:", err)
	}

	if len(certificate.Subject.Organization) == 0 || certificate.Subject.Organization[0] != org {
		t.Error("Expected organization", org, "got", certificate.Subject.Organization)
	}
}

func TestGenerateSelfSignedCertWithCountry(t *testing.T) {
	var (
		host    = "wayneenterprises.com"
		country = "US"
	)
	cert, key, err := MakeCertificate(host, WithCountry(country))
	if err != nil {
		t.Fatal("Failed to generate certificate with country:", err)
	}

	if len(cert) == 0 {
		t.Error("Certificate PEM is empty")
	}

	if len(key) == 0 {
		t.Error("Key PEM is empty")
	}

	// Parse the certificate to verify the country
	block, _ := pem.Decode(cert)
	if block == nil {
		t.Fatal("Failed to decode certificate PEM")
	}

	certificate, err := x509.ParseCertificate(block.Bytes)
	if err != nil {
		t.Fatal("Failed to parse certificate:", err)
	}

	if len(certificate.Subject.Country) == 0 || certificate.Subject.Country[0] != country {
		t.Error("Expected country", country, "got", certificate.Subject.Country)
	}
}

func TestGenerateSelfSignedCertWithNumDays(t *testing.T) {
	var (
		host    = "wayneenterprises.com"
		numDays = 30
	)
	cert, key, err := MakeCertificate(host, WithNumDays(numDays))
	if err != nil {
		t.Fatal("Failed to generate certificate with custom validity:", err)
	}

	if len(cert) == 0 {
		t.Error("Certificate PEM is empty")
	}

	if len(key) == 0 {
		t.Error("Key PEM is empty")
	}

	// Parse the certificate to verify the validity period
	block, _ := pem.Decode(cert)
	if block == nil {
		t.Fatal("Failed to decode certificate PEM")
	}

	certificate, err := x509.ParseCertificate(block.Bytes)
	if err != nil {
		t.Fatal("Failed to parse certificate:", err)
	}

	expiry := time.Now().Add(time.Duration(numDays) * 24 * time.Hour)
	// Allow for small time differences in test execution
	diff := certificate.NotAfter.Sub(expiry)
	if diff < -time.Minute || diff > time.Minute {
		t.Error("Expected expiry around", expiry, "got", certificate.NotAfter)
	}
}

func TestGenerateSelfSignedCertWithProvince(t *testing.T) {
	var (
		host     = "wayneenterprises.com"
		province = "California"
	)
	cert, key, err := MakeCertificate(host, WithProvince(province))
	if err != nil {
		t.Fatal("Failed to generate certificate with province:", err)
	}

	if len(cert) == 0 {
		t.Error("Certificate PEM is empty")
	}

	if len(key) == 0 {
		t.Error("Key PEM is empty")
	}

	// Parse the certificate to verify the province
	block, _ := pem.Decode(cert)
	if block == nil {
		t.Fatal("Failed to decode certificate PEM")
	}

	certificate, err := x509.ParseCertificate(block.Bytes)
	if err != nil {
		t.Fatal("Failed to parse certificate:", err)
	}

	if len(certificate.Subject.Province) == 0 || certificate.Subject.Province[0] != province {
		t.Error("Expected province", province, "got", certificate.Subject.Province)
	}
}

func TestGenerateSelfSignedCertWithLocality(t *testing.T) {
	var (
		host     = "wayneenterprises.com"
		locality = "Gotham City"
	)
	cert, key, err := MakeCertificate(host, WithLocality(locality))
	if err != nil {
		t.Fatal("Failed to generate certificate with locality:", err)
	}

	if len(cert) == 0 {
		t.Error("Certificate PEM is empty")
	}

	if len(key) == 0 {
		t.Error("Key PEM is empty")
	}

	// Parse the certificate to verify the locality
	block, _ := pem.Decode(cert)
	if block == nil {
		t.Fatal("Failed to decode certificate PEM")
	}

	certificate, err := x509.ParseCertificate(block.Bytes)
	if err != nil {
		t.Fatal("Failed to parse certificate:", err)
	}

	if len(certificate.Subject.Locality) == 0 || certificate.Subject.Locality[0] != locality {
		t.Error("Expected locality", locality, "got", certificate.Subject.Locality)
	}
}

func TestGenerateSelfSignedCertWithEmailAddress(t *testing.T) {
	var (
		host  = "wayneenterprises.com"
		email = "bruce.wayne@wayneenterprises.com"
	)
	cert, key, err := MakeCertificate(host, WithEmailAddress(email))
	if err != nil {
		t.Fatal("Failed to generate certificate with email address:", err)
	}

	if len(cert) == 0 {
		t.Error("Certificate PEM is empty")
	}

	if len(key) == 0 {
		t.Error("Key PEM is empty")
	}

	// Parse the certificate to verify the email address
	block, _ := pem.Decode(cert)
	if block == nil {
		t.Fatal("Failed to decode certificate PEM")
	}

	certificate, err := x509.ParseCertificate(block.Bytes)
	if err != nil {
		t.Fatal("Failed to parse certificate:", err)
	}

	if len(certificate.EmailAddresses) == 0 || certificate.EmailAddresses[0] != email {
		t.Error("Expected email address", email, "got", certificate.EmailAddresses)
	}
}

func TestGenerateSelfSignedCertWithOrganizationalUnit(t *testing.T) {
	var (
		host = "wayneenterprises.com"
		ou   = "R&D Department"
	)
	cert, key, err := MakeCertificate(host, WithOrganizationalUnit(ou))
	if err != nil {
		t.Fatal("Failed to generate certificate with organizational unit:", err)
	}

	if len(cert) == 0 {
		t.Error("Certificate PEM is empty")
	}

	if len(key) == 0 {
		t.Error("Key PEM is empty")
	}

	// Parse the certificate to verify the organizational unit
	block, _ := pem.Decode(cert)
	if block == nil {
		t.Fatal("Failed to decode certificate PEM")
	}

	certificate, err := x509.ParseCertificate(block.Bytes)
	if err != nil {
		t.Fatal("Failed to parse certificate:", err)
	}

	if len(certificate.Subject.OrganizationalUnit) == 0 || certificate.Subject.OrganizationalUnit[0] != ou {
		t.Error("Expected organizational unit", ou, "got", certificate.Subject.OrganizationalUnit)
	}
}

func TestGenerateSelfSignedCertWithPostalCode(t *testing.T) {
	var (
		host       = "wayneenterprises.com"
		postalCode = "12345"
	)
	cert, key, err := MakeCertificate(host, WithPostalCode(postalCode))
	if err != nil {
		t.Fatal("Failed to generate certificate with postal code:", err)
	}

	if len(cert) == 0 {
		t.Error("Certificate PEM is empty")
	}

	if len(key) == 0 {
		t.Error("Key PEM is empty")
	}

	// Parse the certificate to verify the postal code
	block, _ := pem.Decode(cert)
	if block == nil {
		t.Fatal("Failed to decode certificate PEM")
	}

	certificate, err := x509.ParseCertificate(block.Bytes)
	if err != nil {
		t.Fatal("Failed to parse certificate:", err)
	}

	if len(certificate.Subject.PostalCode) == 0 || certificate.Subject.PostalCode[0] != postalCode {
		t.Error("Expected postal code", postalCode, "got", certificate.Subject.PostalCode)
	}
}

func TestMakePEMFile(t *testing.T) {
	host := "wayneenterprises.com"
	certs, err := MakeTLSCertificate(host)
	if err != nil {
		t.Fatal("Failed to generate TLS certificate:", err)
	}

	filename := tempname("TestMakePEMFile", ".pem")
	defer os.Remove(filename) // Clean up

	// Write certificate to file
	err = MakePEMFile(filename, certs)
	if err != nil {
		t.Fatal("Failed to write certificate to file:", err)
	}

	// Verify PEM file exists and has content
	content, err := os.ReadFile(filename)
	if err != nil {
		t.Fatal("Failed to read PEM file:", err)
	}

	t.Log("length:", filename, "<"+strconv.Itoa(len(content))+"> bytes")

	if len(content) == 0 {
		t.Error("PEM file is empty")
	}

	// Verify the file contains both certificate and private key
	var certFound, keyFound bool
	for len(content) > 0 {
		var block *pem.Block
		block, content = pem.Decode(content)
		if block == nil {
			break
		}
		switch block.Type {
		case "CERTIFICATE":
			certFound = true
		case "RSA PRIVATE KEY":
			keyFound = true
		}
	}

	if !certFound {
		t.Error("PEM file does not contain certificate")
	}
	if !keyFound {
		t.Error("PEM file does not contain private key")
	}
}

func TestReadPEMFile(t *testing.T) {
	host := "wayneenterprises.com"
	original, err := MakeTLSCertificate(host)
	if err != nil {
		t.Fatal("Failed to generate TLS certificate:", err)
	}

	filename := tempname("TestReadPEMFile", ".pem")
	defer os.Remove(filename) // Clean up

	// Write certificate to file
	err = MakePEMFile(filename, original)
	if err != nil {
		t.Fatal("Failed to write certificate to file:", err)
	}

	// Read file to check length
	content, err := os.ReadFile(filename)
	if err != nil {
		t.Fatal("Failed to read file for length check:", err)
	}
	t.Log("length:", filename, "<"+strconv.Itoa(len(content))+"> bytes")

	// Read certificate back from file
	read, err := ReadPEMFile(filename)
	if err != nil {
		t.Fatal("Failed to read certificate from file:", err)
	}

	if len(read) != 1 {
		t.Error("Expected 1 certificate, got", len(read))
	}

	// Verify the read certificate matches the original
	var (
		oc = original[0]
		rc = read[0]
	)

	// Compare certificate chains
	if len(oc.Certificate) != len(rc.Certificate) {
		t.Error("Certificate chain length mismatch: original", len(oc.Certificate), "read", len(rc.Certificate))
	}

	// Compare private keys (by comparing their string representation)
	if oc.PrivateKey == nil {
		t.Error("Original certificate has no private key")
	}
	if rc.PrivateKey == nil {
		t.Error("Read certificate has no private key")
	}

	// Verify the certificate can be parsed
	if len(rc.Certificate) > 0 {
		certificate, err := x509.ParseCertificate(rc.Certificate[0])
		if err != nil {
			t.Error("Failed to parse read certificate:", err)
		} else if certificate.Subject.CommonName != host {
			t.Error("Certificate common name mismatch: expected", host, "got", certificate.Subject.CommonName)
		}
	}
}
