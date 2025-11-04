package bitbuffer

import (
	"fmt"
	"unsafe"
)

var hexes [256]byte
var hexDigits = "0123456789abcdef"

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
		v := (*uint64)(unsafe.Add(unsafe.Pointer(src), i))

		h0 := hexes[uint8(*v>>0x00)]
		l0 := hexes[uint8(*v>>0x08)]
		h1 := hexes[uint8(*v>>0x10)]
		l1 := hexes[uint8(*v>>0x18)]
		h2 := hexes[uint8(*v>>0x20)]
		l2 := hexes[uint8(*v>>0x28)]
		h3 := hexes[uint8(*v>>0x30)]
		l3 := hexes[uint8(*v>>0x38)]

		if h0|l0|h1|l1|h2|l2|h3|l3 == 0xFF {
			return nil, fmt.Errorf("invalid hex at pos %d", i)
		}

		hi := uint16(h1)<<0x0C | uint16(l1)<<0x08 | uint16(h0)<<0x04 | uint16(l0)
		lo := uint16(h3)<<0x0C | uint16(l3)<<0x08 | uint16(h2)<<0x04 | uint16(l2)

		p := unsafe.Add(unsafe.Pointer(dst), j)
		*(*uint16)(p) = hi
		*(*uint16)(unsafe.Add(p, 2)) = lo

		i += 8
		j += 4
	}

	if n-i >= 4 {
		v := (*uint32)(unsafe.Add(unsafe.Pointer(src), i))
		h0 := hexes[uint8(*v>>0x00)]
		l0 := hexes[uint8(*v>>0x08)]
		h1 := hexes[uint8(*v>>0x10)]
		l1 := hexes[uint8(*v>>0x18)]
		if h0|l0|h1|l1 == 0xFF {
			return nil, fmt.Errorf("invalid hex")
		}
		*(*uint16)(unsafe.Add(unsafe.Pointer(dst), j)) = uint16(h1)<<0x0C | uint16(l1)<<0x08 | uint16(h0)<<0x04 | uint16(l0)
		i += 4
		j += 2
	}

	if n-i >= 2 {
		v := (*uint16)(unsafe.Add(unsafe.Pointer(src), i))
		h := hexes[uint8(*v>>0x00)]
		l := hexes[uint8(*v>>0x08)]
		if h == 0xFF || l == 0xFF {
			return nil, fmt.Errorf("invalid hex")
		}
		*(*byte)(unsafe.Add(unsafe.Pointer(dst), j)) = (h << 0x04) | l
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

	for n-i >= 4 {
		v := (*uint32)(unsafe.Add(unsafe.Pointer(src), i))

		h0 := byte(*v >> 0x00)
		l0 := byte(*v >> 0x08)
		h1 := byte(*v >> 0x10)
		l1 := byte(*v >> 0x18)

		var (
			h00 = h0 >> 0x04
			h01 = h0 & 0x0F
			l00 = l0 >> 0x04
			l01 = l0 & 0x0F
			h10 = h1 >> 0x04
			h11 = h1 & 0x0F
			l10 = l1 >> 0x04
			l11 = l1 & 0x0F
		)

		value := uint64(hexDigits[h00]) |
			uint64(hexDigits[h01])<<0x08 |
			uint64(hexDigits[l00])<<0x10 |
			uint64(hexDigits[l01])<<0x18 |
			uint64(hexDigits[h10])<<0x20 |
			uint64(hexDigits[h11])<<0x28 |
			uint64(hexDigits[l10])<<0x30 |
			uint64(hexDigits[l11])<<0x38

		*(*uint64)(unsafe.Add(unsafe.Pointer(dst), j)) = value

		i += 4
		j += 8
	}

	if n-i >= 2 {
		v := (*uint16)(unsafe.Add(unsafe.Pointer(src), i))

		h0 := byte(*v >> 0x00)
		l0 := byte(*v >> 0x08)

		var (
			h00 = h0 >> 0x04
			h01 = h0 & 0x0F
			l00 = l0 >> 0x04
			l01 = l0 & 0x0F
		)

		value := uint32(hexDigits[h00]) |
			uint32(hexDigits[h01])<<0x08 |
			uint32(hexDigits[l00])<<0x10 |
			uint32(hexDigits[l01])<<0x18

		*(*uint32)(unsafe.Add(unsafe.Pointer(dst), j)) = value

		i += 2
		j += 4
	}

	if n-i >= 1 {
		v := *(*byte)(unsafe.Add(unsafe.Pointer(src), i))

		var (
			v0 = v >> 0x04
			v1 = v & 0x0F
		)

		value := uint16(hexDigits[v0]) | uint16(hexDigits[v1])<<0x08

		*(*uint16)(unsafe.Add(unsafe.Pointer(dst), j)) = value
	}

	return unsafe.String(unsafe.SliceData(result), len(result))
}
