#include <memory.h>
#include <string.h>
#include "Scanner.h"

// string handling, wide character

char *coco_string_create(const char *value, int startIndex, int length)
{
	int len = 0;
	char *data;

	if (value)
	{
		len = length;
	}
	data = new char[len + 1];
	strncpy(data, &(value[startIndex]), len);
	data[len] = 0;

	return data;
}

char *coco_string_create_upper(const char *data)
{
	if (!data)
	{
		return NULL;
	}

	int dataLen = 0;
	if (data)
	{
		dataLen = strlen(data);
	}

	char *newData = new char[dataLen + 1];

	for (int i = 0; i <= dataLen; i++)
	{
		if (('a' <= data[i]) && (data[i] <= 'z'))
		{
			newData[i] = data[i] + ('A' - 'a');
		}
		else
		{
			newData[i] = data[i];
		}
	}

	newData[dataLen] = '\0';
	return newData;
}

char *coco_string_create_lower(const char *data)
{
	if (!data)
	{
		return NULL;
	}
	int dataLen = strlen(data);
	return coco_string_create_lower(data, 0, dataLen);
}

char *coco_string_create_lower(const char *data, int startIndex, int dataLen)
{
	if (!data)
	{
		return NULL;
	}

	char *newData = new char[dataLen + 1];

	for (int i = 0; i <= dataLen; i++)
	{
		char ch = data[startIndex + i];
		if (('A' <= ch) && (ch <= 'Z'))
		{
			newData[i] = ch - ('A' - 'a');
		}
		else
		{
			newData[i] = ch;
		}
	}
	newData[dataLen] = '\0';
	return newData;
}

char *coco_string_create_append(const char *data1, const char *data2)
{
	char *data;
	int data1Len = 0;
	int data2Len = 0;

	if (data1)
	{
		data1Len = strlen(data1);
	}
	if (data2)
	{
		data2Len = strlen(data2);
	}

	data = new char[data1Len + data2Len + 1];

	if (data1)
	{
		strcpy(data, data1);
	}
	if (data2)
	{
		strcpy(data + data1Len, data2);
	}

	data[data1Len + data2Len] = 0;

	return data;
}

char *coco_string_create_append(const char *target, const char appendix)
{
	int targetLen = coco_string_length(target);
	char *data = new char[targetLen + 2];
	strncpy(data, target, targetLen);
	data[targetLen] = appendix;
	data[targetLen + 1] = 0;
	return data;
}

int coco_string_length(const char *data)
{
	if (data)
	{
		return strlen(data);
	}
	return 0;
}

bool coco_string_endswith(const char *data, const char *end)
{
	int dataLen = strlen(data);
	int endLen = strlen(end);
	return (endLen <= dataLen) && (strcmp(data + dataLen - endLen, end) == 0);
}

int coco_string_indexof(const char *data, const char value)
{
	const char *chr = strchr(data, value);

	if (chr)
	{
		return (chr - data);
	}
	return -1;
}

int coco_string_lastindexof(const char *data, const char value)
{
	const char *chr = strchr(data, value);

	if (chr)
	{
		return (chr - data);
	}
	return -1;
}

void coco_string_merge(char *&target, const char *appendix)
{
	if (!appendix)
	{
		return;
	}
	char *data = coco_string_create_append(target, appendix);
	delete[] target;
	target = data;
}

bool coco_string_equal(const char *data1, const char *data2)
{
	return strcmp(data1, data2) == 0;
}

int coco_string_compareto(const char *data1, const char *data2)
{
	return strcmp(data1, data2);
}

unsigned int coco_string_hash(const char *data)
{
	unsigned int h = 0;
	if (!data)
	{
		return 0;
	}
	while (*data != 0)
	{
		h = (h * 7) ^ *data;
		++data;
	}
	return h;
}

// string handling, ascii character

char *coco_string_create(const char *value)
{
	int len = 0;
	if (value)
	{
		len = strlen(value);
	}
	char *data = new char[len + 1];
	for (int i = 0; i < len; ++i)
	{
		data[i] = (char)value[i];
	}
	data[len] = 0;
	return data;
}

