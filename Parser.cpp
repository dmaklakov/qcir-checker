#include <cwchar>
#include <cwctype>
#include "iostream"
#include "Parser.h"
#include "Scanner.h"

void Parser::SynErr(int n)
{
	if (errDist >= minErrDist)
		errors->SynErr(la->line, la->col, n);
	errDist = 0;
}

void Parser::SemErr(const char *msg)
{
	if (errDist >= minErrDist)
		errors->Error(t->line, t->col, msg);
	errDist = 0;
}

void Parser::SemErr(const char *msg1, const char *msg2)
{
	if (errDist >= minErrDist)
		errors->Error(t->line, t->col, msg1, msg2);
	errDist = 0;
}

void Parser::Warning(const char *msg) const
{
	errors->Warning(t->line, t->col, msg);
}

void Parser::Warning(const char *msg1, const char *msg2) const
{
	errors->Warning(t->line, t->col, msg1, msg2);
}

void Parser::Err(const char *msg)
{
	errors->Error(t->line, t->col, msg);
}

void Parser::Err(const char *msg1, const char *msg2)
{
	errors->Error(t->line, t->col, msg1, msg2);
}

void Parser::Get()
{
	for (;;)
	{
		t = la;
		la = scanner->Scan();
		if (la->kind <= maxT)
		{
			++errDist;
			break;
		}

		if (dummyToken != t)
		{
			dummyToken->kind = t->kind;
			dummyToken->pos = t->pos;
			dummyToken->col = t->col;
			dummyToken->line = t->line;
			dummyToken->next = NULL;
			coco_string_delete(dummyToken->val);
			dummyToken->val = coco_string_create(t->val);
			t = dummyToken;
		}
		la = t;
	}
}

void Parser::Expect(int n)
{
	if (la->kind == n)
		Get();
	else
	{
		SynErr(n);
	}
}

void Parser::ExpectWeak(int n, int follow)
{
	if (la->kind == n)
		Get();
	else
	{
		SynErr(n);
		while (!StartOf(follow))
			Get();
	}
}

bool Parser::WeakSeparator(int n, int syFol, int repFol)
{
	if (la->kind == n)
	{
		Get();
		return true;
	}
	else if (StartOf(repFol))
	{
		return false;
	}
	else
	{
		SynErr(n);
		while (!(StartOf(syFol) || StartOf(repFol) || StartOf(0)))
		{
			Get();
		}
		return StartOf(syFol);
	}
}

void Parser::Qcir_file()
{
	if (la->kind == 3 /* "#QCIR-G14" */)
	{
		Format_id();
	}
	else
	{
		correct_cleansed = false;
		SynErr(3);
	}
	while (!(StartOf(1)))
	{
		SynErr(248);
		Get();
	}
	Qblock_stmt();
	while (!(StartOf(2)))
	{
		SynErr(249);
		Get();
	}
	if (StartOf(3))
	{
		Output_stmt();
	}
	else
	{
		correct_cleansed = false;
		Err("output gate definition is missing");
	}
	while (!(la->kind == _EOF || la->kind == _ident || la->kind == _number_ident))
	{
		SynErr(250);
		Get();
	}
	while (la->kind == _ident || la->kind == _number_ident)
	{
		Gate_stmt();
		nl();
		while (!(StartOf(4)))
		{
			SynErr(251);
			Get();
		}
		while (la->kind == 246 /* "\n" */)
		{
			nl();
		}
	}

	n_variables_real = gate_variables.size() + global_variables.size();

	if (output_gate_defined)
	{
		const size_t output_gate_symbol = getSymbol(output_gate);
		if (gate_variables.count(output_gate_symbol) == 0)
		{
			SemErr(output_gate, "output gate was not defined");
			correct_cleansed = false;
		}
		else if (global_variables.count(output_gate_symbol) > 0)
		{
			SemErr(output_gate, "is not a gate variable");
			correct_cleansed = false;
		}
		else
		{
			Gate output = gate_variables[output_gate_symbol];
			if (!output.unresolved_variables.empty())
			{
				for (const auto unresolved_variable : output.unresolved_variables)
				{
					for (const auto &pair : symbols)
					{
						if (pair.second == unresolved_variable)
						{
							SemErr(pair.first.data(), "was unresolved");
							break;
						}
					}
				}
				correct_cleansed = false;
			}
		}
	}
}

