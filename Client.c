#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

#define PORT 8080
#define totalThreads 2

int chunkSize;
int* primes;
int primeCount = 0;
pthread_mutex_t mutex;

bool is_prime(int n) {
    if (n <= 1) return false;
    if (n <= 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    for (int i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) return false;
    }
    return true;
}

typedef struct {
    int start;
} unitThread;

void* printPrimes(void* param) {
    unitThread* data = (unitThread*)param;
    for (int i = 0; i < chunkSize; i++) {
        int number = data->start + i;
        if (is_prime(number)) {
            pthread_mutex_lock(&mutex);  // Lock for thread-safe access
            primes[primeCount++] = number;
            pthread_mutex_unlock(&mutex);  // Unlock after writing
        }
    }
    return NULL;
}

int main() {
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    
    // Initialize mutex
    pthread_mutex_init(&mutex, NULL);

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket to the port
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    while (1) {
        // Accept incoming connection
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        // Read total number of integers from the client
        valread = read(new_socket, buffer, 1024);
        int totalNumbers = atoi(buffer);
        printf("Total numbers received from client: %d\n", totalNumbers);

        chunkSize = totalNumbers / totalThreads;
        primes = (int*)malloc(totalNumbers * sizeof(int));

        // Create threads to find primes
        pthread_t threadHandle[totalThreads];
        unitThread thread[totalThreads];
        int startPoint[totalThreads];

        for (int i = 0; i < totalThreads; i++) {
            startPoint[i] = i * chunkSize;
            thread[i].start = startPoint[i];
            pthread_create(&threadHandle[i], NULL, printPrimes, &thread[i]);
        }

        // Wait for threads to finish
        for (int i = 0; i < totalThreads; i++) {
            pthread_join(threadHandle[i], NULL);
        }

        // Send prime numbers back to client
        char primeStr[10000] = {0}; // Buffer for prime numbers
        for (int i = 0; i < primeCount; i++) {
            char temp[12];
            sprintf(temp, "%d ", primes[i]);
            strcat(primeStr, temp);
        }
        send(new_socket, primeStr, strlen(primeStr), 0);
        printf("Prime numbers sent to client.\n");

        // Clean up
        free(primes);
        primeCount = 0;
        close(new_socket);
    }

    // Destroy mutex
    pthread_mutex_destroy(&mutex);

    return 0;
}
