#include <asio.hpp>
#include <iostream>
#include <thread>
using namespace std;
using namespace asio;
using namespace asio::ip;


int main() {
	cout << "Start" << endl;
	io_context io_context;

	tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 45555));
	tcp::socket client1 = acceptor.accept();
	tcp::socket client2 = acceptor.accept();

	while (true) {
		char buffer[1024];
		size_t len1 = client1.read_some(asio::buffer(buffer, sizeof(buffer)));
		string ans1(buffer, len1);

		cout << "Client1: " << ans1 << endl;


		size_t len2 = client2.read_some(asio::buffer(buffer, sizeof(buffer)));
		string ans2(buffer, len2);

		cout << "Client2: " << ans2 << endl;

		client2.write_some(asio::buffer(ans1, ans1.size()));
		client1.write_some(asio::buffer(ans2, ans2.size()));
	}

	return 0;
}