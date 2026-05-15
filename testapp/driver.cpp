#include "driver.h"
#include "constants.h"
#include <memory>

driver::driver() : id() {};

driver::driver(unsigned int id_in) : id(id_in), status(Status::Unavailable), paid(false), matched(false)
{
    // Drivers start at (0, 0)
    this->location = {0, 0};
}

void driver::ready_up()
{
    driver_ready(this->id, this->location);

    // Mark driver as available, and reset status from previous delivery
    this->paid = false;
    this->matched = false;
    this->status = Status::Available;
}

void driver::deliver_pizza(std::shared_ptr<customer> recipient)
{ // Delivery pizza and mark customer as completed after completed delivery
    location_t destination = recipient->get_location();
    drive(this->id, this->location, destination);
}

unsigned int driver::get_id()
{
    return this->id;
}

bool driver::is_ready()
{
    return this->status == Status::Available;
}

location_t driver::get_location()
{
    return this->location;
}

void driver::set_unavailable()
{
    this->status = Status::Unavailable;
}

void driver::recieve_payment()
{
    this->paid = true;
}

bool driver::is_paid()
{
    return this->paid;
}

void driver::set_customer(unsigned int customer_id)
{
    this->assigned_customer = customer_id;
    this->matched = true;
}

unsigned int driver::get_customer()
{
    return this->assigned_customer;
}

bool driver::is_matched()
{
    return this->matched;
}

void driver::set_location(location_t new_location)
{
    this->location = new_location;
}