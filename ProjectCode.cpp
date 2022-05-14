#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <iomanip>
#include <cstring>
#include <occi.h>

using oracle::occi::Environment;
using oracle::occi::Connection;
using namespace oracle::occi;
using namespace std;

//struct
struct ShoppingCart {
	int product_id;
	double price;
	int quantity;
};

//function prototype

int mainMenu();
int customerLogin(Connection* conn, int custId);
int addToCart(Connection* conn, struct ShoppingCart cart[]);
double findProduct(Connection* conn, int product_id);
void displayProducts(struct ShoppingCart cart[], int productCount);
int checkout(Connection* conn, struct ShoppingCart car[], int customerId, int productCount);

//Function Utilities prototype
int getInt(int min, int max);
char yesNo();

const int MAX = 100000;

int main(void) {
	// OCCI Variables
	Environment* env{ nullptr };
	Connection* conn{ nullptr };
	// User Variables
	string str;
	string user{ "USERNAME" }; //Login
	string pass{ "PASSWORD" }; //Password
	string constr{ "HOST_ADDRESS" }; //host address

	//Operation variable initialized to safe state
	int option{ -1 };
	int custId{ -1 };
	int count{ 0 };
	ShoppingCart custCart[5];

	try {
		env = Environment::createEnvironment(Environment::DEFAULT);
		conn = env->createConnection(user, pass, constr);

		while (option != 0) {

			option = mainMenu();

			if (option == 1) {
				cout << "Enter the customer ID: ";
				custId = getInt(0, MAX);
				if (customerLogin(conn, custId) == 1) {
					count = addToCart(conn, custCart);
					displayProducts(custCart, count);
					checkout(conn, custCart, custId, count);
				}
				else {
					cout << "The customer does not exist." << endl;
				}
			}
			else if (option == 0) {
				cout << "Good bye!..." << endl;

				env->terminateConnection(conn); //closes connection
				Environment::terminateEnvironment(env); //closes environment
			}
		}
	}

	catch (SQLException & sqlExcp) {
		cout << sqlExcp.getErrorCode() << ": " << sqlExcp.getMessage();
	}

	return 0;
}

//Function definitions
int mainMenu() { //displays menu to user and ask user for a menu selection
	int option{ -1 };
	cout << "*************Main Menu********************";
	cout << endl << "1)      Login";
	cout << endl << "0)      Exit";
	cout << endl << "Enter an option(0-1): ";

	option = getInt(0, 1);

	while (option < 0) {
		cout << "*************Main Menu********************";
		cout << endl << "1)      Login";
		cout << endl << "0)      Exit";
		cout << endl << "You entered a wrong value. Enter an option (0-1): ";
		option = getInt(0, 1);
	}
	return option;
}

int customerLogin(Connection* conn, int custId) {
	int valid{ 0 };

	Statement* stmt = conn->createStatement();

	stmt->setSQL("BEGIN find_customer(:1, :2); END;");
	stmt->setInt(1, custId);
	stmt->registerOutParam(2, OCCIINT);
	stmt->executeUpdate();
	valid = stmt->getInt(2);

	conn->terminateStatement(stmt);

	return valid;
}

int addToCart(Connection* conn, struct ShoppingCart cart[]) { //adds product to customer's cart
	ShoppingCart temp;
	int prodId{ -1 };
	int prodQuantity{ -1 };
	int numProd{ 0 };
	int choice{ 1 };

	while (choice == 1) { //chooses to not enter any more products or reach max amount of 5
		cout << "-------------- Add Products to Cart --------------" << endl;
		cout << "Enter the product ID: ";
		prodId = getInt(0, MAX);
		while (findProduct(conn, prodId) == 0) {
			cout << "The product does not exist. Try again..." << endl;
			cout << "Enter the product ID:";
			prodId = getInt(0, MAX);
		}
		cout << "Product Price: " << findProduct(conn, prodId) << endl;
		cout << "Enter the product Quantity: ";
		prodQuantity = getInt(0, MAX);

		temp.product_id = prodId;
		temp.price = findProduct(conn, prodId);
		temp.quantity = prodQuantity;
		cart[numProd] = temp;

		numProd++;
		if (numProd < 5) {
			cout << "Enter 1 to add more products or 0 to checkout: ";
			choice = getInt(0, 1);
		}
		else {
			cout << "Max amount of products in cart [5]." << endl;
			choice = 0;
		}
	}

	return numProd;
}

