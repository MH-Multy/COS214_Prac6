#TIVIDY

TIVIDY is a generic workflow management system designed to define, execute and manage workflows. The system will allow organisations to create, execute, manage and assign work to appropriate participants. It will also allow organisations to record and change the workflow requirements at any time depending on their need. The overall design will not be bound to any one organisation but rather customized to fit their need. 

---

## Contributors
* **Nokhaya Foloti**
* **Emmanuel Boateng**
* **Mohammad H Jafari**
* **Zander Spies**
* **Lebogang Nxumalo** 

---

## Build & Run Instructions

### Prerequisites
* A C++ compiler supporting C++11
* GNU `make`
* Docker and Docker Compose (optional for containerized runs)
* Doxygen (optional for documentation generation)

---

### 1. Build & Run Locally

#### Compile the Main Application
```bash
make
```

#### Run the Interactive Application
```bash
./main
# or: make run
```

#### Run Automated Unit & Integration Tests
```bash
make test
```

#### Clean Build Artifacts
```bash
make clean
```

---

### 2. Build & Run with Docker

#### Launch the Interactive Application
```bash
docker compose run --rm app
```

#### Run Tests Inside the Container
```bash
docker compose run --rm app make test
```

#### Run Memory Leak Analysis (Valgrind)
```bash
docker compose run --rm app make valgrind
```

---

### 3. Generate Documentation (Doxygen)

Generate HTML class hierarchies and API reference docs:
```bash
doxygen Doxyfile
# or: make docs
```
Open `docs/html/index.html` in your browser.
