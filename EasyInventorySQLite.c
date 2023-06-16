#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <wchar.h>
#include <locale.h>
#include <unistd.h>

#define DB_NAME "EasyInventorySQLite.db"


/////////////////
// Štruktúra pre objekt user

typedef struct {
    int id;
    char username[50];
    char password[50];  // Pridané pole pre heslo
    int role;
} User;
////////////////////////



//////////////////////////////////
// Užitočné funkcie

void clearConsole() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void clearBuff(){
    int c;
        while ((c = getchar()) != '\n' && c != EOF);
}
void enterwait(){
    wprintf(L"\nStlač ENTER pre pokračovanie ");
    clearBuff();
    getchar();
}
//////////////////////////////////////

int get_inventory_id(sqlite3* db, User *user) {
    int inventory_id = 0;

    // Vytvorenie SQL dotazu
    char query[100];
    sprintf(query, "SELECT id FROM inventories WHERE owner_id = %d;", user->id);

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Chyba pri príprave SQL dotazu: %s\n", sqlite3_errmsg(db));
    }

    // Vykonanie dotazu a získanie výsledku
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        inventory_id = sqlite3_column_int(stmt, 0);
    }
    return inventory_id;
}

void print_inv_items(sqlite3* db, int inventory_id, int reason){
    clearConsole();
    //setup sql query commandu
    char query[100];
    sprintf(query, "SELECT * FROM items WHERE inventory_id = '%d';",inventory_id);
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Chyba pri príprave SQL dotazu: %s\n", sqlite3_errmsg(db));
    }

    // print všetkých inventárov v databáze ID a názov
    wprintf(L"\x1b[32m-- Zoznam položiek v inventáry --\x1b[0m\n");
    wprintf(L"ID    Názov\n");
    wprintf(L"--------------------------\n");

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        char *name = (char *)sqlite3_column_text(stmt, 2);
        wprintf(L"%-5d %-20s\n", id, name);
    }

    sqlite3_finalize(stmt);
    if(reason==1){
        enterwait();
    }
}



///////////////////////////////////
/// Hotovo print inventories

void print_inventories(sqlite3 *db, User *user, int reason) {
    clearConsole();
    // Overenie oprávnenia iba pre adminov
    if(user->role==1){
        //setup sql query commandu
        char query[100];
        sprintf(query, "SELECT * FROM inventories;");
        sqlite3_stmt *stmt;

        if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
            fprintf(stderr, "Chyba pri príprave SQL dotazu: %s\n", sqlite3_errmsg(db));
            return;
        }

        // print všetkých inventárov v databáze ID a názov
        wprintf(L"\x1b[32m-- Zoznam inventárov --\x1b[0m\n");
        wprintf(L"ID    Názov\n");
        wprintf(L"--------------------------\n");

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int id = sqlite3_column_int(stmt, 0);
            char *name = (char *)sqlite3_column_text(stmt, 1);
            wprintf(L"%-5d %-20s\n", id, name);
        }

        sqlite3_finalize(stmt);
        if(reason==1){
            enterwait();
        }
    }else{
        // Iba ak uživatel nieje admin
        wprintf(L"\n\x1b[31m[ERROR]\x1b[0m Na túto akciu nemáš dostočné práva.");
        sleep(5);
    }
}

//////////////////////////////////


///////////////////////////////////
/// Hotovo print users

void print_users(sqlite3 *db, User *user, int reason) {
    clearConsole();
    // Overenie oprávnenia iba pre adminov
    if(user->role==1){
       char *query = "SELECT * FROM users;";
        sqlite3_stmt *stmt;

        if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
            fprintf(stderr, "Chyba pri príprave SQL dotazu: %s\n", sqlite3_errmsg(db));
            return;
        }

        // Výpis všetkých uživatelov v databáze ich ID, username a rola
        wprintf(L"\x1b[32m-- Zoznam používateľov --\x1b[0m\n");
        wprintf(L"%-5s %-20s %-10s\n", "ID", "Meno", "Rola");
        wprintf(L"--------------------------------------\n");

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int id = sqlite3_column_int(stmt, 0);
            char *username = (char *)sqlite3_column_text(stmt, 1);
            char *role = (char *)sqlite3_column_text(stmt, 3);
            wprintf(L"%-5d %-20s %-10s\n", id, username, role);
        }

        sqlite3_finalize(stmt);
        if(reason==1){
            enterwait();
        }
        
    }else{
        // Iba ak uživatel nieje admin
        wprintf(L"\n\x1b[31m[ERROR]\x1b[0m Na túto akciu nemáš dostočné práva.");
        sleep(5);
    }
    
}
///////////////////////////////////


