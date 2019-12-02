#include <windows.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <sstream>
#include <iomanip>

#define CF_RTF	(49299)
#define CF_HTML (49444)
#define CF_CSV	(50166)

typedef struct {
 UINT format;
 char *name;
} FORMATDATA;

FORMATDATA formatlist[] = {
	{CF_TEXT, "ANSI text"},
	{CF_BITMAP, "Handle to a bitmap (GDI object)"},
	{CF_METAFILEPICT, "Windows-Format Metafiles picture"},
	{CF_SYLK, "Microsoft Symbolic Link"},
	{CF_DIF, "Software Arts Data Interchange Format"},
	{CF_TIFF, "TIFF image"},
	{CF_OEMTEXT, "8-Bit DOS text"},
	{CF_DIB, "Device Independent Bitmap"},
	{CF_PALETTE, "Handle to a color palette (GDI object)"},
	{CF_PENDATA, "Windows 3.1 pen extension data"},
	{CF_RIFF, "Resource Interchange File Format (RIFF) audio"},
	{CF_WAVE, "WAVE audio"},
	{CF_UNICODETEXT, "Unicode text"},
	{CF_ENHMETAFILE, "Enhanced-Format Metafiles handle"},
	{CF_HDROP, "List of file names"},
	{CF_LOCALE, "LCID for CF_TEXT to CF_UNICODE conversion"},
	{CF_DIBV5, "Structure followed by bitmap bits"},
	{CF_DSPTEXT, "ANSI text"},
	{CF_DSPBITMAP, "Handle to a bitmap (GDI object)"},
	{CF_DSPMETAFILEPICT, "Windows-Format Metafiles picture"},
	{CF_DSPENHMETAFILE, "Enhanced-Format Metafiles handle"},
	{0, NULL}
};


std::map<UINT, std::string> supportedFormats = {
	{CF_UNICODETEXT, "Plain Text"},
	{CF_HTML, "HTML source"},
	{CF_RTF, "Rich Text Format"},
	{CF_HDROP, "List of filenames"},
	{CF_CSV, "Commma-separated values"}
};


static void StopWithError(int code, std::string message)  
{
	printf("{\n");
	printf("\t\"format\": \"none\",\n");
	printf("\t\"errnr\": %d,\n", code);
	printf("\t\"error\": \"%s\"\n", message.c_str());
	printf("}\n");
	CloseClipboard();
	exit(1);
}


// base64 encode code by René Nyffenegger (rene.nyffenegger@adp-gmbh.ch)

static const std::string base64_chars = 
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"abcdefghijklmnopqrstuvwxyz"
	"0123456789+/";

std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len) 
{
  std::string ret;
  int i = 0;
  int j = 0;
  unsigned char char_array_3[3];
  unsigned char char_array_4[4];

  while (in_len--) {
    char_array_3[i++] = *(bytes_to_encode++);
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for(i = 0; i < 4; i++) {
        ret += base64_chars[char_array_4[i]];
			}

      i = 0;
    }
  }

  if (i) {
    for(j = i; j < 3; j++) {
      char_array_3[j] = '\0';
		}

    char_array_4[0] = ( char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

    for (j = 0; j < i + 1; j++) {
      ret += base64_chars[char_array_4[j]];
		}

    while (i++ < 3) {
      ret += '=';
		}

  }

  return ret;
}


static std::string escape_json(const std::string &s) 
{
	std::ostringstream o;
	for (auto c = s.cbegin(); c != s.cend(); c++) {
		if (*c == '"' || *c == '\\' || ('\x00' <= *c && *c <= '\x1f')) {
			o << "\\u" << std::hex << std::setw(4) << std::setfill('0') << (int)*c;
		} else {
			o << *c;
		}
	}
	return o.str();
}


static std::string unicode2utf8(const WCHAR *unicode)
{
	int utf8_size = WideCharToMultiByte(CP_UTF8, 0, unicode, -1, NULL, 0, NULL, NULL) + 1;
	char* utf8_str = (char*)malloc(utf8_size);
	WideCharToMultiByte(CP_UTF8, 0, unicode, -1, utf8_str, utf8_size, NULL, NULL);
	return utf8_str;
}


