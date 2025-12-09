package arena

import "math/bits"

// RoundPow2 returns the smallest power of two that is >= n.
func RoundPow2(n uint64) uint64 {
	return uint64(1) << uint64(bits.UintSize-bits.LeadingZeros64(n-1))
}

// Pow2 returns 2^n.
func Pow2(n uint64) uint64 {
	return 1 << n
}

// Log2 returns log2(n).
func Log2(n uint64) uint64 {
	return uint64(bits.Len64(n) - 1)
}
