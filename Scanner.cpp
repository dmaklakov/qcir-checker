

#include <memory.h>
#include <string.h>
#include "Scanner.h"




// string handling, wide character


wchar_t* coco_string_create(const wchar_t* value) {
	return coco_string_create(value, 0);
}

wchar_t* coco_string_create(const wchar_t *value, int startIndex) {
	int valueLen = 0;
	int len = 0;

	if (value) {
		valueLen = wcslen(value);
		len = valueLen - startIndex;
	}

	return coco_string_create(value, startIndex, len);
}

wchar_t* coco_string_create(const wchar_t *value, int startIndex, int length) {
	int len = 0;
	wchar_t* data;

	if (value) { len = length; }
	data = new wchar_t[len + 1];
	wcsncpy(data, &(value[startIndex]), len);
	data[len] = 0;

	return data;
}

wchar_t* coco_string_create_upper(const wchar_t* data) {
	if (!data) { return NULL; }

	int dataLen = 0;
	if (data) { dataLen = wcslen(data); }

	wchar_t *newData = new wchar_t[dataLen + 1];

	for (int i = 0; i <= dataLen; i++) {
		if ((L'a' <= data[i]) && (data[i] <= L'z')) {
			newData[i] = data[i] + (L'A' - L'a');
		}
		else { newData[i] = data[i]; }
	}

	newData[dataLen] = L'\0';
	return newData;
}

wchar_t* coco_string_create_lower(const wchar_t* data) {
	if (!data) { return NULL; }
	int dataLen = wcslen(data);
	return coco_string_create_lower(data, 0, dataLen);
}

wchar_t* coco_string_create_lower(const wchar_t* data, int startIndex, int dataLen) {
	if (!data) { return NULL; }

	wchar_t* newData = new wchar_t[dataLen + 1];

	for (int i = 0; i <= dataLen; i++) {
		wchar_t ch = data[startIndex + i];
		if ((L'A' <= ch) && (ch <= L'Z')) {
			newData[i] = ch - (L'A' - L'a');
		}
		else { newData[i] = ch; }
	}
	newData[dataLen] = L'\0';
	return newData;
}

wchar_t* coco_string_create_append(const wchar_t* data1, const wchar_t* data2) {
	wchar_t* data;
	int data1Len = 0;
	int data2Len = 0;

	if (data1) { data1Len = wcslen(data1); }
	if (data2) {data2Len = wcslen(data2); }

	data = new wchar_t[data1Len + data2Len + 1];

	if (data1) { wcscpy(data, data1); }
	if (data2) { wcscpy(data + data1Len, data2); }

	data[data1Len + data2Len] = 0;

	return data;
}

wchar_t* coco_string_create_append(const wchar_t *target, const wchar_t appendix) {
	int targetLen = coco_string_length(target);
	wchar_t* data = new wchar_t[targetLen + 2];
	wcsncpy(data, target, targetLen);
	data[targetLen] = appendix;
	data[targetLen + 1] = 0;
	return data;
}

void coco_string_delete(wchar_t* &data) {
	delete [] data;
	data = NULL;
}

int coco_string_length(const wchar_t* data) {
	if (data) { return wcslen(data); }
	return 0;
}

bool coco_string_endswith(const wchar_t* data, const wchar_t *end) {
	int dataLen = wcslen(data);
	int endLen = wcslen(end);
	return (endLen <= dataLen) && (wcscmp(data + dataLen - endLen, end) == 0);
}

int coco_string_indexof(const wchar_t* data, const wchar_t value) {
	const wchar_t* chr = wcschr(data, value);

	if (chr) { return (chr-data); }
	return -1;
}

int coco_string_lastindexof(const wchar_t* data, const wchar_t value) {
	const wchar_t* chr = wcsrchr(data, value);

	if (chr) { return (chr-data); }
	return -1;
}

void coco_string_merge(wchar_t* &target, const wchar_t* appendix) {
	if (!appendix) { return; }
	wchar_t* data = coco_string_create_append(target, appendix);
	delete [] target;
	target = data;
}

