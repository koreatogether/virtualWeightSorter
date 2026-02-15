// Quick validation of the menu input logic
#include <iostream>
#include <string>

// Simulate the logic
enum AppState
{
    NORMAL_OPERATION = 0,
    MENU_ACTIVE = 1,
    EDIT_INDIVIDUAL_ID = 2,
    EDIT_SELECTIVE_ID = 3,
    AUTO_ASSIGN_IDS = 4,
    RESET_ALL_IDS = 5
};

#ifdef UNIT_TEST
int main()
{
    // Test 1: Input "1" should change MENU_ACTIVE to EDIT_INDIVIDUAL_ID
    AppState state = MENU_ACTIVE;
    std::string input = "1";

    std::cout << "Test 1 - ID Management:" << std::endl;
    std::cout << "Initial state: " << state << " (MENU_ACTIVE)" << std::endl;
    std::cout << "Input: " << input << std::endl;

    if (input.length() == 1 && input[0] >= '1' && input[0] <= '6')
    {
        int num = std::stoi(input);
        switch (num)
        {
        case 1:
            state = EDIT_INDIVIDUAL_ID;
            break;
        case 2:
            state = EDIT_SELECTIVE_ID;
            break;
        case 5:
            state = NORMAL_OPERATION;
            break;
        }
    }

    std::cout << "Final state: " << state << " (should be 2 = EDIT_INDIVIDUAL_ID)" << std::endl;
    std::cout << "PASS: " << (state == EDIT_INDIVIDUAL_ID ? "YES" : "NO") << std::endl;
    std::cout << std::endl;

    // Test 2: Input "5" should change MENU_ACTIVE to NORMAL_OPERATION
    state = MENU_ACTIVE;
    input = "5";

    std::cout << "Test 2 - Menu Cancel:" << std::endl;
    std::cout << "Initial state: " << state << " (MENU_ACTIVE)" << std::endl;
    std::cout << "Input: " << input << std::endl;

    if (input.length() == 1 && input[0] >= '1' && input[0] <= '6')
    {
        int num = std::stoi(input);
        switch (num)
        {
        case 1:
            state = EDIT_INDIVIDUAL_ID;
            break;
        case 2:
            state = EDIT_SELECTIVE_ID;
            break;
        case 5:
            state = NORMAL_OPERATION;
            break;
        }
    }

    std::cout << "Final state: " << state << " (should be 0 = NORMAL_OPERATION)" << std::endl;
    std::cout << "PASS: " << (state == NORMAL_OPERATION ? "YES" : "NO") << std::endl;

    return 0;
}
#endif
