#include<iostream>
#include<string>
#include<fstream>
#include<vector>
#include<cstdlib>  
#include<limits>
#include<sstream>
#include<iomanip>
#include<cctype>
#include<cerrno>
#include<functional>
#include<chrono>
#include<ctime>
#ifdef _WIN32
#include <conio.h>
#include <direct.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif
using namespace std;


// ── Colour helpers ──────────────────────────────────────────────
void red()    { cout << "\033[1;31m"; }
void yellow() { cout << "\033[1;33m"; }
void blue()   { cout << "\033[1;34m"; }
void green()  { cout << "\033[1;32m"; }
void purple() { cout << "\033[1;35m"; }
void white()  { cout << "\033[1;37m"; }  

class BankAccount {
private:
    static const string bankname;
    static const string bankprefix;   // prefix for account numbers  (was "bankcode")
    static const string accountsDir;
    static constexpr double MIN_BALANCE = 500.0;
    static constexpr int COUNTER_START = 1000;
    static constexpr int PASSWORD_MIN_LENGTH = 8;
    static constexpr int MAX_LOGIN_ATTEMPTS = 3;
    static constexpr double DEFAULT_INTEREST_RATE = 3.5;
    static constexpr int MAX_INTEREST_YEARS = 10;

    string accountholdername;
    string accountnumber;
    string accountpassword;
    string email;
    string phonenumber;
    double balance;
    vector<string> transactionhistory;

public:
    BankAccount();
    void savetofile();
    bool loadfromfile();

    void mainmenu();
    void createaccount();
    void logingbankaccount();
    void depositmoney();
    void withdrawmoney();
    void calculateInterest();
    void checkbalance();
    void accountdetails();
    void alltransactionhistory();
};

// Helper: format amount as fixed with 2 decimal places and commas
static string formatAmount(double amount)
{
    ostringstream oss;
    oss << fixed << setprecision(2) << amount;
    string s = oss.str();
    int dotPos = static_cast<int>(s.find('.'));
    int start = (s[0] == '-') ? 1 : 0;
    int insertPos = (dotPos == string::npos ? static_cast<int>(s.length()) : dotPos) - 3;
    while (insertPos > start) {
        s.insert(insertPos, ",");
        insertPos -= 3;
    }
    return s;
}

static bool parseAmount(const string& text, double& amount)
{
    string cleaned;
    for (char ch : text) {
        if (ch != ',' && ch != ' ')
            cleaned.push_back(ch);
    }
    if (cleaned.empty())
        return false;
    try {
        size_t idx = 0;
        amount = stod(cleaned, &idx);
        return idx == cleaned.size();
    } catch (...) {
        return false;
    }
}

static bool isValidEmail(const string& email)
{
    size_t atPos = email.find('@');
    return atPos != string::npos && atPos + 1 < email.size() && email.find(".com", atPos) != string::npos;
}

static bool isValidPhone(const string& phone)
{
    if (phone.size() != 10)
        return false;
    for (char ch : phone)
        if (!isdigit(static_cast<unsigned char>(ch)))
            return false;
    return true;
}

