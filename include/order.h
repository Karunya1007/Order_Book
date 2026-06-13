#pragma once

#include<cstdint>
namespace order_book
{
    enum class Side: std::uint8_t
    {
        BUY, SELL
    };
    using OrderId = std::uint64_t;
    struct alignas(64) Order
    {
        Order* prev;
        Order* next;
        OrderId id;
        Side side;
        std::int64_t price;
        std::int64_t quantity;
        std::int64_t remaining_quantity;
        std::int64_t timestamp;
        std::int64_t flags;
    };
    struct Trade
    {
        OrderId maker_id;
        OrderId taker_id;
        std::int64_t timestamp;
        std::int64_t price;
        std::int64_t quantity;
    };
}