#include <iostream>
#include <vector>
#include <deque>
#include <fstream>
#include <memory>
#include "../cpu.h"
#include "../thread.h"
#include "../mutex.h"
#include "../cv.h"
#include "pizza.h"
#include "customer.h"
#include "driver.h"
#include "utils.h"

// mutex for operations on driver and customer vectors
mutex pizza_lock;

// cv for each driver and customer
std::vector<cv> driver_cv;
std::vector<cv> customer_cv;

// Containers for customers and drivers
std::vector<std::shared_ptr<customer>> customers;
std::vector<std::shared_ptr<driver>> drivers;

// cv for manager
cv manager_cv = cv();

/**
 * @brief Have a driver deliver a pizza
 *
 * @param arg pointer to a driver
 */
void run_driver(uintptr_t arg)
{
    driver *delivery_driver = reinterpret_cast<driver *>(arg);
    unsigned int driver_id = delivery_driver->get_id();

    // Initialize driver to be ready, and signal the manager that they are ready
    delivery_driver->ready_up();
    manager_cv.signal();

    while (true)
    {
        pizza_lock.lock();
        while (!delivery_driver->is_matched())
        {
            driver_cv[driver_id].wait(pizza_lock);
        }
        unsigned int customer_id = delivery_driver->get_customer();
        auto recipient = customers[customer_id];
        pizza_lock.unlock();

        // Run delivery concurrently
        delivery_driver->deliver_pizza(recipient);

        pizza_lock.lock();
        delivery_driver->set_location(recipient->get_location()); // Update driver location
        recipient->receive_pizza();
        customer_cv[customer_id].signal(); // Signal customer that delivery is complete

        while (!delivery_driver->is_paid())
        { // Driver cannot ready up until they are paid
            driver_cv[driver_id].wait(pizza_lock);
        }

        delivery_driver->ready_up();
        manager_cv.signal(); // Signal manager that a driver is ready
        pizza_lock.unlock();
    }
}

/**
 * @brief Have a customer pay for a pizza
 *
 * @param arg pointer to a customer
 */
void run_customer(uintptr_t arg)
{
    customer *recipient = reinterpret_cast<customer *>(arg);
    unsigned int customer_id = recipient->get_id();

    // Initialize customer to be ready, and signal the manager that they are ready
    recipient->ready_up();
    manager_cv.signal();

    pizza_lock.lock();
    while (!recipient->is_done())
    {
        while (!recipient->check_pizza() || !recipient->is_matched())
        { // Customer cannot pay until they have received the pizza and they have a driver
            customer_cv[customer_id].wait(pizza_lock);
        }
        unsigned int driver_id = recipient->get_driver();
        auto delivery_driver = drivers[driver_id];

        // Pay driver and mark driver as paid
        recipient->pay_driver(delivery_driver);
        delivery_driver->recieve_payment();

        driver_cv[driver_id].signal(); // Signal the driver that they have been paid
        recipient->ready_up();
        manager_cv.signal(); // Signal the manager that a customer is ready
    }
    pizza_lock.unlock();
}

/**
 * @brief run pizza deliveries until no requests are left
 *        - a thread is created for each driver and customer
 *
 * @param args command line arguments
 *             Usage: ./program <numDrivers> <customerInput...>"
 */
void run_pizza_manager(uintptr_t args)
{
    Args cmd_line_args = *reinterpret_cast<Args *>(args);

    int argc = cmd_line_args.argc;
    char **argv = cmd_line_args.argv;

    // Get number of drivers and customers from command line args
    auto [num_drivers, num_customers] = load_input(argc, argv);

    // Resize the cvs so each driver and customer has their own
    driver_cv.resize(num_drivers);
    customer_cv.resize(num_customers);

    drivers.reserve(num_drivers);
    customers.reserve(num_customers);

    // Create drivers and customers
    for (int i = 0; i < num_drivers; i++)
    {
        std::shared_ptr<driver> driver_ptr = std::make_shared<driver>(driver(i));
        drivers.push_back(driver_ptr);
        thread(run_driver, reinterpret_cast<uintptr_t>(driver_ptr.get()));
    }
    for (int i = 2; i < argc; i++)
    { // Load in each customer requests from the file specified in command line
        std::shared_ptr<customer> customer_ptr =
            std::make_shared<customer>(customer(static_cast<unsigned int>(i - 2), argv[i]));
        customers.push_back(customer_ptr);
        thread(run_customer, reinterpret_cast<uintptr_t>(customer_ptr.get()));
    }

    while (true)
    {
        pizza_lock.lock();
        while (!has_available_delivery(customers, drivers))
        // Wait until atleast a driver and customer is available before matching
        {
            manager_cv.wait(pizza_lock);
        }

        auto [pizza_recipient, pizza_deliverer] = match_delivery(customers, drivers);

        // Mark matched driver and customer as unavailable;
        pizza_deliverer->set_unavailable();
        pizza_recipient->set_unavailable();

        unsigned int customer_id = pizza_recipient->get_id();
        unsigned int driver_id = pizza_deliverer->get_id();

        // Store the driver's customer and vice versa
        pizza_deliverer->set_customer(customer_id);
        pizza_recipient->set_driver(driver_id);

        // Signal the driver and customer to start
        driver_cv[driver_id].signal();
        customer_cv[customer_id].signal();

        pizza_lock.unlock();
    }
}

int main(int argc, char *argv[])
{
    Args args = Args(argc, argv);
    cpu::boot(1, run_pizza_manager, reinterpret_cast<uintptr_t>(&args), 0, 0, 0);
};