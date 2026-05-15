#pragma once

#include <memory>
#include "pizza.h"
#include "customer.h"
#include "constants.h"

class customer; // forward declaration of customer

/**
 * @brief Representation of a driver
 */
class driver
{
private:
    unsigned int id;
    Status status;
    location_t location;
    unsigned int assigned_customer;
    bool paid; // indicator of payment
    bool matched;

public:
    driver();
    driver(unsigned int id_in);

    // Getter methods
    unsigned int get_id();
    location_t get_location();
    unsigned int get_customer();

    // Setter methods
    void set_unavailable();
    void recieve_payment();
    void set_customer(unsigned int customer_id);
    void set_location(location_t new_location);

    // Methods to check status of a driver
    bool is_ready();
    bool is_paid();
    bool is_matched();

    /**
     * @brief Mark a driver as ready
     */
    void ready_up();

    /**
     * @brief Move a driver to a customer location, and notify customer when pizza is delivered
     *
     * @param destination Customer location
     */
    void deliver_pizza(std::shared_ptr<customer> recipient);
};
