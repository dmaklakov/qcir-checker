#include <cstdlib>
#include <iostream>
#include <ctime>

#include "Parser.h"
#include "Scanner.h"

using namespace std;

int main(int argc, char *argv[])
{

    if (argc < 2)
    {
        cerr << "Usage: qcir [-cleansed] [-light] <filename> " << endl;
        return EXIT_FAILURE;
    }

    std::cout << "Filename: " << argv[argc - 1] << std::endl;
    const char *f = argv[argc - 1];
    size_t len = strlen(f) + 1;
    wchar_t *filename = new wchar_t[len];
    mbstowcs(filename, f, len);

    bool check_for_cleansed = false;
    bool light_check = false;
    if (argc == 3)
    {
        check_for_cleansed = strcmp(argv[1], "-cleansed") == 0;
        light_check = strcmp(argv[1], "-light") == 0;
    }
    else if (argc == 4)
    {
        check_for_cleansed = strcmp(argv[1], "-cleansed") == 0 || strcmp(argv[2], "-cleansed") == 0;
        light_check = strcmp(argv[1], "-light") == 0 || strcmp(argv[2], "-light") == 0;
    }

    if (check_for_cleansed)
    {
        cout << "Check for cleansed form" << endl;
    }
    else
    {
        cout << "Check for non-prenex form" << endl;
    }

    Scanner *scanner = new Scanner(filename);
    Parser *parser = new Parser(scanner, check_for_cleansed);

    time_t start = time(0);
    try
    {
        parser->Parse();
    }
    catch (std::exception &e)
    {
        std::cout << e.what() << std::endl;
    }
    time_t end = time(0);

    int n_errors = parser->errors->count + scanner->errors_count;
    std::cout << "Number of errors: " << n_errors << std::endl;
    if (parser->n_variables_expected > 0)
    {
        std::cout << "Number of variables expected: " << parser->n_variables_expected << std::endl;
    }
    std::cout << "Number of variables found: " << parser->n_variables_real << std::endl;
    std::wcout << L"Output gate: " << parser->output_gate << std::endl;
    if (check_for_cleansed)
    {
        std::cout << "Correct cleansed: " << (parser->correct_cleansed ? "true" : "false") << std::endl;
    }
    std::cout << "Time of execution: " << difftime(end, start) << "s" << std::endl;

    return EXIT_SUCCESS;
    // return check_for_cleansed ? (parser->errors->count == 0 && parser->correct_cleansed ? EXIT_SUCCESS : EXIT_FAILURE) : (parser->errors->count == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}