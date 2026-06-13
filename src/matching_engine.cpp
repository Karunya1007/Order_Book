#include "matching_engine.h"

namespace order_book
{
    MatchingEngine::MatchingEngine(std::size_t max_orders): pool(max_orders), bids(Side::BUY), asks(Side::SELL), lookup(){}
    std::vector<Trade> MatchingEngine::add_order(const NewOrder& new_order)
    {
        std::vector<Trade> trades;
        Order* order = pool.allocate();
        if(!order)
        {
            return trades;
        }
        order->id = new_order.id;
        order->next = nullptr;
        order->price = new_order.price;
        order->prev = nullptr;
        order->quantity = new_order.quantity;
        order->remaining_quantity = new_order.quantity;
        order->timestamp = new_order.timestamp;
        order->flags = new_order.flags;
        order->side = new_order.side;
        OrderBookSide& ownside = (order->side == Side::BUY)? bids: asks;
        OrderBookSide& oppside = (order->side == Side::BUY)? asks: bids;
        while(order->remaining_quantity>0 && !oppside.empty())
        {
            std::int64_t best_price = oppside.get_best_price();
            bool crosses = ((order->side==Side::BUY)&&(order->price >= best_price)) || ((order->side==Side::SELL)&&(order->price <= best_price));
            if(!crosses)break;
            PriceLevel* level = oppside.get_best_price_level();
            Order* oldest = level->front();
            std::int64_t oldest_rem = oldest->remaining_quantity;
            std::int64_t qty = (order->remaining_quantity < oldest_rem)?order->remaining_quantity:oldest_rem;
            Trade t;
            t.taker_id = order->id;
            t.maker_id = oldest->id;
            t.price = oldest->price;
            t.quantity = qty;
            t.timestamp = new_order.timestamp;
            trades.push_back(t);
            order->remaining_quantity -= qty;
            if(qty == oldest_rem)
            {
                level->pop_front();
                oppside.erase_level_if_empty(oldest->price);
                lookup.erase(oldest->id);
                pool.deallocate(oldest);
            }
            else
            {
                oldest->remaining_quantity = oldest_rem - qty;
                level->reduce_total_quantity(qty);
            }
        }
        if(order->remaining_quantity > 0)
        {
            ownside.insert_order(order);
            OrderHandle handle{order->side, order};
            lookup.insert(order->id, handle);
        }
        else
        {
            pool.deallocate(order);
        }
        return trades;
    }

    bool MatchingEngine::cancel_order(OrderId id)
    {
        OrderHandle* handle = lookup.find(id);
        if(!handle)return false;
        Order* o = handle->order;
        Side side = handle->side;
        OrderBookSide& book_side = ((side == Side::BUY)? bids: asks);
        PriceLevel* level = book_side.find_price_level(o->price);
        if(!level)
        {
            lookup.erase(id);
            return false;
        }
        level->remove_order(o);
        book_side.erase_level_if_empty(o->price);
        lookup.erase(id);
        pool.deallocate(o);
        return true;
    }

    std::vector<Trade> MatchingEngine::modify_order(OrderId id, const NewOrder& new_order)
    {
        OrderHandle* handle = lookup.find(id);
        if(!handle)return {};
        Order* o = handle->order;
        Side side = handle->side;
        OrderBookSide& book_side = ((side == Side::BUY)? bids: asks);
        PriceLevel* level = book_side.find_price_level(o->price);
        if(level)
        {
            level->remove_order(o);
            book_side.erase_level_if_empty(o->price);
        }
        lookup.erase(id);
        pool.deallocate(o);
        NewOrder replaced = new_order;
        replaced.id = id;
        return add_order(replaced);
    }
}