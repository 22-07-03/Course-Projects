#include <cmath>
#include <cstdio>
#include <vector>
#include <iostream>
#include <algorithm>
#include <string>
#include <iomanip>
using namespace std;

class Node {
private:
    unsigned int accountNumber;
    int balance;
    Node* next;
public:
    Node(int no) {
        accountNumber = no;
        balance = 1000;
        next = NULL;
    }
    friend class AccountList;
    friend class Transaction;
    friend class DoublyLinkedList;
};

class AccountList {
private:
    Node* head;
    Node* tail;
    int length;
public:
    AccountList() {
        head = NULL;
        tail = NULL;
        length = 0;
    }
    void addAccount(int);
    void printList() {
        Node* temp = head;
        if (head == NULL) return;
        while (temp != NULL) {
            cout << temp->balance << ' ';
            temp = temp->next;
        }
        cout << '\n';
    }
    Node* getAccount(int no) {
        Node* temp = head;
        while (temp != nullptr && temp->accountNumber != no) {
            temp = temp->next;
        }
        return temp;
    }
    int countAccountsWithBalance(int x) {
        int count = 0;
        if (head == nullptr) return count;
        Node* temp = head;
        while (temp != nullptr) {
            if (temp->balance >= x) count++;
            temp = temp->next;
        }
        return count;
    }
    void printMaxBalanceAccounts() {
        if (head == nullptr) { return; }
        int maxBalance = INT32_MIN;
        Node* temp = head;
        while (temp != nullptr) {
            if (temp->balance > maxBalance) { maxBalance = temp->balance; }
            temp = temp->next;
        }
        temp = head;
        int count = 0;
        while (temp != nullptr) {
            if (temp->balance == maxBalance) {
                count++;
                cout << temp->accountNumber << ' ';
            }
            temp = temp->next;
        }
        if (count) { cout << '\n'; }
    }
    void printBalance(int accountNumber) {
        if (head == nullptr) {
            return;
        }
        Node* temp = head;
        while (temp != nullptr) {
            if (temp->accountNumber == accountNumber) {
                cout << temp->balance << '\n';
                break;
            }
            temp = temp->next;
        }
    }
    friend class Node;
    friend class Transaction;
    friend class DoublyLinkedList;
};

void AccountList::addAccount(int accountNumber) {
    Node* newNode = new Node(accountNumber);
    if (head == NULL) {
        head = newNode;
        tail = newNode;
        length++;
    }
    else {
        tail->next = newNode;
        tail = tail->next;
        tail->next = NULL;
        length++;
    }
}

class Transaction {
private:
    unsigned int accountNumber;
    char transactionType;
    int amount;
    Transaction* prev;
    Transaction* next;
public:
    Transaction* getNext() { return next; }
    Transaction* getPrev() { return prev; }
    Transaction() {
        accountNumber = 10001;
        transactionType = 'i';
        amount = -1;
        prev = nullptr;
        next = nullptr;
    }
    Transaction(unsigned int accountNum, char type, int amt) {
        accountNumber = accountNum;
        transactionType = type;
        amount = amt;
    }
    void process(AccountList& accounts) {
        Node* target = accounts.head;
        while (target != nullptr && target->accountNumber != accountNumber) {
            target = target->next;
        }
        if (target == nullptr) return;
        if (transactionType == 'D') { target->balance += amount; }
        else { target->balance -= amount; }
    }
    void deprocess(AccountList& accounts) {
        Node* target = accounts.head;
        while (target != nullptr && target->accountNumber != accountNumber) {
            target = target->next;
        }
        if (target == nullptr) return;
        if (transactionType == 'D') { target->balance -= amount; }
        else { target->balance += amount; }
    }
    void printTransaction() {
        cout << accountNumber << ' ' << transactionType << ' ' << amount << '\n';
    }
    friend class DoublyLinkedList;
    friend class AccountList;
};

