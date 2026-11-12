#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <string>
#include <iomanip>
#include <limits>
#include <stdexcept>
using namespace std;

class Meal {
private:
    string date;
    double prepared, wasted;
protected:
    Meal(string d, double p, double w) : date(d), prepared(p), wasted(w) {
        if (date.empty() || p < 0 || w < 0 || w > p)
            throw invalid_argument("Wasted food cannot exceed prepared food or quantities are invalid.");
    }
public:
    virtual string getName() const = 0;
    virtual double costPerKg() const = 0;
    virtual double co2PerKg() const = 0;
    virtual string advice() const = 0;

    string getDate() const { return date; }
    double getPrepared() const { return prepared; }
    double getWasted() const { return wasted; }
    double wastePercent() const {
        return prepared == 0 ? 0 : wasted * 100.0 / prepared;
    }
    double moneyLost() const { return wasted * costPerKg(); }
    double co2Impact() const { return wasted * co2PerKg(); }

    virtual void display() const {
        cout << left << setw(12) << getName() << setw(13) << date
             << right << setw(10) << fixed << setprecision(1) << prepared
             << setw(10) << wasted << setw(10) << wastePercent() << "%\n";
    }
    virtual ~Meal() {}
};

class Breakfast : public Meal {
public:
    Breakfast(string d, double p, double w) : Meal(d, p, w) {}
    string getName() const override { return "Breakfast"; }
    double costPerKg() const override { return 60.0; }
    double co2PerKg() const override { return 2.0; }
    string advice() const override {
        return "Check attendance before preparing breakfast.";
    }
};

class Lunch : public Meal {
public:
    Lunch(string d, double p, double w) : Meal(d, p, w) {}
    string getName() const override { return "Lunch"; }
    double costPerKg() const override { return 90.0; }
    double co2PerKg() const override { return 2.5; }
    string advice() const override {
        return "Adjust lunch portions according to student attendance.";
    }
};

class Dinner : public Meal {
public:
    Dinner(string d, double p, double w) : Meal(d, p, w) {}
    string getName() const override { return "Dinner"; }
    double costPerKg() const override { return 110.0; }
    double co2PerKg() const override { return 3.0; }
    string advice() const override {
        return "Take a headcount by 6 pm and cook less on low-attendance days.";
    }
};

unique_ptr<Meal> makeMeal(const string& type, const string& date,
                          double prepared, double wasted) {
    if (type == "Breakfast") return make_unique<Breakfast>(date, prepared, wasted);
    if (type == "Lunch") return make_unique<Lunch>(date, prepared, wasted);
    if (type == "Dinner") return make_unique<Dinner>(date, prepared, wasted);
    throw invalid_argument("Unknown meal. Use Breakfast, Lunch or Dinner.");
}

class Mess {
private:
    vector<unique_ptr<Meal>> meals;

public:
    void addEntry() {
        string type, date;
        double prepared, wasted;
        cout << "Enter meal (Breakfast/Lunch/Dinner): ";
        cin >> type;
        cout << "Enter date (YYYY-MM-DD): ";
        cin >> date;
        cout << "Enter prepared food (kg): ";
        if (!(cin >> prepared)) { invalidInput(); return; }
        cout << "Enter wasted food (kg): ";
        if (!(cin >> wasted)) { invalidInput(); return; }

        try {
            meals.push_back(makeMeal(type, date, prepared, wasted));
            cout << "Entry added successfully.\n";
        } catch (const exception& e) {
            cout << "Error: " << e.what() << "\n";
        }
    }

    void invalidInput() {
        cout << "Invalid numeric input.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    void viewEntries() const {
        if (meals.empty()) { cout << "No entries available.\n"; return; }
        cout << "\nMEAL RECORDS\n";
        cout << left << setw(12) << "Meal" << setw(13) << "Date"
             << right << setw(10) << "Prepared" << setw(10) << "Wasted"
             << setw(11) << "Waste %\n";
        for (const auto& meal : meals) meal->display();
    }

