package mkcert

import (
	"bytes"
	"crypto/ecdsa"
	"crypto/rand"
	"crypto/rsa"
	"crypto/tls"
	"crypto/x509"
	"crypto/x509/pkix"
	"encoding/pem"
	"errors"
	"math/big"
	"os"
	"time"
)

// CertOptions holds options for certificate generation
type CertOptions struct {
	Organization       string
	OrganizationalUnit string
	Country            string
	Province           string
	Locality           string
	PostalCode         string
	EmailAddress       string
	NumDays            int
}

// Option is a functional option for configuring certificate generation
type Option func(*CertOptions)

// WithOrganization sets the organization name for the certificate
func WithOrganization(org string) Option {
	return func(opts *CertOptions) {
		opts.Organization = org
	}
}

// WithOrganizationalUnit sets the organizational unit for the certificate
func WithOrganizationalUnit(ou string) Option {
	return func(opts *CertOptions) {
		opts.OrganizationalUnit = ou
	}
}

// WithCountry sets the country for the certificate
func WithCountry(country string) Option {
	return func(opts *CertOptions) {
		opts.Country = country
	}
}

// WithProvince sets the province/state for the certificate
func WithProvince(province string) Option {
	return func(opts *CertOptions) {
		opts.Province = province
	}
}

// WithLocality sets the locality/city for the certificate
func WithLocality(locality string) Option {
	return func(opts *CertOptions) {
		opts.Locality = locality
	}
}

// WithPostalCode sets the postal code for the certificate
func WithPostalCode(postalCode string) Option {
	return func(opts *CertOptions) {
		opts.PostalCode = postalCode
	}
}

// WithEmailAddress sets the email address for the certificate
func WithEmailAddress(email string) Option {
	return func(opts *CertOptions) {
		opts.EmailAddress = email
	}
}

// WithNumDays sets the number of days the certificate is valid for
func WithNumDays(days int) Option {
	return func(opts *CertOptions) {
		opts.NumDays = days
	}
}

// MakeCertificate generates a self-signed certificate for the given host
//
// Example usage with combined options:
//
//	cert, key, err := MakeCertificate("example.com",
//	    WithOrganization("Wayne Enterprises"),
//	    WithOrganizationalUnit("R&D Department"),
//	    WithCountry("US"),
//	    WithProvince("New Jersey"),
//	    WithLocality("Gotham City"),
//	    WithPostalCode("12345"),
//	    WithEmailAddress("bruce.wayne@wayneenterprises.com"),
//	    WithNumDays(365))
func MakeCertificate(host string, opts ...Option) (certPEM, keyPEM []byte, err error) {
	// Wayne Enterprises is an iconic fictional company from DC Comics, owned by Bruce Wayne (Batman)
	options := &CertOptions{
		Organization: "Wayne Enterprises",
		NumDays:      365,
	}
	for _, opt := range opts {
		opt(options)
	}

	// Generate private key
	privateKey, err := rsa.GenerateKey(rand.Reader, 2048)
	if err != nil {
		return nil, nil, err
	}

	// Create certificate template
	template := x509.Certificate{
		SerialNumber: big.NewInt(1),
		Subject: pkix.Name{
			Organization:       []string{options.Organization},
			OrganizationalUnit: []string{options.OrganizationalUnit},
			Country:            []string{options.Country},
			Province:           []string{options.Province},
			Locality:           []string{options.Locality},
			PostalCode:         []string{options.PostalCode},
			CommonName:         host,
		},
		DNSNames:              []string{host},
		EmailAddresses:        []string{options.EmailAddress},
		NotBefore:             time.Now(),
		NotAfter:              time.Now().Add(time.Duration(options.NumDays) * 24 * time.Hour),
		KeyUsage:              x509.KeyUsageKeyEncipherment | x509.KeyUsageDigitalSignature,
		ExtKeyUsage:           []x509.ExtKeyUsage{x509.ExtKeyUsageServerAuth},
		BasicConstraintsValid: true,
	}

	// Create certificate
	certDER, err := x509.CreateCertificate(rand.Reader, &template, &template, &privateKey.PublicKey, privateKey)
	if err != nil {
		return nil, nil, err
	}

	// Encode certificate to PEM
	certPEM = pem.EncodeToMemory(&pem.Block{
		Type:  "CERTIFICATE",
		Bytes: certDER,
	})

	// Encode private key to PEM
	keyDER := x509.MarshalPKCS1PrivateKey(privateKey)
	keyPEM = pem.EncodeToMemory(&pem.Block{
		Type:  "RSA PRIVATE KEY",
		Bytes: keyDER,
	})

	return certPEM, keyPEM, nil
}