bool coco_string_equal(const wchar_t* data1, const wchar_t* data2) {
	return wcscmp( data1, data2 ) == 0;
}

int coco_string_compareto(const wchar_t* data1, const wchar_t* data2) {
	return wcscmp(data1, data2);
}

unsigned int coco_string_hash(const wchar_t *data) {
	unsigned int h = 0;
	if (!data) { return 0; }
	while (*data != 0) {
		h = (h * 7) ^ *data;
		++data;
	}
	return h;
}

// string handling, ascii character

wchar_t* coco_string_create(const char* value) {
	int len = 0;
	if (value) { len = strlen(value); }
	wchar_t* data = new wchar_t[len + 1];
	for (int i = 0; i < len; ++i) { data[i] = (wchar_t) value[i]; }
	data[len] = 0;
	return data;
}

char* coco_string_create_char(const wchar_t *value) {
	int len = coco_string_length(value);
	char *res = new char[len + 1];
	for (int i = 0; i < len; ++i) { res[i] = (char) value[i]; }
	res[len] = 0;
	return res;
}

void coco_string_delete(char* &data) {
	delete [] data;
	data = NULL;
}


Token::Token() {
	kind = 0;
	pos  = 0;
	col  = 0;
	line = 0;
	val  = NULL;
	next = NULL;
}

Token::~Token() {
	coco_string_delete(val);
}

Buffer::Buffer(FILE* s, bool isUserStream) {
// ensure binary read on windows
#if _MSC_VER >= 1300
	_setmode(_fileno(s), _O_BINARY);
#endif
	stream = s; this->isUserStream = isUserStream;
	if (CanSeek()) {
		fseek(s, 0, SEEK_END);
		fileLen = ftell(s);
		fseek(s, 0, SEEK_SET);
		bufLen = (fileLen < COCO_MAX_BUFFER_LENGTH) ? fileLen : COCO_MAX_BUFFER_LENGTH;
		bufStart = INT_MAX; // nothing in the buffer so far
	} else {
		fileLen = bufLen = bufStart = 0;
	}
	bufCapacity = (bufLen>0) ? bufLen : COCO_MIN_BUFFER_LENGTH;
	buf = new unsigned char[bufCapacity];	
	if (fileLen > 0) SetPos(0);          // setup  buffer to position 0 (start)
	else bufPos = 0; // index 0 is already after the file, thus Pos = 0 is invalid
	if (bufLen == fileLen && CanSeek()) Close();
}

Buffer::Buffer(Buffer *b) {
	buf = b->buf;
	bufCapacity = b->bufCapacity;
	b->buf = NULL;
	bufStart = b->bufStart;
	bufLen = b->bufLen;
	fileLen = b->fileLen;
	bufPos = b->bufPos;
	stream = b->stream;
	b->stream = NULL;
	isUserStream = b->isUserStream;
}

Buffer::Buffer(const unsigned char* buf, int len) {
	this->buf = new unsigned char[len];
	memcpy(this->buf, buf, len*sizeof(unsigned char));
	bufStart = 0;
	bufCapacity = bufLen = len;
	fileLen = len;
	bufPos = 0;
	stream = NULL;
}

Buffer::~Buffer() {
	Close(); 
	if (buf != NULL) {
		delete [] buf;
		buf = NULL;
	}
}

void Buffer::Close() {
	if (!isUserStream && stream != NULL) {
		fclose(stream);
		stream = NULL;
	}
}

int Buffer::Read() {
	if (bufPos < bufLen) {
		return buf[bufPos++];
	} else if (GetPos() < fileLen) {
		SetPos(GetPos()); // shift buffer start to Pos
		return buf[bufPos++];
	} else if ((stream != NULL) && !CanSeek() && (ReadNextStreamChunk() > 0)) {
		return buf[bufPos++];
	} else {
		return EoF;
	}
}

int Buffer::Peek() {
	int curPos = GetPos();
	int ch = Read();
	SetPos(curPos);
	return ch;
}

