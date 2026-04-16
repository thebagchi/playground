// Implement a thread-safe rate limiter in Go.
//
// Problem Statement
// Multiple users are making requests concurrently. Each request is handled by a goroutine.
//
// You need to ensure that:
//
// Each user can make at most 3 requests in any 5-second window.
// If the limit is exceeded, the request should be rejected.

// Each user can have n to k

// Task
// Implement the following type:

package main

import (
	"fmt"
	"sync"
	"time"
)

type Rate struct {
	mu               sync.Mutex
	numberOfRequests uint64
	windowSize       uint64
	lastAccess       int64
	numTokens        uint64
}

func NewRate(n, k uint64) *Rate {
	return &Rate{
		numberOfRequests: n,
		windowSize:       k,
		lastAccess:       time.Now().UnixMilli(),
		numTokens:        n,
	}
}

func (r *Rate) Tokens() int {
	r.mu.Lock()
	defer r.mu.Unlock()
	var (
		result               int   = 0
		tokenConsumptionRate       = float64(r.numberOfRequests) / float64(r.windowSize)
		currentAccess        int64 = time.Now().UnixMilli()
		diff                       = currentAccess - r.lastAccess
	)
	// fmt.Println("diff: ", diff, currentAccess, r.lastAccess)
	// Start thinking in terms of milliseconds.
	// TODO: use READTSC for getting accurate ticks

	// number of tokens availabe has to be calculated.
	if diff >= 0 {
		numTokens := float64(diff) * tokenConsumptionRate
		r.numTokens = min(r.numberOfRequests, r.numTokens+uint64(numTokens))
		r.lastAccess = currentAccess
		if r.numTokens >= 1 {
			r.numTokens -= 1
			result = int(r.numTokens) + 1
		}
	}
	return result
}

type RateLimiter struct {
	// Token Buckets for each user
	mu          sync.RWMutex
	users       map[string]*Rate
	maxRequests int
	windowSize  int64
}

func NewRateLimiter(maxRequests int, windowSizeMillis int64) *RateLimiter {
	return &RateLimiter{
		maxRequests: maxRequests,
		windowSize:  windowSizeMillis,
		users:       make(map[string]*Rate),
	}
}

func (rl *RateLimiter) Register(userId string, n, k uint64) {
	rl.mu.Lock()
	defer rl.mu.Unlock()
	if _, ok := rl.users[userId]; !ok {
		var (
			windowSize  = k * uint64(rl.windowSize)
			numRequests = min(n, uint64(rl.maxRequests))
		)
		rl.users[userId] = NewRate(numRequests, windowSize)
	} else {
		// User exists
		// TODO: Handle the case where user already exists
	}
}

func (rl *RateLimiter) AllowRequest(userId string) bool {
	rl.mu.RLock()
	rate, ok := rl.users[userId]
	rl.mu.RUnlock()
	if ok && rate != nil {
		if rate.Tokens() >= 1 {
			return true
		}
	}
	return false
}

// Requirements
// Use the current system time inside AllowRequest.
// The implementation must be thread-safe.
// Requests for different users should not block each other unnecessarily.
// Old requests outside the time window should be removed efficiently.
// Example Behavior
// Assume the limit is 3 requests per 5 seconds:

// AllowRequest("u1") -> true
// AllowRequest("u1") -> true
// AllowRequest("u1") -> true
// AllowRequest("u1") -> false
// (after 5 seconds)
// AllowRequest("u1") -> true

func main() {
	fmt.Println("Starting the rate limiter example")

	rl := NewRateLimiter(10, 1000)
	rl.Register("u1", 5, 5)

	fmt.Println(rl.AllowRequest("u1")) // 1: true
	fmt.Println(rl.AllowRequest("u1")) // 2: true
	fmt.Println(rl.AllowRequest("u1")) // 3: true
	fmt.Println(rl.AllowRequest("u1")) // 4: true
	fmt.Println(rl.AllowRequest("u1")) // 5: true
	fmt.Println(rl.AllowRequest("u1")) // 6: false (bucket exhausted)

	// (after 5 seconds)
	time.Sleep(5 * time.Second)
	fmt.Println(rl.AllowRequest("u1")) // 7: true (tokens replenished)
}
