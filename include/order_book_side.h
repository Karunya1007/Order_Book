#pragma once

#include <boost/container/flat_map.hpp>
#include <map>
#include <cstdint>
#include "price_level.h"
#include "order.h"

namespace order_book
{
    class OrderBookSide
    {
        private:
            Side side;
            using flat_map = boost::container::flat_map<std::int64_t, PriceLevel>;
            flat_map levels;
        public:
            explicit OrderBookSide(Side side);
            bool empty() const;
            void insert_order(Order* order);
            std::int64_t get_best_price() const;
            PriceLevel* get_best_price_level();
            const PriceLevel* get_best_price_level() const;
            PriceLevel* find_price_level(std::int64_t price);
            void erase_level_if_empty(std::int64_t price);
    };
}