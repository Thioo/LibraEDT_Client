#include "pch.h"
//// Client - Windows
//
//#include <iostream>
//#include <winsock2.h>
//#include <ws2tcpip.h>
//#include <cstring>
//
//#pragma comment(lib, "ws2_32.lib")
//
//int main()
//{
//	WSADATA wsaData;
//	int iResult;
//
//	// Initialize Winsock
//	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
//	if (iResult != 0) {
//		std::cerr << "WSAStartup failed with error: " << iResult << std::endl;
//		return 1;
//	}
//
//	// Create a socket
//	SOCKET ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
//	if (ConnectSocket == INVALID_SOCKET) {
//		std::cerr << "socket failed with error: " << WSAGetLastError() << std::endl;
//		WSACleanup();
//		return 1;
//	}
//
//	// Connect to the server
//	sockaddr_in clientService;
//	clientService.sin_family = AF_INET;
//	clientService.sin_addr.s_addr = inet_addr("192.168.1.111"); // IP address of the server
//	clientService.sin_port = htons(55555); // port number
//
//	iResult = connect(ConnectSocket, (SOCKADDR*)&clientService, sizeof(clientService));
//	if (iResult == SOCKET_ERROR) {
//		std::cerr << "connect failed with error: " << WSAGetLastError() << std::endl;
//		closesocket(ConnectSocket);
//		WSACleanup();
//		return 1;
//	}
//
//	// Send a message to the server
//	const char* sendbuf = "Take an image";
//	iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
//	if (iResult == SOCKET_ERROR) {
//		std::cerr << "send failed with error: " << WSAGetLastError() << std::endl;
//		closesocket(ConnectSocket);
//		WSACleanup();
//		return 1;
//	}
//
//	// Shutdown the connection
//	iResult = shutdown(ConnectSocket, SD_SEND);
//	if (iResult == SOCKET_ERROR) {
//		std::cerr << "shutdown failed with error: " << WSAGetLastError() << std::endl;
//		closesocket(ConnectSocket);
//		WSACleanup();
//		return 1;
//	}
//
//	// Cleanup
//	closesocket(ConnectSocket);
//	WSACleanup();
//
//	return 0;
//}
//
//
//
//// Server - Linux
//#include <iostream>
//#include <cstring>
//#include <sys/socket.h>
//#include <arpa/inet.h>
//#include <unistd.h>
//#include <dlfcn.h>
//
//void* lib_handle = nullptr;
//void (*grab_image)() = nullptr;
//
//void load_library()
//{
//	lib_handle = dlopen("libmpxhwrelaxd.so", RTLD_LAZY);
//	if (!lib_handle) {
//		std::cerr << "Failed to load library: " << dlerror() << std::endl;
//		return;
//	}
//
//	grab_image = (void(*)())dlsym(lib_handle, "grab_image");
//	if (!grab_image) {
//		std::cerr << "Failed to load symbol: " << dlerror() << std::endl;
//		dlclose(lib_handle);
//		lib_handle = nullptr;
//		return;
//	}
//}
//
//void unload_library()
//{
//	if (lib_handle) {
//		dlclose(lib_handle);
//		lib_handle = nullptr;
//	}
//}
//
//int main()
//{
//	int listenfd = socket(AF_INET, SOCK_STREAM, 0);
//	if (listenfd == -1) {
//		std::cerr << "Failed to create socket" << std::endl;
//		return 1;
//	}
//
//	sockaddr_in serverAddress{};
//	serverAddress.sin_family = AF_INET;
//	serverAddress.sin_addr.s_addr = INADDR_ANY;
//	serverAddress.sin_port = htons(55555);
//
//	if (bind(listenfd, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
//		std::cerr << "Failed to bind socket" << std::endl;
//		close(listenfd);
//		return 1;
//	}
//
//	if (listen(listenfd, 10) == -1) {
//		std::cerr << "Failed to listen on socket" << std::endl;
//		close(listenfd);
//		return 1;
//	}
//
//	load_library();
//
//	while (true) {
//		std::cout << "Waiting for client connection..." << std::endl;
//
//		int clientfd = accept(listenfd, nullptr, nullptr);
//		if (clientfd == -1) {
//			std::cerr << "Failed to accept client connection" << std::endl;
//			close(listenfd);
//			unload_library();
//			return 1;
//		}
//
//		std::cout << "Client connected" << std::endl;
//
//		// Read message from client
//		char buffer[256];
//		ssize_t bytesRead = read(clientfd, buffer, sizeof(buffer));
//		if (bytesRead <= 0) {
//			std::cerr << "Failed to read from client" << std::endl;
//			close(clientfd);
//			continue;
//		}
//
//		// Process the request
//		if (strcmp(buffer, "Take an image") == 0) {
//			if (grab_image) {
//				grab_image(); // Call the function from the loaded library
//				std::cout << "Image captured" << std::endl;
//			}
//		}
//
//		close(clientfd);
//	}
//
//	close(listenfd);
//	unload_library();
//
//	return 0;
//}
