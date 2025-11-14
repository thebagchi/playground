package bitbuffer

import (
	"fmt"
	"unsafe"
)

var numes [256]byte
var numps [200]byte

func init() {
	for i := range numes {
		numes[i] = 0xFF
	}
	for i, b := range []byte("0123456789") {
		numes[b] = byte(i)
	}

	// Initialize digit pairs lookup table (00-99)
	for i := 0; i < 100; i++ {
		numps[i*2+0] = byte('0' + i/10)
		numps[i*2+1] = byte('0' + i%10)
	}
}

func DecodeNumString(data string) (int64, error) {
	n := len(data)
	if n == 0 {
		return 0, nil
	}
	negative := false
	start := 0
	switch data[0] {
	case '-':
		negative = true
		start = 1
	case '+':
		start = 1
	}

	remaining := len(data) - start
	if remaining == 0 {
		return 0, fmt.Errorf("invalid numeric string")
	}

	var result int64
	src := unsafe.StringData(data)
	i := start

	// Process 8 bytes at a time
	for remaining >= 8 {
		v := *(*uint64)(unsafe.Add(unsafe.Pointer(src), i))

		d0 := numes[byte(v>>0x00)]
		d1 := numes[byte(v>>0x08)]
		d2 := numes[byte(v>>0x10)]
		d3 := numes[byte(v>>0x18)]
		d4 := numes[byte(v>>0x20)]
		d5 := numes[byte(v>>0x28)]
		d6 := numes[byte(v>>0x30)]
		d7 := numes[byte(v>>0x38)]

		if d0|d1|d2|d3|d4|d5|d6|d7 == 0xFF {
			return 0, fmt.Errorf("invalid numeric character at position %d", i)
		}

		result = result*100000000 +
			int64(d0)*10000000 +
			int64(d1)*1000000 +
			int64(d2)*100000 +
			int64(d3)*10000 +
			int64(d4)*1000 +
			int64(d5)*100 +
			int64(d6)*10 +
			int64(d7)

		i = i + 8
		remaining = remaining - 8
	}

	// Process 4 bytes at a time
	if remaining >= 4 {
		v := *(*uint32)(unsafe.Add(unsafe.Pointer(src), i))

		d0 := numes[byte(v>>0x00)]
		d1 := numes[byte(v>>0x08)]
		d2 := numes[byte(v>>0x10)]
		d3 := numes[byte(v>>0x18)]

		if d0|d1|d2|d3 == 0xFF {
			return 0, fmt.Errorf("invalid numeric character at position %d", i)
		}

		result = result*10000 +
			int64(d0)*1000 +
			int64(d1)*100 +
			int64(d2)*10 +
			int64(d3)

		i = i + 4
		remaining = remaining - 4
	}

	// Process 2 bytes at a time
	if remaining >= 2 {
		v := *(*uint16)(unsafe.Add(unsafe.Pointer(src), i))

		d0 := numes[byte(v>>0x00)]
		d1 := numes[byte(v>>0x08)]

		if d0|d1 == 0xFF {
			return 0, fmt.Errorf("invalid numeric character at position %d", i)
		}

		result = result*100 +
			int64(d0)*10 +
			int64(d1)

		i = i + 2
		remaining = remaining - 2
	}

	// Process remaining single byte
	if remaining >= 1 {
		b := *(*byte)(unsafe.Add(unsafe.Pointer(src), i))
		d := numes[b]
		if d == 0xFF {
			return 0, fmt.Errorf("invalid numeric character at position %d", i)
		}
		result = result*10 + int64(d)
	}

	if negative {
		result = -result
	}

	return result, nil
}

func EncodeNumString(data int64) string {
	if data == 0 {
		return "0"
	}

	var (
		neg = data < 0
		abs = uint64(data)
		buf [24]byte
		pos = len(buf) - 1
		dst = unsafe.Pointer(&buf[0])
		lut = unsafe.Pointer(&numps[0])
	)
	if neg {
		abs = uint64(-data)
	}

	for abs >= 10000 {
		rem := abs % 10000
		abs = abs / 10000
		hi := rem / 100
		lo := rem % 100
		a := uint32(*(*uint16)(unsafe.Add(lut, hi*2)))
		b := uint32(*(*uint16)(unsafe.Add(lut, lo*2)))
		*(*uint32)(unsafe.Add(dst, pos-3)) = b<<16 | a
		pos = pos - 4
	}

	if abs >= 1000 {
		rem := abs % 100
		abs = abs / 100
		*(*uint16)(unsafe.Add(dst, pos-1)) = *(*uint16)(unsafe.Add(lut, rem*2))
		pos = pos - 2
	}
	if abs >= 100 {
		rem := abs % 100
		abs = abs / 100
		*(*uint16)(unsafe.Add(dst, pos-1)) = *(*uint16)(unsafe.Add(lut, rem*2))
		pos = pos - 2
	}
	if abs >= 10 {
		rem := abs % 100
		abs = abs / 100
		*(*uint16)(unsafe.Add(dst, pos-1)) = *(*uint16)(unsafe.Add(lut, rem*2))
		pos = pos - 2
	}
	if abs > 0 {
		buf[pos] = byte('0' + abs)
		pos = pos - 1
	}

	if neg {
		buf[pos] = '-'
		pos = pos - 1
	}

	// Return string from the first used position
	return unsafe.String(&buf[pos+1], len(buf)-pos-1)
}
