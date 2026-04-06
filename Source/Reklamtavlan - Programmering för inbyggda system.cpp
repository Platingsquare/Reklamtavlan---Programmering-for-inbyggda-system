#include <vector>
#include <string>
#include <iostream>
#include <random>
#include <ctime>
#include <thread>
#include <chrono>
//#include "Advertisements.h"
//#include "Main.h"
#include "Advertisment.h"

// Main function
int main() {
    // Initialize advertisements
    std::vector<Advertisement*> ads = {
        new HederligeHarrysBilar(),
        new FarmorAnkasPajerAB(),
        new SvartePettersSvartbyggen(),
        new LångbensDetektivbyrå()
    };
    // Advertisement class definition (from the previous example)
    class Advertisement {
    public:
        std::string text;
        int cost, displayTime, displayCount; // default display count is 0
        bool visible;

        Advertisement(std::string text, int cost) : text(text), cost(cost), displayCount(0), visible(false) {}

        virtual void display() {
            if (!visible && displayCount < cost) {
                displayCount++;
                visible = true;
                std::cout << text << "\n";
                std::this_thread::sleep_for(std::chrono::seconds(displayTime));
                visible = false;
            }
        }
    };

    // Specializations for different advertisers (from the previous example)
    class HederligeHarrysBilar : public Advertisement {
    public:
        std::vector<std::string> messages = {"Köp bil hos Harry" , "En god bilaffär (för Harry!)", "Hederlige Harrys Bilar"};

        HederligeHarrysBilar() : Advertisement(messages[rand() % 3], 5000) {}
    };

    class FarmorAnkasPajerAB : public Advertisement {
    public:
        std::vector<std::string> messages = {"Köp paj hos Farmor Anka", "Skynda innan Mårten ätit alla pajer"};

        FarmorAnkasPajerAB() : Advertisement(messages[rand() % 2], 3000) {}
    };

    class SvartePettersSvartbyggen : public Advertisement {
    public:
        SvartePettersSvartbyggen() : Advertisement("Låt Petter bygga åt dig", 1500) {}

        void display(int minute) override {
            if (minute % 2 == 0) {
                text = "Låt Petter bygga åt dig" ; // even minutes
            } else {
                text = "Bygga svart? Ring Petter"; // odd minutes
            }
            Advertisement::display();
        }
    };

    class LångbensDetektivbyrå : public Advertisement {
    public:
        std::vector<std::string> messages = {"Mysterier? Ring Långben", "Långben fixar biffen"};

        LångbensDetektivbyrå() : Advertisement(messages[rand() % 2], 4000) {}
    };

    // Main function
    int main() {
        srand(time(nullptr));

        // Initialize advertisements
        std::vector<Advertisement*> ads = {
            new HederligeHarrysBilar(),
            new FarmorAnkasPajerAB(),
            new SvartePettersSvartbyggen(),
            new LångbensDetektivbyrå()
        };

        int lastDisplayedAdIndex = -1;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0.0, 1.0);

        // Main loop for displaying ads
        while (true) {
            // Ensure the same customer is not displayed twice in quick succession
            int currentAdIndex = std::uniform_int_distribution<int>(0, ads.size()-1)(rd);
            while (currentAdIndex == lastDisplayedAdIndex) {
                currentAdIndex = std::uniform_int_distribution<int>(0, ads.size()-1)(rd);
            }

            // Display the selected advertisement and update the last displayed index
            ads[lastDisplayedAdIndex]->display();
            lastDisplayedAdIndex = currentAdIndex;

            // Sleep for 20 seconds between each advertisement
            std::this_thread::sleep_for(std::chrono::seconds(20));
        }

        return 0;
    }
}
