🚀 SM.AI Console Calculator

SM.AI (Simple Math Artificial Intelligence) is a robust, cross-platform console utility for quick mathematical calculations and unit conversions. Built in C++, it utilizes the Reverse Polish Notation (RPN) algorithm for reliable expression parsing and evaluation.

✨ Key Features

    Robust Math Parser: Supports standard arithmetic (+, -, *, /, ^), parentheses, and the unary minus (-). Includes the essential abs() function.

    Versatile Unit Conversion: Handles conversions across Speed, Temperature, and Mass (kg ↔ lb, C ↔ F ↔ K), preventing mixing of unit types.

    Interactive Shell: Includes console commands: exit, help, and clear.

    Cross-Platform Source Code: The source is portable, but the included binary is built specifically for Linux/macOS.

⚙️ Usage and Execution (Linux/macOS)

Running the Binary

The executable file (sm_ai_core_en_stable) is built specifically for Linux and macOS systems and can be run directly from your terminal.

    Navigate to the directory where you placed the file.

    Execute the program:
    (Bash)
    ./sm_ai_core
    
Mathematical Expressions

Enter any expression to calculate. The parser correctly handles operator precedence.

    Example 1: 10 + 5 * 2

    Example 2: (15 - 3) / 4

    Example 3: 4 ^ 2 + abs(-5)

Unit Conversion

Use CONVERT (or conv) to change units.

    Syntax: conv [value] [unit_from] TO [unit_to]

    Example: conv 100 kg to lb

🔮 Future Enhancements

    Variable Support: Implement support for defining and using variables (e.g., let x = 10; 2 * x + 1).

    Function Restoration: Re-implement trigonometric, logarithmic, and square root functions with a more stable RPN parsing logic.

🤝 Contribution

Feel free to suggest improvements or report any bugs you find. Thank you for making this tool robust!
