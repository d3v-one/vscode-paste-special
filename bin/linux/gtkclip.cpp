// https://developer.gnome.org/gtk3/stable/gtk3-Clipboards.html
// https://developer.gnome.org/gtk3/stable/gtk-compiling.html
// sudo apt-get install libgtk-3-dev

#include <gtk/gtk.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <sstream>
#include <iomanip>


std::map<std::string, std::string> supportedFormats = {
	{"UTF8_STRING", "Plain Text"},
	{"text/html", "HTML source"},
	{"text/rtf", "RTF Text"},
	{"text/uri-list", "URI List"},
	{"image/png", "PNG image (as base64 endoded data url)"},
	{"image/jpeg", "JPG image (as base64 endoded data url)"},
	{"image/tiff", "TIFF image (as base64 endoded data url)"},
};


static void EmitError(int code, std::string message)  
{
	printf("{\n");
	printf("\t\"format\": \"none\",\n");
	printf("\t\"errnr\": %d,\n", code);
	printf("\t\"error\": \"%s\"\n", message.c_str());
	printf("}\n");
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


static void CreateResponse(const std::string& format, const std::string text)
{
	std::string json_text = escape_json(text);
	printf("{\n");
	printf("\t\"format\": \"%s\",\n", format.c_str());
	printf("\t\"data\": \"%s\"\n", json_text.c_str());
	printf("}\n");
}
 

static void EnumAllFormats(GtkClipboard *clipboard)
{
	GdkAtom* targets;
	gint n_targets = 0;
	if (gtk_clipboard_wait_for_targets(clipboard, &targets, &n_targets)) {
		for (gint i = 0; i < n_targets; i++) {
			gchar *name = gdk_atom_name(targets[i]);
			printf("%d: %s\n", i, name);
			g_free(name);
		}
		g_free(targets);
	}
}


static void EnumFormats(GtkClipboard *clipboard)
{
	std::vector<std::string> list;

	GdkAtom* targets;
	gint n_targets = 0;
	if (gtk_clipboard_wait_for_targets(clipboard, &targets, &n_targets)) {
		for (gint i = 0; i < n_targets; i++) {
			gchar *name = gdk_atom_name(targets[i]);
			if (supportedFormats.count(name) > 0) {
				list.push_back(name);
			}
			g_free(name);
		}
		g_free(targets);
	}

	std::sort(list.begin(), list.end()); 

	int count = 0;
	printf("[\n");
	for(auto const& value: list) {
		if (count++ > 0) printf("\t,{\n"); else printf("\t{\n");
		printf("\t\t\"format\": \"%s\",\n", value.c_str());
		printf("\t\t\"name\": \"%s\"\n", supportedFormats[value].c_str());
		printf("\t}\n");
	}	
	printf("]\n");
}


static void GetData(GtkClipboard *clipboard, const std::string format)
{
	GdkAtom target = gdk_atom_intern(format.c_str(), TRUE);
	GtkSelectionData *selection_data = gtk_clipboard_wait_for_contents(clipboard, target);
	if (selection_data) {
		if (format == "UTF8_STRING") {
			guchar *text = gtk_selection_data_get_text(selection_data);     
			if (text) {
				CreateResponse(format, (char*)text);
				g_free(text);
			} else {
				EmitError(-3, "no text data found for format");
			}
		}	else if (
				format == "text/html" || 
				format == "text/rtf" ||
				format == "text/uri-list"
			) {
			const guchar *text = gtk_selection_data_get_data(selection_data);     
			if (text) {
				CreateResponse(format, (char*)text);
			} else {
				EmitError(-3, "no raw data found for format");
			}
		}	else if (
				format == "image/png" ||
				format == "image/jpeg" ||
				format == "image/tiff"
			) {
			const guchar *data = gtk_selection_data_get_data(selection_data);     
			gint length = gtk_selection_data_get_length(selection_data);
			std::string base64 = base64_encode(data, length);
			if (data) {
				CreateResponse(format, "data:" + format + ";base64," + base64);
			} else {
				EmitError(-3, "no raw data found for format");
			}
		}
		gtk_selection_data_free(selection_data);
	} else {
		EmitError(-2, "no data found for format");
	}
}


int main(int argc, char **argv)
{
	if (gtk_init_check(NULL, NULL) == FALSE) {
		return 1;
	}
	GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);

	if (argc > 1) {
		if (argv[1][0] == '*') {
			EnumAllFormats(clipboard);
		} else {
			GetData(clipboard, argv[1]);
		}
	} else {
		EnumFormats(clipboard);
	}

	return 0;
}
