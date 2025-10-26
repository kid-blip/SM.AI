#include <iostream>
#include <string>
#include <stack>
#include <sstream>
#include <cmath> 
#include <algorithm> 
#include <vector>
#include <stdexcept>
#include <cctype> 
#include <iomanip>

// === [ Utility Functions for Shell ] ===

void clear_screen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

std::string to_lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), 
                   [](unsigned char c){ return std::tolower(c); });
    return s;
}

/**
 * @brief Prints the help message, adjusted for removed functions.
 */
void print_help() {
    std::cout << "\n--- SM.AI HELP ---" << std::endl;
    std::cout << "  Available Shell Commands:" << std::endl;
    std::cout << "    exit            - Terminate the program." << std::endl;
    std::cout << "    help            - Show this help message." << std::endl;
    std::cout << "    clear           - Clear the screen." << std::endl;
    std::cout << "\n  Mathematical Expressions (RPN Parser):" << std::endl;
    std::cout << "    Supports: +, -, *, /, ^, ( ), unary minus." << std::endl;
    std::cout << "    FUNCTIONS: abs()." << std::endl; // Only abs() remains
    std::cout << "    Example: 5 * 2 + abs(-10)" << std::endl;
    std::cout << "\n  Physics Calculations (Conversion):" << std::endl;
    std::cout << "    CONVERT [value] [unit_1] TO [unit_2] (or 'conv' ... 'in' ...)" << std::endl;
    std::cout << "    SPEED: m/s <-> km/h" << std::endl;
    std::cout << "    TEMPERATURE: C <-> F <-> K (Celsius, Fahrenheit, Kelvin)" << std::endl;
    std::cout << "    MASS: kg <-> lb (kilograms, pounds)" << std::endl;
    std::cout << "    Example: conv 100 kg to lb" << std::endl;
    std::cout << "----------------------" << std::endl;
}

std::string format_double(double value) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(6) << value;
    
    std::string s = ss.str();
    s.erase(s.find_last_not_of('0') + 1, std::string::npos);
    if (s.back() == '.') {
        s.pop_back();
    }
    return s;
}

// === [ MathEvaluator Core ] ===

class MathEvaluator {
public:
    double evaluate(const std::string& expression) {
        std::string cleaned_expr = expression;
        cleaned_expr.erase(std::remove_if(cleaned_expr.begin(), cleaned_expr.end(), ::isspace), cleaned_expr.end());

        if (cleaned_expr.empty()) {
            throw std::runtime_error("Error: Empty expression."); 
        }

        std::vector<std::string> rpn = shuntingYard(cleaned_expr);
        return evaluateRPN(rpn);
    }

    // Conversion logic remains the same (it was stable)
    std::string evaluateConversion(const std::string& expression) {
        std::stringstream ss(expression);
        std::string command, value_str, unit_from, keyword_to, unit_to;
        double value;

        ss >> command; 
        
        if (!(ss >> value_str >> unit_from >> keyword_to >> unit_to)) {
            return "Error: Insufficient arguments for conversion. Expected: CONVERT [value] [unit_1] TO [unit_2]";
        }

        if (to_lower(keyword_to) != "to") {
            return "Syntax Error: Expected keyword 'TO'. Use: CONVERT [value] [unit_1] TO [unit_2]";
        }
        
        try {
            value = std::stod(value_str);
        } catch (...) {
            return "Error: Invalid number for conversion (" + value_str + ").";
        }

        std::string unit_from_lower = to_lower(unit_from);
        std::string unit_to_lower = to_lower(unit_to);

        double value_in_base_unit = value; 
        bool is_speed_conversion = false;
        bool is_temp_conversion = false;
        bool is_mass_conversion = false;

        // --- 1. Determine Type and Convert to Base Unit ---

        if (unit_from_lower == "km/h" || unit_from_lower == "m/s") {
            is_speed_conversion = true;
            if (unit_from_lower == "km/h") {
                value_in_base_unit = value * 1000.0 / 3600.0; 
            }
        } 
        else if (unit_from_lower == "c" || unit_from_lower == "f" || unit_from_lower == "k") {
            is_temp_conversion = true;
            if (unit_from_lower == "c") {
                value_in_base_unit = value + 273.15; 
            } else if (unit_from_lower == "f") {
                value_in_base_unit = (value + 459.67) / 1.8; 
            }
        }
        else if (unit_from_lower == "kg" || unit_from_lower == "lb") {
            is_mass_conversion = true;
            if (unit_from_lower == "lb") {
                value_in_base_unit = value / 2.20462; 
            }
        }
        else {
            return "Error: Unsupported source unit: " + unit_from;
        }

        // --- 2. Convert from Base Unit to Target Unit and Type Check ---
        
        double final_result = value_in_base_unit;
        
        if (unit_to_lower == "m/s") {
            if (!is_speed_conversion) return "Error: Cannot convert " + unit_from + " to " + unit_to + ".";
        } else if (unit_to_lower == "km/h") {
            if (!is_speed_conversion) return "Error: Cannot convert " + unit_from + " to " + unit_to + ".";
            final_result = value_in_base_unit * 3600.0 / 1000.0;
        } 
        else if (unit_to_lower == "c" || unit_to_lower == "f" || unit_to_lower == "k") {
            if (!is_temp_conversion) return "Error: Cannot convert " + unit_from + " to " + unit_to + ".";
            if (unit_to_lower == "c") final_result = value_in_base_unit - 273.15;
            else if (unit_to_lower == "f") final_result = value_in_base_unit * 1.8 - 459.67;
        }
        else if (unit_to_lower == "kg") {
            if (!is_mass_conversion) return "Error: Cannot convert " + unit_from + " to " + unit_to + ".";
        } else if (unit_to_lower == "lb") {
            if (!is_mass_conversion) return "Error: Cannot convert " + unit_from + " to " + unit_to + ".";
            final_result = value_in_base_unit * 2.20462;
        }
        else {
            return "Error: Unsupported target unit: " + unit_to;
        }

        return format_double(final_result) + " " + unit_to; 
    }

private:
    // --- Parser Helpers ---

