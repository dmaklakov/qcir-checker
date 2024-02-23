#include <cwchar>
#include <cwctype>
#include "ParserLight.h"
#include "Scanner.h"

void ParserLight::Qcir_file_mod() {
		Format_id();
		Qblock_stmt();
		Output_stmt();
		while (la->kind == _ident || la->kind == _number_ident) {
			Gate_stmt();
			nl();
			while (la->kind == 246 /* "\n" */) {
				nl();
			}
		}

		if (global_variables.count(output_gate) > 0) {
			SemErr(output_gate, L"is not a gate variable");
			correct_cleansed = false;
		} else if (gate_variables_set.count(output_gate) == 0) {
			SemErr(output_gate, L"output gate was not defined");
			correct_cleansed = false;
		} else {
			if (!unresolved_variables.empty()) {
				for (auto unresolved_variable : unresolved_variables) {
					SemErr(unresolved_variable, L"was unresolved");
				}
				correct_cleansed = false;
			}
		}
}

void ParserLight::Gate_stmt() {
	while (!(la->kind == _EOF || la->kind == _ident || la->kind == _number_ident)) {SynErr(251); Get();}
	
	Var();
	const wchar_t *name = wcsdup(t->val);
	if (global_variables.count(name) > 0 || gate_variables_set.count(name) > 0) {
		SemErr(name, L"was already defined");
		correct_cleansed = false;
	}

	if (unresolved_variables.count(name) > 0) {
		SemErr(name, L"was used before defined");
		unresolved_variables.erase(name);
		correct_cleansed = false;
	}

	Expect(7 /* "=" */);
	if (StartOf(5)) {
		Ngate_type();
		Expect(4 /* "(" */);
		if (la->kind == _ident || la->kind == _number_ident || la->kind == 9 /* "-" */) {
			Lit();
			if (global_variables.count(t->val) == 0 && gate_variables_set.count(t->val) == 0) {
				unresolved_variables.insert(wcsdup(t->val));
			}
			while (la->kind == 5 /* "," */) {
				Get();
				Lit();
				if (global_variables.count(t->val) == 0 && gate_variables_set.count(t->val) == 0) {
					unresolved_variables.insert(wcsdup(t->val));
				}
			}
		}
		Expect(6 /* ")" */);
	} else if (StartOf(6)) {
		if (check_for_cleansed) {
			SemErr(L"xor gate is not allowed in cleansed form");
			correct_cleansed = false;
		} else {
			Warning(L"xor gate is used");
		}
		Xor();
		
		Expect(4 /* "(" */);
		Lit();
		if (global_variables.count(t->val) == 0 && gate_variables_set.count(t->val) == 0) {
			unresolved_variables.insert(wcsdup(t->val));
		}
		
		Expect(5 /* "," */);
		Lit();
		if (global_variables.count(t->val) == 0 && gate_variables_set.count(t->val) == 0) {
			unresolved_variables.insert(wcsdup(t->val));
		}
		Expect(6 /* ")" */);
	} else if (StartOf(7)) {
		if (check_for_cleansed) {
			SemErr(L"ite gate is not allowed in cleansed form");
			correct_cleansed = false;
		} else {
			Warning(L"ite gate is used");
		}
		Ite();
		
		Expect(4 /* "(" */);
		Lit();
		if (global_variables.count(t->val) == 0 && gate_variables_set.count(t->val) == 0) {
			unresolved_variables.insert(wcsdup(t->val));
		} 
		Expect(5 /* "," */);
		Lit();
		if (global_variables.count(t->val) == 0 && gate_variables_set.count(t->val) == 0) {
			unresolved_variables.insert(wcsdup(t->val));
		}  
		Expect(5 /* "," */);
		Lit();
		if (global_variables.count(t->val) == 0 && gate_variables_set.count(t->val) == 0) {
			unresolved_variables.insert(wcsdup(t->val));
		}
		Expect(6 /* ")" */);
	} else if (StartOf(3)) {
		Quant();
		Expect(4 /* "(" */);
		
		Var();
		if (gate_variables_set.count(t->val) > 0) {
			SemErr(t->val, L"was already defined as gate variable");
			correct_cleansed = false;
		} else if (check_for_cleansed && (global_variables.count(t->val) > 0 || resolved_variables.count(t->val) > 0)) {
			SemErr(t->val, L"was already quantified/defined");
			correct_cleansed = false;
		}
		unresolved_variables.erase(wcsdup(t->val));
		resolved_variables.insert(wcsdup(t->val));
		while (la->kind == 5 /* "," */) {
			Get();
			Var();
			if (gate_variables_set.count(t->val) > 0) {
				SemErr(t->val, L"was already defined as gate variable");
				correct_cleansed = false;
			} else if (check_for_cleansed && (global_variables.count(t->val) > 0 || resolved_variables.count(t->val) > 0)) {
				SemErr(t->val, L"was already quantified/defined");
				correct_cleansed = false;
			}
			unresolved_variables.erase(wcsdup(t->val));
			resolved_variables.insert(wcsdup(t->val));
		}
		Expect(8 /* ";" */);
		Lit();
        if (global_variables.count(t->val) == 0 && gate_variables_set.count(t->val) == 0) {
			unresolved_variables.insert(wcsdup(t->val));
		}

		Expect(6 /* ")" */);
	} else SynErr(252);
	
	gate_variables_set.insert(name);
}

ParserLight::ParserLight(Scanner *scanner, bool check_for_cleansed) : Parser::Parser(scanner, check_for_cleansed) {
	this->gate_variables_set = unordered_set<const wchar_t*, WCharPtrHash, WCharPtrEqual>();
	this->unresolved_variables = unordered_set<const wchar_t*, WCharPtrHash, WCharPtrEqual>();
	this->resolved_variables = unordered_set<const wchar_t*, WCharPtrHash, WCharPtrEqual>();
}

ParserLight::~ParserLight() {
	gate_variables_set.clear();
	unresolved_variables.clear();
	resolved_variables.clear();
}
