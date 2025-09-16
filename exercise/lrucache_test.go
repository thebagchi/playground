package exercise

import (
	"fmt"
	"testing"
)

type Node struct {
	key   int
	value int
	prev  *Node
	next  *Node
}

func MakeNode(key int, value int) *Node {
	return &Node{
		key:   key,
		value: value,
		prev:  nil,
		next:  nil,
	}
}

type LRUCache struct {
	capacity int
	values   map[int]*Node
	head     *Node
	tail     *Node
}

func MakeLRUCache(capacity int) *LRUCache {
	return &LRUCache{
		capacity: capacity,
		values:   make(map[int]*Node),
		head:     nil,
		tail:     nil,
	}
}

func (c *LRUCache) Get(key int) int {
	if node, ok := c.values[key]; ok {
		c.moveFront(node)
		return node.value
	}
	return -1
}

func (c *LRUCache) Put(key, value int) {
	if node, ok := c.values[key]; ok {
		node.value = value
		c.moveFront(node)
		return
	}
	if len(c.values) == c.capacity {
		c.removeTail()
	}
	node := MakeNode(key, value)
	c.values[key] = node
	c.addFront(node)
}

func (c *LRUCache) moveFront(node *Node) {
	var (
		prev = node.prev
		next = node.next
	)
	if nil != prev {
		prev.next = next
	} else {
		c.head = next
	}
	if nil != next {
		next.prev = prev
	} else {
		c.tail = prev
	}
	c.addFront(node)
}

func (c *LRUCache) removeTail() {
	if c.tail == nil {
		return
	}
	var (
		tail = c.tail
		prev = tail.prev
	)
	if prev == nil {
		c.head = nil
		c.tail = nil
	} else {
		delete(c.values, tail.key)
		prev.next = nil
		c.tail = prev
	}
}

func (c *LRUCache) addFront(node *Node) {
	var (
		currHead = c.head
		currTail = c.tail
	)
	if nil != currHead && nil != currTail {
		node.prev = nil
		node.next = currHead
		currHead.prev = node
		c.head = node
	} else {
		c.tail = node
		c.head = node
	}
}

func TestLRUCache(t *testing.T) {
	cache := MakeLRUCache(10)
	for i := range 100 {
		cache.Put(i, i)
	}
	for i := range 100 {
		v := cache.Get(i)
		if i < 90 {
			if v == i {
				fmt.Println("expecting: -1 got: ", v)
				t.FailNow()
			}
		}
		if i >= 90 {
			if v != i {
				fmt.Println("got: -1 expecting: ", i)
				t.FailNow()
			}
		}
	}
}