// beg .. begin, zero-based, inclusive, in byte
// end .. end, zero-based, exclusive, in byte
wchar_t* Buffer::GetString(int beg, int end) {
	int len = 0;
	wchar_t *buf = new wchar_t[end - beg];
	int oldPos = GetPos();
	SetPos(beg);
	while (GetPos() < end) buf[len++] = (wchar_t) Read();
	SetPos(oldPos);
	wchar_t *res = coco_string_create(buf, 0, len);
	coco_string_delete(buf);
	return res;
}

int Buffer::GetPos() {
	return bufPos + bufStart;
}

void Buffer::SetPos(int value) {
	if ((value >= fileLen) && (stream != NULL) && !CanSeek()) {
		// Wanted position is after buffer and the stream
		// is not seek-able e.g. network or console,
		// thus we have to read the stream manually till
		// the wanted position is in sight.
		while ((value >= fileLen) && (ReadNextStreamChunk() > 0));
	}

	if ((value < 0) || (value > fileLen)) {
		wprintf(L"--- buffer out of bounds access, position: %d\n", value);
		exit(1);
	}

	if ((value >= bufStart) && (value < (bufStart + bufLen))) { // already in buffer
		bufPos = value - bufStart;
	} else if (stream != NULL) { // must be swapped in
		fseek(stream, value, SEEK_SET);
		bufLen = fread(buf, sizeof(unsigned char), bufCapacity, stream);
		bufStart = value; bufPos = 0;
	} else {
		bufPos = fileLen - bufStart; // make Pos return fileLen
	}
}

// Read the next chunk of bytes from the stream, increases the buffer
// if needed and updates the fields fileLen and bufLen.
// Returns the number of bytes read.
int Buffer::ReadNextStreamChunk() {
	int free = bufCapacity - bufLen;
	if (free == 0) {
		// in the case of a growing input stream
		// we can neither seek in the stream, nor can we
		// foresee the maximum length, thus we must adapt
		// the buffer size on demand.
		bufCapacity = bufLen * 2;
		unsigned char *newBuf = new unsigned char[bufCapacity];
		memcpy(newBuf, buf, bufLen*sizeof(unsigned char));
		delete [] buf;
		buf = newBuf;
		free = bufLen;
	}
	int read = fread(buf + bufLen, sizeof(unsigned char), free, stream);
	if (read > 0) {
		fileLen = bufLen = (bufLen + read);
		return read;
	}
	// end of stream reached
	return 0;
}

bool Buffer::CanSeek() {
	return (stream != NULL) && (ftell(stream) != -1);
}

int UTF8Buffer::Read() {
	int ch;
	do {
		ch = Buffer::Read();
		// until we find a utf8 start (0xxxxxxx or 11xxxxxx)
	} while ((ch >= 128) && ((ch & 0xC0) != 0xC0) && (ch != EoF));
	if (ch < 128 || ch == EoF) {
		// nothing to do, first 127 chars are the same in ascii and utf8
		// 0xxxxxxx or end of file character
	} else if ((ch & 0xF0) == 0xF0) {
		// 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
		int c1 = ch & 0x07; ch = Buffer::Read();
		int c2 = ch & 0x3F; ch = Buffer::Read();
		int c3 = ch & 0x3F; ch = Buffer::Read();
		int c4 = ch & 0x3F;
		ch = (((((c1 << 6) | c2) << 6) | c3) << 6) | c4;
	} else if ((ch & 0xE0) == 0xE0) {
		// 1110xxxx 10xxxxxx 10xxxxxx
		int c1 = ch & 0x0F; ch = Buffer::Read();
		int c2 = ch & 0x3F; ch = Buffer::Read();
		int c3 = ch & 0x3F;
		ch = (((c1 << 6) | c2) << 6) | c3;
	} else if ((ch & 0xC0) == 0xC0) {
		// 110xxxxx 10xxxxxx
		int c1 = ch & 0x1F; ch = Buffer::Read();
		int c2 = ch & 0x3F;
		ch = (c1 << 6) | c2;
	}
	return ch;
}

