#include "order_book.hpp"

#include <iostream>

namespace{

const char* result_name(lob::ResultCode code){
    switch(code){
        case lob::ResultCode::Ok: return "ok";
        case lob::ResultCode::InvalidId: return "invalid id";
        case lob::ResultCode::InvalidSide: return "invalid side";
        case lob::ResultCode::InvalidPrice: return "invalid price";
        case lob::ResultCode::InvalidQuantity: return "invalid quantity";
        case lob::ResultCode::DuplicateId: return "duplicate id";
        case lob::ResultCode::NotFound: return "order not found";
    }
    return "unknown result";
}

bool accepted(const lob::OperationResult& result,const char* operation){
    if(result.code==lob::ResultCode::Ok)
        return true;

    std::cerr<<operation<<" failed: "<<result_name(result.code)<<'\n';
    return false;
}

void print_trades(const lob::OperationResult& result){
    for(const lob::Trade& trade:result.trades)
        std::cout<<"TRADE buy="<<trade.buy_id
            <<" sell="<<trade.sell_id
            <<" price="<<trade.price
            <<" quantity="<<trade.quantity<<'\n';
}

void print_snapshot(const lob::OrderBook& book){
    const lob::Snapshot current=book.snapshot();

    if(current.bids.empty())
        std::cout<<"BIDS empty\n";
    else
        for(const lob::Order& order:current.bids)
            std::cout<<"BID id="<<order.id
                <<" price="<<order.price
                <<" remaining="<<order.remaining<<'\n';

    if(current.asks.empty())
        std::cout<<"ASKS empty\n";
    else
        for(const lob::Order& order:current.asks)
            std::cout<<"ASK id="<<order.id
                <<" price="<<order.price
                <<" remaining="<<order.remaining<<'\n';
}

} // namespace

int main(){
    lob::OrderBook book;

    auto result=book.submit({1,lob::Side::Sell,101,10});
    if(!accepted(result,"submit"))
        return 1;

    result=book.submit({2,lob::Side::Sell,101,5});
    if(!accepted(result,"submit"))
        return 1;

    result=book.submit({3,lob::Side::Buy,102,12});
    if(!accepted(result,"submit"))
        return 1;
    print_trades(result);

    result=book.submit({4,lob::Side::Buy,99,5});
    if(!accepted(result,"submit"))
        return 1;

    result=book.submit({5,lob::Side::Buy,99,4});
    if(!accepted(result,"submit"))
        return 1;

    result=book.cancel(5);
    if(!accepted(result,"cancel"))
        return 1;

    result=book.modify(4,101,6);
    if(!accepted(result,"modify"))
        return 1;
    print_trades(result);

    print_snapshot(book);
    return 0;
}