////////////////////////////////////////////////
//Hotovo funkčné

void create_tables(sqlite3 *db) {
    char *create_user_table_query = "CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY AUTOINCREMENT, username TEXT NOT NULL, password TEXT NOT NULL, role TEXT NOT NULL);";
    char *create_inventory_table_query = "CREATE TABLE IF NOT EXISTS inventories (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL, owner_id INTEGER NOT NULL);";
    char *create_item_table_query = "CREATE TABLE IF NOT EXISTS items (id INTEGER PRIMARY KEY AUTOINCREMENT, inventory_id INTEGER NOT NULL, name TEXT NOT NULL, FOREIGN KEY(inventory_id) REFERENCES inventories(id));";

    sqlite3_exec(db, create_user_table_query, 0, 0, 0);
    sqlite3_exec(db, create_inventory_table_query, 0, 0, 0);
    sqlite3_exec(db, create_item_table_query, 0, 0, 0);
}

//////////////////////////////////////////////


////////////////////////////////////////////////
//Hotovo funkčné

void create_user(sqlite3 *db, User *user) {
    if(user->role != 1){
        wprintf(L"Nemas opravnenenie na vytvaranie uzivatelov.");
    } else {
        User user;
        wprintf(L"-- Vytvorenie nového používateľa --\n");
        wprintf(L"Zadajte meno používateľa: ");
        scanf("%s", user.username);
        wprintf(L"Zadajte heslo používateľa: ");
        scanf("%s", user.password);
        wprintf(L"Zadajte rolu používateľa (1-admin, 2-default): ");
        scanf("%d", &user.role);

        char query[200];
        sprintf(query, "INSERT INTO users (username, password, role) VALUES ('%s', '%s', %d);", user.username, user.password, user.role);

        if (sqlite3_exec(db, query, 0, 0, 0) != SQLITE_OK) {
            fprintf(stderr, "Chyba pri vytváraní používateľa: %s\n", sqlite3_errmsg(db));
            return;
        }

        wprintf(L"Používateľ bol vytvorený.\n");
    }
}


//////////////////////////////////////////////




////////////////////////////////////////////////
//Hotovo funkčné


