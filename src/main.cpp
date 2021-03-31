#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/bind/bind.hpp>
#include <boost/coroutine/detail/coroutine_context.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <iostream>
#include <thread>

// clang format
// find src/ -regex '.*\.\(cpp\|hpp\|cc\|cxx\)' -exec clang-format -style=file -i {} \;

void foo(boost::asio::steady_timer& timer,
         boost::asio::yield_context yield,
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
    boost::asio::post(strand, []() {

    });
  });
  // Pass the same strand to the thread, so that the thread may post
  // handlers synchronized with the foo coroutine.
  std::thread t(&bar, std::ref(strand), std::ref(timer));

  io_context.run();
  t.join();
}
