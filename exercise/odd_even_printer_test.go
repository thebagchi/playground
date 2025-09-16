package exercise

import (
	"fmt"
	"testing"
)

const MAX = 20

type empty struct{}

func makeEmpty() empty {
	return empty{}
}

func emitter(eChan, oChan chan<- int, pChan <-chan int, done chan<- empty) {
	defer close(eChan)
	defer close(oChan)

	for i := 1; i <= MAX; i++ {
		if i%2 == 0 {
			eChan <- i
		} else {
			oChan <- i
		}
		num := <-pChan
		fmt.Printf("Printer: %d\n", num)
	}
	done <- makeEmpty()
}

func evenProcessor(eChan <-chan int, pChan chan<- int) {
	for num := range eChan {
		// Simulate processing
		pChan <- num
	}
}

func oddProcessor(oChan <-chan int, pChan chan<- int) {
	for num := range oChan {
		// Simulate processing
		pChan <- num
	}
}

func TestOEPrinter(t *testing.T) {
	var (
		eChan = make(chan int)
		oChan = make(chan int)
		pChan = make(chan int)
		dChan = make(chan empty)
	)

	defer close(pChan)
	defer close(dChan)

	go evenProcessor(eChan, pChan)
	go oddProcessor(oChan, pChan)
	go emitter(eChan, oChan, pChan, dChan)

	<-dChan
}