    bool isNumber(const std::string& token) const {
        if (token.empty()) return false;
        char* p;
        strtod(token.c_str(), &p);
        return (*p == 0);
    }
    
    // isFunction is simplified to only check for "abs"
    bool isFunction(const std::string& token) const {
        return token == "abs"; 
    }
    
    int getPrecedence(const std::string& op) const {
        if (op == "+" || op == "-") return 1;
        if (op == "*" || op == "/") return 2;
        if (op == "^") return 3; 
        if (op == "~") return 4; 
        return 0;
    }

    bool isRightAssociative(const std::string& op) const {
        return op == "^" || op == "~";
    }

    bool isOperator(const std::string& token) const {
        return token == "+" || token == "-" || token == "*" || token == "/" || token == "^" || token == "~";
    }
    
    bool isParenthesis(const std::string& token) const {
        return token == "(" || token == ")";
    }


    // --- Shunting-yard Algorithm ---
    std::vector<std::string> shuntingYard(const std::string& expression) {
        std::vector<std::string> outputQueue;
        std::stack<std::string> operatorStack;
        bool expect_value = true; 

        for (size_t i = 0; i < expression.length(); ++i) {
            char c = expression[i];

            if (isdigit(c) || c == '.') {
                std::string currentToken;
                while (i < expression.length() && (isdigit(expression[i]) || expression[i] == '.')) {
                    currentToken += expression[i];
                    i++;
                }
                i--; 
                outputQueue.push_back(currentToken);
                expect_value = false;
            } else if (c == '(') {
                operatorStack.push("(");
                expect_value = true;
            } else if (c == ')') {
                while (!operatorStack.empty() && operatorStack.top() != "(") {
                    outputQueue.push_back(operatorStack.top());
                    operatorStack.pop();
                }
                if (!operatorStack.empty()) {
                    operatorStack.pop(); 
                    
                    if (!operatorStack.empty() && isFunction(operatorStack.top())) {
                        outputQueue.push_back(operatorStack.top());
                        operatorStack.pop();
                    }
                } else {
                    throw std::runtime_error("Error: Mismatched parentheses.");
                }
                expect_value = false;
            } else if (c == '-' && expect_value) { 
                std::string op1 = "~"; 
                while (!operatorStack.empty()) {
                    std::string op2 = operatorStack.top();
                    if (isOperator(op2) && 
                        ((!isRightAssociative(op1) && getPrecedence(op1) <= getPrecedence(op2)) || 
                         (isRightAssociative(op1) && getPrecedence(op1) < getPrecedence(op2)))) 
                    {
                        outputQueue.push_back(op2);
                        operatorStack.pop();
                    } else {
                        break;
                    }
                }
                operatorStack.push(op1);
                expect_value = true;
            }
            else if (isOperator(std::string(1, c))) { 
                std::string op1(1, c);
                while (!operatorStack.empty()) {
                    std::string op2 = operatorStack.top();
                    if (isOperator(op2) && 
                        ((!isRightAssociative(op1) && getPrecedence(op1) <= getPrecedence(op2)) || 
                         (isRightAssociative(op1) && getPrecedence(op1) < getPrecedence(op2)))) 
                    {
                        outputQueue.push_back(op2);
                        operatorStack.pop();
                    } else {
                        break;
                    }
                }
                operatorStack.push(op1);
                expect_value = true;
            }
            // FUNCTION HANDLER (Only looks for "abs")
            else if (isalpha(c)) {
                std::string currentToken;
                while (i < expression.length() && isalpha(expression[i])) {
                    currentToken += expression[i];
                    i++;
                }
                i--; 

                if (isFunction(currentToken)) {
                    operatorStack.push(currentToken);
                } else {
                    throw std::runtime_error("Error: Unknown function or identifier: " + currentToken + ". Only 'abs' is supported.");
                }
            }
        }

        while (!operatorStack.empty()) {
            if (isParenthesis(operatorStack.top())) {
                throw std::runtime_error("Error: Mismatched parentheses.");
            }
            outputQueue.push_back(operatorStack.top());
            operatorStack.pop();
        }

        return outputQueue;
    }

