package exercise

import (
	"bytes"
	"fmt"
	"testing"
	"time"
)

func MakeBoard(n int) [][]int {
	board := make([][]int, n)
	for row := range board {
		board[row] = make([]int, n)
	}
	return board
}

func PrintBoard(board [][]int) {
	buffer := new(bytes.Buffer)
	for row := range board {
		buffer.WriteByte('|')
		for col := range board[row] {
			buffer.WriteByte(' ')
			if board[row][col] == 1 {
				buffer.WriteByte('Q')
			} else {
				buffer.WriteByte(' ')
			}
			buffer.WriteByte(' ')
			buffer.WriteByte('|')
		}
		buffer.WriteString("\n")
	}
	fmt.Println(buffer.String())
}

func Attacked(board [][]int, row, col int) bool {
	var (
		rows = len(board)
		cols = len(board[0])
	)
	attacked := false
	for {
		// No Queen in Same Row
		for i := 0; i < cols; i++ {
			if i != col {
				if board[row][i] == 1 {
					// Under Seige ...
					attacked = true
				}
			}
		}
		// No Queen in Same Col
		for i := 0; i < rows; i++ {
			if i != row {
				if board[i][col] == 1 {
					// Under Seige ...
					attacked = true
				}
			}
		}
		// No Queen in Diagonal "\"
		for i, j := row, col; i >= 0 && j >= 0; i, j = i-1, j-1 {
			if i != row && j != col {
				if board[i][j] == 1 {
					// Under Seige ...
					attacked = true
				}
			}
		}
		for i, j := row, col; i < rows && j < cols; i, j = i+1, j+1 {
			if i != row && j != col {
				if board[i][j] == 1 {
					// Under Seige ...
					attacked = true
				}
			}
		}
		// No Queen in Diagonal "/"
		for i, j := row, col; i < rows && j >= 0; i, j = i+1, j-1 {
			if i != row && j != col {
				if board[i][j] == 1 {
					// Under Seige ...
					attacked = true
				}
			}
		}
		for i, j := row, col; i >= 0 && j < cols; i, j = i-1, j+1 {
			if i != row && j != col {
				if board[i][j] == 1 {
					// Under Seige ...
					attacked = true
				}
			}
		}
		break
	}
	return attacked
}

func MoveQueen(board [][]int, row int) bool {
	// Each gets a row. Free to occupy any column in that row.
	var (
		rows = len(board)
		cols = len(board[0])
		ok   = false
	)
	for {
		if row >= rows {
			ok = true
			break
		}
		for i := 0; i < cols; i++ {
			if !Attacked(board, row, i) {
				board[row][i] = 1
				if MoveQueen(board, row+1) {
					ok = true
					break
				}
				board[row][i] = 0
			}
		}
		break
	}
	return ok
}

func NQueens(n int) {
	start := time.Now()
	defer func() {
		fmt.Println("Execution Time:", time.Since(start))
	}()
	board := MakeBoard(n)
	if MoveQueen(board, 0) {
		print(fmt.Sprintf("Solved n queens problem for n = %d", n))
		print("\n")
		PrintBoard(board)
	} else {
		print(fmt.Sprintf("Unable to solve n queens problem for n = %d", n))
		print("\n")
	}
}

func TestNQueens(t *testing.T) {
	for i := 1; i <= 32; i++ {
		NQueens(i)
	}
}
