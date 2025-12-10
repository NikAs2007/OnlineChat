#include <asio.hpp>
#include <iostream>
#include <string>
using namespace std;
using namespace asio;
using namespace asio::ip;


int main() {
	io_context io_context;

	tcp::socket me(io_context);
	me.connect(tcp::endpoint(make_address("192.168.241.230"), 45555));

	while (true) {
		string mess;
		getline(cin, mess);

		me.write_some(asio::buffer(mess.data(), mess.size()));

		char buffer[1024];
		size_t len = me.read_some(asio::buffer(buffer, sizeof(buffer)));
		string ans(buffer, len);

		cout << "-" << ans << endl;
	}

	return 0;
}