void create_inventory(sqlite3 *db) {
    char name[50];
    int owner_id;

    wprintf(L"-- Vytvorenie nového inventára --\n");
    wprintf(L"Zadajte názov inventára: ");
    scanf("%s", &name);
    /*fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = '\0';  // Odstráni znak nového riadku zo vstupu

    int c;
    while ((c = getchar()) != '\n' && c != EOF);  // Odstráni zvyšný znak nového riadku zo vstupného bufferu
*/
    wprintf(L"Zadajte ID majiteľa inventára: ");
    scanf("%d", &owner_id);

    sqlite3_stmt *stmt;
    const char *query = "INSERT INTO inventories (name, owner_id) VALUES (?, ?);";
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Chyba pri príprave SQL dotazu: %s\n", sqlite3_errmsg(db));
        return;
    }

    if (sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC) != SQLITE_OK) {
        fprintf(stderr, "Chyba pri väzbe názvu inventára: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return;
    }

    if (sqlite3_bind_int(stmt, 2, owner_id) != SQLITE_OK) {
        fprintf(stderr, "Chyba pri väzbe ID majiteľa inventára: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return;
    }

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        fprintf(stderr, "Chyba pri vykonávaní SQL dotazu: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return;
    }

    sqlite3_finalize(stmt);
    wprintf(L"Inventár bol vytvorený.\n");
}


////////////////////////////////////////////////

// Callback funkcia pre získanie počtu riadkov z SQL dotazu
static int callback_count(void *data, int argc, char **argv, char **azColName) {
    int *count = (int *)data;
    *count = atoi(argv[0]);
    return 0;
}
void create_default_user(sqlite3 *db) {
    char *check_user_query = "SELECT COUNT(*) FROM users WHERE username = 'admin';";
    int user_count = 0;

    // Overenie, či užívateľ existuje
    sqlite3_exec(db, check_user_query, callback_count, &user_count, 0);

    if (user_count == 0) {
        char *insert_user_query = "INSERT INTO users (username, password, role) VALUES ('admin', 'admin', '1');";

        // Vytvorenie užívateľa iba ak neexistuje
        sqlite3_exec(db, insert_user_query, 0, 0, 0);
    }
}

int login(sqlite3 *db, User *user) {
    char username[50];
    char password[50];

    wprintf(L"-- Prihlásenie --\n");
    wprintf(L"Zadaj používateľské meno: ");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = '\0';  // Odstráni znak nového riadku zo vstupu

    wprintf(L"Zadaj heslo: ");
    fgets(password, sizeof(password), stdin);
    password[strcspn(password, "\n")] = '\0';  // Odstráni znak nového riadku zo vstupu

    sqlite3_stmt *stmt;
    char *query = "SELECT * FROM users WHERE username = ?;";
    if (sqlite3_prepare_v2(db, query, -1, &stmt, 0) != SQLITE_OK) {
        fprintf(stderr, "Chyba pri príprave SQL dotazu: %s\n", sqlite3_errmsg(db));
        return 0;
    }

    // Väzba používateľského mena
    if (sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC) != SQLITE_OK) {
        fprintf(stderr, "Chyba pri väzbe používateľského mena: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 0;
    }

    int result = sqlite3_step(stmt);
    if (result == SQLITE_ROW) {
        const unsigned char *storedPassword = sqlite3_column_text(stmt, 2);
        if (strcmp(password, (const char*)storedPassword) == 0) {
            printf("Heslo z db: %s",(const char*)storedPassword);
            printf("Heslo od usera: %s",password);
            user->id = sqlite3_column_int(stmt, 0);
            strcpy(user->username, sqlite3_column_text(stmt, 1));
            strcpy(user->password, storedPassword);
            user->role = sqlite3_column_int(stmt, 3);
            clearConsole();
            wprintf(L"\x1b[32mPrihlásenie úspešné. Vitaj, %s!\x1b[0m\n", user->username);
            sqlite3_finalize(stmt);
            return 1;
        }
    } else if (result != SQLITE_DONE) {
        fprintf(stderr, "Chyba pri vykonávaní SQL dotazu: %s\n", sqlite3_errmsg(db));
    }
    clearConsole();
    wprintf(L"\x1b[31m[ERROR]\x1b[0m Nesprávne používateľské meno alebo heslo.\n");
    sqlite3_finalize(stmt);
    return 0;
}

void change_password(sqlite3 *db, User *user) {
    char new_password[50];

    wprintf(L"-- Zmena hesla --\n");
    wprintf(L"Zadajte nové heslo: ");
    scanf("%s", new_password);

    char query[100];
    sprintf(query, "UPDATE users SET password = '%s' WHERE id = %d;", new_password, user->id);

    if (sqlite3_exec(db, query, 0, 0, 0) != SQLITE_OK) {
        fprintf(stderr, "Chyba pri vykonávaní SQL dotazu: %s\n", sqlite3_errmsg(db));
        return;
    }

    wprintf(L"Heslo bolo úspešne zmenené.\n");
}


void update_user(sqlite3 *db, User *user) {
    int user_id;
    char new_username[50];
    char query[100];

    wprintf(L"-- Úprava používateľa --\n");
    if(user->role == 1){
        print_users(db, user, 0);
        wprintf(L"\nVaše ID je : %d\n",user->id);
        wprintf(L"\nZadajte ID používateľa, ktorého chcete upraviť: ");
        scanf("%d", &user_id);
    }
    else{
        user_id = user->id;
    }

    wprintf(L"Zadajte nové používateľské meno: ");
    scanf("%s", new_username);

    sprintf(query, "UPDATE users SET username = '%s' WHERE id = %d;", new_username, user_id);

    if (sqlite3_exec(db, query, 0, 0, 0) != SQLITE_OK) {
        fprintf(stderr, "Chyba pri vykonávaní SQL dotazu: %s\n", sqlite3_errmsg(db));
        return;
    }

    wprintf(L"Používateľ bol úspešne upravený.\n");
}

/////////////
////// Nedotýkať komplet hotové 

void update_inventory(sqlite3 *db, User *user) {
    int inventory_id;
    char new_name[50];
    char query[100];
    
    if(user->role==1){
        print_inventories(db, user, 0);
        wprintf(L"\n-- Úprava inventáru --\n");
        wprintf(L"Zadaj ID inventáru, ktorý chceš upraviť: ");
        scanf("%d", &inventory_id);
    }else{
        wprintf(L"-- Úprava inventáru --\n");
    }

    wprintf(L"Zadaj nový názov inventáru: ");
    scanf("%s", new_name);

    sprintf(query, "UPDATE inventories SET name = '%s' WHERE id = %d;", new_name, inventory_id);

    if (sqlite3_exec(db, query, 0, 0, 0) != SQLITE_OK) {
        fprintf(stderr, "Chyba pri vykonávaní SQL dotazu: %s\n", sqlite3_errmsg(db));
        return;
    }

    wprintf(L"Inventár bol úspešne upravený.\n");
}


/////////////
////// Nedotýkať komplet hotové 
void add_item_to_inventory(sqlite3 *db, User *user) {
    clearConsole();
    int inventory_id;
    char name[50];
    char query[100];
    if (user->role==1){
        print_inventories(db, user, 0);
        wprintf(L"-- Pridanie položky do inventáru --\n");
        wprintf(L"Zadajte ID inventáru: ");
        scanf("%d", &inventory_id);
        wprintf(L"Zadajte názov položky: ");
        scanf("%s", name);

        sprintf(query, "INSERT INTO items (inventory_id, name) VALUES (%d, '%s');", inventory_id, name);
        if (sqlite3_exec(db, query, 0, 0, 0) != SQLITE_OK) {
            fprintf(stderr, "Chyba pri vykonávaní SQL dotazu: %s\n", sqlite3_errmsg(db));
            return;
        }
        
        wprintf(L"Položka '%s' bola úspešne pridaná do inventáru s ID '%d'\n",name,inventory_id);
        enterwait();
    }else{
        inventory_id = get_inventory_id(db, user);
        wprintf(L"-- Pridanie položky do tvojho inventáru --\n");
        wprintf(L"Zadaj názov položky: ");
        scanf("%s", name);

        sprintf(query, "INSERT INTO items (inventory_id, name) VALUES (%d, '%s');", inventory_id, name);
        if (sqlite3_exec(db, query, 0, 0, 0) != SQLITE_OK) {
            fprintf(stderr, "Chyba pri vykonávaní SQL dotazu: %s\n", sqlite3_errmsg(db));
            return;
        }
        
        wprintf(L"Položka '%s' bola úspešne pridaná do tvojho inventáru\n",name);
        enterwait();
    }
    
}

void remove_item_from_inventory(sqlite3 *db, User *user) {
    int item_id;
    int inventory_id;
    if(user->role==1){
        print_inventories(db, user, 0);
        wprintf(L"\nZadaj ID inventáru z ktorého chceš mazať položku : ");
        scanf("%d", &inventory_id);
    } else {
        inventory_id = get_inventory_id(db, user);
    }

    print_inv_items(db, inventory_id, 0);
    wprintf(L"\n-- Odstránenie položky z inventáru --\n");
    wprintf(L"Zadajte ID položky, ktorú chcete odstrániť: ");
    scanf("%d", &item_id);

    char query[100];
    sprintf(query, "DELETE FROM items WHERE id = %d;", item_id);

    if (sqlite3_exec(db, query, 0, 0, 0) != SQLITE_OK) {
        fprintf(stderr, "Chyba pri vykonávaní SQL dotazu: %s\n", sqlite3_errmsg(db));
        return;
    }

    wprintf(L"Položka s ID '%d' bola úspešne odstránená z inventáru s ID '%d'\n", item_id,inventory_id);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////
////////// main

int main() {

    // V podstate nepodstatné pre fungovanie programu set locale je na funkciu diskritiky v printoch a clear console estetické.
    setlocale(LC_ALL, ""); 
    clearConsole();


    // Definícia pointru na databázu a iniciovanie databázy
    sqlite3 *db;
    int rc = sqlite3_open(DB_NAME, &db);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Chyba pri otváraní databázy: %s\n", sqlite3_errmsg(db));
        return 1;
    }


    // Vytvorenie tabuliek ak neexistujú a vytvorenie defaultneho usera (admin s heslo admin)
    create_tables(db);
    create_default_user(db);



    // Vytvorenie objektu user info ktorý sa následne vloží do pointru user a overenie loginu
    User user_info; 
    while (1==1)
    {
        if (login(db, &user_info) == 1){
            break;
        }
    }
    User *user = &user_info;
    sleep(3);


    // Administrátorske menu iba ak má uživatel rolu 1-admin
    int choice;
    if(user->role==1){
        do {
            clearConsole();
            wprintf(L"\n\x1b[32m--  Admin Menu  --\x1b[0m\n");
            wprintf(L"1. Vytvorenie nového používateľa\n");
            wprintf(L"2. Vytvorenie nového inventára\n");
            wprintf(L"3. Zmena hesla\n");
            wprintf(L"4. Úprava používateľa\n");
            wprintf(L"5. Úprava inventáru\n");
            wprintf(L"6. Pridanie položky do inventáru\n");
            wprintf(L"7. Odstránenie položky z inventáru\n");
            wprintf(L"8. Výpis položiek v inventári\n");
            wprintf(L"9. Zoznam používateľov\n");
            wprintf(L"10. Zoznam inventárov\n");
            wprintf(L"0. Ukončiť program\n");

            wprintf(L"\n\x1b[32mZvoľ možnosť: \x1b[0m");
            scanf("%d", &choice);

            switch (choice) {
                case 1:
                    create_user(db, user);
                    break;
                case 2:
                    create_inventory(db);
                    break;
                case 3:
                    change_password(db, user);
                    break;
                case 4:
                    update_user(db, user);
                    break;
                case 5:
                    update_inventory(db, user);
                    break;
                case 6:
                    add_item_to_inventory(db, user);
                    break;
                case 7:
                    remove_item_from_inventory(db, user);
                    break;
                case 8:
                    print_inv_items(db, get_inventory_id(db, user), 1);
                    break;
                case 9:
                    print_users(db, user, 1);
                    break;
                case 10:
                    print_inventories(db, user, 1);
                    break;
                case 0:
                    wprintf(L"EasyInventory ukončený.\n");
                    wprintf(L"Ďakujeme za využívanie našho InventoryManagementu");
                    sleep(5);
                    clearConsole();
                    break;
                default:
                    wprintf(L"\x1b[31m[ERROR]\x1b[0m Neplatná voľba.\n");
                    sleep(2);
            }
        } while (choice != 0);
    }

    // Default user menu ak má používatel akúkoľvek inú rolu ako 1-admin

    else{
        do {
            clearConsole();
            wprintf(L"\n\x1b[32m--  Menu  --\x1b[0m\n");
            wprintf(L"1. Zmena hesla\n");
            wprintf(L"2. Úprava inventáru\n");
            wprintf(L"3. Pridanie položky do inventáru\n");
            wprintf(L"4. Odstránenie položky z inventáru\n");
            wprintf(L"5. Výpis položiek v inventári\n");
            wprintf(L"0. Ukončiť program\n");

            wprintf(L"\n\x1b[32mZvoľ možnosť: \x1b[0m");
            scanf("%d", &choice);

            switch (choice) {
                case 1:
                    change_password(db, user);
                    break;
                case 2:
                    update_inventory(db, user);
                    break;
                case 3:
                    add_item_to_inventory(db, user);
                    break;
                case 4:
                    remove_item_from_inventory(db, user);
                    break;
                case 5:
                    print_inv_items(db, get_inventory_id(db, user), 1);
                    break;
                case 0:
                    wprintf(L"EasyInventory ukončený.\n");
                    wprintf(L"Ďakujeme za využívanie našho InventoryManagementu");
                    sleep(5);
                    clearConsole();
                    break;
                default:
                    wprintf(L"\x1b[31m[ERROR]\x1b[0m Neplatná voľba.\n");
                    sleep(2);
            }
        } while (choice != 0);
    }


    // Uzavretie databázy a koniec programu
    sqlite3_close(db);
    return 0;
}
