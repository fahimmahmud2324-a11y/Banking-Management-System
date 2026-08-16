#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---------- Transaction History Stack Node (per account) ---------- */
typedef struct TransNode {
    int type;        /* 1 = Deposit, 2 = Withdraw */
    float amount;
    int serial;
    struct TransNode *next;
} TransNode;

/* ---------- Account Node (Linked List) ---------- */
typedef struct Account {
    int acc_no;
    char name[50];
    float balance;
    TransNode *history_top;   /* stack top for this account's history */
    int trans_count;
    struct Account *next;
} Account;

/* ---------- Pending Transaction Queue Node ---------- */
typedef struct QueueNode {
    int acc_no;
    int type;         /* 1 deposit, 2 withdraw */
    float amount;
    struct QueueNode *next;
} QueueNode;

Account *acc_head = NULL;
QueueNode *q_front = NULL, *q_rear = NULL;
int global_serial = 1;

/* ---- Linked list functions ---- */
Account* find_account(int acc_no) {
    Account *cur = acc_head;
    while (cur) {
        if (cur->acc_no == acc_no) return cur;
        cur = cur->next;
    }
    return NULL;
}

void create_account() {
    int acc_no;
    char name[50];
    float balance;
    printf("Enter Account Number: ");
    scanf("%d", &acc_no);
    if (find_account(acc_no)) {
        printf("Account already exists!\n");
        return;
    }
    printf("Enter Name: ");
    scanf(" %[^\n]", name);
    printf("Enter Initial Balance: ");
    scanf("%f", &balance);

    Account *new_acc = (Account*)malloc(sizeof(Account));
    new_acc->acc_no = acc_no;
    strcpy(new_acc->name, name);
    new_acc->balance = balance;
    new_acc->history_top = NULL;
    new_acc->trans_count = 0;
    new_acc->next = acc_head;
    acc_head = new_acc;

    printf("Account created successfully!\n");
}

void view_all_accounts() {
    Account *cur = acc_head;
    if (!cur) { printf("No accounts found.\n"); return; }
    printf("\n%-10s %-20s %-10s\n", "AccNo", "Name", "Balance");
    while (cur) {
        printf("%-10d %-20s %-10.2f\n", cur->acc_no, cur->name, cur->balance);
        cur = cur->next;
    }
}

/* ---- Queue functions ---- */
void enqueue_transaction(int acc_no, int type, float amount) {
    QueueNode *node = (QueueNode*)malloc(sizeof(QueueNode));
    node->acc_no = acc_no;
    node->type = type;
    node->amount = amount;
    node->next = NULL;
    if (!q_rear) {
        q_front = q_rear = node;
    } else {
        q_rear->next = node;
        q_rear = node;
    }
    printf("Transaction request added to queue.\n");
}

int dequeue_transaction(QueueNode *out) {
    if (!q_front) return 0;
    QueueNode *temp = q_front;
    *out = *temp;
    q_front = q_front->next;
    if (!q_front) q_rear = NULL;
    free(temp);
    return 1;
}

void view_queue() {
    QueueNode *cur = q_front;
    if (!cur) { printf("Queue is empty.\n"); return; }
    printf("\nPending Transactions (Front -> Rear):\n");
    while (cur) {
        printf("AccNo: %d | %s | Amount: %.2f\n", cur->acc_no,
               cur->type == 1 ? "Deposit" : "Withdraw", cur->amount);
        cur = cur->next;
    }
}

/* ---- Stack functions (per account history) ---- */
void push_history(Account *acc, int type, float amount) {
    TransNode *node = (TransNode*)malloc(sizeof(TransNode));
    node->type = type;
    node->amount = amount;
    node->serial = global_serial++;
    node->next = acc->history_top;
    acc->history_top = node;
    acc->trans_count++;
}

