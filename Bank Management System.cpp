#include<iostream>
#include<string>
#include<fstream>
#include<vector>
#include<cstdlib>  
#include<limits>
#ifdef _WIN32
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
void white()  { cout << "\033[1;37m"; }  // also used as "reset colour"

// ── Class declaration ────────────────────────────────────────────
class BankAccount {
private:
    static const string bankname;
    static const string bankprefix;   // prefix for account numbers  (was "bankcode")
    string accountholdername;
    string accountnumber;
    string accountpassword;
    string email;
    string phonenumber;
    double balance;
    vector<string> transactionhistory;

public:
    // FIX 1 – both helpers return void, not string
    void savetofile();
    void loadfromfile();

    void mainmenu();
    void createaccount();
    void logingbankaccount();
    void depositmoney();
    void withdrawmoney();
    void checkbalance();
    void accountdetails();
    void alltransactionhistory();
};

// ── Static member definitions ────────────────────────────────────
    
    // Ensure Accounts directory exists (cross-platform)
    void ensure_accounts_dir()
    {
#ifdef _WIN32
        _mkdir("Accounts");
#else
        mkdir("Accounts", 0755);
#endif
    }
const string BankAccount::bankname   = "ABC Bank";
const string BankAccount::bankprefix = "ABC";   // account numbers → ABC1, ABC2 …

// ── savetofile ───────────────────────────────────────────────────
void BankAccount::savetofile()
{
    ensure_accounts_dir();

    string filename = "Accounts/" + accountnumber + ".txt";

    ofstream file(filename);

    if (!file)
    {
        red();
        cout << "ERROR: Cannot create account file.\n";
        cout << "File: " << filename << endl;
        white();
        return;
    }

    file << accountholdername << '\n';
    file << accountnumber << '\n';
    file << accountpassword << '\n';
    file << email << '\n';
    file << phonenumber << '\n';
    file << balance << '\n';

    for (const auto& t : transactionhistory)
        file << t << '\n';

    file.close();
}

// ── loadfromfile ─────────────────────────────────────────────────
void BankAccount::loadfromfile()
{
    string filename = "Accounts/" + accountnumber + ".txt";

    ifstream file(filename);

    if (!file)
    {
        red();
        cout << "ERROR: Account file not found.\n";
        white();
        return;
    }

    getline(file, accountholdername);
    getline(file, accountnumber);
    getline(file, accountpassword);
    getline(file, email);
    getline(file, phonenumber);

    file >> balance;
    file.ignore(numeric_limits<streamsize>::max(), '\n');

    transactionhistory.clear();

    string transaction;

    while (getline(file, transaction))
    {
        if (!transaction.empty())
            transactionhistory.push_back(transaction);
    }

    file.close();
}

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
        cout << "3. Check balance\n";
        cout << "4. Account details\n";
        cout << "5. Transaction history\n";
        cout << "6. Logout / Exit\n";
        cout << "-----------------------------\n";
        cout << "Enter your choice: ";
        cin  >> choice;
        cin.ignore();
        system("cls");

        switch (choice) {
            case 1: depositmoney();        break;
            case 2: withdrawmoney();       break;
            case 3: checkbalance();        break;
            case 4: accountdetails();      break;
            case 5: alltransactionhistory(); break;
            case 6:
                yellow();
                cout << "Thank you for using " << bankname << "!\n";
                white();
                break;
            default:
                red();
                cout << "Invalid choice. Please try again.\n";
                white();   // FIX 4 – replaced undefined ResetTextcolor() with white()
        }
    } while (choice != 6);   // FIX 3 – } while now properly closes the single do-while
}

