#include <iostream>

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

using std::cout;
using std::endl;

void startServer() {
    const int PORT = 8080;
    const int BUFFER_SIZE = 1024;

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0) {
        std::cerr << "Failed to create socket" << endl;

        exit(1);
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Bind failed" << endl;
        close(server_fd);

        exit(1);
    }

    if (listen(server_fd, 3) < 0) {
        std::cerr << "Listen failed" << endl;
        close(server_fd);

        exit(1);
    }

    std::cout << "Echo server is listening on port " << PORT << "..." << endl;

    sockaddr_in client_addr { };

    while (true) {
        socklen_t addrlen = sizeof(server_addr);
        int client_socket = accept(server_fd, (struct sockaddr *) &server_addr, &addrlen);

        if (client_socket < 0) {
            std::cerr << "Failed to accept connection\n";

            continue;
        }

        std::cout << "Client connected from " << inet_ntoa(client_addr.sin_addr) << "\n";

        // Read incoming request data (for simple demonstration, we won't fully parse it)
        char buffer[1024] = { 0 };
        auto bytesCount = read(client_socket, buffer, 1024);
        std::cout << "Received Request:\n" << buffer << "\n";

        // 5. Formulate a raw HTTP response
        std::string http_response = 
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 20\r\n"
            "\r\n"
            "Hello from native!\r\n";

        // Send response and close connection
        auto bytesWritten = write(client_socket, http_response.c_str(), http_response.size());
        close(client_socket);
    }

    close(server_fd);
}