void view_history(Account *acc) {
    TransNode *cur = acc->history_top;
    if (!cur) { printf("No transactions yet.\n"); return; }
    printf("\nTransaction History for %s (Acc: %d) [Latest first]:\n", acc->name, acc->acc_no);
    while (cur) {
        printf("#%d - %s : %.2f\n", cur->serial, cur->type == 1 ? "Deposit" : "Withdraw", cur->amount);
        cur = cur->next;
    }
}

/* ---- Process one transaction from queue ---- */
void process_one() {
    QueueNode qnode;
    if (!dequeue_transaction(&qnode)) {
        printf("No pending transactions.\n");
        return;
    }
    Account *acc = find_account(qnode.acc_no);
    if (!acc) {
        printf("Account %d not found. Transaction skipped.\n", qnode.acc_no);
        return;
    }
    if (qnode.type == 1) {
        acc->balance += qnode.amount;
        printf("Processed Deposit of %.2f for Acc %d. New Balance: %.2f\n", qnode.amount, acc->acc_no, acc->balance);
    } else {
        if (acc->balance < qnode.amount) {
            printf("Insufficient balance for Acc %d. Withdraw failed.\n", acc->acc_no);
            return;
        }
        acc->balance -= qnode.amount;
        printf("Processed Withdraw of %.2f for Acc %d. New Balance: %.2f\n", qnode.amount, acc->acc_no, acc->balance);
    }
    push_history(acc, qnode.type, qnode.amount);
}

void process_all() {
    while (q_front) process_one();
}

/* ---- Undo last transaction ---- */
void undo_last(int acc_no) {
    Account *acc = find_account(acc_no);
    if (!acc) { printf("Account not found.\n"); return; }
    if (!acc->history_top) { printf("No transaction to undo.\n"); return; }
    TransNode *top = acc->history_top;
    if (top->type == 1) {
        acc->balance -= top->amount;
        printf("Undo Deposit of %.2f. Balance reverted to %.2f\n", top->amount, acc->balance);
    } else {
        acc->balance += top->amount;
        printf("Undo Withdraw of %.2f. Balance reverted to %.2f\n", top->amount, acc->balance);
    }
    acc->history_top = top->next;
    acc->trans_count--;
    free(top);
}

int main() {
    int choice;
    while (1) {
        printf("\n===== Banking Transaction Management =====\n");
        printf("1. Create Account\n");
        printf("2. Add Transaction Request (Deposit/Withdraw)\n");
        printf("3. Process Next Transaction\n");
        printf("4. Process All Pending Transactions\n");
        printf("5. View All Accounts\n");
        printf("6. View Transaction History (Stack)\n");
        printf("7. Undo Last Transaction\n");
        printf("8. View Pending Queue\n");
        printf("0. Exit\n");
        printf("Enter choice: ");
        if (scanf("%d", &choice) != 1) break;

        if (choice == 1) create_account();
        else if (choice == 2) {
            int acc_no, type;
            float amount;
            printf("Enter Account Number: ");
            scanf("%d", &acc_no);
            if (!find_account(acc_no)) { printf("Account not found!\n"); continue; }
            printf("1. Deposit  2. Withdraw\nChoice: ");
            scanf("%d", &type);
            printf("Enter Amount: ");
            scanf("%f", &amount);
            enqueue_transaction(acc_no, type, amount);
        }
        else if (choice == 3) process_one();
        else if (choice == 4) process_all();
        else if (choice == 5) view_all_accounts();
        else if (choice == 6) {
            int acc_no;
            printf("Enter Account Number: ");
            scanf("%d", &acc_no);
            Account *acc = find_account(acc_no);
            if (acc) view_history(acc); else printf("Account not found.\n");
        }
        else if (choice == 7) {
            int acc_no;
            printf("Enter Account Number: ");
            scanf("%d", &acc_no);
            undo_last(acc_no);
        }
        else if (choice == 8) view_queue();
        else if (choice == 0) { printf("Exiting...\n"); break; }
        else printf("Invalid choice!\n");
    }
    return 0;
}