    // --- RPN Evaluation (Simplified) ---
    double evaluateRPN(const std::vector<std::string>& rpn) {
        std::stack<double> valueStack;

        for (const std::string& token : rpn) {
            if (isNumber(token)) {
                valueStack.push(std::stod(token));
            } else if (token == "~") { 
                if (valueStack.empty()) {
                    throw std::runtime_error("Error: Insufficient operands for unary operator.");
                }
                double val = valueStack.top(); valueStack.pop();
                valueStack.push(-val);
            } 
            // FUNCTION EXECUTION (Only abs)
            else if (isFunction(token)) {
                 if (valueStack.empty()) {
                    throw std::runtime_error("Error: Insufficient arguments for function " + token);
                }
                double val = valueStack.top(); valueStack.pop();

                if (token == "abs") {
                    valueStack.push(std::abs(val)); 
                } 
                // Removed other functions: sin, cos, tan, sqrt, log, log10
            }
            // BINARY OPERATORS
            else if (isOperator(token)) {
                if (valueStack.size() < 2) {
                    throw std::runtime_error("Error: Insufficient operands for operation.");
                }
                double val2 = valueStack.top(); valueStack.pop();
                double val1 = valueStack.top(); valueStack.pop();
                double result = 0;

                if (token == "+") result = val1 + val2;
                else if (token == "-") result = val1 - val2;
                else if (token == "*") result = val1 * val2;
                else if (token == "/") {
                    if (val2 == 0) throw std::runtime_error("Error: Division by zero.");
                    result = val1 / val2;
                }
                else if (token == "^") result = std::pow(val1, val2);
                
                valueStack.push(result);
            }
        }
        
        if (valueStack.size() != 1) {
            throw std::runtime_error("Error: Expression contains extraneous values or operators.");
        }

        return valueStack.top();
    }
};

// === [ Main Shell Function ] ===

int main() {
    MathEvaluator evaluator;
    std::string expression;

    print_help(); 

    while (true) {
        std::cout << "\nSM.AI> "; 
        std::getline(std::cin, expression);

        std::string first_token = "";
        std::stringstream ss(expression);
        ss >> first_token;
        
        std::string command_lower = to_lower(first_token);

        // 1. Handle Shell Commands
        if (command_lower == "exit") {
            break;
        } else if (command_lower == "help") {
            print_help();
            continue;
        } else if (command_lower == "clear") {
            clear_screen();
            print_help(); 
            continue;
        } else if (expression.empty() || first_token.empty()) {
            continue;
        }
        
        // 2. Handle Conversion Command
        bool is_conversion = (command_lower == "convert") || (command_lower == "conv");
        
        if (is_conversion) {
            std::string conversion_expr = expression;
            size_t first_token_len = first_token.length();

            if (command_lower == "conv") {
                conversion_expr.replace(0, first_token_len, "CONVERT");
                
                size_t in_pos_en = conversion_expr.find(" in ");
                size_t in_pos_ru = conversion_expr.find(" в ");
                
                if (in_pos_en != std::string::npos) {
                    conversion_expr.replace(in_pos_en, 4, " TO ");
                } else if (in_pos_ru != std::string::npos) {
                    conversion_expr.replace(in_pos_ru, 3, " TO ");
                }
            }
            
            std::string result = evaluator.evaluateConversion(conversion_expr);
            std::cout << "Result: " << result << std::endl;
            continue;
        }

        // 3. Handle Mathematical Expressions
        try {
            double result = evaluator.evaluate(expression);
            std::cout << "Result: " << format_double(result) << std::endl;
        } catch (const std::runtime_error& e) {
            std::cerr << "!!! ERROR: " << e.what() << std::endl;
        } catch (...) {
            std::cerr << "!!! ERROR: Unknown calculation error." << std::endl;
        }
    }

    std::cout << "SM.AI finished work. Goodbye!" << std::endl;
    return 0;
}
