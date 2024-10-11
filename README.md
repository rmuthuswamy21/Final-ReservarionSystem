# Program README

## Steps to Compile and Run:

1. Navigate to the project directory:
   ```sh
   cd finalcode
   ```

2. Clean previous outputs (if any):
   ```sh
   make clean
   ```

3. Compile the project:
   ```sh
   make
   ```

4. Run the program:
   ```sh
   ./program
   ```

## Steps to Use the Program:

To use the program, follow the menu after logging in. Logins are essential for running the program. Here are the default logins with their budgets:

### Default User Accounts (from `user.txt`):
```
User      Password    UserType  Budget
john      pass123     0         1000.00
alice     pass456     1         1000.00
cityuser  citypass    2         1000.00
org       orgpass     3         1000.00
sarah     sarahpass   0         800.00
mike      mikepass    1         1200.00
```

### Account Format:
- `User` - Username
- `Password` - Password
- `UserType` - Type of user, where:
  - `0` - Resident
  - `1` - Non-Resident
  - `2` - City
  - `3` - Organization
- `Budget` - Initial budget for the user

### User Details:
- A user has a username, password, user type (one of: resident, non-resident, city, or organization), and a budget.

### Facility Manager:
The facility manager is a key component responsible for approving and checking all event requests and payments. This class handles the creation, planning, payment, and persistence of events.

Follow the menu options provided to each user based on their role as specified.

---

Enjoy using the program!