Scanner::Scanner(const unsigned char* buf, int len) {
	buffer = new Buffer(buf, len);
	Init();
}

Scanner::Scanner(const wchar_t* fileName) {
	FILE* stream;
	char *chFileName = coco_string_create_char(fileName);
	if ((stream = fopen(chFileName, "rb")) == NULL) {
		wprintf(L"--- Cannot open file %ls\n", fileName);
		exit(1);
	}
	coco_string_delete(chFileName);
	buffer = new Buffer(stream, false);
	Init();
}

Scanner::Scanner(FILE* s) {
	buffer = new Buffer(s, true);
	Init();
}

Scanner::~Scanner() {
	char* cur = (char*) firstHeap;

	while(cur != NULL) {
		cur = *(char**) (cur + COCO_HEAP_BLOCK_SIZE);
		free(firstHeap);
		firstHeap = cur;
	}
	delete [] tval;
	delete buffer;
}

void Scanner::Init() {
	EOL    = '\n';
	eofSym = 0;
	maxT = 247;
	noSym = 247;
	int i;
	for (i = 65; i <= 90; ++i) start.set(i, 1);
	for (i = 95; i <= 95; ++i) start.set(i, 1);
	for (i = 97; i <= 122; ++i) start.set(i, 1);
	for (i = 48; i <= 57; ++i) start.set(i, 2);
	start.set(35, 3);
	start.set(40, 12);
	start.set(44, 13);
	start.set(41, 14);
	start.set(61, 15);
	start.set(59, 16);
	start.set(45, 17);
	start.set(10, 18);
		start.set(Buffer::EoF, -1);
	keywords.set(L"xor", 10);
	keywords.set(L"xoR", 11);
	keywords.set(L"xOr", 12);
	keywords.set(L"xOR", 13);
	keywords.set(L"Xor", 14);
	keywords.set(L"XoR", 15);
	keywords.set(L"XOr", 16);
	keywords.set(L"XOR", 17);
	keywords.set(L"ite", 18);
	keywords.set(L"itE", 19);
	keywords.set(L"iTe", 20);
	keywords.set(L"iTE", 21);
	keywords.set(L"Ite", 22);
	keywords.set(L"ItE", 23);
	keywords.set(L"ITe", 24);
	keywords.set(L"ITE", 25);
	keywords.set(L"and", 26);
	keywords.set(L"anD", 27);
	keywords.set(L"aNd", 28);
	keywords.set(L"aND", 29);
	keywords.set(L"And", 30);
	keywords.set(L"AnD", 31);
	keywords.set(L"ANd", 32);
	keywords.set(L"AND", 33);
	keywords.set(L"or", 34);
	keywords.set(L"oR", 35);
	keywords.set(L"Or", 36);
	keywords.set(L"OR", 37);
	keywords.set(L"output", 38);
	keywords.set(L"outpuT", 39);
	keywords.set(L"outpUt", 40);
	keywords.set(L"outpUT", 41);
	keywords.set(L"outPut", 42);
	keywords.set(L"outPuT", 43);
	keywords.set(L"outPUt", 44);
	keywords.set(L"outPUT", 45);
	keywords.set(L"ouTput", 46);
	keywords.set(L"ouTpuT", 47);
	keywords.set(L"ouTpUt", 48);
	keywords.set(L"ouTpUT", 49);
	keywords.set(L"ouTPut", 50);
	keywords.set(L"ouTPuT", 51);
	keywords.set(L"ouTPUt", 52);
	keywords.set(L"ouTPUT", 53);
	keywords.set(L"oUtput", 54);
	keywords.set(L"oUtpuT", 55);
	keywords.set(L"oUtpUt", 56);
	keywords.set(L"oUtpUT", 57);
	keywords.set(L"oUtPut", 58);
	keywords.set(L"oUtPuT", 59);
	keywords.set(L"oUtPUt", 60);
	keywords.set(L"oUtPUT", 61);
	keywords.set(L"oUTput", 62);
	keywords.set(L"oUTpuT", 63);
	keywords.set(L"oUTpUt", 64);
	keywords.set(L"oUTpUT", 65);
	keywords.set(L"oUTPut", 66);
	keywords.set(L"oUTPuT", 67);
	keywords.set(L"oUTPUt", 68);
	keywords.set(L"oUTPUT", 69);
	keywords.set(L"Output", 70);
	keywords.set(L"OutpuT", 71);
	keywords.set(L"OutpUt", 72);
	keywords.set(L"OutpUT", 73);
	keywords.set(L"OutPut", 74);
	keywords.set(L"OutPuT", 75);
	keywords.set(L"OutPUt", 76);
	keywords.set(L"OutPUT", 77);
	keywords.set(L"OuTput", 78);
	keywords.set(L"OuTpuT", 79);
	keywords.set(L"OuTpUt", 80);
	keywords.set(L"OuTpUT", 81);
	keywords.set(L"OuTPut", 82);
	keywords.set(L"OuTPuT", 83);
	keywords.set(L"OuTPUt", 84);
	keywords.set(L"OuTPUT", 85);
	keywords.set(L"OUtput", 86);
	keywords.set(L"OUtpuT", 87);
	keywords.set(L"OUtpUt", 88);
	keywords.set(L"OUtpUT", 89);
	keywords.set(L"OUtPut", 90);
	keywords.set(L"OUtPuT", 91);
	keywords.set(L"OUtPUt", 92);
	keywords.set(L"OUtPUT", 93);
	keywords.set(L"OUTput", 94);
	keywords.set(L"OUTpuT", 95);
	keywords.set(L"OUTpUt", 96);
	keywords.set(L"OUTpUT", 97);
	keywords.set(L"OUTPut", 98);
	keywords.set(L"OUTPuT", 99);
	keywords.set(L"OUTPUt", 100);
	keywords.set(L"OUTPUT", 101);
	keywords.set(L"free", 102);
	keywords.set(L"freE", 103);
	keywords.set(L"frEe", 104);
	keywords.set(L"frEE", 105);
	keywords.set(L"fRee", 106);
	keywords.set(L"fReE", 107);
	keywords.set(L"fREe", 108);
	keywords.set(L"fREE", 109);
	keywords.set(L"Free", 110);
	keywords.set(L"FreE", 111);
	keywords.set(L"FrEe", 112);
	keywords.set(L"FrEE", 113);
	keywords.set(L"FRee", 114);
	keywords.set(L"FReE", 115);
	keywords.set(L"FREe", 116);
	keywords.set(L"FREE", 117);
	keywords.set(L"exists", 118);
	keywords.set(L"existS", 119);
	keywords.set(L"exisTs", 120);
	keywords.set(L"exisTS", 121);
	keywords.set(L"exiSts", 122);
	keywords.set(L"exiStS", 123);
	keywords.set(L"exiSTs", 124);
	keywords.set(L"exiSTS", 125);
	keywords.set(L"exIsts", 126);
	keywords.set(L"exIstS", 127);
	keywords.set(L"exIsTs", 128);
	keywords.set(L"exIsTS", 129);
	keywords.set(L"exISts", 130);
	keywords.set(L"exIStS", 131);
	keywords.set(L"exISTs", 132);
	keywords.set(L"exISTS", 133);
	keywords.set(L"eXists", 134);
	keywords.set(L"eXistS", 135);
	keywords.set(L"eXisTs", 136);
	keywords.set(L"eXisTS", 137);
	keywords.set(L"eXiSts", 138);
	keywords.set(L"eXiStS", 139);
	keywords.set(L"eXiSTs", 140);
	keywords.set(L"eXiSTS", 141);
	keywords.set(L"eXIsts", 142);
	keywords.set(L"eXIstS", 143);
	keywords.set(L"eXIsTs", 144);
	keywords.set(L"eXIsTS", 145);
	keywords.set(L"eXISts", 146);
	keywords.set(L"eXIStS", 147);
	keywords.set(L"eXISTs", 148);
	keywords.set(L"eXISTS", 149);
	keywords.set(L"Exists", 150);
	keywords.set(L"ExistS", 151);
	keywords.set(L"ExisTs", 152);
	keywords.set(L"ExisTS", 153);
	keywords.set(L"ExiSts", 154);
	keywords.set(L"ExiStS", 155);
	keywords.set(L"ExiSTs", 156);
	keywords.set(L"ExiSTS", 157);
	keywords.set(L"ExIsts", 158);
	keywords.set(L"ExIstS", 159);
	keywords.set(L"ExIsTs", 160);
	keywords.set(L"ExIsTS", 161);
	keywords.set(L"ExISts", 162);
	keywords.set(L"ExIStS", 163);
	keywords.set(L"ExISTs", 164);
	keywords.set(L"ExISTS", 165);
	keywords.set(L"EXists", 166);
	keywords.set(L"EXistS", 167);
	keywords.set(L"EXisTs", 168);
	keywords.set(L"EXisTS", 169);
	keywords.set(L"EXiSts", 170);
	keywords.set(L"EXiStS", 171);
	keywords.set(L"EXiSTs", 172);
	keywords.set(L"EXiSTS", 173);
	keywords.set(L"EXIsts", 174);
	keywords.set(L"EXIstS", 175);
	keywords.set(L"EXIsTs", 176);
	keywords.set(L"EXIsTS", 177);
	keywords.set(L"EXISts", 178);
	keywords.set(L"EXIStS", 179);
	keywords.set(L"EXISTs", 180);
	keywords.set(L"EXISTS", 181);
	keywords.set(L"forall", 182);
	keywords.set(L"foralL", 183);
	keywords.set(L"foraLl", 184);
	keywords.set(L"foraLL", 185);
	keywords.set(L"forAll", 186);
	keywords.set(L"forAlL", 187);
	keywords.set(L"forALl", 188);
	keywords.set(L"forALL", 189);
	keywords.set(L"foRall", 190);
	keywords.set(L"foRalL", 191);
	keywords.set(L"foRaLl", 192);
	keywords.set(L"foRaLL", 193);
	keywords.set(L"foRAll", 194);
	keywords.set(L"foRAlL", 195);
	keywords.set(L"foRALl", 196);
	keywords.set(L"foRALL", 197);
	keywords.set(L"fOrall", 198);
	keywords.set(L"fOralL", 199);
	keywords.set(L"fOraLl", 200);
	keywords.set(L"fOraLL", 201);
	keywords.set(L"fOrAll", 202);
	keywords.set(L"fOrAlL", 203);
	keywords.set(L"fOrALl", 204);
	keywords.set(L"fOrALL", 205);
	keywords.set(L"fORall", 206);
	keywords.set(L"fORalL", 207);
	keywords.set(L"fORaLl", 208);
	keywords.set(L"fORaLL", 209);
	keywords.set(L"fORAll", 210);
	keywords.set(L"fORAlL", 211);
	keywords.set(L"fORALl", 212);
	keywords.set(L"fORALL", 213);
	keywords.set(L"Forall", 214);
	keywords.set(L"ForalL", 215);
	keywords.set(L"ForaLl", 216);
	keywords.set(L"ForaLL", 217);
	keywords.set(L"ForAll", 218);
	keywords.set(L"ForAlL", 219);
	keywords.set(L"ForALl", 220);
	keywords.set(L"ForALL", 221);
	keywords.set(L"FoRall", 222);
	keywords.set(L"FoRalL", 223);
	keywords.set(L"FoRaLl", 224);
	keywords.set(L"FoRaLL", 225);
	keywords.set(L"FoRAll", 226);
	keywords.set(L"FoRAlL", 227);
	keywords.set(L"FoRALl", 228);
	keywords.set(L"FoRALL", 229);
	keywords.set(L"FOrall", 230);
	keywords.set(L"FOralL", 231);
	keywords.set(L"FOraLl", 232);
	keywords.set(L"FOraLL", 233);
	keywords.set(L"FOrAll", 234);
	keywords.set(L"FOrAlL", 235);
	keywords.set(L"FOrALl", 236);
	keywords.set(L"FOrALL", 237);
	keywords.set(L"FORall", 238);
	keywords.set(L"FORalL", 239);
	keywords.set(L"FORaLl", 240);
	keywords.set(L"FORaLL", 241);
	keywords.set(L"FORAll", 242);
	keywords.set(L"FORAlL", 243);
	keywords.set(L"FORALl", 244);
	keywords.set(L"FORALL", 245);


	tvalLength = 128;
	tval = new wchar_t[tvalLength]; // text of current token

	// COCO_HEAP_BLOCK_SIZE byte heap + pointer to next heap block
	heap = malloc(COCO_HEAP_BLOCK_SIZE + sizeof(void*));
	firstHeap = heap;
	heapEnd = (void**) (((char*) heap) + COCO_HEAP_BLOCK_SIZE);
	*heapEnd = 0;
	heapTop = heap;
	if (sizeof(Token) > COCO_HEAP_BLOCK_SIZE) {
		wprintf(L"--- Too small COCO_HEAP_BLOCK_SIZE\n");
		exit(1);
	}

	pos = -1; line = 1; col = 0; charPos = -1;
	oldEols = 0;
	NextCh();
	if (ch == 0xEF) { // check optional byte order mark for UTF-8
		NextCh(); int ch1 = ch;
		NextCh(); int ch2 = ch;
		if (ch1 != 0xBB || ch2 != 0xBF) {
			wprintf(L"Illegal byte order mark at start of file");
			exit(1);
		}
		Buffer *oldBuf = buffer;
		buffer = new UTF8Buffer(buffer); col = 0; charPos = -1;
		delete oldBuf; oldBuf = NULL;
		NextCh();
	}


	pt = tokens = CreateToken(); // first token is a dummy
}

