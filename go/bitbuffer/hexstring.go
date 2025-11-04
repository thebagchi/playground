package bitbuffer

import (
	"fmt"
	"unsafe"
)

var hexes [256]byte

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
