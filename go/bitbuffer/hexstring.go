package bitbuffer

import (
	"fmt"
	"unsafe"
)

var hexes [256]byte
var hexds = "0123456789abcdef"

func init() {
	for i := range hexes {
		hexes[i] = 0xFF
	}
	for i, b := range []byte("0123456789") {
		hexes[b] = byte(i)
	}
	for i, b := range []byte("ABCDEF") {
		hexes[b] = byte(i + 10)
	}
	for i, b := range []byte("abcdef") {
		hexes[b] = byte(i + 10)
	}
}

func DecodeHexString(data string) ([]byte, error) {
	n := len(data)
	if n%2 != 0 {
		return nil, fmt.Errorf("hex string length must be even")
	}
	if n == 0 {
		return []byte{}, nil
	}

	result := make([]byte, n/2)
	src := unsafe.StringData(data)
	dst := unsafe.SliceData(result)

	i, j := 0, 0

	for n-i >= 8 {
		v := *(*uint64)(unsafe.Add(unsafe.Pointer(src), i))

		v0 := uint8(v >> 0x00)
		v1 := uint8(v >> 0x08)
		v2 := uint8(v >> 0x10)
		v3 := uint8(v >> 0x18)
		v4 := uint8(v >> 0x20)
		v5 := uint8(v >> 0x28)
		v6 := uint8(v >> 0x30)
		v7 := uint8(v >> 0x38)

		h0 := hexes[v0]
		l0 := hexes[v1]
		h1 := hexes[v2]
		l1 := hexes[v3]
		h2 := hexes[v4]
		l2 := hexes[v5]
		h3 := hexes[v6]
		l3 := hexes[v7]

		if h0|l0|h1|l1|h2|l2|h3|l3 == 0xFF {
			return nil, fmt.Errorf("invalid hex character at position %d", i)
		}

		*(*uint32)(unsafe.Add(unsafe.Pointer(dst), j)) = uint32(h3)<<0x1C | uint32(l3)<<0x18 | uint32(h2)<<0x14 | uint32(l2)<<0x10 | uint32(h1)<<0x0C | uint32(l1)<<0x08 | uint32(h0)<<0x04 | uint32(l0)
		i += 8
		j += 4
	}

	if n-i >= 4 {
		v := *(*uint32)(unsafe.Add(unsafe.Pointer(src), i))

		v0 := uint8(v >> 0x00)
		v1 := uint8(v >> 0x08)
		v2 := uint8(v >> 0x10)
		v3 := uint8(v >> 0x18)

		h0 := hexes[v0]
		l0 := hexes[v1]
		h1 := hexes[v2]
		l1 := hexes[v3]

		if h0|l0|h1|l1 == 0xFF {
			return nil, fmt.Errorf("invalid hex character at position %d", i)
		}

		*(*uint16)(unsafe.Add(unsafe.Pointer(dst), j)) = uint16(h1)<<0x0C | uint16(l1)<<0x08 | uint16(h0)<<0x04 | uint16(l0)
		i += 4
		j += 2
	}

	if n-i >= 2 {
		v := *(*uint16)(unsafe.Add(unsafe.Pointer(src), i))
		v0 := uint8(v >> 0x00)
		v1 := uint8(v >> 0x08)
		h0 := hexes[v0]
		l0 := hexes[v1]
		if h0|l0 == 0xFF {
			return nil, fmt.Errorf("invalid hex character at position %d", i)
		}
		*(*byte)(unsafe.Add(unsafe.Pointer(dst), j)) = (h0 << 0x04) | l0
	}

	return result, nil
}