class DoublyLinkedList {
private:
    Transaction* head;
    Transaction* tail;
    int length = 0;
public:
    DoublyLinkedList() {
        head = new Transaction;
        tail = new Transaction;
        head->next = tail;
        tail->prev = head;
    }
    void addTransaction(unsigned int, char, int);
    Transaction* getHead() { return head; }
    Transaction* getTail() { return tail; }
    void addTransactionAtK(int k, Transaction* newTransaction, int& cursorPosition, AccountList& accounts) {
        if (k > length || k < 0) { return; }
        length++;
        if (k < cursorPosition) {
            newTransaction->process(accounts);
            cursorPosition++;
        }
        Transaction* temp = head;
        for (int i = 0; i < k; i++) {
            temp = temp->next;
        }
        newTransaction->next = temp->next;
        (temp->next)->prev = newTransaction;
        newTransaction->prev = temp;
        temp->next = newTransaction;
    }
    void printTransactionsByAccount(int accountNumber, Transaction* cursor) {
        Transaction* temp = head->next;
        if (cursor == head) { return; }
        while (temp != cursor->next) {
            if (temp->accountNumber == accountNumber) {
                temp->printTransaction();
            }
            temp = temp->next;
        }
    }
    void printAllTransactions() {
        Transaction* temp = head->next;
        while (temp != tail) {
            temp->printTransaction();
            temp = temp->next;
        }
    }
    void manageTransactions(Transaction* cursor, int accountNumber, int count, AccountList& accounts) {
        Node* account = accounts.head;
        if (account == nullptr) { return; }
        while (account != nullptr) {
            if (account->accountNumber == accountNumber) { break; }
            account = account->next;
        }
        if (account == nullptr) { return; }
        if (count == 0) { return; }
        if (count > 0) {
            Transaction* temp = cursor->next;
            int cnt = 0;
            while (cnt < count && temp != tail) {
                if (temp->accountNumber == accountNumber) {
                    cnt++;
                    temp->prev->next = temp->next;
                    temp->next->prev = temp->prev;
                    Transaction* temp1 = temp;
                    temp = temp->next;
                    delete temp1;
                }
                else { temp = temp->next; }
            }
        }
        else {
            int M = (-1) * count;
            Transaction* temp = cursor->prev;
            int cnt = 0;
            while (cnt < M && temp != head) {
                if (temp->accountNumber == accountNumber) {
                    cnt++;
                    temp->prev->next = temp->next;
                    temp->next->prev = temp->prev;
                    Transaction* temp1 = temp;
                    temp = temp->prev;
                    if (temp1->transactionType == 'D') { account->balance -= temp1->amount; }
                    else { account->balance += temp1->amount; }
                    delete temp1;
                }
                else { temp = temp->prev; }
            }
        }
    }
};

void DoublyLinkedList::addTransaction(unsigned int accountNumber, char type, int amt) {
    length++;
    Transaction* newTransaction = new Transaction(accountNumber, type, amt);
    (tail->prev)->next = newTransaction;
    newTransaction->prev = tail->prev;
    newTransaction->next = tail;
    tail->prev = newTransaction;
}

int main() {
    int c;
    cin >> c;
    AccountList accounts;
    for (int i = 0; i < c; i++) {
        int accountNumber;
        cin >> accountNumber;
        accounts.addAccount(accountNumber);
    }
    int n;
    cin >> n;
    DoublyLinkedList transactions;
    for (int i = 0; i < n; i++) {
        unsigned int accountNumber;
        char type;
        int amt;
        cin >> accountNumber >> type >> amt;
        transactions.addTransaction(accountNumber, type, amt);
    }
    Transaction* head = transactions.getHead();
    Transaction* tail = transactions.getTail();
    Transaction* cursor = head;
    int cursorPosition = 0;
    string command;
    cin >> command;
    while (command != "END") {
        if (command == "F") {
            int x;
            cin >> x;
            for (int i = 0; i < x && (cursor->getNext() != tail); i++) {
                cursor->getNext()->process(accounts);
                cursor = cursor->getNext();
                cursorPosition++;
            }
        }
        if (command == "R") {
            int y;
            cin >> y;
            for (int i = 0; i < y && cursor != head; i++) {
                cursor->deprocess(accounts);
                cursor = cursor->getPrev();
                cursorPosition--;
            }
        }
        if (command == "I") {
            int accountNumber;
            char type;
            int amt;
            cin >> accountNumber >> type >> amt;
            Transaction* newTransaction = new Transaction(accountNumber, type, amt);
            int k;
            cin >> k;
            transactions.addTransactionAtK(k, newTransaction, cursorPosition, accounts);
        }
        if (command == "C") {
            while (cursor->getNext() != tail) {
                (cursor->getNext())->process(accounts);
                cursor = cursor->getNext();
                cursorPosition++;
            }
        }
        if (command == "S") {
            int accountNumber;
            cin >> accountNumber;
            transactions.printTransactionsByAccount(accountNumber, cursor);
        }
        if (command == "G") {
            int x;
            cin >> x;
            cout << accounts.countAccountsWithBalance(x) << '\n';
        }
        if (command == "M") {
            accounts.printMaxBalanceAccounts();
        }
        if (command == "V") {
            int accountNumber;
            cin >> accountNumber;
            accounts.printBalance(accountNumber);
        }
        if (command == "D") {
            int accountNumber, count;
            cin >> accountNumber >> count;
            transactions.manageTransactions(cursor, accountNumber, count, accounts);
        }
        cin >> command;
    }
    return 0;
}
