package bitbuffer

import (
	"fmt"
	"math/rand"
	"testing"
	"time"
)

func ReadWrite(bits uint8, value uint64) error {
	writer := CreateWriter()
	if err := writer.Write(bits, value); err != nil {
		return fmt.Errorf("write error: %v", err)
	}
	if writer.NumWritten() != uint64(bits) {
		return fmt.Errorf("unexpected bits written: got %d, want %d", writer.NumWritten(), bits)
	}
	reader := CreateReader(writer.Buff)
	temp, err := reader.Read(bits)
	if err != nil {
		return fmt.Errorf("read error: %v", err)
	}
	if temp != value {
		return fmt.Errorf("mismatch: wrote %d, read %d", value, temp)
	}
	if reader.NumRead() != uint64(bits) {
		return fmt.Errorf("unexpected bits read: got %d, want %d", reader.NumRead(), bits)
	}
	return nil
}

func MaxValueForBits(n int) uint64 {
	if n <= 0 {
		return 0
	}
	if n >= 64 {
		return ^uint64(0)
	}
	return (1 << n) - 1
}

func TestWriteReadBits(t *testing.T) {
	rng := rand.New(rand.NewSource(time.Now().UnixNano()))
	for bits := 1; bits <= 63; bits++ {
		mval := uint64(MaxValueForBits(bits))
		rval := uint64(rng.Int63n(int64(MaxValueForBits(bits))))
		values := []uint64{0, mval, rval}
		for _, value := range values {
			if err := ReadWrite(uint8(bits), value); err != nil {
				t.Errorf("bits: %d, value: %d, err: %v", bits, value, err)
			}
		}
	}
	values := []uint64{0, MaxValueForBits(64), rng.Uint64()}
	for _, value := range values {
		if err := ReadWrite(64, value); err != nil {
			t.Errorf("bits: %d, value: %d, err: %v", 64, value, err)
		}
	}
}

type Empty struct {
	// Empty
}

func MakeEmpty() Empty {
	return Empty{}
}

type Tuple struct {
	Bits  uint8
	Value uint64
}

func MakeTuple(bits uint8, value uint64) *Tuple {
	return &Tuple{
		Bits:  bits,
		Value: value,
	}
}

func TestMultipleWriteReadBits(t *testing.T) {
	var (
		rng     = rand.New(rand.NewSource(time.Now().UnixNano()))
		indexes = make(map[uint8]Empty, 0)
		data    = make([]*Tuple, 0)
	)
	writer := CreateWriter()
	for len(indexes) < 64 {
		index := uint8(rng.Int63n(64) + 1)
		value := rng.Uint64()
		if index != 64 {
			value = uint64(rng.Int63n(int64(MaxValueForBits(int(index)))))
		}
		if _, ok := indexes[index]; !ok {
			indexes[index] = MakeEmpty()
		}
		if err := writer.Write(index, value); nil != err {
			t.Errorf("failed writing bits: %d, value: %d, err: %v", index, value, err)
		}
		data = append(data, MakeTuple(index, value))
	}
	reader := CreateReader(writer.Buff)
	for _, item := range data {
		temp, err := reader.Read(item.Bits)
		if err != nil {
			t.Errorf("failed reading bits: %d, err: %v", item.Bits, err)
		}
		if temp != item.Value {
			t.Errorf("mismatch: wrote %d, read %d", item.Value, temp)
		}
	}
}