void Parser::Format_id()
{
	Expect(3 /* "#QCIR-G14" */);
	if (la->kind == _number_ident)
	{
		Get();
		if (isAllDigits(t->val))
		{
			char *end;
			n_variables_expected = strtol(t->val, &end, 10);
			if (n_variables_expected <= 0)
			{
				SemErr(t->val, "is not a positive number");
				n_variables_expected = -1;
				correct_cleansed = false;
			}
		}
		else
		{
			SemErr(t->val, "is not a positive number");
			n_variables_expected = -1;
			correct_cleansed = false;
		}
	}
	else if (la->kind == _ident)
	{
		Get();
		if (check_for_cleansed)
		{
			SemErr("Number of variables is not defined");
			n_variables_expected = -1;
			correct_cleansed = false;
		}
	}
	else if (check_for_cleansed)
	{
		SemErr("Number of variables is not defined");
		n_variables_expected = -1;
		correct_cleansed = false;
	}

	nl();
	while (la->kind == 246 /* "\n" */)
	{
		nl();
	}
}

void Parser::Qblock_stmt()
{
	if (StartOf(5))
	{
		Free();
		Expect(4 /* "(" */);
		Var();
		char *var = strdup(t->val);
		size_t var_symbol = getSymbol(var);
		Warning(var, "is a free variable");
		if (global_variables.count(var_symbol) == 0)
		{
			global_variables.insert(var_symbol);
		}
		else if (check_for_cleansed)
		{
			SemErr(t->val, "was already defined");
			correct_cleansed = false;
		}

		while (la->kind == 5 /* "," */)
		{
			Get();
			Var();
			var = strdup(t->val);
			size_t var_symbol = getSymbol(var);
			Warning(var, "is a free variable");
			if (global_variables.count(var_symbol) == 0)
			{
				global_variables.insert(var_symbol);
			}
			else if (check_for_cleansed)
			{
				SemErr(t->val, "was already defined");
				correct_cleansed = false;
			}
		}
		Expect(6 /* ")" */);
		nl();
		while (la->kind == 246 /* "\n" */)
		{
			nl();
		}
	}
	while (StartOf(6))
	{
		Qblock_quant();
	}
}

void Parser::Output_stmt()
{
	Output();
	Expect(4 /* "(" */);
	Lit();
	output_gate = strdup(t->val);
	output_gate_defined = true;
	Expect(6 /* ")" */);
	nl();
	while (la->kind == 246 /* "\n" */)
	{
		nl();
	}
}

