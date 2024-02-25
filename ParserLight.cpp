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
		size_t output_gate_symbol = symbols[output_gate];
		if (global_variables.count(output_gate_symbol) > 0) {
			SemErr(output_gate, L"is not a gate variable");
			correct_cleansed = false;
		} else if (gate_variables_set.count(output_gate_symbol) == 0) {
			SemErr(output_gate, L"output gate was not defined");
			correct_cleansed = false;
		} else {
			if (!unresolved_variables.empty()) {
				for (const auto unresolved_variable : unresolved_variables) {
					for(const auto pair : symbols) {
						if (pair.second == unresolved_variable) {
							SemErr(pair.first, L" was unresolved");
							break;
						}
					}
				}
				correct_cleansed = false;
			}
		}
}

void ParserLight::Gate_stmt() {
	while (!(la->kind == _EOF || la->kind == _ident || la->kind == _number_ident)) {SynErr(251); Get();}
	
	Var();
	const wchar_t *name = wcsdup(t->val);
	size_t name_symbol = symbols[name];
	if (global_variables.count(name_symbol) > 0 || gate_variables_set.count(name_symbol) > 0) {
		SemErr(name, L"was already defined");
		correct_cleansed = false;
	}

	if (unresolved_variables.count(name_symbol) > 0) {
		SemErr(name, L"was used before defined");
		unresolved_variables.erase(name_symbol);
		correct_cleansed = false;
	}

	wchar_t *var;
	size_t var_symbol;
	Expect(7 /* "=" */);
	if (StartOf(5)) {
		Ngate_type();
		Expect(4 /* "(" */);
		if (la->kind == _ident || la->kind == _number_ident || la->kind == 9 /* "-" */) {
			Lit();
			var = wcsdup(t->val);
			var_symbol = symbols[var];
			if (global_variables.count(var_symbol) == 0 && gate_variables_set.count(var_symbol) == 0) {
				unresolved_variables.insert(var_symbol);
			}
			while (la->kind == 5 /* "," */) {
				Get();
				Lit();	
				var = wcsdup(t->val);
				var_symbol = symbols[var];
				if (global_variables.count(var_symbol) == 0 && gate_variables_set.count(var_symbol) == 0) {
					unresolved_variables.insert(var_symbol);
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
		var = wcsdup(t->val);
		var_symbol = symbols[var];
		if (global_variables.count(var_symbol) == 0 && gate_variables_set.count(var_symbol) == 0) {
			unresolved_variables.insert(var_symbol);
		}
		
		Expect(5 /* "," */);
		Lit();
		var = wcsdup(t->val);
		var_symbol = symbols[var];
		if (global_variables.count(var_symbol) == 0 && gate_variables_set.count(var_symbol) == 0) {
			unresolved_variables.insert(var_symbol);
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
		var = wcsdup(t->val);
		var_symbol = symbols[var];
		if (global_variables.count(var_symbol) == 0 && gate_variables_set.count(var_symbol) == 0) {
			unresolved_variables.insert(var_symbol);
		}
		Expect(5 /* "," */);
		Lit();
		var = wcsdup(t->val);
		var_symbol = symbols[var];
		if (global_variables.count(var_symbol) == 0 && gate_variables_set.count(var_symbol) == 0) {
			unresolved_variables.insert(var_symbol);
		}
		Expect(5 /* "," */);
		Lit();
		var = wcsdup(t->val);
		var_symbol = symbols[var];
		if (global_variables.count(var_symbol) == 0 && gate_variables_set.count(var_symbol) == 0) {
			unresolved_variables.insert(var_symbol);
		}
		Expect(6 /* ")" */);
	} else if (StartOf(3)) {
		Quant();
		Expect(4 /* "(" */);
		
		Var();
		var = wcsdup(t->val);
		var_symbol = symbols[var];
		if (gate_variables_set.count(var_symbol) > 0) {
			SemErr(t->val, L"was already defined as gate variable");
			correct_cleansed = false;
		} else if (check_for_cleansed && (global_variables.count(var_symbol) > 0 || resolved_variables.count(var_symbol) > 0)) {
			SemErr(t->val, L"was already quantified/defined");
			correct_cleansed = false;
		}
		unresolved_variables.erase(var_symbol);
		resolved_variables.insert(var_symbol);
		while (la->kind == 5 /* "," */) {
			Get();
			Var();
			var = wcsdup(t->val);
			var_symbol = symbols[var];
			if (gate_variables_set.count(var_symbol) > 0) {
				SemErr(t->val, L"was already defined as gate variable");
				correct_cleansed = false;
			} else if (check_for_cleansed && (global_variables.count(var_symbol) > 0 || resolved_variables.count(var_symbol) > 0)) {
				SemErr(t->val, L"was already quantified/defined");
				correct_cleansed = false;
			}
			unresolved_variables.erase(var_symbol);
			resolved_variables.insert(var_symbol);
		}
		Expect(8 /* ";" */);
		Lit();
        var = wcsdup(t->val);
		var_symbol = symbols[var];
		if (global_variables.count(var_symbol) == 0 && gate_variables_set.count(var_symbol) == 0) {
			unresolved_variables.insert(var_symbol);
		}

		Expect(6 /* ")" */);
	} else SynErr(252);
	
	gate_variables_set.insert(name_symbol);
}

ParserLight::ParserLight(Scanner *scanner, bool check_for_cleansed) : Parser::Parser(scanner, check_for_cleansed) {
	this->gate_variables_set = unordered_set<size_t>();
	this->unresolved_variables = unordered_set<size_t>();
}

ParserLight::~ParserLight() {
	gate_variables_set.clear();
	unresolved_variables.clear();
	resolved_variables.clear();
}
