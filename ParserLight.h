#if !defined(COCO_PARSER_LIGHT_H__)
#define COCO_PARSER_LIGHT_H__

#include <unordered_set>
#include <unordered_map>

#include "Scanner.h"
#include "Parser.h"

using namespace std;

class ParserLight : public Parser {

public:
	unordered_set<const wchar_t*, WCharPtrHash, WCharPtrEqual> unresolved_variables;

	unordered_set<const wchar_t*, WCharPtrHash, WCharPtrEqual> gate_variables_set;

	ParserLight(Scanner *scanner, bool check_for_cleansed);
	~ParserLight();

	void Qcir_file_mod();
	void Gate_stmt();

}; // end Parser

#endif