static char *GetFormatName(UINT format)
{
	for (int i=0; formatlist[i].format; i++) {
		if (format == formatlist[i].format) return formatlist[i].name;
	}

	static char name[128];
	if (GetClipboardFormatNameA(format, name, sizeof(name)) > 0) {
		return name;
	}

	return "unknown";
}


static void EnumAllFormats(void)
{
	UINT format = 0;
	while ((format = EnumClipboardFormats(format))) {
		char *name = GetFormatName(format);
		printf("%5u %s\n", format, name);
	}
}


static void EnumFormats(void)
{
	std::vector<UINT> list;
	UINT format = 0;
	while ((format = EnumClipboardFormats(format))) {
		if (supportedFormats.count(format) > 0) {
			list.push_back(format);
		}
	}

	std::sort(list.begin(), list.end()); 

	int count = 0;
	printf("[\n");
	for(auto const& value: list) {
		if (count++ > 0) printf("\t,{\n"); else printf("\t{\n");
		printf("\t\t\"format\": %d,\n", value);
		printf("\t\t\"name\": \"%s\"\n", supportedFormats[value].c_str());
		printf("\t}\n");
	}	
	printf("]\n");
}


static void CreateResponse(UINT format, const std::string& text)
{
	std::string json_text = escape_json(text);
	printf("{\n");
	printf("\t\"format\": %d,\n", format);
	printf("\t\"data\": \"%s\"\n", json_text.c_str());
	printf("}\n");
}


static std::string ExtractEntity(const std::string s)
{
	const std::string start_token = "StartFragment:";
	const std::string end_token = "EndFragment:";
	unsigned long int start = 0, end = 0;
	std::istringstream iss(s);
	std::string line;
	while (std::getline(iss, line)) {
		if (line.substr(0, start_token.length()) == start_token) {
			start = strtoul(line.substr(start_token.length()).c_str(), NULL, 10);
		}
		if (line.substr(0, end_token.length()) == end_token) {
			end = strtoul(line.substr(end_token.length()).c_str(), NULL, 10);
		}
	}
	if (start > 0 && end > 0) {
		return s.substr(start, end - start);
	}
	return s;
}


static void GetData(UINT format)
{
	HANDLE hData = GetClipboardData(format);
	if (hData == NULL) {
		StopWithError(-2, "no data found for format");
		return;
	}
	void *pData = GlobalLock(hData); 
	if (pData == NULL) {
		StopWithError(-3, "error retrieving data for format");
		return;
	}
	switch (format) {
		case CF_UNICODETEXT: {
				std::string text = unicode2utf8((WCHAR*)pData);
				CreateResponse(format, text);
			}
			break;
		case CF_HDROP: {
				HDROP hDrop =  (HDROP)pData;
				UINT fileCount = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
				WCHAR buffer[MAX_PATH];
				std::string filelist;
				for (UINT i = 0; i < fileCount; i++) {
					DragQueryFileW(hDrop, i, buffer, MAX_PATH);
					std::string filename = unicode2utf8(buffer);
					filelist += filename + "\r\n";
				}			
				CreateResponse(format, filelist);
			}
			break;
		case CF_HTML: {
				std::string text = (char*)pData;
				text = ExtractEntity(text);
				CreateResponse(format, text);
			}
			break;
		case CF_CSV: {
				std::string text = (char*)pData;
				CreateResponse(format, text);
			}
			break;
		case CF_RTF: {
				std::string text = (char*)pData;
				CreateResponse(format, text);
			}
			break;
		case 49948: {
				std::string text = unicode2utf8((WCHAR*)pData);
				CreateResponse(format, text);
			}
			break;
	}
	GlobalUnlock(hData); 
}



int main(int argc, char **argv)
{
	if (OpenClipboard(NULL) == FALSE) return 1;

	if (argc > 1) {
		if (argv[1][0] == '*') {
			EnumAllFormats();
		} else {
			char *endptr;
			UINT format = strtoul(argv[1], &endptr, 10);
			if (format == 0 && argv[1] == endptr) {
				StopWithError(-1, "parameter not a number");
				return 1;
			}
			GetData(format);
		}
	} else {
		EnumFormats();
	}

	CloseClipboard();
	return 0;
}
