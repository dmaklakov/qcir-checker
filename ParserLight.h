#if !defined(COCO_PARSER_LIGHT_H__)
#define COCO_PARSER_LIGHT_H__

#include <ankerl/unordered_dense.h>

#include "Scanner.h"
#include "Parser.h"

using namespace std;

class ParserLight : public Parser
{

public:
	ankerl::unordered_dense::set<size_t> unresolved_variables;

	ankerl::unordered_dense::set<size_t> gate_variables_set;

	ParserLight(Scanner *scanner, bool check_for_cleansed);
	~ParserLight();

	void Qcir_file_mod();
	void Gate_stmt();

}; // end Parser

#endif