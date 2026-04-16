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
	"errors"
	"fmt"
	"sync"
	"sync/atomic"
	"time"
)

var ErrUserNotFound = errors.New("user not registered")

type rateState struct {
	numTokens  uint64
	lastAccess int64
}

type Rate struct {
	state            atomic.Pointer[rateState]
	numberOfRequests uint64
	consumptionRate  float64
}

func NewRate(n, k uint64) *Rate {
	r := &Rate{
		numberOfRequests: n,
		consumptionRate:  float64(n) / float64(k),
	}
	r.state.Store(&rateState{
		numTokens:  n,
		lastAccess: time.Now().UnixMilli(),
	})
	return r
}

func (r *Rate) Allow() bool {
	for {
		old := r.state.Load()
		currentAccess := time.Now().UnixMilli()
		diff := currentAccess - old.lastAccess

		newNumTokens := old.numTokens
		newLastAccess := old.lastAccess
		if diff >= 0 {
			added := float64(diff) * r.consumptionRate
			newNumTokens = min(r.numberOfRequests, old.numTokens+uint64(added))
			// Advance lastAccess only by the time consumed by whole tokens,
			// preserving the fractional remainder for future calls.
			wholeTokens := uint64(added)
			if wholeTokens > 0 {
				newLastAccess = old.lastAccess + int64(float64(wholeTokens)/r.consumptionRate)
			}
		}

		if newNumTokens < 1 {
			return false
		}

		if r.state.CompareAndSwap(old, &rateState{
			numTokens:  newNumTokens - 1,
			lastAccess: newLastAccess,
		}) {
			return true
		}
	}
}

type RateLimiter struct {
	// Token Buckets for each user
	mu          sync.RWMutex
	users       map[string]*Rate
	maxRequests int
}

func NewRateLimiter(maxRequests int) *RateLimiter {
	return &RateLimiter{
		maxRequests: maxRequests,
		users:       make(map[string]*Rate),
	}
}

// Register adds a user with a rate limit of n requests per windowMillis milliseconds.
func (rl *RateLimiter) Register(userId string, n uint64, windowMillis int64) {
	rl.mu.Lock()
	defer rl.mu.Unlock()
	if _, ok := rl.users[userId]; !ok {
		numRequests := min(n, uint64(rl.maxRequests))
		rl.users[userId] = NewRate(numRequests, uint64(windowMillis))
	}
}

func (rl *RateLimiter) AllowRequest(userId string) (bool, error) {
	rl.mu.RLock()
	rate, ok := rl.users[userId]
	rl.mu.RUnlock()
	if !ok || rate == nil {
		return false, ErrUserNotFound
	}
	return rate.Allow(), nil
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

	rl := NewRateLimiter(10)
	rl.Register("u1", 5, 5000) // 5 requests per 5000ms

	for i := 1; i <= 6; i++ {
		allowed, err := rl.AllowRequest("u1")
		fmt.Printf("%d: allowed=%v err=%v\n", i, allowed, err)
	}

	// unregistered user
	allowed, err := rl.AllowRequest("u2")
	fmt.Printf("u2: allowed=%v err=%v\n", allowed, err)

	// (after 5 seconds)
	time.Sleep(5 * time.Second)
	allowed, err = rl.AllowRequest("u1")
	fmt.Printf("after sleep: allowed=%v err=%v\n", allowed, err)
}
