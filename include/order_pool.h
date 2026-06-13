#pragma once
#include <vector>
#include <cstddef>
#include <cstdint>
#include "order.h"

namespace order_book
{
    class OrderPool
    {
        private:
            struct Node
            {
                Order order;
                std::uint32_t next;
            };
            std::vector<Node> nodes;
            std::uint32_t free_head;
            std::size_t size;
            std::size_t capacity;
            std::uint32_t get_idx_from_ptr(const Order* order) const;
        public:
            explicit OrderPool(std::size_t capacity);
            Order* allocate();
            void deallocate(Order* order);
            void reset();
            std::size_t get_capacity() const {return capacity;}
            std::size_t get_size() const {return size;}
    };
}