void Scanner::NextCh() {
	if (oldEols > 0) { ch = EOL; oldEols--; }
	else {
		pos = buffer->GetPos();
		// buffer reads unicode chars, if UTF8 has been detected
		ch = buffer->Read(); col++; charPos++;
		// replace isolated '\r' by '\n' in order to make
		// eol handling uniform across Windows, Unix and Mac
		if (ch == L'\r' && buffer->Peek() != L'\n') ch = EOL;
		if (ch == EOL) { line++; col = 0; }
	}

}

void Scanner::AddCh() {
	if (tlen >= tvalLength) {
		tvalLength *= 2;
		wchar_t *newBuf = new wchar_t[tvalLength];
		memcpy(newBuf, tval, tlen*sizeof(wchar_t));
		delete [] tval;
		tval = newBuf;
	}
	if (ch != Buffer::EoF) {
		tval[tlen++] = ch;
		NextCh();
	}
}



void Scanner::CreateHeapBlock() {
	void* newHeap;
	char* cur = (char*) firstHeap;

	while(((char*) tokens < cur) || ((char*) tokens > (cur + COCO_HEAP_BLOCK_SIZE))) {
		cur = *((char**) (cur + COCO_HEAP_BLOCK_SIZE));
		free(firstHeap);
		firstHeap = cur;
	}

	// COCO_HEAP_BLOCK_SIZE byte heap + pointer to next heap block
	newHeap = malloc(COCO_HEAP_BLOCK_SIZE + sizeof(void*));
	*heapEnd = newHeap;
	heapEnd = (void**) (((char*) newHeap) + COCO_HEAP_BLOCK_SIZE);
	*heapEnd = 0;
	heap = newHeap;
	heapTop = heap;
}

