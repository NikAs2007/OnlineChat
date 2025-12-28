#include <asio.hpp>
#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>

using std::cout;
using std::cin;
using std::endl;
using std::string;
using std::thread;
using std::mutex;
using std::lock_guard;
using std::atomic;

using asio::ip::tcp;

void receive_messages(tcp::socket& socket, atomic<bool>& running) {
    try {
        while (running) {
            char buffer[1024];
            asio::error_code ec;

            size_t len = socket.read_some(asio::buffer(buffer, sizeof(buffer)), ec);

            if (ec) {
                if (ec == asio::error::eof) {
                    cout << "\nСоединение закрыто сервером" << endl;
                }
                else {
                    cout << "\nОшибка чтения: " << ec.message() << endl;
                }
                running = false;
                break;
            }

            if (len > 0) {
                string message(buffer, len);
                cout << message;
                cout.flush();
            }
        }
    }
    catch (const std::exception& e) {
        cout << "\nИсключение в потоке приема: " << e.what() << endl;
        running = false;
    }
}

void send_messages(tcp::socket& socket, atomic<bool>& running) {
    try {
        while (running) {
            string message;
            std::getline(cin, message);

            if (!running) break;

            if (message == "exit") {
                running = false;
                break;
            }

            if (!message.empty()) {
                // Добавляем перевод строки
                message += "\n";
                asio::error_code ec;
                socket.write_some(asio::buffer(message.data(), message.size()), ec);

                if (ec) {
                    cout << "Ошибка отправки: " << ec.message() << endl;
                    running = false;
                    break;
                }
            }
        }
    }
    catch (const std::exception& e) {
        cout << "\nИсключение в потоке отправки: " << e.what() << endl;
        running = false;
    }
}

int main() {
    setlocale(LC_ALL, "Ru");
    try {
        asio::io_context io_context;
        tcp::socket socket(io_context);

        cout << "Подключение к серверу..." << endl;
        socket.connect(tcp::endpoint(asio::ip::make_address("127.0.0.1"), 45555));

        cout << "Подключено к серверу!" << endl;
        cout << "Для выхода введите 'exit' или нажмите Ctrl+C\n" << endl;

        atomic<bool> running(true);
        thread receiver(receive_messages, std::ref(socket), std::ref(running));
        thread sender(send_messages, std::ref(socket), std::ref(running));

        // Ждем завершения потоков
        receiver.join();
        sender.join();

        if (socket.is_open()) {
            socket.close();
        }

        cout << "Клиент завершен." << endl;
    }
    catch (const std::exception& e) {
        cout << "Ошибка: " << e.what() << endl;
    }

    return 0;
}