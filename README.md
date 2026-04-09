# Bus Reservation System

A complete bus ticket booking and management system featuring Object-Oriented Programming in C++ and a Python/Flask web interface.

Developed for `BACSE104`: Structured and Object Oriented Programming - Digital Assignment 2

## 🚀 Features

- **Route Management:** Add and list bus routes with dynamic seat capacity.
- **Seat Management:** Real-time seat tracking with double-booking prevention.
- **Ticketing:** Generate unique Ticket IDs, book multiple seats, and calculate total fare.
- **Cancellation:** Ticket-based cancellation with automatic seat release.
- **Revenue Analytics:** 
  - Revenue per route.
  - Booking counts.
  - Identification of the most popular route.
- **Data Persistence:** Uses `fstream` to store routes and tickets in `.dat` files.

## 🏗️ Architecture
- **Backend (C++):** Core logic implemented using classes (`Route`, `Ticket`, `ReservationSystem`). Handles file I/O and business logic.
- **Frontend (Python/Flask):** Acts as a wrapper, using `subprocess` to communicate with the C++ executable.

## 🛠️ Local Setup
### Prerequisites
- G++ (C++ Compiler)
- Python 3.x

### Installation
1. **Clone the repository:**
   ```bash
   git clone https://github.com/0xMrNight/bus-reservation
   cd bus-reservation
   ```

2. **Install the requirements and compile the program**
   This will compile the C++ backend into an `bus_booking` executable and install Flask dependencies.
   ```bash
   pip install -r requirements.txt
   g++ main.cpp -o bus_booking
   ```

3. **Start the application:**
   ```bash
   python app.py 
   # or
   gunicorn app:app
   ```
4. Open your browser and navigate to `http://127.0.0.1:8000`.