Token* Scanner::CreateToken() {
	Token *t;
	if (((char*) heapTop + (int) sizeof(Token)) >= (char*) heapEnd) {
		CreateHeapBlock();
	}
	t = (Token*) heapTop;
	heapTop = (void*) ((char*) heapTop + sizeof(Token));
	t->val = NULL;
	t->next = NULL;
	return t;
}

void Scanner::AppendVal(Token *t) {
	int reqMem = (tlen + 1) * sizeof(wchar_t);
	if (((char*) heapTop + reqMem) >= (char*) heapEnd) {
		if (reqMem > COCO_HEAP_BLOCK_SIZE) {
			wprintf(L"--- Too long token value\n");
			exit(1);
		}
		CreateHeapBlock();
	}
	t->val = (wchar_t*) heapTop;
	heapTop = (void*) ((char*) heapTop + reqMem);

	wcsncpy(t->val, tval, tlen);
	t->val[tlen] = L'\0';
}

Token* Scanner::NextToken() {
	while (ch == ' ' ||
			ch == 9 || ch == 13
	) NextCh();

	int recKind = noSym;
	int recEnd = pos;
	t = CreateToken();
	t->pos = pos; t->col = col; t->line = line; t->charPos = charPos;
	int state = start.state(ch);
	tlen = 0; AddCh();

	switch (state) {
		case -1: { t->kind = eofSym; break; } // NextCh already done
		case 0: {
			case_0:
			if (recKind != noSym) {
				tlen = recEnd - t->pos;
				SetScannerBehindT();
			}
			t->kind = recKind; break;
		} // NextCh already done
		case 1:
			case_1:
			recEnd = pos; recKind = 1;
			if ((ch >= L'0' && ch <= L'9') || (ch >= L'A' && ch <= L'Z') || ch == L'_' || (ch >= L'a' && ch <= L'z')) {AddCh(); goto case_1;}
			else {t->kind = 1; wchar_t *literal = coco_string_create(tval, 0, tlen); t->kind = keywords.get(literal, t->kind); coco_string_delete(literal); break;}
		case 2:
			case_2:
			recEnd = pos; recKind = 2;
			if ((ch >= L'0' && ch <= L'9') || (ch >= L'A' && ch <= L'Z') || ch == L'_' || (ch >= L'a' && ch <= L'z')) {AddCh(); goto case_2;}
			else {t->kind = 2; break;}
		case 3:
			if (ch == L'Q') {AddCh(); goto case_4;}
			else {goto case_0;}
		case 4:
			case_4:
			if (ch == L'C') {AddCh(); goto case_5;}
			else {goto case_0;}
		case 5:
			case_5:
			if (ch == L'I') {AddCh(); goto case_6;}
			else {goto case_0;}
		case 6:
			case_6:
			if (ch == L'R') {AddCh(); goto case_7;}
			else {goto case_0;}
		case 7:
			case_7:
			if (ch == L'-') {AddCh(); goto case_8;}
			else {goto case_0;}
		case 8:
			case_8:
			if (ch == L'G') {AddCh(); goto case_9;}
			else {goto case_0;}
		case 9:
			case_9:
			if (ch == L'1') {AddCh(); goto case_10;}
			else {goto case_0;}
		case 10:
			case_10:
			if (ch == L'4') {AddCh(); goto case_11;}
			else {goto case_0;}
		case 11:
			case_11:
			{t->kind = 3; break;}
		case 12:
			{t->kind = 4; break;}
		case 13:
			{t->kind = 5; break;}
		case 14:
			{t->kind = 6; break;}
		case 15:
			{t->kind = 7; break;}
		case 16:
			{t->kind = 8; break;}
		case 17:
			{t->kind = 9; break;}
		case 18:
			{t->kind = 246; break;}

	}
	AppendVal(t);
	return t;
}

void Scanner::SetScannerBehindT() {
	buffer->SetPos(t->pos);
	NextCh();
	line = t->line; col = t->col; charPos = t->charPos;
	for (int i = 0; i < tlen; i++) NextCh();
}

// get the next token (possibly a token already seen during peeking)
Token* Scanner::Scan() {
	if (tokens->next == NULL) {
		return pt = tokens = NextToken();
	} else {
		pt = tokens = tokens->next;
		return tokens;
	}
}

// peek for the next token, ignore pragmas
Token* Scanner::Peek() {
	do {
		if (pt->next == NULL) {
			pt->next = NextToken();
		}
		pt = pt->next;
	} while (pt->kind > maxT); // skip pragmas

	return pt;
}

// make sure that peeking starts at the current scan position
void Scanner::ResetPeek() {
	pt = tokens;
}


