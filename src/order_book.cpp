#include "order_book.hpp"

#include <algorithm>
#include <iterator>

namespace lob{
namespace{

bool valid_side(Side side){
    return side==Side::Buy || side==Side::Sell;
}

OperationResult rejected(ResultCode code){
    return {code,{},0};
}

} // namespace

OperationResult OrderBook::submit(Order order){
    if(order.id==0)
        return rejected(ResultCode::InvalidId);
    if(!valid_side(order.side))
        return rejected(ResultCode::InvalidSide);
    if(order.price<=0)
        return rejected(ResultCode::InvalidPrice);
    if(order.remaining<=0)
        return rejected(ResultCode::InvalidQuantity);
    if(active_.find(order.id)!=active_.end())
        return rejected(ResultCode::DuplicateId);

    OperationResult result{ResultCode::Ok,{},0};

    if(order.side==Side::Buy){
        while(order.remaining>0 && !asks_.empty() && order.price>=asks_.begin()->first){
            auto level=asks_.begin();
            auto resting=level->second.begin();
            const std::int64_t quantity=std::min(order.remaining,resting->remaining);

            result.trades.push_back({order.id,resting->id,resting->price,quantity});
            order.remaining-=quantity;
            resting->remaining-=quantity;

            if(resting->remaining==0){
                active_.erase(resting->id);
                level->second.erase(resting);
            }
            if(level->second.empty())
                asks_.erase(level);
        }
    }
    else{
        while(order.remaining>0 && !bids_.empty() && order.price<=bids_.begin()->first){
            auto level=bids_.begin();
            auto resting=level->second.begin();
            const std::int64_t quantity=std::min(order.remaining,resting->remaining);

            result.trades.push_back({resting->id,order.id,resting->price,quantity});
            order.remaining-=quantity;
            resting->remaining-=quantity;

            if(resting->remaining==0){
                active_.erase(resting->id);
                level->second.erase(resting);
            }
            if(level->second.empty())
                bids_.erase(level);
        }
    }

    if(order.remaining>0){
        if(order.side==Side::Buy){
            auto& queue=bids_[order.price];
            queue.push_back(order);
            active_.emplace(order.id,OrderLocation{order.side,order.price,std::prev(queue.end())});
        }
        else{
            auto& queue=asks_[order.price];
            queue.push_back(order);
            active_.emplace(order.id,OrderLocation{order.side,order.price,std::prev(queue.end())});
        }
    }

    result.remaining=order.remaining;
    return result;
}

OperationResult OrderBook::cancel(std::uint64_t id){
    auto active=active_.find(id);
    if(active==active_.end())
        return rejected(ResultCode::NotFound);

    const OrderLocation location=active->second;
    active_.erase(active);

    if(location.side==Side::Buy){
        auto level=bids_.find(location.price);
        level->second.erase(location.order);
        if(level->second.empty())
            bids_.erase(level);
    }
    else{
        auto level=asks_.find(location.price);
        level->second.erase(location.order);
        if(level->second.empty())
            asks_.erase(level);
    }

    return {ResultCode::Ok,{},0};
}

OperationResult OrderBook::modify(std::uint64_t id,std::int64_t price,std::int64_t remaining){
    auto active=active_.find(id);
    if(active==active_.end())
        return rejected(ResultCode::NotFound);
    if(price<=0)
        return rejected(ResultCode::InvalidPrice);
    if(remaining<=0)
        return rejected(ResultCode::InvalidQuantity);

    const OrderLocation location=active->second;
    Order updated=*location.order;

    if(price==location.price && remaining<=location.order->remaining){
        location.order->remaining=remaining;
        return {ResultCode::Ok,{},remaining};
    }

    updated.price=price;
    updated.remaining=remaining;
    active_.erase(active);

    if(location.side==Side::Buy){
        auto level=bids_.find(location.price);
        level->second.erase(location.order);
        if(level->second.empty())
            bids_.erase(level);
    }
    else{
        auto level=asks_.find(location.price);
        level->second.erase(location.order);
        if(level->second.empty())
            asks_.erase(level);
    }

    return submit(updated);
}

Snapshot OrderBook::snapshot() const{
    Snapshot result;

    for(const auto& level:bids_)
        for(const Order& order:level.second)
            result.bids.push_back(order);

    for(const auto& level:asks_)
        for(const Order& order:level.second)
            result.asks.push_back(order);

    return result;
}

} // namespace lob
