#include <deque>
#include <cstdlib>
#include <vector>
#include <fstream>
#include <iostream>
#include <memory>
#include "pizza.h"
#include "pizza.h"
#include "customer.h"
#include "driver.h"
#include "../thread.h"
#include "../cv.h"
#include "../mutex.h"
#include "utils.h"

std::deque<location_t> create_requests(char *requests_file_name)
{
    std::deque<location_t> requests;
    std::ifstream requests_file(requests_file_name);

    // Output error message and exit if file could not open
    if (!requests_file.is_open())
    {
        std::cerr << "Error: Could not open file " << requests_file_name << std::endl;
        exit(1);
    }

    unsigned int x_pos;
    unsigned int y_pos;

    // Read in request locations from the file
    while (requests_file >> x_pos >> y_pos)
    {
        location_t request_location = {x_pos, y_pos};
        requests.push_back(request_location);
    }

    return requests;
}

unsigned int calc_dist(location_t start, location_t end)
{
    // Get absolute x dist
    int x_dist;
    if (start.x > end.x)
    {
        x_dist = start.x - end.x;
    }
    else
    {
        x_dist = end.x - start.x;
    }

    // Get absolute y dist
    int y_dist;
    if (start.y > end.y)
    {
        y_dist = start.y - end.y;
    }
    else
    {
        y_dist = end.y - start.y;
    }

    return x_dist + y_dist;
}

std::pair<customer *, driver *> match_delivery(const std::vector<std::shared_ptr<customer>> &customers, const std::vector<std::shared_ptr<driver>> &drivers)
{
    unsigned int min_dist = UINT_MAX;
    std::shared_ptr<customer> min_customer = nullptr;
    std::shared_ptr<driver> min_driver = nullptr;

    // Check each pair of customer and drivers to find a match
    for (auto recipient : customers)
    {
        // Skip if the customer is not available
        if (!recipient->is_ready())
            continue;

        for (auto delivery_driver : drivers)
        {
            // Skip if driver is not available
            if (!delivery_driver->is_ready())
                continue;

            // Compare calculated distance to previous best distance
            unsigned int curr_dist = calc_dist(recipient->get_location(), delivery_driver->get_location());
            if (curr_dist < min_dist)
            {
                min_customer = recipient;
                min_driver = delivery_driver;
                min_dist = curr_dist;
            }
        }
    }

    match(min_customer->get_id(), min_driver->get_id());
    return std::make_pair<customer *, driver *>(min_customer.get(), min_driver.get());
}

bool has_available_delivery(const std::vector<std::shared_ptr<customer>> &customers, const std::vector<std::shared_ptr<driver>> &drivers)
{
    bool is_customer_available = false;
    bool is_driver_available = false;

    // Check for an available customer
    for (auto recipient : customers)
    {
        if (recipient->is_ready())
            is_customer_available = true;
    }

    // Check for an available driver
    for (auto delivery_driver : drivers)
    {
        if (delivery_driver->is_ready())
            is_driver_available = true;
    }

    // Make sure both a driver and customer are available
    return is_customer_available && is_driver_available;
}

std::pair<int, int> load_input(int argc, char **argv)
{
    int num_drivers;
    int num_customers;

    // Check for user input errors
    try
    {
        if (argc < 2)
        {
            throw std::invalid_argument("Not enough arguments. Usage: ./program <numDrivers> <customerInput...>");
        }

        num_drivers = std::stoi(argv[1]);
        num_customers = argc - 2;

        if (num_customers <= 0 || num_drivers <= 0)
        {
            throw std::invalid_argument("Number of drivers and customers must be a positive number");
        }
    }
    catch (const std::invalid_argument &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        exit(1);
    }

    return std::make_pair(num_drivers, num_customers);
}