double findProduct(Connection* conn, int product_id) {
	double price{ 0 };
	Statement* stmt = conn->createStatement();

	stmt->setSQL("BEGIN find_product(:1, :2); END;");
	stmt->setInt(1, product_id);
	stmt->registerOutParam(2, OCCIDOUBLE);
	stmt->executeUpdate();
	price = stmt->getDouble(2);

	conn->terminateStatement(stmt);
	return price;
}

void displayProducts(struct ShoppingCart cart[], int productCount) {
	double total{ 0 };

	cout << "------- Ordered Products --------" << endl;
	for (size_t i = 0; i < productCount; i++) {
		cout << "---Item " << i + 1 << endl;
		cout << "Product ID: " << cart[i].product_id << endl;
		cout << "Price: " << cart[i].price << endl;
		cout << "Quantity: " << cart[i].quantity << endl;
		total += (cart[i].price * cart[i].quantity);
	}

	cout << "---------------------------------" << endl;
	cout << "Total: " << total << endl;
}

int checkout(Connection* conn, struct ShoppingCart cart[], int customerId, int productCount) {
	char yn{ '\0' };
	int newOrder{ 0 };

	while (yn == '\0') {
		cout << "Would you like to checkout? (Y/y or N/n) ";
		yn = yesNo();
	}
	if (yn == 'y' || yn == 'Y') {
		cout << "The order is succesfully completed." << endl;
		Statement* stmt = conn->createStatement();

		stmt->setSQL("BEGIN add_order(:1, :2); END;");
		stmt->setInt(1, customerId);
		stmt->registerOutParam(2, OCCIINT);
		stmt->executeUpdate();
		newOrder = stmt->getInt(2);

		stmt->setSQL("BEGIN add_orderline(:1, :2, :3, :4, :5); END;");
		for (size_t i = 0; i < productCount; i++) {
			stmt->setInt(1, newOrder);
			stmt->setInt(2, i + 1);
			stmt->setInt(3, cart[i].product_id);
			stmt->setInt(4, cart[i].quantity);
			stmt->setDouble(5, cart[i].price);
			stmt->executeUpdate();
		}

		conn->terminateStatement(stmt);
		return 1;
	}
	else {
		cout << "The order is cancelled." << endl;
		return 0;
	}
}

int getInt(int min, int max) { //gets integer from user until a valid integer is inputted and return the value
	int igr{ 0 };
	char bfr;

	bool integer{ false };
	while (!integer) {
		if (cin >> igr) { //if integer is input first
			if (cin.get(bfr)) { //obtains buffer input
				cin.putback(bfr); //place bfr back into buffer 
				if (bfr == '\n') {  //if the input after the integer is enter
					if (igr < min || igr > max) {
						if (max != MAX) {
							igr = -1;
							integer = true;
						}
					}
					else {
						integer = true;
					}
				}
				else { //if the input after the integer is not enter
					cout << "Invalid trailing characters, try again: ";
				}
			}
			cin.clear(); //clears buffers
		}
		else { //if integer is not obtained from input
			cout << "Invalid Integer, try again: ";
			cin.clear();
		}
		cin.ignore(100, '\n'); //ignore and discard 100 chacters before the \n
	}

	return igr;
}

char yesNo() { //one instance of (y)es or (n)o, if not valid, return an empty char
	char choice[16] = { '\0' }; //takes up to 16 characters, initialize choice
	cin >> choice;
	cin.clear(); //clears buffer
	cin.ignore(1000, '\n');

	if ((choice[0] != 'Y' && choice[0] != 'N' && choice[0] != 'y' && choice[0] != 'n') || choice[1] != '\0') {
		cout << "Wrong Input. Try again..." << endl;

		return '\0';
	}

	return choice[0];
}