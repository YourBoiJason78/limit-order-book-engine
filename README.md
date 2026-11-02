# Limit Order Book Engine

A small, single-symbol limit order book in C++17. The matching library is
independent of the demo and has no third-party dependencies.

## Build and run

From the repository root, configure and build with CMake 3.16 or newer:

```sh
cmake -S projects/order_book -B projects/order_book/build -DCMAKE_BUILD_TYPE=Release
cmake --build projects/order_book/build
```

Run `projects/order_book/build/order_book_demo` on Linux or macOS. On Windows,
run `projects/order_book/build/order_book_demo.exe` with a single-configuration
generator, or the executable in `projects/order_book/build/Debug/` or
`projects/order_book/build/Release/` with a multi-configuration generator.

## Matching rules

- Each `OrderBook` instance represents one instrument and is used by one thread.
- Prices are positive 64-bit integer ticks. Quantities are positive 64-bit
  integer units; the engine does not parse decimal prices or compute monetary
  totals.
- Better prices match first. Orders at the same price match in arrival order.
- Trades execute at the resting order's price.
- An incoming order's unfilled quantity rests at its limit price.
- A fully filled or cancelled order leaves the active book, so its ID can be
  reused.
- Reducing an order's remaining quantity at the same price preserves its place
  in the queue. Changing price or increasing quantity loses priority: the
  modified order is reprocessed as a fresh incoming limit order and can trade.
- Set the new remaining quantity with `modify`; use `cancel` to remove an order.

## Library API

Include `order_book.hpp` and link the `order_book` CMake target. Public types are
in namespace `lob`:

```cpp
lob::OrderBook book;
auto result=book.submit({1,lob::Side::Buy,100,8});
auto current=book.snapshot();
```

`submit` and `modify` return an `OperationResult` with a `ResultCode`, trades in
execution order, and the order's remaining quantity after a successful action.
A zero remainder means it filled; a positive remainder means it is resting.
Rejected actions return no trades and zero remaining, so check `code` before
using the quantity. `cancel` returns `Ok` when it removes an active order and
`NotFound` otherwise. Submission can also return `InvalidId`, `InvalidSide`,
`InvalidPrice`, `InvalidQuantity`, or `DuplicateId`; modification can return
`InvalidPrice` or `InvalidQuantity` for a known order. Failed actions do not
change the book.

`snapshot()` returns owning copies: bids are ordered from highest to lowest
price, asks from lowest to highest, and orders at each price remain in FIFO
order.

## Data structures and scope

Each side uses an ordered price map whose levels own FIFO lists. An ID index
points to active orders so cancellation and modification can find a list node
directly. The ordered maps make the best price easy to inspect, while lists keep
price-level priority explicit and allow one order to be removed without moving
the others.

This project covers limit orders, matching, cancellation, modification, and
book snapshots. It does not include market orders, multiple symbols, networking,
exchange-message decoding, replay, concurrency, persistence, strategy logic, or
backtesting. The demo is an example, not a performance benchmark.