    void summary() const {
        if (meals.empty()) {
            cout << "No entries available. Add records first.\n";
            return;
        }
        double totalPrepared = 0, totalWasted = 0, totalMoney = 0, totalCO2 = 0;
        double prepared[3] = {0, 0, 0}, wasted[3] = {0, 0, 0};
        const string names[3] = {"Breakfast", "Lunch", "Dinner"};

        for (const auto& meal : meals) {
            totalPrepared += meal->getPrepared();
            totalWasted += meal->getWasted();
            totalMoney += meal->moneyLost();
            totalCO2 += meal->co2Impact();
            for (int i = 0; i < 3; ++i) {
                if (meal->getName() == names[i]) {
                    prepared[i] += meal->getPrepared();
                    wasted[i] += meal->getWasted();
                }
            }
        }

        cout << fixed << setprecision(1);
        cout << "\nSUMMARY AND ADVICE\n";
        cout << "Prepared: " << totalPrepared << " kg\n";
        cout << "Wasted: " << totalWasted << " kg\n";
        cout << "Overall waste: "
             << (totalPrepared == 0 ? 0 : totalWasted * 100.0 / totalPrepared)
             << "%\n";
        cout << "Money lost: Rs " << totalMoney << "\n";
        cout << "Estimated CO2: " << totalCO2 << " kg\n\n";

        int highest = 0;
        for (int i = 1; i < 3; ++i) {
            double current = prepared[i] == 0 ? 0 : wasted[i] * 100.0 / prepared[i];
            double best = prepared[highest] == 0 ? 0 :
                          wasted[highest] * 100.0 / prepared[highest];
            if (current > best) highest = i;
        }

        for (int i = 0; i < 3; ++i) {
            double percent = prepared[i] == 0 ? 0 : wasted[i] * 100.0 / prepared[i];
            int bars = static_cast<int>(percent / 2.0 + 0.5);
            cout << left << setw(12) << names[i] << right << setw(5)
                 << percent << "%   ";
            for (int j = 0; j < bars; ++j) cout << "#";
            cout << "\n";
        }

        cout << "\nHighest waste: " << names[highest] << "\n";
        cout << "Advice: ";
        if (prepared[highest] == 0) cout << "No records for this meal.\n";
        else if (highest == 0) cout << "Check attendance before preparing breakfast.\n";
        else if (highest == 1) cout << "Adjust lunch portions according to student attendance.\n";
        else cout << "Many students may skip dinner on some days. Take a headcount by 6 pm and cook 10% less on low days.\n";
    }

    void deleteEntry() {
        if (meals.empty()) { cout << "No entries to delete.\n"; return; }
        viewEntries();
        int index;
        cout << "Enter entry number to delete (1-" << meals.size() << "): ";
        if (!(cin >> index)) { invalidInput(); return; }
        if (index < 1 || index > static_cast<int>(meals.size())) {
            cout << "Invalid entry number.\n";
            return;
        }
        meals.erase(meals.begin() + index - 1);
        cout << "Entry deleted.\n";
    }

    void save() const {
        ofstream fout("mess_data.txt");
        if (!fout) { cout << "Unable to open file for saving.\n"; return; }
        for (const auto& meal : meals)
            fout << meal->getName() << " " << meal->getDate() << " "
                 << meal->getPrepared() << " " << meal->getWasted() << "\n";
        cout << "Records saved to mess_data.txt\n";
    }

    void load() {
        ifstream fin("mess_data.txt");
        if (!fin) { cout << "No saved file found.\n"; return; }
        vector<unique_ptr<Meal>> loaded;
        string type, date;
        double prepared, wasted;
        try {
            while (fin >> type >> date >> prepared >> wasted)
                loaded.push_back(makeMeal(type, date, prepared, wasted));
            meals = move(loaded);
            cout << "Records loaded successfully.\n";
        } catch (const exception& e) {
            cout << "File contains invalid data: " << e.what() << "\n";
        }
    }
};

int main() {
    Mess mess;
    int choice;
    do {
        cout << "\n===== MESS WASTE TRACKER =====\n";
        cout << "1. Add meal entry\n2. View all entries\n3. Summary and advice\n";
        cout << "4. Delete entry\n5. Save records\n6. Load records\n0. Exit\n";
        cout << "Enter choice: ";
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input.\n";
            continue;
        }
        switch (choice) {
            case 1: mess.addEntry(); break;
            case 2: mess.viewEntries(); break;
            case 3: mess.summary(); break;
            case 4: mess.deleteEntry(); break;
            case 5: mess.save(); break;
            case 6: mess.load(); break;
            case 0: cout << "Thank you for using Mess Waste Tracker.\n"; break;
            default: cout << "Invalid choice. Try again.\n";
        }
    } while (choice != 0);
    return 0;
}
