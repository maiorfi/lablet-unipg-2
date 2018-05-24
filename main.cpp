#include "mbed.h"

#include "easy-connect.h"

// buffer sizes for socket-related operations (read/write)
#define SOCKET_SEND_BUFFER_SIZE 32
#define SOCKET_RECEIVE_BUFFER_SIZE 32

// WiFi access point SSID and PASSWORD
#define WIFI_SSID "**INSERT_HERE_ACCESS_POINT_SSID**"
#define WIFI_PASS "**INSERT_HERE_ACCESS_POINT_PASSWORD**"

/* host address and port of (sample) TCP server/listener
  * we will connect with
  */
#define TCP_SERVER_ADDRESS "ws.mqtt.it"
#define TCP_SERVER_PORT 8888

// definition of led1 object of DigitalOut class
static DigitalOut led1(LED1, false);

static InterruptIn btn(BUTTON1);

/* reference to "NetworkInterface" object that will provide
  * for network-related operation (connect, read/write, disconnect)
  */
static NetworkInterface *s_network;

/* Thread/EventQueue pair that will manage network operations
  * This is done in order to move into a queue the program instructions
  * to be executed after the interrupt of the network operations.
  * EventQueue ensures that "atomic" operations will not be interrupted
  * until completion
  */
static Thread s_thread_manage_network;
static EventQueue s_eq_manage_network;

/* definition of a new enum type which includes:
  * 1) a set of states for a simple finite-state-machine
  * whose main purpose is to keep network connection
  * "as much open as possible" (automatic reconnect)
  * 2) the ConnectionState alias
  */
typedef enum _ConnectionState
{
    NETWORK_STATE_DISCONNECTED,
    NETWORK_STATE_CONNECTED
} ConnectionState;

/* a new "enum _ConnectionState" named "s_connectionState"
  * and initialized to the current state "disconnected"
  */
static ConnectionState s_connectionState = NETWORK_STATE_DISCONNECTED;

// this callback (scheduled every 5 secs) implements network reconnect
policy void event_proc_manage_network_connection()
{
    if (s_connectionState == NETWORK_STATE_CONNECTED)
        return;

    // print debug text to the default output console (virtual COM)
    printf("> Initializing Network...\n");

    /* easy-connect library provides for a "all-in-one"
      * easy-connect(log, ssid, pass) function
      */
    s_network = easy_connect(true, WIFI_SSID, WIFI_PASS);

    /* this statement could be joined with the previous
      * static NetworkInterface* s_network;
      * as in the following statement:
      * static NetworkInterface* s_network
      *        = easy_connect(true, WIFI_SSID, WIFI_PASS);
      * This is what can be found in some mbed OS examples:
      * the network is started before the execution of the main().
      * However in our case we preferred to define s_network
      * in the preamble of the program and actually start
      * the network in the main() where the eventqueque
      * for this function is scheduled.
      */

    if (!s_network)
    {
        printf("> ...connection FAILED\n");
        return;
    }

    printf("> ...connection SUCCEEDED\n");

    s_connectionState = NETWORK_STATE_CONNECTED;
}

/* this callback (scheduled every second) actually implements
  * a request+reply transaction sample
  */