void Parser::Gate_stmt()
{
	Var();
	Gate *gate = new Gate();
	const char *name = strdup(t->val);
	size_t name_symbol = getSymbol(name);
	if (global_variables.count(name_symbol) > 0 || gate_variables.count(name_symbol) > 0 || resolved_variables.count(name_symbol) > 0)
	{
		SemErr(name, "was already defined");
		correct_cleansed = false;
	}

	char *var;
	size_t var_symbol;
	Expect(7 /* "=" */);
	if (StartOf(7))
	{
		Ngate_type();
		Expect(4 /* "(" */);
		if (la->kind == _ident || la->kind == _number_ident || la->kind == 9 /* "-" */)
		{
			Lit();
			var = strdup(t->val);
			var_symbol = getSymbol(var);
			if (gate_variables.count(var_symbol) > 0)
			{
				Gate *gate_var = &gate_variables.at(var_symbol);
				for (const auto &elem : gate_var->unresolved_variables)
				{
					gate->unresolved_variables.insert(elem);
				}
				if (gate_var->type == EXIST_QUANTIFIERS)
				{
					gate_var->type = WAS_USED;
					gate->type = EXIST_QUANTIFIERS;
				}
				else if (gate_var->type == WAS_USED)
				{
					if (check_for_cleansed)
					{
						SemErr(var, "was already used");
						correct_cleansed = false;
					}
					gate->type = EXIST_QUANTIFIERS;
				}
			}
			else if (global_variables.count(var_symbol) == 0)
			{
				gate->unresolved_variables.insert(var_symbol);
			}
			while (la->kind == 5 /* "," */)
			{
				Get();
				Lit();
				var = strdup(t->val);
				var_symbol = getSymbol(var);
				if (gate_variables.count(var_symbol) > 0)
				{
					Gate *gate_var = &gate_variables.at(var_symbol);
					for (const auto &elem : gate_var->unresolved_variables)
					{
						gate->unresolved_variables.insert(elem);
					}
					if (gate_var->type == EXIST_QUANTIFIERS)
					{
						gate_var->type = WAS_USED;
						gate->type = EXIST_QUANTIFIERS;
					}
					else if (gate_var->type == WAS_USED)
					{
						if (check_for_cleansed)
						{
							SemErr(var, "was already used");
							correct_cleansed = false;
						}
						gate->type = EXIST_QUANTIFIERS;
					}
				}
				else if (global_variables.count(var_symbol) == 0)
				{
					gate->unresolved_variables.insert(var_symbol);
				}
			}
		}
		Expect(6 /* ")" */);
	}
	else if (StartOf(8))
	{
		if (check_for_cleansed)
		{
			Err("xor gate is not allowed in cleansed form");
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
		if (gate_variables.count(var_symbol) > 0)
		{
			Gate *gate_var = &gate_variables.at(var_symbol);
			for (const auto &elem : gate_var->unresolved_variables)
			{
				gate->unresolved_variables.insert(elem);
			}
			if (gate_var->type == EXIST_QUANTIFIERS)
			{
				gate_var->type = WAS_USED;
				gate->type = EXIST_QUANTIFIERS;
			}
			else if (gate_var->type == WAS_USED)
			{
				if (check_for_cleansed)
				{
					SemErr(var, "was already used");
					correct_cleansed = false;
				}
				gate->type = EXIST_QUANTIFIERS;
			}
		}
		else if (global_variables.count(var_symbol) == 0)
		{
			gate->unresolved_variables.insert(var_symbol);
		}

		Expect(5 /* "," */);
		Lit();
		var = strdup(t->val);
		var_symbol = getSymbol(var);
		if (gate_variables.count(var_symbol) > 0)
		{
			Gate *gate_var = &gate_variables.at(var_symbol);
			for (const auto &elem : gate_var->unresolved_variables)
			{
				gate->unresolved_variables.insert(elem);
			}
			if (gate_var->type == EXIST_QUANTIFIERS)
			{
				gate_var->type = WAS_USED;
				gate->type = EXIST_QUANTIFIERS;
			}
			else if (gate_var->type == WAS_USED)
			{
				if (check_for_cleansed)
				{
					SemErr(var, "was already used");
					correct_cleansed = false;
				}
				gate->type = EXIST_QUANTIFIERS;
			}
		}
		else if (global_variables.count(var_symbol) == 0)
		{
			gate->unresolved_variables.insert(var_symbol);
		}
		Expect(6 /* ")" */);
	}
	else if (StartOf(9))
	{
		if (check_for_cleansed)
		{
			Err("ite gate is not allowed in cleansed form");
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
		if (gate_variables.count(var_symbol) > 0)
		{
			Gate *gate_var = &gate_variables.at(var_symbol);
			for (const auto &elem : gate_var->unresolved_variables)
			{
				gate->unresolved_variables.insert(elem);
			}
			if (gate_var->type == EXIST_QUANTIFIERS)
			{
				gate_var->type = WAS_USED;
				gate->type = EXIST_QUANTIFIERS;
			}
			else if (gate_var->type == WAS_USED)
			{
				if (check_for_cleansed)
				{
					SemErr(var, "was already used");
					correct_cleansed = false;
				}
				gate->type = EXIST_QUANTIFIERS;
			}
		}
		else if (global_variables.count(var_symbol) == 0)
		{
			gate->unresolved_variables.insert(var_symbol);
		}

		Expect(5 /* "," */);
		Lit();
		var = strdup(t->val);
		var_symbol = getSymbol(var);
		if (gate_variables.count(var_symbol) > 0)
		{
			Gate *gate_var = &gate_variables.at(var_symbol);
			for (const auto &elem : gate_var->unresolved_variables)
			{
				gate->unresolved_variables.insert(elem);
			}
			if (gate_var->type == EXIST_QUANTIFIERS)
			{
				gate_var->type = WAS_USED;
				gate->type = EXIST_QUANTIFIERS;
			}
			else if (gate_var->type == WAS_USED)
			{
				if (check_for_cleansed)
				{
					SemErr(var, "was already used");
					correct_cleansed = false;
				}
				gate->type = EXIST_QUANTIFIERS;
			}
		}
		else if (global_variables.count(var_symbol) == 0)
		{
			gate->unresolved_variables.insert(var_symbol);
		}

		Expect(5 /* "," */);
		Lit();
		var = strdup(t->val);
		var_symbol = getSymbol(var);
		if (gate_variables.count(var_symbol) > 0)
		{
			Gate *gate_var = &gate_variables.at(var_symbol);
			for (const auto &elem : gate_var->unresolved_variables)
			{
				gate->unresolved_variables.insert(elem);
			}
			if (gate_var->type == EXIST_QUANTIFIERS)
			{
				gate_var->type = WAS_USED;
				gate->type = EXIST_QUANTIFIERS;
			}
			else if (gate_var->type == WAS_USED)
			{
				if (check_for_cleansed)
				{
					SemErr(var, "was already used");
					correct_cleansed = false;
				}
				gate->type = EXIST_QUANTIFIERS;
			}
		}
		else if (global_variables.count(var_symbol) == 0)
		{
			gate->unresolved_variables.insert(var_symbol);
		}

		Expect(6 /* ")" */);
	}
	else if (StartOf(6))
	{
		Quant();
		gate->type = EXIST_QUANTIFIERS;

		Expect(4 /* "(" */);
		Var();
		var = strdup(t->val);
		var_symbol = getSymbol(var);
		if (gate_variables.count(var_symbol) > 0)
		{
			SemErr(var, "was already defined as gate variable");
			correct_cleansed = false;
		}
		else if (check_for_cleansed && (global_variables.count(var_symbol) > 0 || resolved_variables.count(var_symbol) > 0))
		{
			SemErr(var, "was already defined");
			correct_cleansed = false;
		}
		resolved_variables.insert(var_symbol);

		while (la->kind == 5 /* "," */)
		{
			Get();
			Var();
			var = strdup(t->val);
			var_symbol = getSymbol(var);
			if (gate_variables.count(var_symbol) > 0)
			{
				SemErr(var, "was already defined as gate variable");
				correct_cleansed = false;
			}
			else if (check_for_cleansed && (resolved_variables.count(var_symbol) > 0 || global_variables.count(var_symbol) > 0))
			{
				SemErr(var, "was already defined");
				correct_cleansed = false;
			}
			resolved_variables.insert(var_symbol);
		}
		Expect(8 /* ";" */);
		Lit();
		var = strdup(t->val);
		var_symbol = getSymbol(var);
		if (gate_variables.count(var_symbol) > 0)
		{
			Gate *gate_var = &gate_variables.at(var_symbol);
			if (check_for_cleansed && gate_var->type == WAS_USED)
			{
				SemErr(var, "was already used");
				correct_cleansed = false;
			}
			gate_var->type = WAS_USED;
			for (const auto &elem : gate_var->unresolved_variables)
			{
				gate->unresolved_variables.insert(elem);
			}
		}
		else if (resolved_variables.count(var_symbol) == 0 && global_variables.count(var_symbol) == 0)
		{
			gate->unresolved_variables.insert(var_symbol);
		}
		Expect(6 /* ")" */);
	}
	else
		SynErr(252);

	gate_variables[name_symbol] = *gate;
}

void Parser::nl()
{
	Expect(246 /* "\n" */);
}

void Parser::Free()
{
	if (la->kind == 102 /* "free" */)
	{
		Get();
	}
	else if (la->kind >= 103 && la->kind <= 117)
	{
		Get();
		if (check_for_cleansed)
		{
			SemErr("free keyword is not in lower case");
			correct_cleansed = false;
		}
	}
	else
		SynErr(253);
}

void Parser::Var()
{
	if (la->kind == _ident)
	{
		Get();
	}
	else if (la->kind == _number_ident)
	{
		Get();
	}
	else
		SynErr(254);

	if (check_for_cleansed)
	{
		if (!isAllDigits(t->val))
		{
			SemErr(t->val, "is not a correct variable name");
			correct_cleansed = false;
		}
		else if (n_variables_expected > 0)
		{
			char *end;
			long number = strtol(t->val, &end, 10);
			if (number <= 0 || number > n_variables_expected)
			{
				SemErr(t->val, "is not in the range of variables");
				correct_cleansed = false;
			}
		}
	}
}

void Parser::Qblock_quant()
{
	Quant();
	Expect(4 /* "(" */);
	Var();
	char *var = strdup(t->val);
	size_t var_symbol = getSymbol(var);
	if (check_for_cleansed)
	{
		if (global_variables.count(var_symbol) > 0)
		{
			SemErr(t->val, " was already defined");
			correct_cleansed = false;
		}
	}
	global_variables.insert(var_symbol);
	while (la->kind == 5 /* "," */)
	{
		Get();
		Var();
		var = strdup(t->val);
		var_symbol = getSymbol(var);
		if (check_for_cleansed)
		{
			if (global_variables.count(var_symbol) > 0)
			{
				SemErr(t->val, " was already defined");
				correct_cleansed = false;
			}
		}
		global_variables.insert(var_symbol);
	}
	Expect(6 /* ")" */);

	nl();
	while (!(StartOf(10)))
	{
		SynErr(255);
		Get();
	}
	while (la->kind == 246 /* "\n" */)
	{
		nl();
	}
}

void Parser::Quant()
{
	if (StartOf(11))
	{
		Exists();
	}
	else if (StartOf(12))
	{
		Forall();
	}
	else
		SynErr(256);
}

void Parser::Output()
{
	if (la->kind == 38 /* "output" */)
	{
		Get();
	}
	else if (la->kind >= 39 && la->kind <= 101)
	{
		Get();
		if (check_for_cleansed)
		{
			SemErr("output keyword is not in lower case");
			correct_cleansed = false;
		}
	}
	else
		SynErr(257);
}

void Parser::Lit()
{
	if (la->kind == _ident || la->kind == _number_ident)
	{
		Var();
	}
	else if (la->kind == 9 /* "-" */)
	{
		Get();
		Var();
	}
	else
		SynErr(258);
}

void Parser::Ngate_type()
{
	if (StartOf(13))
	{
		And();
	}
	else if (StartOf(14))
	{
		Or();
	}
	else
		SynErr(259);
}

void Parser::Xor()
{
	if (la->kind == 10 /* "xor" */)
	{
		Get();
	}
	else if (la->kind >= 11 && la->kind <= 17)
	{
		Get();
		if (check_for_cleansed)
		{
			SemErr("xor keyword is not in lower case");
			correct_cleansed = false;
		}
	}
	else
		SynErr(260);
}

void Parser::Ite()
{
	if (la->kind == 18 /* "ite" */)
	{
		Get();
	}
	else if (la->kind >= 19 && la->kind <= 25)
	{
		Get();
		if (check_for_cleansed)
		{
			SemErr("ite keyword is not in lower case");
			correct_cleansed = false;
		}
	}
	else
		SynErr(261);
}

void Parser::Exists()
{
	if (la->kind == 118 /* exists */)
	{
		Get();
	}
	else if (la->kind >= 119 && la->kind <= 181)
	{
		Get();
		if (check_for_cleansed)
		{
			SemErr("exists keyword is not in lower case");
			correct_cleansed = false;
		}
	}
	else
		SynErr(262);
}

void Parser::Forall()
{
	if (la->kind == 182 /* "forall" */)
	{
		Get();
	}
	else if (la->kind >= 183 && la->kind <= 245)
	{
		Get();
		if (check_for_cleansed)
		{
			SemErr("forall keyword is not in lower case");
			correct_cleansed = false;
		}
	}
	else
		SynErr(263);
}

void Parser::And()
{
	if (la->kind == 26 /* "and" */)
	{
		Get();
	}
	else if (la->kind >= 27 && la->kind <= 33)
	{
		Get();
		if (check_for_cleansed)
		{
			SemErr("and keyword is not in lower case");
			correct_cleansed = false;
		}
	}
	else
		SynErr(264);
}

void Parser::Or()
{
	if (la->kind == 34 /* "or" */)
	{
		Get();
	}
	else if (la->kind >= 35 && la->kind <= 37)
	{
		Get();
		if (check_for_cleansed)
		{
			SemErr("or keyword is not in lower case");
			correct_cleansed = false;
		}
	}
	else
		SynErr(265);
}

bool Parser::isAllDigits(const char *arr) const
{
	while (*arr != L'\0')
	{
		if (!iswdigit(*arr))
		{
			return false;
		}
		arr++;
	}
	return true;
}

size_t Parser::getSymbol(const char *str)
{
	std::string_view str_view(str);
	const auto it = this->symbols.find(str_view);
	if (it == this->symbols.end()) {
		size_t symbol = this->symbols_size++;
		symbols.emplace(std::pair(str_view, symbol));
		return symbol;
	}
	else
	{
		return it->second;
	}
}

// If the user declared a method Init and a mehtod Destroy they should
// be called in the contructur and the destructor respctively.
//
// The following templates are used to recognize if the user declared
// the methods Init and Destroy.

template <typename T>
struct ParserInitExistsRecognizer
{
	template <typename U, void (U::*)() = &U::Init>
	struct ExistsIfInitIsDefinedMarker
	{
	};

	struct InitIsMissingType
	{
		char dummy1;
	};

	struct InitExistsType
	{
		char dummy1;
		char dummy2;
	};

	// exists always
	template <typename U>
	static InitIsMissingType is_here(...);

	// exist only if ExistsIfInitIsDefinedMarker is defined
	template <typename U>
	static InitExistsType is_here(ExistsIfInitIsDefinedMarker<U> *);

	enum
	{
		InitExists = (sizeof(is_here<T>(NULL)) == sizeof(InitExistsType))
	};
};

template <typename T>
struct ParserDestroyExistsRecognizer
{
	template <typename U, void (U::*)() = &U::Destroy>
	struct ExistsIfDestroyIsDefinedMarker
	{
	};

	struct DestroyIsMissingType
	{
		char dummy1;
	};

	struct DestroyExistsType
	{
		char dummy1;
		char dummy2;
	};

	// exists always
	template <typename U>
	static DestroyIsMissingType is_here(...);

	// exist only if ExistsIfDestroyIsDefinedMarker is defined
	template <typename U>
	static DestroyExistsType is_here(ExistsIfDestroyIsDefinedMarker<U> *);

	enum
	{
		DestroyExists = (sizeof(is_here<T>(NULL)) == sizeof(DestroyExistsType))
	};
};

// The folloing templates are used to call the Init and Destroy methods if they exist.

// Generic case of the ParserInitCaller, gets used if the Init method is missing
template <typename T, bool = ParserInitExistsRecognizer<T>::InitExists>
struct ParserInitCaller
{
	static void CallInit(T *t)
	{
		// nothing to do
	}
};

// True case of the ParserInitCaller, gets used if the Init method exists
template <typename T>
struct ParserInitCaller<T, true>
{
	static void CallInit(T *t)
	{
		t->Init();
	}
};

// Generic case of the ParserDestroyCaller, gets used if the Destroy method is missing
template <typename T, bool = ParserDestroyExistsRecognizer<T>::DestroyExists>
struct ParserDestroyCaller
{
	static void CallDestroy(T *t)
	{
		// nothing to do
	}
};

// True case of the ParserDestroyCaller, gets used if the Destroy method exists
template <typename T>
struct ParserDestroyCaller<T, true>
{
	static void CallDestroy(T *t)
	{
		t->Destroy();
	}
};

void Parser::Parse()
{
	t = NULL;
	la = dummyToken = new Token();
	la->val = coco_string_create("Dummy Token");
	Get();
	Qcir_file();
	Expect(0);
}

Parser::Parser(Scanner *scanner, bool check_for_cleansed)
{
	maxT = 247;

	ParserInitCaller<Parser>::CallInit(this);
	dummyToken = NULL;
	t = la = NULL;
	minErrDist = 2;
	errDist = minErrDist;
	this->scanner = scanner;
	this->check_for_cleansed = check_for_cleansed;
	this->output_gate = "undefined";
	this->output_gate_defined = false;
	this->symbols = ankerl::unordered_dense::map<std::string_view, size_t>();
	this->symbols_size = 0;
	this->gate_variables = ankerl::unordered_dense::map<size_t, Gate>();
	this->global_variables = ankerl::unordered_dense::set<size_t>();
	this->resolved_variables = ankerl::unordered_dense::set<size_t>();
	errors = new Errors();
}

bool Parser::StartOf(int s) const
{
	const bool T = true;
	const bool x = false;

	static bool set[15][249] = {
		{T, T, T, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, x, x},
		{T, T, T, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, x, x, x},
		{T, T, T, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x},
		{x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x},
		{T, T, T, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, T, x, x},
		{x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x},
		{x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, x, x, x},
		{x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, T, T, T, T, T, T, T, T, T, T, T, T, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x},
		{x, x, x, x, x, x, x, x, x, x, T, T, T, T, T, T, T, T, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x},
		{x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, T, T, T, T, T, T, T, T, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x},
		{T, T, T, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, x, x},
		{x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x},
		{x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, T, x, x, x},
		{x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, T, T, T, T, T, T, T, T, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x},
		{x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, T, T, T, T, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x, x}};

	return set[s][la->kind];
}

Parser::~Parser()
{
	ParserDestroyCaller<Parser>::CallDestroy(this);
	delete errors;
	delete dummyToken;

	delete output_gate;
	symbols.clear();
	gate_variables.clear();
	global_variables.clear();
	resolved_variables.clear();
}

Gate::Gate()
{
	type = NO_QUANTIFIERS;
	unresolved_variables = ankerl::unordered_dense::set<size_t>();
}

Gate::~Gate()
{
	unresolved_variables.clear();
}

Errors::Errors()
{
	count = 0;
}

void Errors::SynErr(int line, int col, int n)
{
	char *s;
	switch (n)
	{
	case 0:
		s = coco_string_create("EOF expected");
		break;
	case 1:
		s = coco_string_create("ident expected");
		break;
	case 2:
		s = coco_string_create("number_ident expected");
		break;
	case 3:
		s = coco_string_create("\"#QCIR-G14\" expected");
		break;
	case 4:
		s = coco_string_create("\"(\" expected");
		break;
	case 5:
		s = coco_string_create("\",\" expected");
		break;
	case 6:
		s = coco_string_create("\")\" expected");
		break;
	case 7:
		s = coco_string_create("\"=\" expected");
		break;
	case 8:
		s = coco_string_create("\";\" expected");
		break;
	case 9:
		s = coco_string_create("\"-\" expected");
		break;
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
	case 15:
	case 16:
	case 17:
		s = coco_string_create("\"xor\" expected");
		break;
	case 18:
	case 19:
	case 20:
	case 21:
	case 22:
	case 23:
	case 24:
	case 25:
		s = coco_string_create("\"ite\" expected");
		break;
	case 26:
	case 27:
	case 28:
	case 29:
	case 30:
	case 31:
	case 32:
	case 33:
		s = coco_string_create("\"and\" expected");
		break;
	case 34:
	case 35:
	case 36:
	case 37:
		s = coco_string_create("\"or\" expected");
		break;
	case 38:
	case 39:
	case 40:
	case 41:
	case 42:
	case 43:
	case 44:
	case 45:
	case 46:
	case 47:
	case 48:
	case 49:
	case 50:
	case 51:
	case 52:
	case 53:
	case 54:
	case 55:
	case 56:
	case 57:
	case 58:
	case 59:
	case 60:
	case 61:
	case 62:
	case 63:
	case 64:
	case 65:
	case 66:
	case 67:
	case 68:
	case 69:
	case 70:
	case 71:
	case 72:
	case 73:
	case 74:
	case 75:
	case 76:
	case 77:
	case 78:
	case 79:
	case 80:
	case 81:
	case 82:
	case 83:
	case 84:
	case 85:
	case 86:
	case 87:
	case 88:
	case 89:
	case 90:
	case 91:
	case 92:
	case 93:
	case 94:
	case 95:
	case 96:
	case 97:
	case 98:
	case 99:
	case 100:
	case 101:
		s = coco_string_create("\"output\" expected");
		break;
	case 102:
	case 103:
	case 104:
	case 105:
	case 106:
	case 107:
	case 108:
	case 109:
	case 110:
	case 111:
	case 112:
	case 113:
	case 114:
	case 115:
	case 116:
	case 117:
		s = coco_string_create("\"FREE\" expected");
		break;
	case 118:
	case 119:
	case 120:
	case 121:
	case 122:
	case 123:
	case 124:
	case 125:
	case 126:
	case 127:
	case 128:
	case 129:
	case 130:
	case 131:
	case 132:
	case 133:
	case 134:
	case 135:
	case 136:
	case 137:
	case 138:
	case 139:
	case 140:
	case 141:
	case 142:
	case 143:
	case 144:
	case 145:
	case 146:
	case 147:
	case 148:
	case 149:
	case 150:
	case 151:
	case 152:
	case 153:
	case 154:
	case 155:
	case 156:
	case 157:
	case 158:
	case 159:
	case 160:
	case 161:
	case 162:
	case 163:
	case 164:
	case 165:
	case 166:
	case 167:
	case 168:
	case 169:
	case 170:
	case 171:
	case 172:
	case 173:
	case 174:
	case 175:
	case 176:
	case 177:
	case 178:
	case 179:
	case 180:
	case 181:
		s = coco_string_create("\"exists\" expected");
		break;
	case 182:
	case 183:
	case 184:
	case 185:
	case 186:
	case 187:
	case 188:
	case 189:
	case 190:
	case 191:
	case 192:
	case 193:
	case 194:
	case 195:
	case 196:
	case 197:
	case 198:
	case 199:
	case 200:
	case 201:
	case 202:
	case 203:
	case 204:
	case 205:
	case 206:
	case 207:
	case 208:
	case 209:
	case 210:
	case 211:
	case 212:
	case 213:
	case 214:
	case 215:
	case 216:
	case 217:
	case 218:
	case 219:
	case 220:
	case 221:
	case 222:
	case 223:
	case 224:
	case 225:
	case 226:
	case 227:
	case 228:
	case 229:
	case 230:
	case 231:
	case 232:
	case 233:
	case 234:
	case 235:
	case 236:
	case 237:
	case 238:
	case 239:
	case 240:
	case 241:
	case 242:
	case 243:
	case 244:
	case 245:
		s = coco_string_create("\"forall\" expected");
		break;
	case 246:
		s = coco_string_create("\"\\n\" expected");
		break;
	case 247:
		s = coco_string_create("??? expected");
		break;
	case 248:
		s = coco_string_create("this symbol not expected in Qblock_stmt");
		break;
	case 249:
		s = coco_string_create("this symbol not expected in Output_stmt");
		break;
	case 250:
		s = coco_string_create("this symbol not expected in Gate_stmt");
		break;
	case 251:
		s = coco_string_create("this symbol not expected in Qcir_file");
		break;
	case 252:
		s = coco_string_create("invalid Gate_stmt");
		break;
	case 253:
		s = coco_string_create("invalid Free");
		break;
	case 254:
		s = coco_string_create("invalid Var");
		break;
	case 255:
		s = coco_string_create("this symbol not expected in Qblock_quant");
		break;
	case 256:
		s = coco_string_create("invalid Quant");
		break;
	case 257:
		s = coco_string_create("invalid Output");
		break;
	case 258:
		s = coco_string_create("invalid Lit");
		break;
	case 259:
		s = coco_string_create("invalid Ngate_type");
		break;
	case 260:
		s = coco_string_create("invalid Xor");
		break;
	case 261:
		s = coco_string_create("invalid Ite");
		break;
	case 262:
		s = coco_string_create("invalid Exists");
		break;
	case 263:
		s = coco_string_create("invalid Forall");
		break;
	case 264:
		s = coco_string_create("invalid And");
		break;
	case 265:
		s = coco_string_create("invalid Or");
		break;

	default:
	{
		char format[20];
		coco_swprintf(format, 20, "error %d", n);
		s = coco_string_create(format);
	}
	break;
	}
	std::wcout << "-- line " << line << " col " << col << ": " << s << std::endl;
	coco_string_delete(s);
	count++;
}

void Errors::Error(int line, int col, const char *s)
{
	std::wcout << "-- Error: line " << line << " col " << col << ": " << s << std::endl;
	count++;
}

void Errors::Error(int line, int col, const char *s1, const char *s2)
{
	std::wcout << "-- Error: line " << line << " col " << col << ": " << s1 << " " << s2 << std::endl;
	count++;
}

void Errors::Warning(int line, int col, const char *s)
{
	std::wcout << "-- Warning: line " << line << " col " << col << ": " << s << std::endl;
}

void Errors::Warning(int line, int col, const char *s1, const char *s2)
{
	std::wcout << "-- Warning: line " << line << " col " << col << ": " << s1 << " " << s2 << std::endl;
}

void Errors::Exception(const char *s)
{
	std::wcout << s << std::endl;
	exit(1);
}
