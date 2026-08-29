#include "RandomPolicy.h"

#include "Action.h"

#include <random>

std::size_t RandomPolicy::SelectAction()
{
    static thread_local std::mt19937 rng{
        std::random_device{}()
    };

    std::uniform_int_distribution<std::size_t> dist(
        0,
        TOTAL_ACTION_COUNT - 1
    );

    return dist(rng);
}