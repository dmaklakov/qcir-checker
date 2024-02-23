#if !defined(COCO_PARSER_H__)
#define COCO_PARSER_H__

#include <unordered_set>
#include <unordered_map>

#include "Scanner.h"

using namespace std;

// Custom hash function for wchar_t*
struct WCharPtrHash {
    size_t operator()(const wchar_t* str) const {
        // Use a simple hash function for illustration
        size_t hash = 0;
        while (*str) {
            hash = (hash * 31) + (*str++);
        }
        return hash;
    }
};

// Custom equality function for wchar_t*
struct WCharPtrEqual {
    bool operator()(const wchar_t* a, const wchar_t* b) const {
        return wcscmp(a, b) == 0;
    }
};

	
enum Gate_type {
	NO_QUANTIFIERS = 0,
	EXIST_QUANTIFIERS = 1,
	WAS_USED = 2
};

struct Gate {
	unordered_set<const wchar_t*, WCharPtrHash, WCharPtrEqual> unresolved_variables;
	Gate_type type;
};


class Errors {
public:
	int count;			// number of errors detected

	Errors();
	void SynErr(int line, int col, int n);
	void Error(int line, int col, const wchar_t *s);
	void Error(int line, int col, const wchar_t *s1, const wchar_t *s2);
	void Warning(int line, int col, const wchar_t *s);
	void Warning(int line, int col, const wchar_t *s1, const wchar_t *s2);
	void Warning(const wchar_t *s);
	void Exception(const wchar_t *s);

}; // Errors

class Parser {
protected:
	enum {
		_EOF=0,
		_ident=1,
		_number_ident=2
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

	bool isAllDigits(const wchar_t* s) const;

public:
	Scanner *scanner;
	Errors  *errors;

	Token *t;			// last recognized token
	Token *la;			// lookahead token

	bool check_for_cleansed = false;
	bool correct_cleansed = true;
	unordered_set<const wchar_t*, WCharPtrHash, WCharPtrEqual> global_variables;
	unordered_map<const wchar_t*, Gate, WCharPtrHash, WCharPtrEqual> gate_variables;
	unordered_set<const wchar_t*, WCharPtrHash, WCharPtrEqual> resolved_variables;

	long n_variables;
	wchar_t *output_gate;

	Parser(Scanner *scanner, bool check_for_cleansed);
	~Parser();
	void SemErr(const wchar_t* msg);
	void SemErr(const wchar_t* msg1, const wchar_t* msg2);
	void Warning(const wchar_t* msg) const;
	void Warning(const wchar_t* msg1, const wchar_t* msg2) const;

	virtual void Qcir_file_mod();
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