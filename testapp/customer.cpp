#include "customer.h"
#include "utils.h"
#include <iostream>
#include <cstdint>
#include <memory>

customer::customer() : id() {};

customer::customer(unsigned int id_in, char *requests_file_name) : id(id_in), status(Status::Unavailable), matched(false)
{
    // load in requests queue from a file
    this->requests = create_requests(requests_file_name);
}

void customer::ready_up()
{
    // if no more requests, mark as finished
    if (this->requests.empty())
    {
        this->status = Status::Finished;
        return;
    }

    // reset status from previous request, and get the next request
    this->has_pizza = false;
    this->matched = false;
    this->location = this->requests.front();
    this->requests.pop_front();

    customer_ready(this->id, this->location);
    this->status = Status::Available; // mark as available
}

bool customer::is_ready()
{ // Check if a customer has a pending request
    return this->status == Status::Available;
}

bool customer::is_done()
{ // Check if a customer is finished with their requests
    return this->status == Status::Finished;
}

void customer::pay_driver(std::shared_ptr<driver> delivery_driver)
{ // Pay driver and mark driver as paid
    pay(this->id, delivery_driver->get_id());
}

unsigned int customer::get_id()
{
    return this->id;
}

Status customer::get_status()
{
    return this->status;
}

location_t customer::get_location()
{
    return this->location;
}

void customer::set_unavailable()
{
    this->status = Status::Unavailable;
}

void customer::receive_pizza()
{
    this->has_pizza = true;
}

bool customer::check_pizza()
{
    return this->has_pizza;
}

void customer::set_driver(unsigned int driver_id)
{
    this->assigned_driver = driver_id;
    this->matched = true;
}

unsigned int customer::get_driver()
{
    return this->assigned_driver;
}

bool customer::is_matched()
{
    return this->matched;
}