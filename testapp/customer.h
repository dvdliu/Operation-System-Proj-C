#pragma once

#include <deque>
#include "pizza.h"
#include "constants.h"
#include "driver.h"
#include <memory>

class driver; // forward declaration of driver

/**
 * @brief Representation of a customer
 */
class customer
{
private:
    unsigned int id;
    Status status;
    location_t location;
    std::deque<location_t> requests; // requests queue
    unsigned int assigned_driver;
    bool has_pizza;
    bool matched;

public:
    customer();
    customer(unsigned int id_in, char *requests_file); // load in requests queue from file

    // Methods to check status of a customer
    bool is_ready();
    bool is_done();
    bool is_matched();

    // Setter methods
    void set_unavailable();
    void receive_pizza();
    void set_driver(unsigned int driver_id);

    // Getter methods
    bool check_pizza();
    unsigned int get_id();
    unsigned int get_driver();
    Status get_status();
    location_t get_location();

    /**
     * @brief Mark a customer as ready, and pop a request off of this customer's requests queue
     *
     */
    void ready_up();

    /**
     * @brief Pay driver and notify driver they have been paid
     *
     * @param delivery_driver
     */
    void pay_driver(std::shared_ptr<driver> delivery_driver);
};
