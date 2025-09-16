package exercise

import (
	"math"
	"testing"
)

type LFUCache struct {
	capacity         int
	size             int
	values           map[int]int
	frequencies      map[int]int
	frequentKeys     map[int]map[int]empty
	minimumFrequency int
}

func MakeLFUCache(capacity int) *LFUCache {
	return &LFUCache{
		capacity:         capacity,
		size:             0,
		values:           make(map[int]int),
		frequencies:      make(map[int]int),
		frequentKeys:     make(map[int]map[int]empty),
		minimumFrequency: 0,
	}
}

func (c *LFUCache) evictLFU() {
	if keys, ok := c.frequentKeys[c.minimumFrequency]; ok {
		for key := range keys {
			delete(keys, key)
			delete(c.frequencies, key)
			delete(c.values, key)
			c.size = c.size - 1
			break
		}
		if len(keys) == 0 {
			temp := math.MaxInt
			for freq := range c.frequentKeys {
				temp = min(temp, freq)
			}
			c.minimumFrequency = temp
		}
	}
}

func (c *LFUCache) updateFrequency(key int) {
	currentFrequency := 1
	if freq, ok := c.frequencies[key]; ok {
		currentFrequency = freq + 1
	}
	if currentFrequency-1 > 0 {
		delete(c.frequentKeys[currentFrequency-1], key)
	}
	if _, ok := c.frequentKeys[currentFrequency]; !ok {
		c.frequentKeys[currentFrequency] = make(map[int]empty)
	}
	c.frequentKeys[currentFrequency][key] = makeEmpty()
	c.frequencies[key] = currentFrequency
	if (len(c.frequentKeys[c.minimumFrequency]) == 0) && (currentFrequency-1 == c.minimumFrequency) {
		c.minimumFrequency = c.minimumFrequency + 1
	}
}

func (c *LFUCache) Put(key, value int) {
	if c.capacity == c.size {
		c.evictLFU()
	}
	c.values[key] = value
	c.updateFrequency(key)
	c.size = c.size + 1
}

func (c *LFUCache) Get(key int) int {
	if v, ok := c.values[key]; !ok {
		return -1
	} else {
		c.updateFrequency(key)
		return v
	}
}

func TestLFUCache(t *testing.T) {
	cache := MakeLFUCache(2)
	cache.Put(1, 1)
	cache.Put(2, 2)
	if cache.Get(1) != 1 {
		t.Errorf("Expected 1, got %d", cache.Get(1))
	}
	cache.Put(3, 3)
	if cache.Get(2) != -1 {
		t.Errorf("Expected -1, got %d", cache.Get(2))
	}
	if cache.Get(1) != 1 {
		t.Errorf("Expected 1, got %d", cache.Get(1))
	}
	if cache.Get(3) != 3 {
		t.Errorf("Expected 1, got %d", cache.Get(3))
	}
}