void event_proc_send_and_receive_data(const char *message_type)
{
    if (s_connectionState != NETWORK_STATE_CONNECTED)
        return;

    // definition of a message counter we will use later
    static int message_counter = 0;

    /* Here we create an uninitialized socket named socket.
      * Must call open to initialize the socket on a network stack
      */
    TCPSocket socket;

    /* definition of a "socket_operation_return_value"
      * as an enum of the nsapi_error_t type
      */
    nsapi_error_t socket_operation_return_value;

    /* debug print of the TCP request message
      * (specifies we are requiring access)
      */
    printf("Sending TCP request to %s:%d...\n", TCP_SERVER_ADDRESS,
           TCP_SERVER_PORT);

    // step 1/2: request (open, connect, send...close and return on error)
    socket.set_timeout(3000);
    socket.open(s_network);
    /* The “open” method is located in socket.h,
      * but the argument is “NetworkStack *stack”
      * But we previously defined "static NetworkInterface* s_network;"
      * NetworkStack derives from NetworkInterface
      */

    /* Initialization of the object “socket_operation_return_value”
      * with the return value of the “connect” method
      * of the socket object
      */
    socket_operation_return_value = socket.connect(TCP_SERVER_ADDRESS,
                                                   TCP_SERVER_PORT);

    /* if the value is not correct then debug print
      * the error and disconnect
      */
    if (socket_operation_return_value != 0)
    {
        printf("...error in socket.connect(): %d\n",
               socket_operation_return_value);
        socket.close();
        s_connectionState = NETWORK_STATE_DISCONNECTED;
        return;
    }

    /* definition of a “sbuffer” char vector
      * whose length has been previously defined
      */
    char sbuffer[SOCKET_SEND_BUFFER_SIZE];

    // convert to "sbuffer" string the message and increment counter
    sprintf(sbuffer, "%s #%d\r", message_type, ++message_counter);

    /* initialization of the "size" enum (belonging to the
      * enum nsapi_size_t) with the number of characters
      * passed to the buffer
      */
    nsapi_size_t size = strlen(sbuffer);

    socket_operation_return_value = 0;

    while (size)
    {
        /* Here we want to implement a series of partial character
          * string delivery. We know that sbuffer[] is a char vector
          * and "sbuffer" is the pointer to the 1st char of sbuffer[].
          * => "sbuffer+delta" is the pointer to the "delta-th" char
          * of sbuffer[] (numbering characters from zero).
          * The following compact C++ instruction sends to
          * the other "peer" (the node-red listener in our case)
          * the content of the string pointed by sbuffer
          * with an offset (starting position in the string)
          * = socket_operation_return_value
          * which is the the quantity of bytes previously sent
          * with a total of "size" bytes
          * while size is decremented at every partial delivery
          */
        socket_operation_return_value = socket.send(sbuffer +
                                                        socket_operation_return_value,
                                                    size);

        if (socket_operation_return_value < 0)
        {
            printf("...error sending data: %d\n",
                   socket_operation_return_value);
            socket.close();
            return;
        }
        else
        {
            // calculate the new size of the remaining string to be sent
            size -= socket_operation_return_value;
            printf("...sent:%d bytes\n", socket_operation_return_value);
        }
    }

    // step 2/2: receive reply (receive, close...close and return on error)
    /* definition of a “rbuffer” char vector
      * whose length has been previously defined
      */
    char rbuffer[SOCKET_RECEIVE_BUFFER_SIZE];

    socket_operation_return_value = socket.recv(rbuffer, sizeof rbuffer);

    if (socket_operation_return_value < 0)
    {
        printf("...error receiving data: %d\n",
               socket_operation_return_value);
    }
    else
    {
        // clear CR/LF chars for a cleaner debug terminal output
        rbuffer[socket_operation_return_value] = '\0';
        if (rbuffer[socket_operation_return_value - 1] == '\n' ||
            rbuffer[socket_operation_return_value - 1] == '\r')
            rbuffer[socket_operation_return_value - 1] = '\0';
        if (rbuffer[socket_operation_return_value - 2] == '\n' ||
            rbuffer[socket_operation_return_value - 2] == '\r')
            rbuffer[socket_operation_return_value - 2] = '\0';

        printf("...received: '%s'\n", rbuffer);
    }

    socket.close();

    // id led is toggling everything is working as expected
    led1.write(!led1.read());
}

/* in case of a hardware interrupt, the isr routine
  * schedules (on EventQueue dedicated to network operations)
  * a call to event_proc_send_and_receive_data(), but with
  * an argument ("btn") different from the one used in
  * periodic request+reply ("test", see below)
  */
void btn_interrupt_handler()
{
    s_eq_manage_network.call(event_proc_send_and_receive_data, "btn");
}

int main()
{
    // Let's start network connection task as soon as possible...
    s_eq_manage_network.call(event_proc_manage_network_connection);

    // ...then schedule a periodic check every 5 secs
    s_eq_manage_network.call_every(5000,
                                   event_proc_manage_network_connection);

    // send "test" data every second
    s_eq_manage_network.call_every(1000,
                                   event_proc_send_and_receive_data, "test");

    btn.fall(&btn_interrupt_handler);

    s_thread_manage_network.start(callback(&s_eq_manage_network,
                                           &EventQueue::dispatch_forever));
}