func EncodeHexString(data []byte) string {
	n := len(data)
	if n == 0 {
		return ""
	}

	result := make([]byte, n*2)
	src := unsafe.SliceData(data)
	dst := unsafe.SliceData(result)
	i, j := 0, 0

	for n-i >= 8 {
		v := (*uint64)(unsafe.Add(unsafe.Pointer(src), i))

		hi := *v & 0x0F0F0F0F0F0F0F0F
		lo := (*v >> 4) & 0x0F0F0F0F0F0F0F0F

		var (
			hi0 = hexds[byte(lo>>0x00)]
			lo0 = hexds[byte(hi>>0x00)]
			hi1 = hexds[byte(lo>>0x08)]
			lo1 = hexds[byte(hi>>0x08)]
			hi2 = hexds[byte(lo>>0x10)]
			lo2 = hexds[byte(hi>>0x10)]
			hi3 = hexds[byte(lo>>0x18)]
			lo3 = hexds[byte(hi>>0x18)]
			hi4 = hexds[byte(lo>>0x20)]
			lo4 = hexds[byte(hi>>0x20)]
			hi5 = hexds[byte(lo>>0x28)]
			lo5 = hexds[byte(hi>>0x28)]
			hi6 = hexds[byte(lo>>0x30)]
			lo6 = hexds[byte(hi>>0x30)]
			hi7 = hexds[byte(lo>>0x38)]
			lo7 = hexds[byte(hi>>0x38)]
		)

		hi = uint64(hi0) |
			uint64(lo0)<<0x08 |
			uint64(hi1)<<0x10 |
			uint64(lo1)<<0x18 |
			uint64(hi2)<<0x20 |
			uint64(lo2)<<0x28 |
			uint64(hi3)<<0x30 |
			uint64(lo3)<<0x38

		lo = uint64(hi4) |
			uint64(lo4)<<0x08 |
			uint64(hi5)<<0x10 |
			uint64(lo5)<<0x18 |
			uint64(hi6)<<0x20 |
			uint64(lo6)<<0x28 |
			uint64(hi7)<<0x30 |
			uint64(lo7)<<0x38

		*(*uint64)(unsafe.Add(unsafe.Pointer(dst), j)) = hi
		*(*uint64)(unsafe.Add(unsafe.Pointer(dst), j+8)) = lo

		i += 8
		j += 16
	}

	for n-i >= 4 {
		v := (*uint32)(unsafe.Add(unsafe.Pointer(src), i))

		hi := *v & 0x0F0F0F0F
		lo := (*v >> 4) & 0x0F0F0F0F

		var (
			hi0 = hexds[byte(lo>>0x00)]
			lo0 = hexds[byte(hi>>0x00)]
			hi1 = hexds[byte(lo>>0x08)]
			lo1 = hexds[byte(hi>>0x08)]
			hi2 = hexds[byte(lo>>0x10)]
			lo2 = hexds[byte(hi>>0x10)]
			hi3 = hexds[byte(lo>>0x18)]
			lo3 = hexds[byte(hi>>0x18)]
		)

		hi = uint32(hi0) |
			uint32(lo0)<<0x08 |
			uint32(hi1)<<0x10 |
			uint32(lo1)<<0x18

		lo = uint32(hi2) |
			uint32(lo2)<<0x08 |
			uint32(hi3)<<0x10 |
			uint32(lo3)<<0x18

		*(*uint32)(unsafe.Add(unsafe.Pointer(dst), j)) = hi
		*(*uint32)(unsafe.Add(unsafe.Pointer(dst), j+4)) = lo

		i += 4
		j += 8
	}

	if n-i >= 2 {
		v := (*uint16)(unsafe.Add(unsafe.Pointer(src), i))

		hi := uint16(*v & 0x0F0F)
		lo := uint16((*v >> 4) & 0x0F0F)

		var (
			hi0 = hexds[byte(lo>>0x00)]
			lo0 = hexds[byte(hi>>0x00)]
			hi1 = hexds[byte(lo>>0x08)]
			lo1 = hexds[byte(hi>>0x08)]
		)

		hi = uint16(hi0) |
			uint16(lo0)<<0x08

		lo = uint16(hi1) |
			uint16(lo1)<<0x08

		*(*uint16)(unsafe.Add(unsafe.Pointer(dst), j)) = hi
		*(*uint16)(unsafe.Add(unsafe.Pointer(dst), j+2)) = lo

		i += 2
		j += 4
	}

	if n-i >= 1 {
		v := *(*byte)(unsafe.Add(unsafe.Pointer(src), i))

		hi := uint8(v & 0x0F)
		lo := uint8((v >> 4) & 0x0F)

		var (
			hi0 = hexds[lo]
			lo0 = hexds[hi]
		)

		hi = uint8(hi0)
		lo = uint8(lo0)

		*(*uint8)(unsafe.Add(unsafe.Pointer(dst), j)) = hi
		*(*uint8)(unsafe.Add(unsafe.Pointer(dst), j+1)) = lo
	}

	return unsafe.String(unsafe.SliceData(result), len(result))
}