// ── createaccount ────────────────────────────────────────────────
// Create a new account, persist it into Accounts/, and update counter
void BankAccount::createaccount()
{
    green();
    cout << "-----------------------------\n";
    cout << " Create Account \n";
    cout << "-----------------------------\n";
    white();

    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "Enter your full name : ";
    getline(cin, accountholdername);
    cout << "Enter your Email id : ";
    getline(cin, email);
    cout << "Enter your phone number : ";
    getline(cin, phonenumber);
    cout << "Enter your password : ";
    getline(cin, accountpassword);

    // Ensure Accounts directory exists
    ensure_accounts_dir();

    // Read/Update counter
    int counter = 1000;
    ifstream counterIn("Accounts/counter.txt");
    if (counterIn)
    {
        counterIn >> counter;
        counterIn.close();
    }

    counter++;
    ofstream counterOut("Accounts/counter.txt");
    if (counterOut)
    {
        counterOut << counter;
        counterOut.close();
    }

    // Compose account number and initial state
    accountnumber = bankprefix + to_string(counter);
    balance = 500.0;
    transactionhistory.clear();
    transactionhistory.push_back(string("Deposit  : Rs. ") + to_string(balance));

    yellow();
    cout << "-------------------------------\n";
    cout << "Account created successfully!\n";
    cout << "Your account number is : " << accountnumber << "\n";
    cout << "Please remember your account number and password for future login.\n";
    cout << " Minimum initial balance is 500.0 \n";
    cout << "------------------------------\n";
    white();

    savetofile();
}

// login follows
void BankAccount::logingbankaccount() {
    string enterpassword;
    blue();
    cout << "-----------------------------\n";
    cout << "           Login             \n";
    cout << "-----------------------------\n";
    white();
    cout << "Enter account number: ";
    getline(cin, accountnumber);   // set member first

    ifstream test(string("Accounts/") + accountnumber + ".txt");
    if (!test.is_open()) {
        red();
        cout << "Account not found. Please try again.\n";
        white();
        return;
    }
    test.close();

    loadfromfile();   // now safe to load

    cout << "Enter password: ";
    getline(cin, enterpassword);

    if (accountpassword == enterpassword) {
        green();
        cout << "Login successful!\n";
        white();
        mainmenu();
    } else {
        red();
        cout << "Incorrect password. Please try again.\n";
        white();
    }
}

// ── depositmoney ─────────────────────────────────────────────────
// FIX 7 – call savetofile() so the updated balance is persisted
void BankAccount::depositmoney() {
    double amount;
    cout << "Enter amount to deposit: ";
    cin  >> amount;
    cin.ignore();

    if (amount > 0) {
        balance += amount;
        string entry = "Deposit  : Rs. " + to_string(amount);
        transactionhistory.push_back(entry);
        savetofile();   // persist full file (balance + history)
        green();
        cout << "Amount deposited successfully!\n";
        cout << "Current balance: Rs. " << balance << "\n";
        white();
    } else {
        red();
        cout << "Invalid amount. Deposit must be greater than 0.\n";
        white();
    }
}

// ── withdrawmoney ────────────────────────────────────────────────
// FIX 7 – call savetofile(); also enforce Rs.500 minimum balance
void BankAccount::withdrawmoney() {
    double amount;
    cout << "Enter amount to withdraw: ";
    cin  >> amount;
    cin.ignore();

    const double MIN_BALANCE = 500.0;

    if (amount <= 0) {
        red();
        cout << "Invalid amount. Withdrawal must be greater than 0.\n";
        white();
    } else if (amount > balance - MIN_BALANCE) {
        red();
        cout << "Insufficient balance. Minimum balance of Rs. 500.00 must be maintained.\n";
        cout << "Maximum withdrawable amount: Rs. " << (balance - MIN_BALANCE) << "\n";
        white();
    } else {
        balance -= amount;
        string entry = "Withdraw : Rs. " + to_string(amount);
        transactionhistory.push_back(entry);
        savetofile();
        green();
        cout << "Amount withdrawn successfully!\n";
        cout << "Current balance: Rs. " << balance << "\n";
        white();
    }
}

// ── checkbalance ─────────────────────────────────────────────────
void BankAccount::checkbalance() {
    blue();
    cout << "-----------------------------\n";
    cout << "  Current Balance: Rs. " << balance << "\n";
    cout << "-----------------------------\n";
    white();
}

// ── accountdetails ───────────────────────────────────────────────
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
    cout << "  Balance        : Rs. " << balance       << "\n";
    cout << "-----------------------------\n";
}

// ── alltransactionhistory ────────────────────────────────────────
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

// ── main ─────────────────────────────────────────────────────────
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
    cout << "Enter your choice: ";
    cin  >> choice;
    cin.ignore();
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