char *coco_string_create_char(const char *value)
{
	int len = coco_string_length(value);
	char *res = new char[len + 1];
	for (int i = 0; i < len; ++i)
	{
		res[i] = (char)value[i];
	}
	res[len] = 0;
	return res;
}

void coco_string_delete(char *&data)
{
	delete[] data;
	data = NULL;
}

Token::Token()
{
	kind = 0;
	pos = 0;
	col = 0;
	line = 0;
	val = NULL;
	next = NULL;
}

Token::~Token()
{
	coco_string_delete(val);
}

Buffer::Buffer(FILE *s, bool isUserStream)
{
// ensure binary read on windows
#if _MSC_VER >= 1300
	_setmode(_fileno(s), _O_BINARY);
#endif
	stream = s;
	this->isUserStream = isUserStream;
	if (CanSeek())
	{
		fseek(s, 0, SEEK_END);
		fileLen = ftell(s);
		fseek(s, 0, SEEK_SET);
		bufLen = (fileLen < COCO_MAX_BUFFER_LENGTH) ? fileLen : COCO_MAX_BUFFER_LENGTH;
		bufStart = INT_MAX; // nothing in the buffer so far
	}
	else
	{
		fileLen = bufLen = bufStart = 0;
	}
	bufCapacity = (bufLen > 0) ? bufLen : COCO_MIN_BUFFER_LENGTH;
	buf = new unsigned char[bufCapacity];
	if (fileLen > 0)
		SetPos(0); // setup  buffer to position 0 (start)
	else
		bufPos = 0; // index 0 is already after the file, thus Pos = 0 is invalid
	if (bufLen == fileLen && CanSeek())
		Close();
}

