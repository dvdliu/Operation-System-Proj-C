class os_guard
{
// A guard class that uses RAII to ensure no interrupts and protection against multiple
// processors during the object lifetime
public:
    os_guard();         // disables interrupts, busy waits for guard
    ~os_guard();        // enables interrupts, sets guard to 0

    // Disable copy constructor and copy assignment
    os_guard(const os_guard&) = delete;
    os_guard& operator=(const os_guard&) = delete;
};