static string currentDateTime()
{
    auto now = chrono::system_clock::now();
    time_t t = chrono::system_clock::to_time_t(now);
    tm localTime;
#ifdef _WIN32
    localtime_s(&localTime, &t);
#else
    localtime_r(&t, &localTime);
#endif
    ostringstream oss;
    oss << put_time(&localTime, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

static bool readInt(const string& prompt, int& value)
{
    string input;
    cout << prompt;
    if (!getline(cin, input))
        return false;
    try {
        size_t idx = 0;
        value = stoi(input, &idx);
        return idx == input.size();
    } catch (...) {
        return false;
    }
}

static bool readDouble(const string& prompt, double& value)
{
    string input;
    cout << prompt;
    if (!getline(cin, input))
        return false;
    return parseAmount(input, value);
}

static string readPassword(const string& prompt)
{
    cout << prompt;
    string password;
#ifdef _WIN32
    int ch;
    while ((ch = _getch()) != '\r' && ch != '\n') {
        if (ch == 8) {
            if (!password.empty()) {
                password.pop_back();
                cout << "\b \b";
            }
        } else if (ch == 3) {
            break;
        } else if (isprint(ch)) {
            password.push_back(static_cast<char>(ch));
            cout << '*';
        }
    }
    cout << '\n';
#else
    getline(cin, password);
#endif
    return password;
}

static string hashPassword(const string& password)
{
    const string salt = "ABCbank-Salt-2026";
    string combined = password + salt;
    size_t hashValue = hash<string>{}(combined);
    ostringstream oss;
    oss << hex << nouppercase << hashValue;
    return oss.str();
}

static bool isStrongPassword(const string& password)
{
    return static_cast<int>(password.size()) >= BankAccount::PASSWORD_MIN_LENGTH;
}

static string buildAccountFilename(const string& accountNumber)
{
    return BankAccount::accountsDir + "/" + accountNumber + ".txt";
}

static double calculateInterestAmount(double principal, double annualRate, int years)
{
    return principal * annualRate / 100.0 * years;
}

void ensure_accounts_dir()
{
#ifdef _WIN32
        _mkdir(BankAccount::accountsDir.c_str());
#else
        mkdir(BankAccount::accountsDir.c_str(), 0755);
#endif
}
const string BankAccount::bankname   = "ABC Bank";
const string BankAccount::bankprefix = "ABC";
const string BankAccount::accountsDir = "Accounts";

//to save user data in file 
void BankAccount::savetofile()
{
    ensure_accounts_dir();

    string filename = accountsDir + "/" + accountnumber + ".txt";

    ofstream file(filename);

    if (!file)
    {
        red();
        cout << "ERROR: Cannot write account file:\n  " << filename << "\n";
        cout << "Please verify the Accounts directory is writable and try again.\n";
        white();
        return;
    }

    file << accountholdername << '\n';
    file << accountnumber << '\n';
    file << accountpassword << '\n';
    file << email << '\n';
    file << phonenumber << '\n';
    file << fixed << setprecision(2) << balance << '\n';

    for (const auto& t : transactionhistory)
        file << t << '\n';
}

//to access the data from saved files 
bool BankAccount::loadfromfile()
{
    string filename = accountsDir + "/" + accountnumber + ".txt";

    ifstream file(filename);

    if (!file)
    {
        red();
        cout << "ERROR: Cannot read account file:\n  " << filename << "\n";
        cout << "The account may not exist or the file is corrupted.\n";
        white();
        return false;
    }

    getline(file, accountholdername);
    getline(file, accountnumber);
    getline(file, accountpassword);
    getline(file, email);
    getline(file, phonenumber);

    if (!(file >> balance))
    {
        red();
        cout << "ERROR: Invalid balance value in account file:\n  " << filename << "\n";
        cout << "Please contact support or recreate your account.\n";
        white();
        return false;
    }
    file.ignore(numeric_limits<streamsize>::max(), '\n');

    transactionhistory.clear();

    string transaction;

    while (getline(file, transaction))
    {
        if (!transaction.empty())
            transactionhistory.push_back(transaction);
    }
    return true;
}

// mainmenu to display many options to users regarding software
void BankAccount::mainmenu() {
    int choice;
    do {
        green();
        cout << "\n-----------------------------\n";
        cout << "  Welcome to " << bankname << "\n";
        cout << "-----------------------------\n";
        white();
        cout << "1. Deposit money\n";
        cout << "2. Withdraw money\n";
        cout << "3. Calculate interest\n";
        cout << "4. Check balance\n";
        cout << "5. Account details\n";
        cout << "6. Transaction history\n";
        cout << "7. Logout / Exit\n";
        cout << "-----------------------------\n";
        if (!readInt("Enter your choice: ", choice)) {
            red();
            cout << "Invalid input. Please enter a number between 1 and 7.\n";
            white();
            continue;
        }
        system("cls");

        switch (choice) {
            case 1: depositmoney();        break;
            case 2: withdrawmoney();       break;
            case 3: calculateInterest();   break;
            case 4: checkbalance();        break;
            case 5: accountdetails();      break;
            case 6: alltransactionhistory(); break;
            case 7:
                yellow();
                cout << "Thank you for using " << bankname << "!\n";
                white();
                break;
            default:
                red();
                cout << "Invalid choice. Please try again.\n";
                white();   // FIX 4 – replaced undefined ResetTextcolor() with white()
        }
    } while (choice != 7);   // user must choose 7 to logout/exit
}

// to create account and save user info
void BankAccount::createaccount()
{
    green();
    cout << "-----------------------------\n";
    cout << " Create Account \n";
    cout << "-----------------------------\n";
    white();

    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    do {
        cout << "Enter your full name : ";
        getline(cin, accountholdername);
        if (accountholdername.empty()) {
            red();
            cout << "Full name cannot be empty. Please try again.\n";
            white();
        }
    } while (accountholdername.empty());

    do {
        cout << "Enter your Email id : ";
        getline(cin, email);
        if (!isValidEmail(email)) {
            red();
            cout << "Email must contain an '@' and end with '.com'. Please try again.\n";
            white();
        }
    } while (!isValidEmail(email));

    do {
        cout << "Enter your phone number : ";
        getline(cin, phonenumber);
        if (!isValidPhone(phonenumber)) {
            red();
            cout << "Phone number must contain exactly 10 digits. Please try again.\n";
            white();
        }
    } while (!isValidPhone(phonenumber));

    string rawPassword;
    do {
        rawPassword = readPassword("Enter your password : ");
        if (rawPassword.empty()) {
            red();
            cout << "Password cannot be empty. Please try again.\n";
            white();
            continue;
        }
        if (!isStrongPassword(rawPassword)) {
            red();
            cout << "Password must be at least " << BankAccount::PASSWORD_MIN_LENGTH << " characters.\n";
            white();
            continue;
        }
        break;
    } while (true);
    accountpassword = hashPassword(rawPassword);

    ensure_accounts_dir();

    int counter = COUNTER_START;
    ifstream counterIn(accountsDir + "/counter.txt");
    if (counterIn && !(counterIn >> counter)) {
        red();
        cout << "Warning: Counter file is invalid. Resetting account numbering.\n";
        white();
        counter = COUNTER_START;
    }

    string filename;
    do {
        counter++;
        accountnumber = bankprefix + to_string(counter);
        filename = accountsDir + "/" + accountnumber + ".txt";
    } while (ifstream(filename));

    ofstream counterOut(accountsDir + "/counter.txt");
    if (!counterOut) {
        red();
        cout << "ERROR: Cannot update account counter file:\n  " << accountsDir << "/counter.txt\n";
        cout << "Please verify write permissions and try again.\n";
        white();
        return;
    }
    counterOut << counter;

    balance = MIN_BALANCE;
    transactionhistory.clear();
    transactionhistory.push_back(string("[") + currentDateTime() + "] Initial deposit: Rs. " + formatAmount(balance));

    yellow();
    cout << "-------------------------------\n";
    cout << "Account created successfully!\n";
    cout << "Your account number is : " << accountnumber << "\n";
    cout << "Please remember your account number and password for future login.\n";
    cout << "Minimum initial balance is Rs. " << formatAmount(MIN_BALANCE) << "\n";
    cout << "------------------------------\n";
    white();

    savetofile();
}

// login page to access account information
void BankAccount::logingbankaccount() {
    string enterpassword;
    blue();
    cout << "-----------------------------\n";
    cout << "           Login             \n";
    cout << "-----------------------------\n";
    white();
    cout << "Enter account number: ";
    getline(cin, accountnumber);   // set member first

    string filename = buildAccountFilename(accountnumber);
    if (accountnumber.empty()) {
        red();
        cout << "Account number cannot be empty.\n";
        white();
        return;
    }

    ifstream test(filename);
    if (!test) {
        red();
        cout << "Account not found. Please verify your account number and try again.\n";
        white();
        return;
    }

    if (!loadfromfile()) {
        return;
    }

    int attempts = 0;
    while (attempts < BankAccount::MAX_LOGIN_ATTEMPTS) {
        enterpassword = readPassword("Enter password: ");
        if (enterpassword.empty()) {
            red();
            cout << "Password cannot be empty. Please try again.\n";
            white();
            continue;
        }

        if (accountpassword == hashPassword(enterpassword)) {
            green();
            cout << "Login successful!\n";
            white();
            mainmenu();
            return;
        }

        attempts++;
        red();
        if (attempts < BankAccount::MAX_LOGIN_ATTEMPTS) {
            cout << "Incorrect password. " << (BankAccount::MAX_LOGIN_ATTEMPTS - attempts) << " attempt(s) left.\n";
        } else {
            cout << "Incorrect password. Maximum login attempts reached.\n";
        }
        white();
    }
}

// to deposit money in your account 
void BankAccount::depositmoney() {
    double amount;
    if (!readDouble("Enter amount to deposit: ", amount)) {
        red();
        cout << "Invalid amount. Please enter a numeric value.\n";
        white();
        return;
    }

    if (amount > 0) {
        balance += amount;
        string entry = string("[") + currentDateTime() + "] Deposit: Rs. " + formatAmount(amount);
        transactionhistory.push_back(entry);
        savetofile();   // persist full file (balance + history)
        green();
        cout << "Amount deposited successfully!\n";
        cout << "Current balance: Rs. " << formatAmount(balance) << "\n";
        white();
    } else {
        red();
        cout << "Invalid amount. Deposit must be greater than 0.\n";
        white();
    }
}

// to withdraw money from your account 
void BankAccount::withdrawmoney() {
    double amount;
    if (!readDouble("Enter amount to withdraw: ", amount)) {
        red();
        cout << "Invalid amount. Please enter a numeric value.\n";
        white();
        return;
    }

    if (amount <= 0) {
        red();
        cout << "Invalid amount. Withdrawal must be greater than 0.\n";
        white();
    } else if (amount > balance - MIN_BALANCE) {
        red();
        cout << "Insufficient balance. Minimum balance of Rs. " << formatAmount(MIN_BALANCE) << " must be maintained.\n";
        cout << "Maximum withdrawable amount: Rs. " << formatAmount(balance - MIN_BALANCE) << "\n";
        white();
    } else {
        balance -= amount;
        string entry = string("[") + currentDateTime() + "] Withdraw: Rs. " + formatAmount(amount);
        transactionhistory.push_back(entry);
        savetofile();
        green();
        cout << "Amount withdrawn successfully!\n";
        cout << "Current balance: Rs. " << formatAmount(balance) << "\n";
        white();
    }
}

void BankAccount::calculateInterest() {
    int years;
    if (!readInt("Enter number of years for interest calculation (1-" + to_string(MAX_INTEREST_YEARS) + "): ", years) || years < 1 || years > MAX_INTEREST_YEARS) {
        red();
        cout << "Invalid year count. Please enter a number between 1 and " << MAX_INTEREST_YEARS << ".\n";
        white();
        return;
    }

    double rate = BankAccount::DEFAULT_INTEREST_RATE;
    string rateInput;
    cout << "Enter annual interest rate (%) [default " << formatAmount(rate) << "]: ";
    if (getline(cin, rateInput) && !rateInput.empty()) {
        if (!parseAmount(rateInput, rate) || rate <= 0) {
            red();
            cout << "Invalid interest rate. Using default rate of " << formatAmount(BankAccount::DEFAULT_INTEREST_RATE) << "%\n";
            white();
            rate = BankAccount::DEFAULT_INTEREST_RATE;
        }
    }

    double interest = calculateInterestAmount(balance, rate, years);
    string entry = string("[") + currentDateTime() + "] Interest calculated: Rs. " + formatAmount(interest) + " at " + formatAmount(rate) + "% for " + to_string(years) + " year(s)";
    transactionhistory.push_back(entry);
    savetofile();

    green();
    cout << "Estimated interest for " << years << " year(s) at " << formatAmount(rate) << "%: Rs. " << formatAmount(interest) << "\n";
    white();
}

// to check your account balance
void BankAccount::checkbalance() {
    blue();
    cout << "-----------------------------\n";
    cout << "  Current Balance: Rs. " << formatAmount(balance) << "\n";
    cout << "-----------------------------\n";
    white();
}

// checks detail regarding your account like name , ac.number etc.
void BankAccount::accountdetails() {
    purple();
    cout << "-----------------------------\n";
    cout << "       Account Details       \n";
    cout << "-----------------------------\n";
    white();
    cout << "  Account Holder : " << accountholdername << "\n";
    cout << "  Account Number : " << accountnumber     << "\n";
    cout << "  Email          : " << email             << "\n";
    cout << "  Phone          : " << phonenumber       << "\n";
    cout << "  Balance        : Rs. " << formatAmount(balance)       << "\n";
    cout << "-----------------------------\n";
}

// to save trabsaction history in file 
void BankAccount::alltransactionhistory() {
    yellow();
    cout << "-----------------------------\n";
    cout << "     Transaction History     \n";
    cout << "-----------------------------\n";
    white();
    if (transactionhistory.empty()) {
        cout << "  No transactions yet.\n";
    } else {
        int idx = 1;
        for (const auto& t : transactionhistory)
            cout << "  " << idx++ << ". " << t << "\n";
    }
    cout << "-----------------------------\n";
}

// main function to call all global variables and functions 
int main() {
    BankAccount account;
    int choice;

    green();
    cout << "============================\n";
    cout << "     Welcome to ABC Bank    \n";
    cout << "============================\n";
    white();
    cout << "1. Create Account\n";
    cout << "2. Login\n";
    cout << "3. Exit\n";
    cout << "============================\n";
    if (!readInt("Enter your choice: ", choice)) {
        red();
        cout << "Invalid input. Please enter a number.\n";
        white();
        return 0;
    }
    system("cls");

    switch (choice) {
        case 1:
            account.createaccount();
            account.mainmenu();
            break;
        case 2:
            account.logingbankaccount();
            break;
        case 3:
            yellow();
            cout << "Thank you for using ABC Bank!\n";
            white();
            break;
        default:
            red();
            cout << "Invalid choice!\n";
            white();
    }

    return 0;
}