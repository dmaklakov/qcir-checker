#if !defined(COCO_PARSER_H__)
#define COCO_PARSER_H__

#include <ankerl/unordered_dense.h>

#include "Scanner.h"

using namespace std;

enum Gate_type
{
	NO_QUANTIFIERS = 0,
	EXIST_QUANTIFIERS = 1,
	WAS_USED = 2
};

class Gate
{
public:
	ankerl::unordered_dense::set<size_t> unresolved_variables;

	Gate_type type;

	Gate();
	~Gate();
};

class Errors
{
public:
	int count; // number of errors detected

	Errors();
	void SynErr(int line, int col, int n);
	void Error(int line, int col, const char *s);
	void Error(int line, int col, const char *s1, const char *s2);
	void Warning(int line, int col, const char *s);
	void Warning(int line, int col, const char *s1, const char *s2);
	void Warning(const char *s);
	void Exception(const char *s);

}; // Errors

class Parser
{
protected:
	enum
	{
		_EOF = 0,
		_ident = 1,
		_number_ident = 2
	};
	int maxT;

	Token *dummyToken;
	int errDist;
	int minErrDist;

	void SynErr(int n);
	void Get();
	void Expect(int n);
	bool StartOf(int s) const;
	void ExpectWeak(int n, int follow);
	bool WeakSeparator(int n, int syFol, int repFol);

	bool isAllDigits(const char *s) const;
	size_t getSymbol(const char *s);
	size_t symbols_size;

public:
	Scanner *scanner;
	Errors *errors;

	Token *t;  // last recognized token
	Token *la; // lookahead token

	bool check_for_cleansed = false;
	bool correct_cleansed = true;
	ankerl::unordered_dense::map<std::string_view, size_t> symbols;

	ankerl::unordered_dense::set<size_t> global_variables;
	ankerl::unordered_dense::map<size_t, Gate> gate_variables;
	ankerl::unordered_dense::set<size_t> resolved_variables;

	long n_variables_expected;
	long n_variables_real;
	char *output_gate;
	bool output_gate_defined = false;

	Parser(Scanner *scanner, bool check_for_cleansed);
	~Parser();
	void Err(const char *msg);
	void Err(const char *msg1, const char *msg2);
	void SemErr(const char *msg);
	void SemErr(const char *msg1, const char *msg2);
	void Warning(const char *msg) const;
	void Warning(const char *msg1, const char *msg2) const;

	virtual void Qcir_file();
	void Format_id();
	virtual void Qblock_stmt();
	void Output_stmt();
	void Gate_stmt();
	void nl();
	void Free();
	void Var();
	void Qblock_quant();
	void Quant();
	void Output();
	void Lit();
	void Ngate_type();
	void Xor();
	void Ite();
	void Exists();
	void Forall();
	void And();
	void Or();

	virtual void Parse();

}; // end Parser

#endif
