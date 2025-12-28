#include <asio.hpp>
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <memory>
#include <algorithm>

using std::cout;
using std::endl;
using std::string;
using std::thread;
using std::vector;
using std::mutex;
using std::lock_guard;
using std::make_shared;
using std::remove_if;

using asio::ip::tcp;

mutex clients_mutex;
vector<std::shared_ptr<tcp::socket>> clients;

void broadcast_message(const string& message, tcp::socket* sender = nullptr) {
    lock_guard<mutex> lock(clients_mutex);

    for (auto& client : clients) {
        if (client && client->is_open() && client.get() != sender) {
            try {
                client->write_some(asio::buffer(message));
            }
            catch (...) {
                // Игнорируем ошибки отправки
            }
        }
    }
}

void handle_client(tcp::socket socket, int id) {
    cout << "Клиент " << id << " подключен" << endl;

    auto client_socket = make_shared<tcp::socket>(std::move(socket));

    {
        lock_guard<mutex> lock(clients_mutex);
        clients.push_back(client_socket);
    }

    try {
        while (true) {
            char buffer[1024];
            asio::error_code ec;
            size_t len = client_socket->read_some(asio::buffer(buffer, sizeof(buffer)), ec);

            if (ec) {
                if (ec == asio::error::eof) {
                    cout << "Клиент " << id << " отключился" << endl;
                }
                break;
            }

            if (len > 0) {
                string message(buffer, len);

                // Убираем символ новой строки, если есть
                if (!message.empty() && message.back() == '\n') {
                    message.pop_back();
                }

                string formatted_msg = "Клиент " + std::to_string(id) + ": " + message + "\n";
                cout << formatted_msg;

                // Отправляем всем остальным клиентам
                broadcast_message(formatted_msg, client_socket.get());
            }
        }
    }
    catch (const std::exception& e) {
        cout << "Ошибка с клиентом " << id << ": " << e.what() << endl;
    }

    // Удаляем клиента из списка
    {
        lock_guard<mutex> lock(clients_mutex);
        auto it = std::remove_if(clients.begin(), clients.end(),
            [&](auto& ptr) { return ptr.get() == client_socket.get(); });
        clients.erase(it, clients.end());
    }
}

int main() {
    setlocale(LC_ALL, "Ru");
    try {
        asio::io_context io_context;
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 45555));

        cout << "Сервер запущен на порту 45555" << endl;

        vector<thread> client_threads;
        int client_id = 1;

        while (true) {
            tcp::socket socket(io_context);
            acceptor.accept(socket);

            client_threads.emplace_back(handle_client, std::move(socket), client_id++);

            // Очищаем завершенные потоки
            client_threads.erase(
                std::remove_if(client_threads.begin(), client_threads.end(),
                    [](thread& t) { return !t.joinable(); }),
                client_threads.end()
            );
        }

        for (auto& t : client_threads) {
            if (t.joinable()) t.join();
        }
    }
    catch (const std::exception& e) {
        cout << "Ошибка сервера: " << e.what() << endl;
    }

    return 0;
}