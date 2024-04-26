#include <cwchar>
#include <cwctype>
#include "ParserLight.h"
#include "Scanner.h"

void ParserLight::Qcir_file_mod()
{
	if (la->kind == 3 /* "#QCIR-G14" */)
	{
		Format_id();
	}
	else
	{
		SynErr(3);
	}
	Qblock_stmt();
	if (StartOf(1))
	{
		Output_stmt();
	}
	else
	{
		output_gate = "undefined";
		SynErr(38);
	}

	while (la->kind == _ident || la->kind == _number_ident)
	{
		Gate_stmt();
		nl();
		while (la->kind == 246 /* "\n" */)
		{
			nl();
		}
	}

	n_variables_real = gate_variables.size() + global_variables.size();

	if (output_gate_defined)
	{
		size_t output_gate_symbol = getSymbol(output_gate);
		if (global_variables.count(output_gate_symbol) > 0)
		{
			SemErr(output_gate, "is not a gate variable");
			correct_cleansed = false;
		}
		else if (gate_variables_set.count(output_gate_symbol) == 0)
		{
			SemErr(output_gate, "output gate was not defined");
			correct_cleansed = false;
		}
		else
		{
			if (!unresolved_variables.empty())
			{
				for (const auto unresolved_variable : unresolved_variables)
				{
					for (const auto& pair : symbols)
					{
						if (pair.second == unresolved_variable)
						{
							SemErr(pair.first.data(), " was unresolved");
							break;
						}
					}
				}
				correct_cleansed = false;
			}
		}
	}
}

void ParserLight::Gate_stmt()
{
	while (!(la->kind == _EOF || la->kind == _ident || la->kind == _number_ident))
	{
		SynErr(251);
		Get();
	}

	Var();
	const char *name = strdup(t->val);
	size_t name_symbol = getSymbol(name);
	if (global_variables.count(name_symbol) > 0 || gate_variables_set.count(name_symbol) > 0)
	{
		SemErr(name, "was already defined");
		correct_cleansed = false;
	}

	if (unresolved_variables.count(name_symbol) > 0)
	{
		SemErr(name, "was used before defined");
		unresolved_variables.erase(name_symbol);
		correct_cleansed = false;
	}

	char *var;
	size_t var_symbol;
	Expect(7 /* "=" */);
	if (StartOf(5))
	{
		Ngate_type();
		Expect(4 /* "(" */);
		if (la->kind == _ident || la->kind == _number_ident || la->kind == 9 /* "-" */)
		{
			Lit();
			var = strdup(t->val);
			var_symbol = getSymbol(var);
			if (global_variables.count(var_symbol) == 0 && gate_variables_set.count(var_symbol) == 0)
			{
				unresolved_variables.insert(var_symbol);
			}
			while (la->kind == 5 /* "," */)
			{
				Get();
				Lit();
				var = strdup(t->val);
				var_symbol = getSymbol(var);
				if (global_variables.count(var_symbol) == 0 && gate_variables_set.count(var_symbol) == 0)
				{
					unresolved_variables.insert(var_symbol);
				}
			}
		}
		Expect(6 /* ")" */);
	}
	else if (StartOf(6))
	{
		if (check_for_cleansed)
		{
			SemErr("xor gate is not allowed in cleansed form");
			correct_cleansed = false;
		}
		else
		{
			Warning("xor gate is used");
		}
		Xor();

		Expect(4 /* "(" */);
		Lit();
		var = strdup(t->val);
		var_symbol = getSymbol(var);
		if (global_variables.count(var_symbol) == 0 && gate_variables_set.count(var_symbol) == 0)
		{
			unresolved_variables.insert(var_symbol);
		}

		Expect(5 /* "," */);
		Lit();
		var = strdup(t->val);
		var_symbol = getSymbol(var);
		if (global_variables.count(var_symbol) == 0 && gate_variables_set.count(var_symbol) == 0)
		{
			unresolved_variables.insert(var_symbol);
		}
		Expect(6 /* ")" */);
	}
	else if (StartOf(7))
	{
		if (check_for_cleansed)
		{
			SemErr("ite gate is not allowed in cleansed form");
			correct_cleansed = false;
		}
		else
		{
			Warning("ite gate is used");
		}
		Ite();

		Expect(4 /* "(" */);
		Lit();
		var = strdup(t->val);
		var_symbol = getSymbol(var);
		if (global_variables.count(var_symbol) == 0 && gate_variables_set.count(var_symbol) == 0)
		{
			unresolved_variables.insert(var_symbol);
		}
		Expect(5 /* "," */);
		Lit();
		var = strdup(t->val);
		var_symbol = getSymbol(var);
		if (global_variables.count(var_symbol) == 0 && gate_variables_set.count(var_symbol) == 0)
		{
			unresolved_variables.insert(var_symbol);
		}
		Expect(5 /* "," */);
		Lit();
		var = strdup(t->val);
		var_symbol = getSymbol(var);
		if (global_variables.count(var_symbol) == 0 && gate_variables_set.count(var_symbol) == 0)
		{
			unresolved_variables.insert(var_symbol);
		}
		Expect(6 /* ")" */);
	}
	else if (StartOf(3))
	{
		Quant();
		Expect(4 /* "(" */);

		Var();
		var = strdup(t->val);
		var_symbol = getSymbol(var);
		if (gate_variables_set.count(var_symbol) > 0)
		{
			SemErr(t->val, "was already defined as gate variable");
			correct_cleansed = false;
		}
		else if (check_for_cleansed && (global_variables.count(var_symbol) > 0 || resolved_variables.count(var_symbol) > 0))
		{
			SemErr(t->val, "was already quantified/defined");
			correct_cleansed = false;
		}
		unresolved_variables.erase(var_symbol);
		resolved_variables.insert(var_symbol);
		while (la->kind == 5 /* "," */)
		{
			Get();
			Var();
			var = strdup(t->val);
			var_symbol = getSymbol(var);
			if (gate_variables_set.count(var_symbol) > 0)
			{
				SemErr(t->val, "was already defined as gate variable");
				correct_cleansed = false;
			}
			else if (check_for_cleansed && (global_variables.count(var_symbol) > 0 || resolved_variables.count(var_symbol) > 0))
			{
				SemErr(t->val, "was already quantified/defined");
				correct_cleansed = false;
			}
			unresolved_variables.erase(var_symbol);
			resolved_variables.insert(var_symbol);
		}
		Expect(8 /* ";" */);
		Lit();
		var = strdup(t->val);
		var_symbol = getSymbol(var);
		if (global_variables.count(var_symbol) == 0 && gate_variables_set.count(var_symbol) == 0)
		{
			unresolved_variables.insert(var_symbol);
		}

		Expect(6 /* ")" */);
	}
	else
		SynErr(252);

	gate_variables_set.insert(name_symbol);
}

ParserLight::ParserLight(Scanner *scanner, bool check_for_cleansed) : Parser::Parser(scanner, check_for_cleansed)
{
	this->gate_variables_set = ankerl::unordered_dense::set<size_t>();
	this->unresolved_variables = ankerl::unordered_dense::set<size_t>();
}

ParserLight::~ParserLight()
{
	gate_variables_set.clear();
	unresolved_variables.clear();
	resolved_variables.clear();
}