Buffer::Buffer(Buffer *b)
{
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

Buffer::Buffer(const unsigned char *buf, int len)
{
	this->buf = new unsigned char[len];
	memcpy(this->buf, buf, len * sizeof(unsigned char));
	bufStart = 0;
	bufCapacity = bufLen = len;
	fileLen = len;
	bufPos = 0;
	stream = NULL;
}

Buffer::~Buffer()
{
	Close();
	if (buf != NULL)
	{
		delete[] buf;
		buf = NULL;
	}
}

void Buffer::Close()
{
	if (!isUserStream && stream != NULL)
	{
		fclose(stream);
		stream = NULL;
	}
}

int Buffer::Read()
{
	if (bufPos < bufLen)
	{
		return buf[bufPos++];
	}
	else if (GetPos() < fileLen)
	{
		SetPos(GetPos()); // shift buffer start to Pos
		return buf[bufPos++];
	}
	else if ((stream != NULL) && !CanSeek() && (ReadNextStreamChunk() > 0))
	{
		return buf[bufPos++];
	}
	else
	{
		return EoF;
	}
}

int Buffer::Peek()
{
	int curPos = GetPos();
	int ch = Read();
	SetPos(curPos);
	return ch;
}

// beg .. begin, zero-based, inclusive, in byte
// end .. end, zero-based, exclusive, in byte
char *Buffer::GetString(int beg, int end)
{
	int len = 0;
	char *buf = new char[end - beg];
	int oldPos = GetPos();
	SetPos(beg);
	while (GetPos() < end)
		buf[len++] = (char)Read();
	SetPos(oldPos);
	char *res = coco_string_create(buf, 0, len);
	coco_string_delete(buf);
	return res;
}

int Buffer::GetPos()
{
	return bufPos + bufStart;
}

void Buffer::SetPos(int value)
{
	if ((value >= fileLen) && (stream != NULL) && !CanSeek())
	{
		// Wanted position is after buffer and the stream
		// is not seek-able e.g. network or console,
		// thus we have to read the stream manually till
		// the wanted position is in sight.
		while ((value >= fileLen) && (ReadNextStreamChunk() > 0))
			;
	}

	if ((value < 0) || (value > fileLen))
	{
		wprintf(L"--- buffer out of bounds access, position: %d\n", value);
		exit(1);
	}

	if ((value >= bufStart) && (value < (bufStart + bufLen)))
	{ // already in buffer
		bufPos = value - bufStart;
	}
	else if (stream != NULL)
	{ // must be swapped in
		fseek(stream, value, SEEK_SET);
		bufLen = fread(buf, sizeof(unsigned char), bufCapacity, stream);
		bufStart = value;
		bufPos = 0;
	}
	else
	{
		bufPos = fileLen - bufStart; // make Pos return fileLen
	}
}

// Read the next chunk of bytes from the stream, increases the buffer
// if needed and updates the fields fileLen and bufLen.
// Returns the number of bytes read.
int Buffer::ReadNextStreamChunk()
{
	int free = bufCapacity - bufLen;
	if (free == 0)
	{
		// in the case of a growing input stream
		// we can neither seek in the stream, nor can we
		// foresee the maximum length, thus we must adapt
		// the buffer size on demand.
		bufCapacity = bufLen * 2;
		unsigned char *newBuf = new unsigned char[bufCapacity];
		memcpy(newBuf, buf, bufLen * sizeof(unsigned char));
		delete[] buf;
		buf = newBuf;
		free = bufLen;
	}
	int read = fread(buf + bufLen, sizeof(unsigned char), free, stream);
	if (read > 0)
	{
		fileLen = bufLen = (bufLen + read);
		return read;
	}
	// end of stream reached
	return 0;
}

bool Buffer::CanSeek()
{
	return (stream != NULL) && (ftell(stream) != -1);
}

int UTF8Buffer::Read()
{
	int ch;
	do
	{
		ch = Buffer::Read();
		// until we find a utf8 start (0xxxxxxx or 11xxxxxx)
	} while ((ch >= 128) && ((ch & 0xC0) != 0xC0) && (ch != EoF));
	if (ch < 128 || ch == EoF)
	{
		// nothing to do, first 127 chars are the same in ascii and utf8
		// 0xxxxxxx or end of file character
	}
	else if ((ch & 0xF0) == 0xF0)
	{
		// 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
		int c1 = ch & 0x07;
		ch = Buffer::Read();
		int c2 = ch & 0x3F;
		ch = Buffer::Read();
		int c3 = ch & 0x3F;
		ch = Buffer::Read();
		int c4 = ch & 0x3F;
		ch = (((((c1 << 6) | c2) << 6) | c3) << 6) | c4;
	}
	else if ((ch & 0xE0) == 0xE0)
	{
		// 1110xxxx 10xxxxxx 10xxxxxx
		int c1 = ch & 0x0F;
		ch = Buffer::Read();
		int c2 = ch & 0x3F;
		ch = Buffer::Read();
		int c3 = ch & 0x3F;
		ch = (((c1 << 6) | c2) << 6) | c3;
	}
	else if ((ch & 0xC0) == 0xC0)
	{
		// 110xxxxx 10xxxxxx
		int c1 = ch & 0x1F;
		ch = Buffer::Read();
		int c2 = ch & 0x3F;
		ch = (c1 << 6) | c2;
	}
	return ch;
}

Scanner::Scanner(const unsigned char *buf, int len)
{
	buffer = new Buffer(buf, len);
	Init();
}

Scanner::Scanner(const char *fileName)
{
	FILE *stream;
	char *chFileName = coco_string_create_char(fileName);
	if ((stream = fopen(chFileName, "rb")) == NULL)
	{
		printf("--- Cannot open file %s\n", fileName);
		exit(1);
	}
	coco_string_delete(chFileName);
	buffer = new Buffer(stream, false);
	Init();
}

Scanner::Scanner(FILE *s)
{
	buffer = new Buffer(s, true);
	Init();
}

Scanner::~Scanner()
{
	char *cur = (char *)firstHeap;

	while (cur != NULL)
	{
		cur = *(char **)(cur + COCO_HEAP_BLOCK_SIZE);
		free(firstHeap);
		firstHeap = cur;
	}
	delete[] tval;
	delete buffer;
}

void Scanner::Init()
{
	EOL = '\n';
	eofSym = 0;
	maxT = 247;
	noSym = 247;
	int i;
	for (i = 65; i <= 90; ++i)
		start.set(i, 1);
	for (i = 95; i <= 95; ++i)
		start.set(i, 1);
	for (i = 97; i <= 122; ++i)
		start.set(i, 1);
	for (i = 48; i <= 57; ++i)
		start.set(i, 2);
	start.set(35, 3);
	start.set(40, 12);
	start.set(44, 13);
	start.set(41, 14);
	start.set(61, 15);
	start.set(59, 16);
	start.set(45, 17);
	start.set(10, 18);
	start.set(Buffer::EoF, -1);
	keywords.set("xor", 10);
	keywords.set("xoR", 11);
	keywords.set("xOr", 12);
	keywords.set("xOR", 13);
	keywords.set("Xor", 14);
	keywords.set("XoR", 15);
	keywords.set("XOr", 16);
	keywords.set("XOR", 17);
	keywords.set("ite", 18);
	keywords.set("itE", 19);
	keywords.set("iTe", 20);
	keywords.set("iTE", 21);
	keywords.set("Ite", 22);
	keywords.set("ItE", 23);
	keywords.set("ITe", 24);
	keywords.set("ITE", 25);
	keywords.set("and", 26);
	keywords.set("anD", 27);
	keywords.set("aNd", 28);
	keywords.set("aND", 29);
	keywords.set("And", 30);
	keywords.set("AnD", 31);
	keywords.set("ANd", 32);
	keywords.set("AND", 33);
	keywords.set("or", 34);
	keywords.set("oR", 35);
	keywords.set("Or", 36);
	keywords.set("OR", 37);
	keywords.set("output", 38);
	keywords.set("outpuT", 39);
	keywords.set("outpUt", 40);
	keywords.set("outpUT", 41);
	keywords.set("outPut", 42);
	keywords.set("outPuT", 43);
	keywords.set("outPUt", 44);
	keywords.set("outPUT", 45);
	keywords.set("ouTput", 46);
	keywords.set("ouTpuT", 47);
	keywords.set("ouTpUt", 48);
	keywords.set("ouTpUT", 49);
	keywords.set("ouTPut", 50);
	keywords.set("ouTPuT", 51);
	keywords.set("ouTPUt", 52);
	keywords.set("ouTPUT", 53);
	keywords.set("oUtput", 54);
	keywords.set("oUtpuT", 55);
	keywords.set("oUtpUt", 56);
	keywords.set("oUtpUT", 57);
	keywords.set("oUtPut", 58);
	keywords.set("oUtPuT", 59);
	keywords.set("oUtPUt", 60);
	keywords.set("oUtPUT", 61);
	keywords.set("oUTput", 62);
	keywords.set("oUTpuT", 63);
	keywords.set("oUTpUt", 64);
	keywords.set("oUTpUT", 65);
	keywords.set("oUTPut", 66);
	keywords.set("oUTPuT", 67);
	keywords.set("oUTPUt", 68);
	keywords.set("oUTPUT", 69);
	keywords.set("Output", 70);
	keywords.set("OutpuT", 71);
	keywords.set("OutpUt", 72);
	keywords.set("OutpUT", 73);
	keywords.set("OutPut", 74);
	keywords.set("OutPuT", 75);
	keywords.set("OutPUt", 76);
	keywords.set("OutPUT", 77);
	keywords.set("OuTput", 78);
	keywords.set("OuTpuT", 79);
	keywords.set("OuTpUt", 80);
	keywords.set("OuTpUT", 81);
	keywords.set("OuTPut", 82);
	keywords.set("OuTPuT", 83);
	keywords.set("OuTPUt", 84);
	keywords.set("OuTPUT", 85);
	keywords.set("OUtput", 86);
	keywords.set("OUtpuT", 87);
	keywords.set("OUtpUt", 88);
	keywords.set("OUtpUT", 89);
	keywords.set("OUtPut", 90);
	keywords.set("OUtPuT", 91);
	keywords.set("OUtPUt", 92);
	keywords.set("OUtPUT", 93);
	keywords.set("OUTput", 94);
	keywords.set("OUTpuT", 95);
	keywords.set("OUTpUt", 96);
	keywords.set("OUTpUT", 97);
	keywords.set("OUTPut", 98);
	keywords.set("OUTPuT", 99);
	keywords.set("OUTPUt", 100);
	keywords.set("OUTPUT", 101);
	keywords.set("free", 102);
	keywords.set("freE", 103);
	keywords.set("frEe", 104);
	keywords.set("frEE", 105);
	keywords.set("fRee", 106);
	keywords.set("fReE", 107);
	keywords.set("fREe", 108);
	keywords.set("fREE", 109);
	keywords.set("Free", 110);
	keywords.set("FreE", 111);
	keywords.set("FrEe", 112);
	keywords.set("FrEE", 113);
	keywords.set("FRee", 114);
	keywords.set("FReE", 115);
	keywords.set("FREe", 116);
	keywords.set("FREE", 117);
	keywords.set("exists", 118);
	keywords.set("existS", 119);
	keywords.set("exisTs", 120);
	keywords.set("exisTS", 121);
	keywords.set("exiSts", 122);
	keywords.set("exiStS", 123);
	keywords.set("exiSTs", 124);
	keywords.set("exiSTS", 125);
	keywords.set("exIsts", 126);
	keywords.set("exIstS", 127);
	keywords.set("exIsTs", 128);
	keywords.set("exIsTS", 129);
	keywords.set("exISts", 130);
	keywords.set("exIStS", 131);
	keywords.set("exISTs", 132);
	keywords.set("exISTS", 133);
	keywords.set("eXists", 134);
	keywords.set("eXistS", 135);
	keywords.set("eXisTs", 136);
	keywords.set("eXisTS", 137);
	keywords.set("eXiSts", 138);
	keywords.set("eXiStS", 139);
	keywords.set("eXiSTs", 140);
	keywords.set("eXiSTS", 141);
	keywords.set("eXIsts", 142);
	keywords.set("eXIstS", 143);
	keywords.set("eXIsTs", 144);
	keywords.set("eXIsTS", 145);
	keywords.set("eXISts", 146);
	keywords.set("eXIStS", 147);
	keywords.set("eXISTs", 148);
	keywords.set("eXISTS", 149);
	keywords.set("Exists", 150);
	keywords.set("ExistS", 151);
	keywords.set("ExisTs", 152);
	keywords.set("ExisTS", 153);
	keywords.set("ExiSts", 154);
	keywords.set("ExiStS", 155);
	keywords.set("ExiSTs", 156);
	keywords.set("ExiSTS", 157);
	keywords.set("ExIsts", 158);
	keywords.set("ExIstS", 159);
	keywords.set("ExIsTs", 160);
	keywords.set("ExIsTS", 161);
	keywords.set("ExISts", 162);
	keywords.set("ExIStS", 163);
	keywords.set("ExISTs", 164);
	keywords.set("ExISTS", 165);
	keywords.set("EXists", 166);
	keywords.set("EXistS", 167);
	keywords.set("EXisTs", 168);
	keywords.set("EXisTS", 169);
	keywords.set("EXiSts", 170);
	keywords.set("EXiStS", 171);
	keywords.set("EXiSTs", 172);
	keywords.set("EXiSTS", 173);
	keywords.set("EXIsts", 174);
	keywords.set("EXIstS", 175);
	keywords.set("EXIsTs", 176);
	keywords.set("EXIsTS", 177);
	keywords.set("EXISts", 178);
	keywords.set("EXIStS", 179);
	keywords.set("EXISTs", 180);
	keywords.set("EXISTS", 181);
	keywords.set("forall", 182);
	keywords.set("foralL", 183);
	keywords.set("foraLl", 184);
	keywords.set("foraLL", 185);
	keywords.set("forAll", 186);
	keywords.set("forAlL", 187);
	keywords.set("forALl", 188);
	keywords.set("forALL", 189);
	keywords.set("foRall", 190);
	keywords.set("foRalL", 191);
	keywords.set("foRaLl", 192);
	keywords.set("foRaLL", 193);
	keywords.set("foRAll", 194);
	keywords.set("foRAlL", 195);
	keywords.set("foRALl", 196);
	keywords.set("foRALL", 197);
	keywords.set("fOrall", 198);
	keywords.set("fOralL", 199);
	keywords.set("fOraLl", 200);
	keywords.set("fOraLL", 201);
	keywords.set("fOrAll", 202);
	keywords.set("fOrAlL", 203);
	keywords.set("fOrALl", 204);
	keywords.set("fOrALL", 205);
	keywords.set("fORall", 206);
	keywords.set("fORalL", 207);
	keywords.set("fORaLl", 208);
	keywords.set("fORaLL", 209);
	keywords.set("fORAll", 210);
	keywords.set("fORAlL", 211);
	keywords.set("fORALl", 212);
	keywords.set("fORALL", 213);
	keywords.set("Forall", 214);
	keywords.set("ForalL", 215);
	keywords.set("ForaLl", 216);
	keywords.set("ForaLL", 217);
	keywords.set("ForAll", 218);
	keywords.set("ForAlL", 219);
	keywords.set("ForALl", 220);
	keywords.set("ForALL", 221);
	keywords.set("FoRall", 222);
	keywords.set("FoRalL", 223);
	keywords.set("FoRaLl", 224);
	keywords.set("FoRaLL", 225);
	keywords.set("FoRAll", 226);
	keywords.set("FoRAlL", 227);
	keywords.set("FoRALl", 228);
	keywords.set("FoRALL", 229);
	keywords.set("FOrall", 230);
	keywords.set("FOralL", 231);
	keywords.set("FOraLl", 232);
	keywords.set("FOraLL", 233);
	keywords.set("FOrAll", 234);
	keywords.set("FOrAlL", 235);
	keywords.set("FOrALl", 236);
	keywords.set("FOrALL", 237);
	keywords.set("FORall", 238);
	keywords.set("FORalL", 239);
	keywords.set("FORaLl", 240);
	keywords.set("FORaLL", 241);
	keywords.set("FORAll", 242);
	keywords.set("FORAlL", 243);
	keywords.set("FORALl", 244);
	keywords.set("FORALL", 245);

	tvalLength = 128;
	tval = new char[tvalLength]; // text of current token

	// COCO_HEAP_BLOCK_SIZE byte heap + pointer to next heap block
	heap = malloc(COCO_HEAP_BLOCK_SIZE + sizeof(void *));
	firstHeap = heap;
	heapEnd = (void **)(((char *)heap) + COCO_HEAP_BLOCK_SIZE);
	*heapEnd = 0;
	heapTop = heap;
	if (sizeof(Token) > COCO_HEAP_BLOCK_SIZE)
	{
		printf("--- Too small COCO_HEAP_BLOCK_SIZE\n");
		exit(1);
	}

	pos = -1;
	line = 1;
	col = 0;
	charPos = -1;
	oldEols = 0;
	NextCh();
	if (ch == 0xEF)
	{ // check optional byte order mark for UTF-8
		NextCh();
		int ch1 = ch;
		NextCh();
		int ch2 = ch;
		if (ch1 != 0xBB || ch2 != 0xBF)
		{
			printf("Illegal byte order mark at start of file");
			exit(1);
		}
		Buffer *oldBuf = buffer;
		buffer = new UTF8Buffer(buffer);
		col = 0;
		charPos = -1;
		delete oldBuf;
		oldBuf = NULL;
		NextCh();
	}

	pt = tokens = CreateToken(); // first token is a dummy
}

void Scanner::NextCh()
{
	if (oldEols > 0)
	{
		ch = EOL;
		oldEols--;
	}
	else
	{
		pos = buffer->GetPos();
		// buffer reads unicode chars, if UTF8 has been detected
		ch = buffer->Read();
		col++;
		charPos++;
		// replace isolated '\r' by '\n' in order to make
		// eol handling uniform across Windows, Unix and Mac
		if (ch == '\r' && buffer->Peek() != '\n')
			ch = EOL;
		if (ch == EOL)
		{
			line++;
			col = 0;
		}
	}
}

void Scanner::AddCh()
{
	if (tlen >= tvalLength)
	{
		tvalLength *= 2;
		char *newBuf = new char[tvalLength];
		memcpy(newBuf, tval, tlen * sizeof(char));
		delete[] tval;
		tval = newBuf;
	}
	if (ch != Buffer::EoF)
	{
		tval[tlen++] = ch;
		NextCh();
	}
}

bool Scanner::Comment0()
{
	int level = 1, pos0 = pos, line0 = line, col0 = col, charPos0 = charPos;
	NextCh();
	if (ch == 'Q')
	{
		NextCh();
		if (ch == 'C')
		{
			NextCh();
			if (ch == 'I')
			{
				NextCh();
				if (ch == 'R')
				{
					NextCh();
					if (ch == '-')
					{
						NextCh();
						if (ch == 'G')
						{
							NextCh();
							if (ch == '1')
							{
								NextCh();
								if (ch == '4')
								{
									buffer->SetPos(pos0);
									NextCh();
									line = line0;
									col = col0;
									charPos = charPos0;
									return false;
								}
							}
						}
					}
				}
			}
		}
	}
	for (;;)
	{
		if (ch == 10)
		{
			std::wcout << "-- SyntaxError: line " << line0 << ": comments are not allowed" << std::endl;
			errors_count++;
			level--;
			if (level == 0)
			{
				oldEols = line - line0;
				NextCh();
				return true;
			}
			NextCh();
		}
		else if (ch == buffer->EoF)
			return false;
		else
			NextCh();
	}
}

void Scanner::CreateHeapBlock()
{
	void *newHeap;
	char *cur = (char *)firstHeap;

	while (((char *)tokens < cur) || ((char *)tokens > (cur + COCO_HEAP_BLOCK_SIZE)))
	{
		cur = *((char **)(cur + COCO_HEAP_BLOCK_SIZE));
		free(firstHeap);
		firstHeap = cur;
	}

	// COCO_HEAP_BLOCK_SIZE byte heap + pointer to next heap block
	newHeap = malloc(COCO_HEAP_BLOCK_SIZE + sizeof(void *));
	*heapEnd = newHeap;
	heapEnd = (void **)(((char *)newHeap) + COCO_HEAP_BLOCK_SIZE);
	*heapEnd = 0;
	heap = newHeap;
	heapTop = heap;
}

Token *Scanner::CreateToken()
{
	Token *t;
	if (((char *)heapTop + (int)sizeof(Token)) >= (char *)heapEnd)
	{
		CreateHeapBlock();
	}
	t = (Token *)heapTop;
	heapTop = (void *)((char *)heapTop + sizeof(Token));
	t->val = NULL;
	t->next = NULL;
	return t;
}

void Scanner::AppendVal(Token *t)
{
	int reqMem = (tlen + 1) * sizeof(char);
	if (((char *)heapTop + reqMem) >= (char *)heapEnd)
	{
		if (reqMem > COCO_HEAP_BLOCK_SIZE)
		{
			printf("--- Too long token value\n");
			exit(1);
		}
		CreateHeapBlock();
	}
	t->val = (char *)heapTop;
	heapTop = (void *)((char *)heapTop + reqMem);

	strncpy(t->val, tval, tlen);
	t->val[tlen] = '\0';
}

Token *Scanner::NextToken()
{
	while (ch == ' ' ||
		   ch == 9 || ch == 13)
		NextCh();
	if ((ch == '#' && Comment0()))
		return NextToken();
	int recKind = noSym;
	int recEnd = pos;
	t = CreateToken();
	t->pos = pos;
	t->col = col;
	t->line = line;
	t->charPos = charPos;
	int state = start.state(ch);
	tlen = 0;
	AddCh();

	switch (state)
	{
	case -1:
	{
		t->kind = eofSym;
		break;
	} // NextCh already done
	case 0:
	{
	case_0:
		if (recKind != noSym)
		{
			tlen = recEnd - t->pos;
			SetScannerBehindT();
		}
		t->kind = recKind;
		break;
	} // NextCh already done
	case 1:
	case_1:
		recEnd = pos;
		recKind = 1;
		if ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'Z') || ch == '_' || (ch >= 'a' && ch <= 'z'))
		{
			AddCh();
			goto case_1;
		}
		else
		{
			t->kind = 1;
			char *literal = coco_string_create(tval, 0, tlen);
			t->kind = keywords.get(literal, t->kind);
			coco_string_delete(literal);
			break;
		}
	case 2:
	case_2:
		recEnd = pos;
		recKind = 2;
		if ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'Z') || ch == '_' || (ch >= 'a' && ch <= 'z'))
		{
			AddCh();
			goto case_2;
		}
		else
		{
			t->kind = 2;
			break;
		}
	case 3:
		if (ch == 'Q')
		{
			AddCh();
			goto case_4;
		}
		else
		{
			goto case_0;
		}
	case 4:
	case_4:
		if (ch == 'C')
		{
			AddCh();
			goto case_5;
		}
		else
		{
			goto case_0;
		}
	case 5:
	case_5:
		if (ch == 'I')
		{
			AddCh();
			goto case_6;
		}
		else
		{
			goto case_0;
		}
	case 6:
	case_6:
		if (ch == 'R')
		{
			AddCh();
			goto case_7;
		}
		else
		{
			goto case_0;
		}
	case 7:
	case_7:
		if (ch == '-')
		{
			AddCh();
			goto case_8;
		}
		else
		{
			goto case_0;
		}
	case 8:
	case_8:
		if (ch == 'G')
		{
			AddCh();
			goto case_9;
		}
		else
		{
			goto case_0;
		}
	case 9:
	case_9:
		if (ch == '1')
		{
			AddCh();
			goto case_10;
		}
		else
		{
			goto case_0;
		}
	case 10:
	case_10:
		if (ch == '4')
		{
			AddCh();
			goto case_11;
		}
		else
		{
			goto case_0;
		}
	case 11:
	case_11:
	{
		t->kind = 3;
		break;
	}
	case 12:
	{
		t->kind = 4;
		break;
	}
	case 13:
	{
		t->kind = 5;
		break;
	}
	case 14:
	{
		t->kind = 6;
		break;
	}
	case 15:
	{
		t->kind = 7;
		break;
	}
	case 16:
	{
		t->kind = 8;
		break;
	}
	case 17:
	{
		t->kind = 9;
		break;
	}
	case 18:
	{
		t->kind = 246;
		break;
	}
	}
	AppendVal(t);
	return t;
}

void Scanner::SetScannerBehindT()
{
	buffer->SetPos(t->pos);
	NextCh();
	line = t->line;
	col = t->col;
	charPos = t->charPos;
	for (int i = 0; i < tlen; i++)
		NextCh();
}

// get the next token (possibly a token already seen during peeking)
Token *Scanner::Scan()
{
	if (tokens->next == NULL)
	{
		return pt = tokens = NextToken();
	}
	else
	{
		pt = tokens = tokens->next;
		return tokens;
	}
}

// peek for the next token, ignore pragmas
Token *Scanner::Peek()
{
	do
	{
		if (pt->next == NULL)
		{
			pt->next = NextToken();
		}
		pt = pt->next;
	} while (pt->kind > maxT); // skip pragmas

	return pt;
}

// make sure that peeking starts at the current scan position
void Scanner::ResetPeek()
{
	pt = tokens;
}
