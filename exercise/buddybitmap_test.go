package exercise

import (
	"fmt"
	"math/bits"
	"testing"
)

func NextPowerOfTwo(n uint64) uint64 {
	return uint64(1) << uint64(bits.UintSize-bits.LeadingZeros64(n-1))
}

func Pow2(n uint64) uint64 {
	return 1 << n
}

func Log2(n uint64) uint64 {
	return uint64(bits.Len64(n) - 1)
}

func IsPow2(n uint64) bool {
	return n&(n-1) == 0
}

type BuddyBitmap struct {
	bitmap    []bool
	totalSize int
	numLevels int
}

func MakeBuddyBitmap(size uint64) *BuddyBitmap {
	var (
		totalSize = NextPowerOfTwo(size)
		numLevels = Log2(totalSize)
	)
	fmt.Println("totalSize: ", totalSize, "numLevels: ", numLevels)
	return &BuddyBitmap{
		bitmap:    make([]bool, totalSize-1),
		totalSize: int(totalSize),
		numLevels: int(numLevels),
	}
}

func (bmp *BuddyBitmap) buddies(level int) (int, int) {
	// 0...0   [0,0] (2^level - 1) ((2^(level+1) -1)-1)
	// 1...2   [1,2]
	// 3...6   [3,6]
	// 7...14  [7,14]
	return int(Pow2(uint64(level))) - 1, int((Pow2(uint64(level+1)) - 1) - 1)
}

func (bmp *BuddyBitmap) buddy(index int) int {
	if index == 0 {
		return 0
	}
	if index%2 == 0 {
		return index - 1
	}
	return index + 1
}

func (bmp *BuddyBitmap) parent(index int) int {
	if index > 0 {
		return (index - 1) / 2
	}
	return -1
}

func (bmp *BuddyBitmap) level(index int) int {
	// 0     [0]
	// 1...2 [1]
	// 3...6 [2]
	return int(Log2(uint64(index + 1)))
}

func (bmp *BuddyBitmap) children(index int) (int, int) {
	// 0  -> 1, 2 (2*index+1), ((2*index+1)+1)
	// 1  -> 3, 4
	// 2  -> 5, 6
	var (
		lchild = (2 * index) + 1
		rchild = lchild + 1
		max    = len(bmp.bitmap)
	)
	if lchild > max || rchild > max {
		return -1, -1
	}
	return lchild, rchild
}

func (bmp *BuddyBitmap) setChildren(index int) {
	lchild, rchild := bmp.children(index)
	if lchild != -1 && rchild != -1 {
		bmp.bitmap[lchild] = true
		bmp.bitmap[rchild] = true
		bmp.setChildren(lchild)
		bmp.setChildren(rchild)
	}
}

func (bmp *BuddyBitmap) setParent(index int) {
	buddy := bmp.buddy(index)
	if buddy == index {
		return
	}
	if bmp.bitmap[buddy] == true {
		parent := bmp.parent(index)
		if parent >= 0 {
			bmp.bitmap[parent] = true
			bmp.setParent(parent)
		}
	}
}

func (bmp *BuddyBitmap) setIndex(index int) {
	lchild, rchild := bmp.children(index)
	if lchild != -1 && rchild != -1 {
		// set children indexes
		bmp.setChildren(index)
	}
	buddy := bmp.buddy(index)
	if bmp.bitmap[buddy] == true {
		// set parent indexes
		bmp.setParent(index)
	}
	bmp.bitmap[index] = true
}

func (bmp *BuddyBitmap) Allocate(size int) int {
	var (
		actualSize   = NextPowerOfTwo(uint64(size))
		desiredLevel = Log2(actualSize)
		bStart, bEnd = bmp.buddies(bmp.numLevels - int(desiredLevel))
	)
	for i := bStart; i <= bEnd; i++ {
		if bmp.bitmap[i] == false {
			bmp.setIndex(i)
			return i
		}
	}
	return -1
}

