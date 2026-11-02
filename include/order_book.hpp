#pragma once

#include <cstdint>
#include <functional>
#include <list>
#include <map>
#include <unordered_map>
#include <vector>

namespace lob{

enum class Side{Buy,Sell};
enum class ResultCode{Ok,InvalidId,InvalidSide,InvalidPrice,InvalidQuantity,DuplicateId,NotFound};

struct Order{
    std::uint64_t id;
    Side side;
    std::int64_t price;
    std::int64_t remaining;
};

struct Trade{
    std::uint64_t buy_id,sell_id;
    std::int64_t price,quantity;
};

struct OperationResult{
    ResultCode code;
    std::vector<Trade> trades;
    std::int64_t remaining;
};

struct Snapshot{
    std::vector<Order> bids,asks;
};

class OrderBook{
public:
    OrderBook()=default;
    OrderBook(const OrderBook&)=delete;
    OrderBook& operator=(const OrderBook&)=delete;
    OrderBook(OrderBook&&)=delete;
    OrderBook& operator=(OrderBook&&)=delete;

    OperationResult submit(Order order);
    OperationResult cancel(std::uint64_t id);
    OperationResult modify(std::uint64_t id,std::int64_t price,std::int64_t remaining);
    Snapshot snapshot() const;

private:
    using OrderQueue=std::list<Order>;
    using BidLevels=std::map<std::int64_t,OrderQueue,std::greater<std::int64_t>>;
    using AskLevels=std::map<std::int64_t,OrderQueue>;

    struct OrderLocation{
        Side side;
        std::int64_t price;
        OrderQueue::iterator order;
    };

    BidLevels bids_;
    AskLevels asks_;
    std::unordered_map<std::uint64_t,OrderLocation> active_;
};

} // namespace lob