// MakeTLSCertificate generates a self-signed certificate and returns it as a tls.Certificate
func MakeTLSCertificate(host string, opts ...Option) ([]tls.Certificate, error) {
	certPEM, keyPEM, err := MakeCertificate(host, opts...)
	if err != nil {
		return nil, err
	}

	cert, err := tls.X509KeyPair(certPEM, keyPEM)
	if err != nil {
		return nil, err
	}

	return []tls.Certificate{cert}, nil
}

// MakePEMFile writes the certificate and private key from tls.Certificate to a single PEM file
func MakePEMFile(filename string, certs []tls.Certificate) error {
	if len(certs) == 0 {
		return errors.New("no certificates provided")
	}

	// Encode all certificates and private keys to PEM
	var buff bytes.Buffer
	for _, cert := range certs {
		// Encode certificate chain to PEM
		for _, data := range cert.Certificate {
			block := &pem.Block{
				Type:  "CERTIFICATE",
				Bytes: data,
			}
			buff.Write(pem.EncodeToMemory(block))
		}

		// Encode private key to PEM
		switch key := cert.PrivateKey.(type) {
		case *rsa.PrivateKey:
			data := x509.MarshalPKCS1PrivateKey(key)
			block := &pem.Block{
				Type:  "RSA PRIVATE KEY",
				Bytes: data,
			}
			buff.Write(pem.EncodeToMemory(block))
		case *ecdsa.PrivateKey:
			data, err := x509.MarshalECPrivateKey(key)
			if err != nil {
				return errors.New("failed to marshal ECDSA private key: " + err.Error())
			}
			block := &pem.Block{
				Type:  "EC PRIVATE KEY",
				Bytes: data,
			}
			buff.Write(pem.EncodeToMemory(block))
		default:
			return errors.New("unsupported private key type")
		}
	}

	// Write combined certificate and key to single PEM file
	if err := os.WriteFile(filename, buff.Bytes(), 0600); err != nil {
		return err
	}

	return nil
}

// ReadPEMFile reads a PEM file containing certificate and private key and returns []tls.Certificate
func ReadPEMFile(filename string) ([]tls.Certificate, error) {
	// Read the PEM file
	data, err := os.ReadFile(filename)
	if err != nil {
		return nil, errors.New("failed to read PEM file: " + err.Error())
	}

	var (
		certBlocks []byte
		keyBlocks  []byte
	)
	// Parse all PEM blocks
	for len(data) > 0 {
		block, rest := pem.Decode(data)
		if block == nil {
			break
		}
		data = rest

		switch block.Type {
		case "CERTIFICATE":
			// Collect certificate blocks
			certBlocks = append(certBlocks, pem.EncodeToMemory(block)...)
		case "RSA PRIVATE KEY", "EC PRIVATE KEY":
			// Store the private key block
			keyBlocks = append(keyBlocks, pem.EncodeToMemory(block)...)
		}
	}

	if len(certBlocks) == 0 {
		return nil, errors.New("no certificate found in PEM file")
	}

	if len(keyBlocks) == 0 {
		return nil, errors.New("no private key found in PEM file")
	}

	// Create tls.Certificate from the PEM data
	cert, err := tls.X509KeyPair(certBlocks, keyBlocks)
	if err != nil {
		return nil, errors.New("failed to create certificate from PEM data: " + err.Error())
	}

	return []tls.Certificate{cert}, nil
}
