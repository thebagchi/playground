package bitbuffer

import (
	"errors"
	"slices"
)

// InitialBufferSize is the initial capacity for the buffer in CreateWriter.
var InitialBufferSize = 64

// Codec manages a bit stream for encoding and decoding.
type Codec struct {
	Buff        []byte
	offset      uint8  // Bit position in the last byte (0–7)
	bitsWritten uint64 // Number of bits written
	bitsRead    uint64 // Number of bits read
}

// CreateWriter creates a new Codec instance for writing with pre-allocated buffer capacity.
func CreateWriter() *Codec {
	return &Codec{Buff: make([]byte, 0, InitialBufferSize)}
}

// CreateReader creates a new Codec instance for reading from the given byte array.
func CreateReader(data []byte) *Codec {
	return &Codec{Buff: data}
}

// Len returns the number of bytes in the buffer.
func (w *Codec) Len() int {
	return len(w.Buff)
}

// Cap returns the capacity of the buffer.
func (w *Codec) Cap() int {
	return cap(w.Buff)
}

// available returns the number of bits available in the buffer.
func (w *Codec) available() int {
	return 8*w.Len() - int(w.offset)
}

// grow extends the buffer by n bytes.
func (w *Codec) grow(n int) {
	if cap(w.Buff) <= len(w.Buff)+n {
		w.Buff = slices.Grow(w.Buff, n+(2*w.Cap()))
	}
	w.Buff = w.Buff[:len(w.Buff)+n]
}

// advance moves to the next byte by removing the first byte.
func (w *Codec) advance() error {
	if w.Len() == 0 {
		return errors.New("no more content")
	}
	w.Buff = w.Buff[1:]
	w.offset = 0
	return nil
}

// append adds a new byte to the buffer and resets offset.
func (w *Codec) append() error {
	w.grow(1)
	w.offset = 0
	return nil
}

// Write writes the least significant num bits of value to the bit stream.
func (w *Codec) Write(num uint8, value uint64) error {
	if num < 1 || num > 64 {
		return errors.New("bit count must be between 1 and 64")
	}
	if w.offset > 7 {
		return errors.New("invalid offset")
	}
	if w.Len() == 0 {
		w.grow(1)
	}
	var (
		mask    = uint64(1<<num) - 1
		written = uint8(0)
	)
	value = value & mask
	for written < num {
		var (
			pending   = num - written
			remaining = 8 - w.offset
			length    = min(pending, remaining)
			shift     = num - written - length
			mask      = uint8(1<<length) - 1
			chunk     = (uint8(value>>shift) & mask) << (remaining - length)
			curr      = w.Buff[w.Len()-1] | chunk
		)
		w.Buff[w.Len()-1] = curr
		w.offset = w.offset + length
		written = written + length
		if w.offset == 8 {
			if err := w.append(); err != nil {
				return err
			}
		}
	}
	w.bitsWritten = w.bitsWritten + uint64(num)
	return nil
}

// Read reads the next num bits from the bit stream, returning them as a uint64.
func (w *Codec) Read(num uint8) (uint64, error) {
	if num == 0 {
		return 0, nil
	}
	if num > 64 {
		return 0, errors.New("bit count must be between 1 and 64")
	}
	if w.offset > 7 {
		return 0, errors.New("invalid offset")
	}
	if w.Len() == 0 {
		return 0, errors.New("buffer is empty")
	}

	// Check if enough bits are available
	if int(w.available()) < int(num) {
		return 0, errors.New("not enough bits in buffer")
	}

	var (
		result = uint64(0)
		read   = uint8(0)
	)

	for read < num {
		var (
			pending   = num - read
			remaining = uint8(8 - w.offset)
			length    = min(pending, remaining)
			shift     = uint8(8 - w.offset - length)
			mask      = uint8((1<<length)-1) << shift
			chunk     = uint64((w.Buff[0] & mask) >> shift)
		)
		result = (result << length) | chunk
		w.offset = w.offset + length
		read = read + length

		if w.offset == 8 {
			if err := w.advance(); err != nil {
				return 0, err
			}
		}
	}
	w.bitsRead = w.bitsRead + uint64(num)
	return result, nil
}

// NumWritten returns the total number of bits written.
func (w *Codec) NumWritten() uint64 {
	return w.bitsWritten
}

// NumRead returns the total number of bits read.
func (w *Codec) NumRead() uint64 {
	return w.bitsRead
}