func (bmp *BuddyBitmap) clearChildren(index int) {
	lchild, rchild := bmp.children(index)
	if lchild != -1 && rchild != -1 {
		bmp.bitmap[lchild] = false
		bmp.bitmap[rchild] = false
		bmp.clearChildren(lchild)
		bmp.clearChildren(rchild)
	}
}

func (bmp *BuddyBitmap) clearParent(index int) {
	buddy := bmp.buddy(index)
	if buddy == index {
		return
	}
	if bmp.bitmap[buddy] == false {
		parent := bmp.parent(index)
		if parent >= 0 {
			bmp.bitmap[parent] = false
			bmp.clearParent(parent)
		}
	}
}

func (bmp *BuddyBitmap) clearIndex(index int) {
	lchild, rchild := bmp.children(index)
	if lchild != -1 && rchild != -1 {
		bmp.clearChildren(index)
	}
	buddy := bmp.buddy(index)
	if bmp.bitmap[buddy] == false {
		bmp.clearParent(index)
	}
	bmp.bitmap[index] = false
}

func (bmp *BuddyBitmap) Deallocate(index int) {
	bmp.clearIndex(index)
}

func TestNextPowerOfTwo(t *testing.T) {
	// Validate NextPowerOfTwo for a range of positive inputs (avoid 0 which the implementation
	// doesn't handle safely).
	for n := uint64(1); n <= 1024*1024; n++ {
		got := NextPowerOfTwo(n)
		// compute expected by simple loop: smallest power of two >= n
		want := uint64(1)
		for want < n {
			want <<= 1
		}
		if got != want {
			t.Fatalf("NextPowerOfTwo(%d) = %d; want %d", n, got, want)
		}
	}
}

func TestPow2(t *testing.T) {
	for i := uint64(0); i < 64; i++ {
		want := uint64(1) << i
		if got := Pow2(i); got != want {
			t.Fatalf("Pow2(%d) = %d; want %d", i, got, want)
		}
	}
}

func TestLog2(t *testing.T) {
	// Spot-check a range of values to increase confidence
	for n := uint64(1); n <= 1024*1024; n *= 3 {
		// compute expected by simple loop
		tmp := n
		var want uint64
		for tmp > 1 {
			tmp >>= 1
			want++
		}
		if got := Log2(n); got != want {
			t.Fatalf("Log2(%d) = %d; want %d", n, got, want)
		}
	}
}

func TestBuddyBitmapAllocateDeallocate(t *testing.T) {
	bmp := MakeBuddyBitmap(100)

	// Allocate two blocks of size 40 (will be rounded up to 64)
	i1 := bmp.Allocate(40)
	if i1 == -1 {
		t.Fatalf("expected first allocation to succeed")
	}

	i2 := bmp.Allocate(40)
	if i2 == -1 {
		t.Fatalf("expected second allocation to succeed")
	}

	if i1 == i2 {
		t.Fatalf("expected allocations to return different indices, got same: %d", i1)
	}

	// Deallocate both allocated blocks
	bmp.Deallocate(i1)
	bmp.Deallocate(i2)

	// After freeing both buddies at that level, their parent should be free
	// Find parent of one of the indices and assert it's marked free (bitmap false)
	p1 := bmp.parent(i1)
	p2 := bmp.parent(i2)
	if p1 < 0 || p1 >= len(bmp.bitmap) {
		t.Fatalf("invalid parent index computed: %d", p1)
	}
	if p2 < 0 || p2 >= len(bmp.bitmap) {
		t.Fatalf("invalid parent index computed: %d", p1)
	}

	if bmp.bitmap[p1] || bmp.bitmap[p2] {
		t.Fatalf("expected parent to be free (false in bitmap) after merging, but was true")
	}

	// Now allocate a larger block that should reuse the merged parent (next power of two)
	// Request a block that fits the combined size of the two freed blocks (e.g. 128)
	index := bmp.Allocate(80) // 80 rounds to 128
	if index == -1 {
		t.Fatalf("expected allocation of combined block to succeed")
	}

	// i3 should be at or above the parent level; ensure it's not equal to child indices
	if index == i1 || index == i2 {
		t.Fatalf("expected combined allocation to return parent/upper index, got child index: %d", index)
	}
}
