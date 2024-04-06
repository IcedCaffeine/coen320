#include "Communication_System.h"
using namespace std;

int main() {
//	Communication_System communicationSystem;
//	pthread_t CommSystemThread = communicationSystem.createCommunication();
//	pthread_join(CommSystemThread, NULL);

	int choice;

		    bool validInput = false;

		    while (!validInput) {
		        cout << "Please choose an option:" << endl;
		        cout << "1. 30 second out" << endl;
		        cout << "2. Input from txt file" << endl;
		        cout << "3. Option 3" << endl;
		        cout << "4. Option 4" << endl;

		        cin >> choice;

		        if (cin.fail() || choice < 1 || choice > 4) {
		            cout << "Invalid input. Please enter a number between 1 and 4." << endl;
		            cin.clear();
		            cin.ignore(numeric_limits<streamsize>::max(), '\n');
		        }
		        else {
		            validInput = true;
		        }
		    }

		    if(choice == 1){
//		    	int position[3] = {345,0,111};
//		        int speed[3] = {50,50,0};
//		        Aircraft aircraft1 = Aircraft(1, 300, position, speed);
//		        aircraft1.PrintAircraft();
//		        aircraft1.Aircraft();
		    }else if(choice == 2){
		    	// Perform action for option 2: input from txt file
		    	cout << "Option 2: Input from txt file" << endl;

		    	// Read file and output contents
		    	string filename;
		    	cout << "Please enter the filename: ";
		    	cin >> filename;

		    	ifstream infile(filename);
		    	if (infile.is_open()) {
		    		string line;
		    		while (getline(infile, line)) {
		    			cout << line << endl;
		    		}
		    		infile.close();
		    	}else {
		    		cout << "Unable to open file " << filename << endl;
		    	}
		    }

	return 0;
}
