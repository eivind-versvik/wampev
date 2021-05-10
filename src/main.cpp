#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/bind/bind.hpp>
#include <boost/coroutine/detail/coroutine_context.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <iostream>
#include <thread>

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <cstdlib>
#include <iostream>
#include <string>

namespace beast = boost::beast;          // from <boost/beast.hpp>
namespace http = beast::http;            // from <boost/beast/http.hpp>
namespace websocket = beast::websocket;  // from <boost/beast/websocket.hpp>
namespace net = boost::asio;             // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;        // from <boost/asio/ip/tcp.hpp>

// Report a failure
void fail(beast::error_code ec, char const* what) {
  std::cerr << what << ": " << ec.message() << "\n";
}

// Sends a WebSocket message and prints the response
void do_session(std::string host,
                std::string const& port,
                std::string const& text,
                net::io_context& ioc,
                const net::yield_context& yield) {
  beast::error_code ec;

  // These objects perform our I/O
  tcp::resolver resolver(ioc);
  websocket::stream<beast::tcp_stream> ws(ioc);

  // Look up the domain name
  auto const results = resolver.async_resolve(host, port, yield[ec]);
  if (ec) {
    return fail(ec, "resolve");
  }
  // Set a timeout on the operation
  beast::get_lowest_layer(ws).expires_after(std::chrono::seconds(30));

  // Make the connection on the IP address we get from a lookup
  auto ep = beast::get_lowest_layer(ws).async_connect(results, yield[ec]);
  if (ec) {
    return fail(ec, "connect");
  }

  // Update the host_ string. This will provide the value of the
  // Host HTTP header during the WebSocket handshake.
  // See https://tools.ietf.org/html/rfc7230#section-5.4
  host += ':' + std::to_string(ep.port());

  // Turn off the timeout on the tcp_stream, because
  // the websocket stream has its own timeout system.
  beast::get_lowest_layer(ws).expires_never();

  // Set suggested timeout settings for the websocket
  ws.set_option(
      websocket::stream_base::timeout::suggested(beast::role_type::client));

  // Set a decorator to change the User-Agent of the handshake
  ws.set_option(
      websocket::stream_base::decorator([](websocket::request_type& req) {
        req.set(
            http::field::user_agent,
            std::string(BOOST_BEAST_VERSION_STRING) + " websocket-client-coro");
      }));

  // Perform the websocket handshake
  ws.async_handshake(host, "/", yield[ec]);
  if (ec) {
    return fail(ec, "handshake");
  }

  // Send the message
  ws.async_write(net::buffer(std::string(text)), yield[ec]);
  if (ec) {
    return fail(ec, "write");
  }

  // This buffer will hold the incoming message
  beast::flat_buffer buffer;

  // Read a message into our buffer
  ws.async_read(buffer, yield[ec]);
  if (ec) {
    return fail(ec, "read");
  }

  // Close the WebSocket connection
  ws.async_close(websocket::close_code::normal, yield[ec]);
  if (ec) {
    return fail(ec, "close");
  }

  // If we get here then the connection is closed gracefully

  // The make_printable() function helps print a ConstBufferSequence
  std::cout << beast::make_printable(buffer.data()) << std::endl;
}

int main2(int argc, char** /* unused */) {
  // Check command line arguments.
  if (argc != 4) {
    std::cerr << "Usage: websocket-client-coro <host> <port> <text>\n"
              << "Example:\n"
              << "    websocket-client-coro echo.websocket.org 80 \"Hello, "
                 "world!\"\n";
    return EXIT_FAILURE;
  }
  auto const host = "127.0.0.1";
  auto const port = "8086";
  auto const text = "hey";

  // The io_context is required for all I/O
  net::io_context ioc;

  // Launch the asynchronous operation
  boost::asio::spawn(
      ioc, std::bind(&do_session, std::string(host), std::string(port),
                     std::string(text), std::ref(ioc), std::placeholders::_1));

  // Run the I/O service. The call will return when
  // the socket is closed.
  ioc.run();

  return EXIT_SUCCESS;
}

// clang format
// find src/ -regex '.*\.\(cpp\|hpp\|cc\|cxx\)' -exec clang-format -style=file
// -i {} \;

void foo(boost::asio::steady_timer& timer,
         const boost::asio::yield_context& yield,
         boost::asio::io_context& io_context) {
  std::cout << "Enter foo" << std::endl;

  timer.expires_from_now(
      boost::asio::steady_timer::clock_type::duration::max());
  boost::system::error_code error;
  timer.async_wait(yield[error]);

  std::cout << "foo error: " << error.message() << std::endl;

  boost::asio::steady_timer timer2(io_context);

  timer2.expires_from_now(std::chrono::milliseconds(2000));

  timer2.async_wait(yield[error]);

  std::cout << "Leave foo" << std::endl;
}

void bar(boost::asio::strand<boost::asio::io_context::executor_type>& strand,
         boost::asio::steady_timer& timer) {
  std::cout << "Enter bar" << std::endl;

  // Wait a little for asio::io_service::run to be executed
  std::this_thread::sleep_for(std::chrono::seconds(5));
  // Post timer cancellation into the strand.

  boost::asio::post(strand, [&timer]() { timer.cancel(); });

  std::cout << "Leave bar" << std::endl;
}

int main() {
  try {
    boost::asio::io_context io_context;
    boost::asio::steady_timer timer(io_context);
    //  boost::asio::io_service::strand strand(io_service);

    boost::asio::strand<boost::asio::io_context::executor_type> strand(
        io_context.get_executor());

    // Use an explicit strand, rather than having the io_service create.
    boost::asio::spawn(strand,
                       std::bind(&foo, std::ref(timer), std::placeholders::_1,
                                 std::ref(io_context)));

    boost::asio::spawn(strand, [&](boost::asio::yield_context ctx) {
      /*boost::asio::post(strand, []() {

      });*/
    });
    // Pass the same strand to the thread, so that the thread may post
    // handlers synchronized with the foo coroutine.
    std::thread t(&bar, std::ref(strand), std::ref(timer));

    io_context.run();
    t.join();
  } catch (const std::exception& e) {
    exit(-1);
  } catch (...) {
    exit(-1);
  }
}
