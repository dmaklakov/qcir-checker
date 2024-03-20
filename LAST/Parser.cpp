

#include <wchar.h>
#include "Parser.h"
#include "Scanner.h"




void Parser::SynErr(int n) {
	if (errDist >= minErrDist) errors->SynErr(la->line, la->col, n);
	errDist = 0;
}

void Parser::SemErr(const wchar_t* msg) {
	if (errDist >= minErrDist) errors->Error(t->line, t->col, msg);
	errDist = 0;
}

void Parser::Get() {
	for (;;) {
		t = la;
		la = scanner->Scan();
		if (la->kind <= maxT) { ++errDist; break; }

		if (dummyToken != t) {
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

void Parser::Expect(int n) {
	if (la->kind==n) Get(); else { SynErr(n); }
}

void Parser::ExpectWeak(int n, int follow) {
	if (la->kind == n) Get();
	else {
		SynErr(n);
		while (!StartOf(follow)) Get();
	}
}

bool Parser::WeakSeparator(int n, int syFol, int repFol) {
	if (la->kind == n) {Get(); return true;}
	else if (StartOf(repFol)) {return false;}
	else {
		SynErr(n);
		while (!(StartOf(syFol) || StartOf(repFol) || StartOf(0))) {
			Get();
		}
		return StartOf(syFol);
	}
}

void Parser::Qcir_file() {
		if (la->kind == 3 /* "#QCIR-G14" */) {
			Format_id();
		}
		while (!(StartOf(1))) {SynErr(248); Get();}
		Qblock_stmt();
		while (!(StartOf(2))) {SynErr(249); Get();}
		if (StartOf(3)) {
			Output_stmt();
		}
		while (!(la->kind == _EOF || la->kind == _ident || la->kind == _number_ident)) {SynErr(250); Get();}
		while (la->kind == _ident || la->kind == _number_ident) {
			while (!(la->kind == _EOF || la->kind == _ident || la->kind == _number_ident)) {SynErr(251); Get();}
			Gate_stmt();
			nl();
			while (la->kind == 246 /* "\n" */) {
				nl();
			}
		}
}

void Parser::Format_id() {
		Expect(3 /* "#QCIR-G14" */);
		if (la->kind == _number_ident) {
			Get();
		}
		nl();
		while (la->kind == 246 /* "\n" */) {
			nl();
		}
}

void Parser::Qblock_stmt() {
		if (StartOf(4)) {
			Free();
			Expect(4 /* "(" */);
			Var();
			while (la->kind == 5 /* "," */) {
				Get();
				Var();
			}
			Expect(6 /* ")" */);
			nl();
			while (la->kind == 246 /* "\n" */) {
				nl();
			}
		}
		while (!(StartOf(5))) {SynErr(252); Get();}
		while (StartOf(6)) {
			while (!(StartOf(7))) {SynErr(253); Get();}
			Qblock_quant();
		}
}

void Parser::Output_stmt() {
		Output();
		Expect(4 /* "(" */);
		Lit();
		Expect(6 /* ")" */);
		nl();
		while (la->kind == 246 /* "\n" */) {
			nl();
		}
}

void Parser::Gate_stmt() {
		Var();
		Expect(7 /* "=" */);
		if (StartOf(8)) {
			Ngate_type();
			Expect(4 /* "(" */);
			if (la->kind == _ident || la->kind == _number_ident || la->kind == 9 /* "-" */) {
				Lit();
				while (la->kind == 5 /* "," */) {
					Get();
					Lit();
				}
			}
			Expect(6 /* ")" */);
		} else if (StartOf(9)) {
			Xor();
			Expect(4 /* "(" */);
			Lit();
			Expect(5 /* "," */);
			Lit();
			Expect(6 /* ")" */);
		} else if (StartOf(10)) {
			Ite();
			Expect(4 /* "(" */);
			Lit();
			Expect(5 /* "," */);
			Lit();
			Expect(5 /* "," */);
			Lit();
			Expect(6 /* ")" */);
		} else if (StartOf(6)) {
			Quant();
			Expect(4 /* "(" */);
			Var();
			while (la->kind == 5 /* "," */) {
				Get();
				Var();
			}
			Expect(8 /* ";" */);
			Lit();
			Expect(6 /* ")" */);
		} else SynErr(254);
}

void Parser::nl() {
		Expect(246 /* "\n" */);
}

void Parser::Free() {
		switch (la->kind) {
		case 102 /* "free" */: {
			Get();
			break;
		}
		case 103 /* "freE" */: {
			Get();
			break;
		}
		case 104 /* "frEe" */: {
			Get();
			break;
		}
		case 105 /* "frEE" */: {
			Get();
			break;
		}
		case 106 /* "fRee" */: {
			Get();
			break;
		}
		case 107 /* "fReE" */: {
			Get();
			break;
		}
		case 108 /* "fREe" */: {
			Get();
			break;
		}
		case 109 /* "fREE" */: {
			Get();
			break;
		}
		case 110 /* "Free" */: {
			Get();
			break;
		}
		case 111 /* "FreE" */: {
			Get();
			break;
		}
		case 112 /* "FrEe" */: {
			Get();
			break;
		}
		case 113 /* "FrEE" */: {
			Get();
			break;
		}
		case 114 /* "FRee" */: {
			Get();
			break;
		}
		case 115 /* "FReE" */: {
			Get();
			break;
		}
		case 116 /* "FREe" */: {
			Get();
			break;
		}
		case 117 /* "FREE" */: {
			Get();
			break;
		}
		default: SynErr(255); break;
		}
}

void Parser::Var() {
		if (la->kind == _ident) {
			Get();
		} else if (la->kind == _number_ident) {
			Get();
		} else SynErr(256);
}

void Parser::Qblock_quant() {
		Quant();
		Expect(4 /* "(" */);
		Var();
		while (la->kind == 5 /* "," */) {
			Get();
			Var();
		}
		Expect(6 /* ")" */);
		nl();
		while (la->kind == 246 /* "\n" */) {
			nl();
		}
}

void Parser::Quant() {
		if (StartOf(11)) {
			Exists();
		} else if (StartOf(12)) {
			Forall();
		} else SynErr(257);
}

void Parser::Output() {
		switch (la->kind) {
		case 38 /* "output" */: {
			Get();
			break;
		}
		case 39 /* "outpuT" */: {
			Get();
			break;
		}
		case 40 /* "outpUt" */: {
			Get();
			break;
		}
		case 41 /* "outpUT" */: {
			Get();
			break;
		}
		case 42 /* "outPut" */: {
			Get();
			break;
		}
		case 43 /* "outPuT" */: {
			Get();
			break;
		}
		case 44 /* "outPUt" */: {
			Get();
			break;
		}
		case 45 /* "outPUT" */: {
			Get();
			break;
		}
		case 46 /* "ouTput" */: {
			Get();
			break;
		}
		case 47 /* "ouTpuT" */: {
			Get();
			break;
		}
		case 48 /* "ouTpUt" */: {
			Get();
			break;
		}
		case 49 /* "ouTpUT" */: {
			Get();
			break;
		}
		case 50 /* "ouTPut" */: {
			Get();
			break;
		}
		case 51 /* "ouTPuT" */: {
			Get();
			break;
		}
		case 52 /* "ouTPUt" */: {
			Get();
			break;
		}
		case 53 /* "ouTPUT" */: {
			Get();
			break;
		}
		case 54 /* "oUtput" */: {
			Get();
			break;
		}
		case 55 /* "oUtpuT" */: {
			Get();
			break;
		}
		case 56 /* "oUtpUt" */: {
			Get();
			break;
		}
		case 57 /* "oUtpUT" */: {
			Get();
			break;
		}
		case 58 /* "oUtPut" */: {
			Get();
			break;
		}
		case 59 /* "oUtPuT" */: {
			Get();
			break;
		}
		case 60 /* "oUtPUt" */: {
			Get();
			break;
		}
		case 61 /* "oUtPUT" */: {
			Get();
			break;
		}
		case 62 /* "oUTput" */: {
			Get();
			break;
		}
		case 63 /* "oUTpuT" */: {
			Get();
			break;
		}
		case 64 /* "oUTpUt" */: {
			Get();
			break;
		}
		case 65 /* "oUTpUT" */: {
			Get();
			break;
		}
		case 66 /* "oUTPut" */: {
			Get();
			break;
		}
		case 67 /* "oUTPuT" */: {
			Get();
			break;
		}
		case 68 /* "oUTPUt" */: {
			Get();
			break;
		}
		case 69 /* "oUTPUT" */: {
			Get();
			break;
		}
		case 70 /* "Output" */: {
			Get();
			break;
		}
		case 71 /* "OutpuT" */: {
			Get();
			break;
		}
		case 72 /* "OutpUt" */: {
			Get();
			break;
		}
		case 73 /* "OutpUT" */: {
			Get();
			break;
		}
		case 74 /* "OutPut" */: {
			Get();
			break;
		}
		case 75 /* "OutPuT" */: {
			Get();
			break;
		}
		case 76 /* "OutPUt" */: {
			Get();
			break;
		}
		case 77 /* "OutPUT" */: {
			Get();
			break;
		}
		case 78 /* "OuTput" */: {
			Get();
			break;
		}
		case 79 /* "OuTpuT" */: {
			Get();
			break;
		}
		case 80 /* "OuTpUt" */: {
			Get();
			break;
		}
		case 81 /* "OuTpUT" */: {
			Get();
			break;
		}
		case 82 /* "OuTPut" */: {
			Get();
			break;
		}
		case 83 /* "OuTPuT" */: {
			Get();
			break;
		}
		case 84 /* "OuTPUt" */: {
			Get();
			break;
		}
		case 85 /* "OuTPUT" */: {
			Get();
			break;
		}
		case 86 /* "OUtput" */: {
			Get();
			break;
		}
		case 87 /* "OUtpuT" */: {
			Get();
			break;
		}
		case 88 /* "OUtpUt" */: {
			Get();
			break;
		}
		case 89 /* "OUtpUT" */: {
			Get();
			break;
		}
		case 90 /* "OUtPut" */: {
			Get();
			break;
		}
		case 91 /* "OUtPuT" */: {
			Get();
			break;
		}
		case 92 /* "OUtPUt" */: {
			Get();
			break;
		}
		case 93 /* "OUtPUT" */: {
			Get();
			break;
		}
		case 94 /* "OUTput" */: {
			Get();
			break;
		}
		case 95 /* "OUTpuT" */: {
			Get();
			break;
		}
		case 96 /* "OUTpUt" */: {
			Get();
			break;
		}
		case 97 /* "OUTpUT" */: {
			Get();
			break;
		}
		case 98 /* "OUTPut" */: {
			Get();
			break;
		}
		case 99 /* "OUTPuT" */: {
			Get();
			break;
		}
		case 100 /* "OUTPUt" */: {
			Get();
			break;
		}
		case 101 /* "OUTPUT" */: {
			Get();
			break;
		}
		default: SynErr(258); break;
		}
}

void Parser::Lit() {
		if (la->kind == _ident || la->kind == _number_ident) {
			Var();
		} else if (la->kind == 9 /* "-" */) {
			Get();
			Var();
		} else SynErr(259);
}

void Parser::Ngate_type() {
		if (StartOf(13)) {
			And();
		} else if (StartOf(14)) {
			Or();
		} else SynErr(260);
}

void Parser::Xor() {
		switch (la->kind) {
		case 10 /* "xor" */: {
			Get();
			break;
		}
		case 11 /* "xoR" */: {
			Get();
			break;
		}
		case 12 /* "xOr" */: {
			Get();
			break;
		}
		case 13 /* "xOR" */: {
			Get();
			break;
		}
		case 14 /* "Xor" */: {
			Get();
			break;
		}
		case 15 /* "XoR" */: {
			Get();
			break;
		}
		case 16 /* "XOr" */: {
			Get();
			break;
		}
		case 17 /* "XOR" */: {
			Get();
			break;
		}
		default: SynErr(261); break;
		}
}

void Parser::Ite() {
		switch (la->kind) {
		case 18 /* "ite" */: {
			Get();
			break;
		}
		case 19 /* "itE" */: {
			Get();
			break;
		}
		case 20 /* "iTe" */: {
			Get();
			break;
		}
		case 21 /* "iTE" */: {
			Get();
			break;
		}
		case 22 /* "Ite" */: {
			Get();
			break;
		}
		case 23 /* "ItE" */: {
			Get();
			break;
		}
		case 24 /* "ITe" */: {
			Get();
			break;
		}
		case 25 /* "ITE" */: {
			Get();
			break;
		}
		default: SynErr(262); break;
		}
}

void Parser::Exists() {
		switch (la->kind) {
		case 118 /* "exists" */: {
			Get();
			break;
		}
		case 119 /* "existS" */: {
			Get();
			break;
		}
		case 120 /* "exisTs" */: {
			Get();
			break;
		}
		case 121 /* "exisTS" */: {
			Get();
			break;
		}
		case 122 /* "exiSts" */: {
			Get();
			break;
		}
		case 123 /* "exiStS" */: {
			Get();
			break;
		}
		case 124 /* "exiSTs" */: {
			Get();
			break;
		}
		case 125 /* "exiSTS" */: {
			Get();
			break;
		}
		case 126 /* "exIsts" */: {
			Get();
			break;
		}
		case 127 /* "exIstS" */: {
			Get();
			break;
		}
		case 128 /* "exIsTs" */: {
			Get();
			break;
		}
		case 129 /* "exIsTS" */: {
			Get();
			break;
		}
		case 130 /* "exISts" */: {
			Get();
			break;
		}
		case 131 /* "exIStS" */: {
			Get();
			break;
		}
		case 132 /* "exISTs" */: {
			Get();
			break;
		}
		case 133 /* "exISTS" */: {
			Get();
			break;
		}
		case 134 /* "eXists" */: {
			Get();
			break;
		}
		case 135 /* "eXistS" */: {
			Get();
			break;
		}
		case 136 /* "eXisTs" */: {
			Get();
			break;
		}
		case 137 /* "eXisTS" */: {
			Get();
			break;
		}
		case 138 /* "eXiSts" */: {
			Get();
			break;
		}
		case 139 /* "eXiStS" */: {
			Get();
			break;
		}
		case 140 /* "eXiSTs" */: {
			Get();
			break;
		}
		case 141 /* "eXiSTS" */: {
			Get();
			break;
		}
		case 142 /* "eXIsts" */: {
			Get();
			break;
		}
		case 143 /* "eXIstS" */: {
			Get();
			break;
		}
		case 144 /* "eXIsTs" */: {
			Get();
			break;
		}
		case 145 /* "eXIsTS" */: {
			Get();
			break;
		}
		case 146 /* "eXISts" */: {
			Get();
			break;
		}
		case 147 /* "eXIStS" */: {
			Get();
			break;
		}
		case 148 /* "eXISTs" */: {
			Get();
			break;
		}
		case 149 /* "eXISTS" */: {
			Get();
			break;
		}
		case 150 /* "Exists" */: {
			Get();
			break;
		}
		case 151 /* "ExistS" */: {
			Get();
			break;
		}
		case 152 /* "ExisTs" */: {
			Get();
			break;
		}
		case 153 /* "ExisTS" */: {
			Get();
			break;
		}
		case 154 /* "ExiSts" */: {
			Get();
			break;
		}
		case 155 /* "ExiStS" */: {
			Get();
			break;
		}
		case 156 /* "ExiSTs" */: {
			Get();
			break;
		}
		case 157 /* "ExiSTS" */: {
			Get();
			break;
		}
		case 158 /* "ExIsts" */: {
			Get();
			break;
		}
		case 159 /* "ExIstS" */: {
			Get();
			break;
		}
		case 160 /* "ExIsTs" */: {
			Get();
			break;
		}
		case 161 /* "ExIsTS" */: {
			Get();
			break;
		}
		case 162 /* "ExISts" */: {
			Get();
			break;
		}
		case 163 /* "ExIStS" */: {
			Get();
			break;
		}
		case 164 /* "ExISTs" */: {
			Get();
			break;
		}
		case 165 /* "ExISTS" */: {
			Get();
			break;
		}
		case 166 /* "EXists" */: {
			Get();
			break;
		}
		case 167 /* "EXistS" */: {
			Get();
			break;
		}
		case 168 /* "EXisTs" */: {
			Get();
			break;
		}
		case 169 /* "EXisTS" */: {
			Get();
			break;
		}
		case 170 /* "EXiSts" */: {
			Get();
			break;
		}
		case 171 /* "EXiStS" */: {
			Get();
			break;
		}
		case 172 /* "EXiSTs" */: {
			Get();
			break;
		}
		case 173 /* "EXiSTS" */: {
			Get();
			break;
		}
		case 174 /* "EXIsts" */: {
			Get();
			break;
		}
		case 175 /* "EXIstS" */: {
			Get();
			break;
		}
		case 176 /* "EXIsTs" */: {
			Get();
			break;
		}
		case 177 /* "EXIsTS" */: {
			Get();
			break;
		}
		case 178 /* "EXISts" */: {
			Get();
			break;
		}
		case 179 /* "EXIStS" */: {
			Get();
			break;
		}
		case 180 /* "EXISTs" */: {
			Get();
			break;
		}
		case 181 /* "EXISTS" */: {
			Get();
			break;
		}
		default: SynErr(263); break;
		}
}

void Parser::Forall() {
		switch (la->kind) {
		case 182 /* "forall" */: {
			Get();
			break;
		}
		case 183 /* "foralL" */: {
			Get();
			break;
		}
		case 184 /* "foraLl" */: {
			Get();
			break;
		}
		case 185 /* "foraLL" */: {
			Get();
			break;
		}
		case 186 /* "forAll" */: {
			Get();
			break;
		}
		case 187 /* "forAlL" */: {
			Get();
			break;
		}
		case 188 /* "forALl" */: {
			Get();
			break;
		}
		case 189 /* "forALL" */: {
			Get();
			break;
		}
		case 190 /* "foRall" */: {
			Get();
			break;
		}
		case 191 /* "foRalL" */: {
			Get();
			break;
		}
		case 192 /* "foRaLl" */: {
			Get();
			break;
		}
		case 193 /* "foRaLL" */: {
			Get();
			break;
		}
		case 194 /* "foRAll" */: {
			Get();
			break;
		}
		case 195 /* "foRAlL" */: {
			Get();
			break;
		}
		case 196 /* "foRALl" */: {
			Get();
			break;
		}
		case 197 /* "foRALL" */: {
			Get();
			break;
		}
		case 198 /* "fOrall" */: {
			Get();
			break;
		}
		case 199 /* "fOralL" */: {
			Get();
			break;
		}
		case 200 /* "fOraLl" */: {
			Get();
			break;
		}
		case 201 /* "fOraLL" */: {
			Get();
			break;
		}
		case 202 /* "fOrAll" */: {
			Get();
			break;
		}
		case 203 /* "fOrAlL" */: {
			Get();
			break;
		}
		case 204 /* "fOrALl" */: {
			Get();
			break;
		}
		case 205 /* "fOrALL" */: {
			Get();
			break;
		}
		case 206 /* "fORall" */: {
			Get();
			break;
		}
		case 207 /* "fORalL" */: {
			Get();
			break;
		}
		case 208 /* "fORaLl" */: {
			Get();
			break;
		}
		case 209 /* "fORaLL" */: {
			Get();
			break;
		}
		case 210 /* "fORAll" */: {
			Get();
			break;
		}
		case 211 /* "fORAlL" */: {
			Get();
			break;
		}
		case 212 /* "fORALl" */: {
			Get();
			break;
		}
		case 213 /* "fORALL" */: {
			Get();
			break;
		}
		case 214 /* "Forall" */: {
			Get();
			break;
		}
		case 215 /* "ForalL" */: {
			Get();
			break;
		}
		case 216 /* "ForaLl" */: {
			Get();
			break;
		}
		case 217 /* "ForaLL" */: {
			Get();
			break;
		}
		case 218 /* "ForAll" */: {
			Get();
			break;
		}
		case 219 /* "ForAlL" */: {
			Get();
			break;
		}
		case 220 /* "ForALl" */: {
			Get();
			break;
		}
		case 221 /* "ForALL" */: {
			Get();
			break;
		}
		case 222 /* "FoRall" */: {
			Get();
			break;
		}
		case 223 /* "FoRalL" */: {
			Get();
			break;
		}
		case 224 /* "FoRaLl" */: {
			Get();
			break;
		}
		case 225 /* "FoRaLL" */: {
			Get();
			break;
		}
		case 226 /* "FoRAll" */: {
			Get();
			break;
		}
		case 227 /* "FoRAlL" */: {
			Get();
			break;
		}
		case 228 /* "FoRALl" */: {
			Get();
			break;
		}
		case 229 /* "FoRALL" */: {
			Get();
			break;
		}
		case 230 /* "FOrall" */: {
			Get();
			break;
		}
		case 231 /* "FOralL" */: {
			Get();
			break;
		}
		case 232 /* "FOraLl" */: {
			Get();
			break;
		}
		case 233 /* "FOraLL" */: {
			Get();
			break;
		}
		case 234 /* "FOrAll" */: {
			Get();
			break;
		}
		case 235 /* "FOrAlL" */: {
			Get();
			break;
		}
		case 236 /* "FOrALl" */: {
			Get();
			break;
		}
		case 237 /* "FOrALL" */: {
			Get();
			break;
		}
		case 238 /* "FORall" */: {
			Get();
			break;
		}
		case 239 /* "FORalL" */: {
			Get();
			break;
		}
		case 240 /* "FORaLl" */: {
			Get();
			break;
		}
		case 241 /* "FORaLL" */: {
			Get();
			break;
		}
		case 242 /* "FORAll" */: {
			Get();
			break;
		}
		case 243 /* "FORAlL" */: {
			Get();
			break;
		}
		case 244 /* "FORALl" */: {
			Get();
			break;
		}
		case 245 /* "FORALL" */: {
			Get();
			break;
		}
		default: SynErr(264); break;
		}
}

void Parser::And() {
		switch (la->kind) {
		case 26 /* "and" */: {
			Get();
			break;
		}
		case 27 /* "anD" */: {
			Get();
			break;
		}
		case 28 /* "aNd" */: {
			Get();
			break;
		}
		case 29 /* "aND" */: {
			Get();
			break;
		}
		case 30 /* "And" */: {
			Get();
			break;
		}
		case 31 /* "AnD" */: {
			Get();
			break;
		}
		case 32 /* "ANd" */: {
			Get();
			break;
		}
		case 33 /* "AND" */: {
			Get();
			break;
		}
		default: SynErr(265); break;
		}
}

void Parser::Or() {
		if (la->kind == 34 /* "or" */) {
			Get();
		} else if (la->kind == 35 /* "oR" */) {
			Get();
		} else if (la->kind == 36 /* "Or" */) {
			Get();
		} else if (la->kind == 37 /* "OR" */) {
			Get();
		} else SynErr(266);
}




// If the user declared a method Init and a mehtod Destroy they should
// be called in the contructur and the destructor respctively.
//
// The following templates are used to recognize if the user declared
// the methods Init and Destroy.

template<typename T>
struct ParserInitExistsRecognizer {
	template<typename U, void (U::*)() = &U::Init>
	struct ExistsIfInitIsDefinedMarker{};

	struct InitIsMissingType {
		char dummy1;
	};
	
	struct InitExistsType {
		char dummy1; char dummy2;
	};

	// exists always
	template<typename U>
	static InitIsMissingType is_here(...);

	// exist only if ExistsIfInitIsDefinedMarker is defined
	template<typename U>
	static InitExistsType is_here(ExistsIfInitIsDefinedMarker<U>*);

	enum { InitExists = (sizeof(is_here<T>(NULL)) == sizeof(InitExistsType)) };
};

template<typename T>
struct ParserDestroyExistsRecognizer {
	template<typename U, void (U::*)() = &U::Destroy>
	struct ExistsIfDestroyIsDefinedMarker{};

	struct DestroyIsMissingType {
		char dummy1;
	};
	
	struct DestroyExistsType {
		char dummy1; char dummy2;
	};

	// exists always
	template<typename U>
	static DestroyIsMissingType is_here(...);

	// exist only if ExistsIfDestroyIsDefinedMarker is defined
	template<typename U>
	static DestroyExistsType is_here(ExistsIfDestroyIsDefinedMarker<U>*);

	enum { DestroyExists = (sizeof(is_here<T>(NULL)) == sizeof(DestroyExistsType)) };
};

// The folloing templates are used to call the Init and Destroy methods if they exist.

// Generic case of the ParserInitCaller, gets used if the Init method is missing
template<typename T, bool = ParserInitExistsRecognizer<T>::InitExists>
struct ParserInitCaller {
	static void CallInit(T *t) {
		// nothing to do
	}
};

// True case of the ParserInitCaller, gets used if the Init method exists
template<typename T>
struct ParserInitCaller<T, true> {
	static void CallInit(T *t) {
		t->Init();
	}
};

// Generic case of the ParserDestroyCaller, gets used if the Destroy method is missing
template<typename T, bool = ParserDestroyExistsRecognizer<T>::DestroyExists>
struct ParserDestroyCaller {
	static void CallDestroy(T *t) {
		// nothing to do
	}
};

// True case of the ParserDestroyCaller, gets used if the Destroy method exists
template<typename T>
struct ParserDestroyCaller<T, true> {
	static void CallDestroy(T *t) {
		t->Destroy();
	}
};

void Parser::Parse() {
	t = NULL;
	la = dummyToken = new Token();
	la->val = coco_string_create(L"Dummy Token");
	Get();
	Qcir_file();
	Expect(0);
}

Parser::Parser(Scanner *scanner) {
	maxT = 247;

	ParserInitCaller<Parser>::CallInit(this);
	dummyToken = NULL;
	t = la = NULL;
	minErrDist = 2;
	errDist = minErrDist;
	this->scanner = scanner;
	errors = new Errors();
}

bool Parser::StartOf(int s) {
	const bool T = true;
	const bool x = false;

	static bool set[15][249] = {
		{T,T,T,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,x,x, x},
		{T,T,T,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,x,x, x},
		{T,T,T,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x},
		{x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x},
		{x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x},
		{T,T,T,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,x,x, x},
		{x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,x,x, x},
		{T,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,x,x, x},
		{x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,T,T, T,T,T,T, T,T,T,T, T,T,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x},
		{x,x,x,x, x,x,x,x, x,x,T,T, T,T,T,T, T,T,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x},
		{x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,T,T, T,T,T,T, T,T,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x},
		{x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x},
		{x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,T,T, T,T,x,x, x},
		{x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,T,T, T,T,T,T, T,T,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x},
		{x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,T,T, T,T,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x,x,x,x, x}
	};



	return set[s][la->kind];
}

Parser::~Parser() {
	ParserDestroyCaller<Parser>::CallDestroy(this);
	delete errors;
	delete dummyToken;
}

Errors::Errors() {
	count = 0;
}

void Errors::SynErr(int line, int col, int n) {
	wchar_t* s;
	switch (n) {
			case 0: s = coco_string_create(L"EOF expected"); break;
			case 1: s = coco_string_create(L"ident expected"); break;
			case 2: s = coco_string_create(L"number_ident expected"); break;
			case 3: s = coco_string_create(L"\"#QCIR-G14\" expected"); break;
			case 4: s = coco_string_create(L"\"(\" expected"); break;
			case 5: s = coco_string_create(L"\",\" expected"); break;
			case 6: s = coco_string_create(L"\")\" expected"); break;
			case 7: s = coco_string_create(L"\"=\" expected"); break;
			case 8: s = coco_string_create(L"\";\" expected"); break;
			case 9: s = coco_string_create(L"\"-\" expected"); break;
			case 10: s = coco_string_create(L"\"xor\" expected"); break;
			case 11: s = coco_string_create(L"\"xoR\" expected"); break;
			case 12: s = coco_string_create(L"\"xOr\" expected"); break;
			case 13: s = coco_string_create(L"\"xOR\" expected"); break;
			case 14: s = coco_string_create(L"\"Xor\" expected"); break;
			case 15: s = coco_string_create(L"\"XoR\" expected"); break;
			case 16: s = coco_string_create(L"\"XOr\" expected"); break;
			case 17: s = coco_string_create(L"\"XOR\" expected"); break;
			case 18: s = coco_string_create(L"\"ite\" expected"); break;
			case 19: s = coco_string_create(L"\"itE\" expected"); break;
			case 20: s = coco_string_create(L"\"iTe\" expected"); break;
			case 21: s = coco_string_create(L"\"iTE\" expected"); break;
			case 22: s = coco_string_create(L"\"Ite\" expected"); break;
			case 23: s = coco_string_create(L"\"ItE\" expected"); break;
			case 24: s = coco_string_create(L"\"ITe\" expected"); break;
			case 25: s = coco_string_create(L"\"ITE\" expected"); break;
			case 26: s = coco_string_create(L"\"and\" expected"); break;
			case 27: s = coco_string_create(L"\"anD\" expected"); break;
			case 28: s = coco_string_create(L"\"aNd\" expected"); break;
			case 29: s = coco_string_create(L"\"aND\" expected"); break;
			case 30: s = coco_string_create(L"\"And\" expected"); break;
			case 31: s = coco_string_create(L"\"AnD\" expected"); break;
			case 32: s = coco_string_create(L"\"ANd\" expected"); break;
			case 33: s = coco_string_create(L"\"AND\" expected"); break;
			case 34: s = coco_string_create(L"\"or\" expected"); break;
			case 35: s = coco_string_create(L"\"oR\" expected"); break;
			case 36: s = coco_string_create(L"\"Or\" expected"); break;
			case 37: s = coco_string_create(L"\"OR\" expected"); break;
			case 38: s = coco_string_create(L"\"output\" expected"); break;
			case 39: s = coco_string_create(L"\"outpuT\" expected"); break;
			case 40: s = coco_string_create(L"\"outpUt\" expected"); break;
			case 41: s = coco_string_create(L"\"outpUT\" expected"); break;
			case 42: s = coco_string_create(L"\"outPut\" expected"); break;
			case 43: s = coco_string_create(L"\"outPuT\" expected"); break;
			case 44: s = coco_string_create(L"\"outPUt\" expected"); break;
			case 45: s = coco_string_create(L"\"outPUT\" expected"); break;
			case 46: s = coco_string_create(L"\"ouTput\" expected"); break;
			case 47: s = coco_string_create(L"\"ouTpuT\" expected"); break;
			case 48: s = coco_string_create(L"\"ouTpUt\" expected"); break;
			case 49: s = coco_string_create(L"\"ouTpUT\" expected"); break;
			case 50: s = coco_string_create(L"\"ouTPut\" expected"); break;
			case 51: s = coco_string_create(L"\"ouTPuT\" expected"); break;
			case 52: s = coco_string_create(L"\"ouTPUt\" expected"); break;
			case 53: s = coco_string_create(L"\"ouTPUT\" expected"); break;
			case 54: s = coco_string_create(L"\"oUtput\" expected"); break;
			case 55: s = coco_string_create(L"\"oUtpuT\" expected"); break;
			case 56: s = coco_string_create(L"\"oUtpUt\" expected"); break;
			case 57: s = coco_string_create(L"\"oUtpUT\" expected"); break;
			case 58: s = coco_string_create(L"\"oUtPut\" expected"); break;
			case 59: s = coco_string_create(L"\"oUtPuT\" expected"); break;
			case 60: s = coco_string_create(L"\"oUtPUt\" expected"); break;
			case 61: s = coco_string_create(L"\"oUtPUT\" expected"); break;
			case 62: s = coco_string_create(L"\"oUTput\" expected"); break;
			case 63: s = coco_string_create(L"\"oUTpuT\" expected"); break;
			case 64: s = coco_string_create(L"\"oUTpUt\" expected"); break;
			case 65: s = coco_string_create(L"\"oUTpUT\" expected"); break;
			case 66: s = coco_string_create(L"\"oUTPut\" expected"); break;
			case 67: s = coco_string_create(L"\"oUTPuT\" expected"); break;
			case 68: s = coco_string_create(L"\"oUTPUt\" expected"); break;
			case 69: s = coco_string_create(L"\"oUTPUT\" expected"); break;
			case 70: s = coco_string_create(L"\"Output\" expected"); break;
			case 71: s = coco_string_create(L"\"OutpuT\" expected"); break;
			case 72: s = coco_string_create(L"\"OutpUt\" expected"); break;
			case 73: s = coco_string_create(L"\"OutpUT\" expected"); break;
			case 74: s = coco_string_create(L"\"OutPut\" expected"); break;
			case 75: s = coco_string_create(L"\"OutPuT\" expected"); break;
			case 76: s = coco_string_create(L"\"OutPUt\" expected"); break;
			case 77: s = coco_string_create(L"\"OutPUT\" expected"); break;
			case 78: s = coco_string_create(L"\"OuTput\" expected"); break;
			case 79: s = coco_string_create(L"\"OuTpuT\" expected"); break;
			case 80: s = coco_string_create(L"\"OuTpUt\" expected"); break;
			case 81: s = coco_string_create(L"\"OuTpUT\" expected"); break;
			case 82: s = coco_string_create(L"\"OuTPut\" expected"); break;
			case 83: s = coco_string_create(L"\"OuTPuT\" expected"); break;
			case 84: s = coco_string_create(L"\"OuTPUt\" expected"); break;
			case 85: s = coco_string_create(L"\"OuTPUT\" expected"); break;
			case 86: s = coco_string_create(L"\"OUtput\" expected"); break;
			case 87: s = coco_string_create(L"\"OUtpuT\" expected"); break;
			case 88: s = coco_string_create(L"\"OUtpUt\" expected"); break;
			case 89: s = coco_string_create(L"\"OUtpUT\" expected"); break;
			case 90: s = coco_string_create(L"\"OUtPut\" expected"); break;
			case 91: s = coco_string_create(L"\"OUtPuT\" expected"); break;
			case 92: s = coco_string_create(L"\"OUtPUt\" expected"); break;
			case 93: s = coco_string_create(L"\"OUtPUT\" expected"); break;
			case 94: s = coco_string_create(L"\"OUTput\" expected"); break;
			case 95: s = coco_string_create(L"\"OUTpuT\" expected"); break;
			case 96: s = coco_string_create(L"\"OUTpUt\" expected"); break;
			case 97: s = coco_string_create(L"\"OUTpUT\" expected"); break;
			case 98: s = coco_string_create(L"\"OUTPut\" expected"); break;
			case 99: s = coco_string_create(L"\"OUTPuT\" expected"); break;
			case 100: s = coco_string_create(L"\"OUTPUt\" expected"); break;
			case 101: s = coco_string_create(L"\"OUTPUT\" expected"); break;
			case 102: s = coco_string_create(L"\"free\" expected"); break;
			case 103: s = coco_string_create(L"\"freE\" expected"); break;
			case 104: s = coco_string_create(L"\"frEe\" expected"); break;
			case 105: s = coco_string_create(L"\"frEE\" expected"); break;
			case 106: s = coco_string_create(L"\"fRee\" expected"); break;
			case 107: s = coco_string_create(L"\"fReE\" expected"); break;
			case 108: s = coco_string_create(L"\"fREe\" expected"); break;
			case 109: s = coco_string_create(L"\"fREE\" expected"); break;
			case 110: s = coco_string_create(L"\"Free\" expected"); break;
			case 111: s = coco_string_create(L"\"FreE\" expected"); break;
			case 112: s = coco_string_create(L"\"FrEe\" expected"); break;
			case 113: s = coco_string_create(L"\"FrEE\" expected"); break;
			case 114: s = coco_string_create(L"\"FRee\" expected"); break;
			case 115: s = coco_string_create(L"\"FReE\" expected"); break;
			case 116: s = coco_string_create(L"\"FREe\" expected"); break;
			case 117: s = coco_string_create(L"\"FREE\" expected"); break;
			case 118: s = coco_string_create(L"\"exists\" expected"); break;
			case 119: s = coco_string_create(L"\"existS\" expected"); break;
			case 120: s = coco_string_create(L"\"exisTs\" expected"); break;
			case 121: s = coco_string_create(L"\"exisTS\" expected"); break;
			case 122: s = coco_string_create(L"\"exiSts\" expected"); break;
			case 123: s = coco_string_create(L"\"exiStS\" expected"); break;
			case 124: s = coco_string_create(L"\"exiSTs\" expected"); break;
			case 125: s = coco_string_create(L"\"exiSTS\" expected"); break;
			case 126: s = coco_string_create(L"\"exIsts\" expected"); break;
			case 127: s = coco_string_create(L"\"exIstS\" expected"); break;
			case 128: s = coco_string_create(L"\"exIsTs\" expected"); break;
			case 129: s = coco_string_create(L"\"exIsTS\" expected"); break;
			case 130: s = coco_string_create(L"\"exISts\" expected"); break;
			case 131: s = coco_string_create(L"\"exIStS\" expected"); break;
			case 132: s = coco_string_create(L"\"exISTs\" expected"); break;
			case 133: s = coco_string_create(L"\"exISTS\" expected"); break;
			case 134: s = coco_string_create(L"\"eXists\" expected"); break;
			case 135: s = coco_string_create(L"\"eXistS\" expected"); break;
			case 136: s = coco_string_create(L"\"eXisTs\" expected"); break;
			case 137: s = coco_string_create(L"\"eXisTS\" expected"); break;
			case 138: s = coco_string_create(L"\"eXiSts\" expected"); break;
			case 139: s = coco_string_create(L"\"eXiStS\" expected"); break;
			case 140: s = coco_string_create(L"\"eXiSTs\" expected"); break;
			case 141: s = coco_string_create(L"\"eXiSTS\" expected"); break;
			case 142: s = coco_string_create(L"\"eXIsts\" expected"); break;
			case 143: s = coco_string_create(L"\"eXIstS\" expected"); break;
			case 144: s = coco_string_create(L"\"eXIsTs\" expected"); break;
			case 145: s = coco_string_create(L"\"eXIsTS\" expected"); break;
			case 146: s = coco_string_create(L"\"eXISts\" expected"); break;
			case 147: s = coco_string_create(L"\"eXIStS\" expected"); break;
			case 148: s = coco_string_create(L"\"eXISTs\" expected"); break;
			case 149: s = coco_string_create(L"\"eXISTS\" expected"); break;
			case 150: s = coco_string_create(L"\"Exists\" expected"); break;
			case 151: s = coco_string_create(L"\"ExistS\" expected"); break;
			case 152: s = coco_string_create(L"\"ExisTs\" expected"); break;
			case 153: s = coco_string_create(L"\"ExisTS\" expected"); break;
			case 154: s = coco_string_create(L"\"ExiSts\" expected"); break;
			case 155: s = coco_string_create(L"\"ExiStS\" expected"); break;
			case 156: s = coco_string_create(L"\"ExiSTs\" expected"); break;
			case 157: s = coco_string_create(L"\"ExiSTS\" expected"); break;
			case 158: s = coco_string_create(L"\"ExIsts\" expected"); break;
			case 159: s = coco_string_create(L"\"ExIstS\" expected"); break;
			case 160: s = coco_string_create(L"\"ExIsTs\" expected"); break;
			case 161: s = coco_string_create(L"\"ExIsTS\" expected"); break;
			case 162: s = coco_string_create(L"\"ExISts\" expected"); break;
			case 163: s = coco_string_create(L"\"ExIStS\" expected"); break;
			case 164: s = coco_string_create(L"\"ExISTs\" expected"); break;
			case 165: s = coco_string_create(L"\"ExISTS\" expected"); break;
			case 166: s = coco_string_create(L"\"EXists\" expected"); break;
			case 167: s = coco_string_create(L"\"EXistS\" expected"); break;
			case 168: s = coco_string_create(L"\"EXisTs\" expected"); break;
			case 169: s = coco_string_create(L"\"EXisTS\" expected"); break;
			case 170: s = coco_string_create(L"\"EXiSts\" expected"); break;
			case 171: s = coco_string_create(L"\"EXiStS\" expected"); break;
			case 172: s = coco_string_create(L"\"EXiSTs\" expected"); break;
			case 173: s = coco_string_create(L"\"EXiSTS\" expected"); break;
			case 174: s = coco_string_create(L"\"EXIsts\" expected"); break;
			case 175: s = coco_string_create(L"\"EXIstS\" expected"); break;
			case 176: s = coco_string_create(L"\"EXIsTs\" expected"); break;
			case 177: s = coco_string_create(L"\"EXIsTS\" expected"); break;
			case 178: s = coco_string_create(L"\"EXISts\" expected"); break;
			case 179: s = coco_string_create(L"\"EXIStS\" expected"); break;
			case 180: s = coco_string_create(L"\"EXISTs\" expected"); break;
			case 181: s = coco_string_create(L"\"EXISTS\" expected"); break;
			case 182: s = coco_string_create(L"\"forall\" expected"); break;
			case 183: s = coco_string_create(L"\"foralL\" expected"); break;
			case 184: s = coco_string_create(L"\"foraLl\" expected"); break;
			case 185: s = coco_string_create(L"\"foraLL\" expected"); break;
			case 186: s = coco_string_create(L"\"forAll\" expected"); break;
			case 187: s = coco_string_create(L"\"forAlL\" expected"); break;
			case 188: s = coco_string_create(L"\"forALl\" expected"); break;
			case 189: s = coco_string_create(L"\"forALL\" expected"); break;
			case 190: s = coco_string_create(L"\"foRall\" expected"); break;
			case 191: s = coco_string_create(L"\"foRalL\" expected"); break;
			case 192: s = coco_string_create(L"\"foRaLl\" expected"); break;
			case 193: s = coco_string_create(L"\"foRaLL\" expected"); break;
			case 194: s = coco_string_create(L"\"foRAll\" expected"); break;
			case 195: s = coco_string_create(L"\"foRAlL\" expected"); break;
			case 196: s = coco_string_create(L"\"foRALl\" expected"); break;
			case 197: s = coco_string_create(L"\"foRALL\" expected"); break;
			case 198: s = coco_string_create(L"\"fOrall\" expected"); break;
			case 199: s = coco_string_create(L"\"fOralL\" expected"); break;
			case 200: s = coco_string_create(L"\"fOraLl\" expected"); break;
			case 201: s = coco_string_create(L"\"fOraLL\" expected"); break;
			case 202: s = coco_string_create(L"\"fOrAll\" expected"); break;
			case 203: s = coco_string_create(L"\"fOrAlL\" expected"); break;
			case 204: s = coco_string_create(L"\"fOrALl\" expected"); break;
			case 205: s = coco_string_create(L"\"fOrALL\" expected"); break;
			case 206: s = coco_string_create(L"\"fORall\" expected"); break;
			case 207: s = coco_string_create(L"\"fORalL\" expected"); break;
			case 208: s = coco_string_create(L"\"fORaLl\" expected"); break;
			case 209: s = coco_string_create(L"\"fORaLL\" expected"); break;
			case 210: s = coco_string_create(L"\"fORAll\" expected"); break;
			case 211: s = coco_string_create(L"\"fORAlL\" expected"); break;
			case 212: s = coco_string_create(L"\"fORALl\" expected"); break;
			case 213: s = coco_string_create(L"\"fORALL\" expected"); break;
			case 214: s = coco_string_create(L"\"Forall\" expected"); break;
			case 215: s = coco_string_create(L"\"ForalL\" expected"); break;
			case 216: s = coco_string_create(L"\"ForaLl\" expected"); break;
			case 217: s = coco_string_create(L"\"ForaLL\" expected"); break;
			case 218: s = coco_string_create(L"\"ForAll\" expected"); break;
			case 219: s = coco_string_create(L"\"ForAlL\" expected"); break;
			case 220: s = coco_string_create(L"\"ForALl\" expected"); break;
			case 221: s = coco_string_create(L"\"ForALL\" expected"); break;
			case 222: s = coco_string_create(L"\"FoRall\" expected"); break;
			case 223: s = coco_string_create(L"\"FoRalL\" expected"); break;
			case 224: s = coco_string_create(L"\"FoRaLl\" expected"); break;
			case 225: s = coco_string_create(L"\"FoRaLL\" expected"); break;
			case 226: s = coco_string_create(L"\"FoRAll\" expected"); break;
			case 227: s = coco_string_create(L"\"FoRAlL\" expected"); break;
			case 228: s = coco_string_create(L"\"FoRALl\" expected"); break;
			case 229: s = coco_string_create(L"\"FoRALL\" expected"); break;
			case 230: s = coco_string_create(L"\"FOrall\" expected"); break;
			case 231: s = coco_string_create(L"\"FOralL\" expected"); break;
			case 232: s = coco_string_create(L"\"FOraLl\" expected"); break;
			case 233: s = coco_string_create(L"\"FOraLL\" expected"); break;
			case 234: s = coco_string_create(L"\"FOrAll\" expected"); break;
			case 235: s = coco_string_create(L"\"FOrAlL\" expected"); break;
			case 236: s = coco_string_create(L"\"FOrALl\" expected"); break;
			case 237: s = coco_string_create(L"\"FOrALL\" expected"); break;
			case 238: s = coco_string_create(L"\"FORall\" expected"); break;
			case 239: s = coco_string_create(L"\"FORalL\" expected"); break;
			case 240: s = coco_string_create(L"\"FORaLl\" expected"); break;
			case 241: s = coco_string_create(L"\"FORaLL\" expected"); break;
			case 242: s = coco_string_create(L"\"FORAll\" expected"); break;
			case 243: s = coco_string_create(L"\"FORAlL\" expected"); break;
			case 244: s = coco_string_create(L"\"FORALl\" expected"); break;
			case 245: s = coco_string_create(L"\"FORALL\" expected"); break;
			case 246: s = coco_string_create(L"\"\\n\" expected"); break;
			case 247: s = coco_string_create(L"??? expected"); break;
			case 248: s = coco_string_create(L"this symbol not expected in Qcir_file"); break;
			case 249: s = coco_string_create(L"this symbol not expected in Qcir_file"); break;
			case 250: s = coco_string_create(L"this symbol not expected in Qcir_file"); break;
			case 251: s = coco_string_create(L"this symbol not expected in Qcir_file"); break;
			case 252: s = coco_string_create(L"this symbol not expected in Qblock_stmt"); break;
			case 253: s = coco_string_create(L"this symbol not expected in Qblock_stmt"); break;
			case 254: s = coco_string_create(L"invalid Gate_stmt"); break;
			case 255: s = coco_string_create(L"invalid Free"); break;
			case 256: s = coco_string_create(L"invalid Var"); break;
			case 257: s = coco_string_create(L"invalid Quant"); break;
			case 258: s = coco_string_create(L"invalid Output"); break;
			case 259: s = coco_string_create(L"invalid Lit"); break;
			case 260: s = coco_string_create(L"invalid Ngate_type"); break;
			case 261: s = coco_string_create(L"invalid Xor"); break;
			case 262: s = coco_string_create(L"invalid Ite"); break;
			case 263: s = coco_string_create(L"invalid Exists"); break;
			case 264: s = coco_string_create(L"invalid Forall"); break;
			case 265: s = coco_string_create(L"invalid And"); break;
			case 266: s = coco_string_create(L"invalid Or"); break;

		default:
		{
			wchar_t format[20];
			coco_swprintf(format, 20, L"error %d", n);
			s = coco_string_create(format);
		}
		break;
	}
	wprintf(L"-- line %d col %d: %ls\n", line, col, s);
	coco_string_delete(s);
	count++;
}

void Errors::Error(int line, int col, const wchar_t *s) {
	wprintf(L"-- line %d col %d: %ls\n", line, col, s);
	count++;
}

void Errors::Warning(int line, int col, const wchar_t *s) {
	wprintf(L"-- line %d col %d: %ls\n", line, col, s);
}

void Errors::Warning(const wchar_t *s) {
	wprintf(L"%ls\n", s);
}

void Errors::Exception(const wchar_t* s) {
	wprintf(L"%ls", s); 
	exit(1);
}


