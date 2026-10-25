FROM ubuntu:24.04

# 1. Prevent interactive time zone prompts during apt-get
ENV DEBIAN_FRONTEND=noninteractive

# 2. Install compiler, build tools, and debugging utilities
# (This layer is cached by Docker and only downloads once)
RUN apt-get update && apt-get install -y \
    build-essential \
    g++ \
    make \
    valgrind \
    gdb \
    && rm -rf /var/lib/apt/lists/*

# 3. Set the working directory inside the container
WORKDIR /app

# 4. Copy source files, headers, and Makefile into the image
COPY . .

# 5. Build and execute on container startup
CMD ["sh", "-c", "make clean && make && ./main"]