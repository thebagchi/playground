package exercise

import (
	"testing"
)

func TestNQueens(t *testing.T) {
	for i := 1; i <= 32; i++ {
		NQueens(i)
	}
}
