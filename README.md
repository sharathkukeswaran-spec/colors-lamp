# COLORS - LAMP Stack Web Application

A web-based color palette management tool built using the LAMP (Linux, Apache, MySQL, PHP) stack. Users can log in, add colors to their personal collection, and search through their saved colors.

## Technologies Used

- **Linux** - Server operating system
- **Apache** - Web server
- **MySQL** - Relational database for storing users and colors
- **PHP** - Server-side scripting for API endpoints
- **HTML/CSS/JavaScript** - Frontend interface
- **Google Fonts (Ubuntu)** - Typography

## Project Structure

```
colors-lamp/
├── api/
│   ├── config.sample.php     # Database config template (copy to config.php)
│   ├── Login.php             # User authentication endpoint
│   ├── AddColor.php          # Add a color to the database
│   └── SearchColors.php      # Search colors by name
├── public/
│   ├── index.html            # Login page
│   ├── color.html            # Main application page
│   ├── css/
│   │   └── styles.css        # Application styles
│   └── js/
│       ├── code.js           # Main application logic
│       └── md5.js            # MD5 hashing library
├── .gitignore
├── LICENSE.md
└── README.md
```

## Prerequisites

- A LAMP server (Linux, Apache, MySQL, PHP) or equivalent environment
- MySQL database with the following tables:

```sql
CREATE TABLE Users (
    ID INT AUTO_INCREMENT PRIMARY KEY,
    firstName VARCHAR(50),
    lastName VARCHAR(50),
    Login VARCHAR(50),
    Password VARCHAR(50)
);

CREATE TABLE Colors (
    ID INT AUTO_INCREMENT PRIMARY KEY,
    UserID INT,
    Name VARCHAR(50)
);
```

## Setup Instructions

1. **Clone the repository:**

   ```bash
   git clone https://github.com/sharathkukeswaran-spec/colors-lamp.git
   ```

2. **Configure the database connection:**

   ```bash
   cp api/config.sample.php api/config.php
   ```

   Edit `api/config.php` and fill in your MySQL database credentials:

   ```php
   $dbHost = "localhost";
   $dbUser = "your_username";
   $dbPass = "your_password";
   $dbName = "COP4331";
   ```

3. **Deploy to your web server:**

   Copy the project files to your Apache web root (e.g., `/var/www/html/`).

4. **Update the API URL:**

   In `public/js/code.js`, update the `urlBase` variable to point to your server:

   ```javascript
   const urlBase = 'http://your-server-address/LAMPAPI';
   ```

5. **Create the database and tables** using the SQL statements in the Prerequisites section.

6. **Insert a test user** into the Users table:

   ```sql
   INSERT INTO Users (firstName, lastName, Login, Password) VALUES ('Test', 'User', 'testuser', 'testpassword');
   ```

## How to Run and Access the Application

1. Ensure Apache and MySQL are running on your server.
2. Navigate to `http://your-server-address/public/index.html` in a web browser.
3. Log in with valid credentials.
4. Once logged in, you will be redirected to the color management page where you can:
   - **Search colors** - Type a color name (or partial name) and click "Search Color" to find matching entries.
   - **Add colors** - Type a new color name and click "Add Color" to save it to your collection.
5. Click "Log Out" to end your session.

## Assumptions and Limitations

- The application requires a working LAMP server with Apache configured to serve PHP files.
- Passwords are currently stored and transmitted in plain text (the MD5 hashing library is included but not active).
- User sessions are managed via browser cookies with a 20-minute expiration.
- Each user can only search and manage their own colors.
- The application does not include user registration; users must be added directly to the database.
- No HTTPS enforcement is configured by default.

## AI Usage Disclosure

Claude (by Anthropic) was used to assist with organizing the repository structure, sanitizing credentials from source files, and generating this README documentation. The core application logic was developed during COP 4331 lab sessions.

## License

This project is licensed under the MIT License. See [LICENSE.md](LICENSE.md) for details.
