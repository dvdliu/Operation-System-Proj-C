#pragma once

#include <deque>
#include <vector>
#include <memory>
#include "pizza.h"
#include "customer.h"
#include "driver.h"
#include "../thread.h"
#include "../cv.h"
#include "../mutex.h"

/**
 * @brief struct to hold command line args to pass to a thread
 *
 */
struct Args
{
    int argc;
    char **argv;

    Args(int argc_in, char **argv_in) : argc(argc_in), argv(argv_in) {};
};

/**
 * @brief check command line arguments and load in the number of drivers and customers specified
 *
 * @param argc number of args
 * @param argv pointer to char[] of args
 * @return std::pair<int, int>
 *         number of customers and drivers specified
 */
std::pair<int, int> load_input(int argc, char **argv);

/**
 * @brief Create a queue of locations given a file of requests
 *
 * @param requests_file_name The file name of requests file
 * @return std::deque<location_t> A queue of request locations
 */
std::deque<location_t> create_requests(char *requests_file_name);

/**
 * @brief match the closest driver and customer out of all available drivers and customers
 *
 * @param customers vector of all customers pointers
 * @param drivers vector of all drivers pointers
 * @return pair<customer *, driver *> object containing closest driver and customer
 */
std::pair<customer *, driver *> match_delivery(const std::vector<std::shared_ptr<customer>> &customers,
                                               const std::vector<std::shared_ptr<driver>> &drivers);

/**
 * @brief calculate the rectilinear distance between two points
 *
 * @param start starting location
 * @param end destination location
 * @return unsigned int
 */
unsigned int calc_dist(location_t start, location_t end);

/**
 * @brief check if there are atleast one driver and customer available to make a pizza delivery
 *
 * @param customers vector of customer pointers
 * @param drivers vector of driver pointers
 * @return true if there is an available driver and customer
 * @return false
 */
bool has_available_delivery(const std::vector<std::shared_ptr<customer>> &customers,
                            const std::vector<std::shared_ptr<driver>> &drivers);