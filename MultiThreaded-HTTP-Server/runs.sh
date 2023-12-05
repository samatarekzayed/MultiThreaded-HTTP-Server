#!/bin/bash

# Server address and port
SERVER_ADDRESS="127.0.0.1"
SERVER_PORT="8080"

# Number of clients to run
NUM_CLIENTS=60

# Run 10 instances of the client in the background
for ((i=1; i<=NUM_CLIENTS; i++)); do
    ./client "$SERVER_ADDRESS" "$SERVER_PORT" &  # Replace with your actual client command
done

# Wait for all background processes to finish
wait

echo "All 60 clients have finished."