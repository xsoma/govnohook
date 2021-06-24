#pragma once
#include <features/inventory/vdf_parser.hpp>
#include <sdk/protect/optional.hpp>
#include <sdk/sdk.hpp>

class Color;

class item_t
{
public:
	uint16_t id = 0;
	std::string item_name;
	std::wstring translated_name;
};

template <typename T = item_t>
class item_list_t
{
public:
	std::vector<T> list;

	virtual std::string to_string() = 0;
};

class rarity_t
{
public:
	int32_t id = 0;
	std::string name;
	std::string Color_name;
	Color Color;
};

class rarity_list_t : public item_list_t<rarity_t>
{
public:

	rarity_t get_by_id(int id)
	{
		for (auto& rarity : list)
		{
			if (rarity.id == id) {
				return rarity;
			}
		}

		return list[0];
	}

	std::string to_string() override { return ""; }
};

class skin_t : public item_t
{
public:

	int32_t rarity = 0;
};

class skin_list_t : public item_list_t<skin_t>
{
private:
	std::string string_result;

public:

	tl::optional<skin_t&> get_by_id(uint16_t id)
	{
		for (auto& skin : list) {
			if (skin.id == id) {
				return skin;
			}
		}

		return tl::nullopt;
	}

	tl::optional<skin_t&> get_by_item_name(std::string name)
	{
		for (auto& skin : list) {
			if (skin.item_name == name) {
				return skin;
			}
		}

		return tl::nullopt;
	}

	std::string to_string() override
	{
		if (string_result.empty())
		{
			for (auto& skin : list)
			{
				std::string str(skin.translated_name.begin(), skin.translated_name.end());

				string_result += str;
				string_result.append("\0");
			}
			string_result.append("\0");
		}

		return string_result;
	}
};

class weapon_t : public item_t
{
public:
	std::string prefab;
	skin_list_t skins;
};

class weapon_list_t : public item_list_t<weapon_t>
{
private:
	std::string string_result;

public:

	tl::optional<weapon_t&> get_by_id(uint16_t id)
	{
		for (auto& weapon : list) {
			if (weapon.id == id) {
				return weapon;
			}
		}

		return tl::nullopt;
	}

	weapon_t* idi_nahui(uint16_t id)
	{
		for (auto& weapon : list) {
			if (weapon.id == id) {
				return &weapon;
			}
		}

		return 0;
	}

	tl::optional<weapon_t&> get_by_item_name(std::string name)
	{
		for (auto& weapon : list) {
			if (weapon.item_name == name) {
				return weapon;
			}
		}

		return tl::nullopt;
	}

	std::string to_string() override
	{
		if (string_result.empty())
		{
			string_result = "";

			for (auto& weapon : list)
			{
				std::string str(weapon.translated_name.begin(), weapon.translated_name.end());

				string_result += str;
				string_result.append("\0");
			}
			string_result.append("\0");
		}

		return string_result;
	}
};

class knife_t : public item_t
{
public:
	int model_player;
	int model_world;
	std::string	model_player_path;
	std::string  model_world_path;
};

class knife_list_t : public item_list_t<knife_t>
{
private:
	std::string string_result;

public:

	tl::optional<knife_t&> get_by_id(uint16_t id)
	{
		for (auto& knife : list) {
			if (knife.id == id) {
				return knife;
			}
		}

		return tl::nullopt;
	}

	std::string to_string() override
	{
		if (string_result.empty())
		{
			for (auto& knife : list)
			{
				std::string str(knife.translated_name.begin(), knife.translated_name.end());

				string_result += str;
				string_result.append("\0");
			}
			string_result.append("\0");
		}

		return string_result;
	}
};
class agents_t
{
public:
	uint16_t id = 0;
	std::string item_name;
	std::wstring translated_name;
	std::string rarity = 0;
	int model_player;
	std::string	model_player_path;
};
class agents_list_t : public item_list_t<agents_t>
{
private:
	std::string string_result;

public:
	std::string to_string() override
	{
		if (string_result.empty())
		{
			for (auto& knife : list)
			{
				std::string str(knife.translated_name.begin(), knife.translated_name.end());

				string_result += str;
				string_result.append("\0");
			}
			string_result.append("\0");
		}

		return string_result;
	}
};

namespace parser
{
	extern bool starts_with(std::string str, std::string v);
	extern bool ends_with(std::string str, std::string v);

	extern std::string to_lower(std::string str);
	extern std::wstring to_wlower(std::wstring str);

	extern vdf::document document;
	extern weapon_list_t weapons;
	extern knife_list_t knifes;
	extern knife_list_t default_knifes;
	extern rarity_list_t rarities;
	extern agents_list_t agentes;
	extern std::vector<skin_t*> get_skins_by_name(std::string name);

	extern const wchar_t* translate(std::string token);

	extern bool parse_rarities();

	extern bool parse_weapons();

	extern bool parse_skins();

	extern bool parse_knifes();

	extern bool sort_weapons();

	extern bool parse();
}
struct eah
{
	int weapon_skin_id;
	int rarity;
};

inline std::map<int, eah> rarity_skins;
namespace valve_parser
{
	class Document;
	class KeyValue;
	class Object;

	enum ENCODING
	{
		UTF8,
		UTF16_LE,
		UTF16_BE,
		UTF32_LE,
		UTF32_BE,
		MAX
	};

	enum
	{
		STRING = '"',
		OBJECT_OPEN = '{',
		OBJECT_CLOSE = '}',
		COMMENT = '/',
	};


	class Str
	{
	public:
		char32_t* _start = 0;
		char32_t* _end = 0;
	public:
		std::string toString()
		{
			if (!_start || !_end)
				return "";

			//convert utf32 to utf16
			std::vector<char16_t> utf16;
			utf16.reserve(200);

			for (char32_t* text = _start; text != _end + 1; text++)
			{
				if ((*text >= 0 && *text <= 0xD7FF) ||
					*text >= 0xE000 && *text <= 0xFFFF)
				{
					utf16.push_back(*text);
				}
				if (*text >= 0x10000 && *text <= 0x10FFFF)
				{
					char32_t offset = *text - 0x10000;
					char16_t hi = (offset & 0xFFC00) >> 10;
					char16_t lo = offset & 0x3FF;
					hi += 0xD800;
					lo += 0xDC00;
					utf16.push_back(hi);
					utf16.push_back(lo);
				}
			}
			return std::string(utf16.begin(), utf16.end());
		}

		//warning: no exception handling
		int toInt()
		{
			return std::stoi(toString());
		}

		//checks whether the next read character equals expectedTag, if not 0 is returned
		static char32_t* ParseTextExpectedTag(char32_t* p, char32_t expectedTag, bool IGNORE_SPACE_TAB_LF_CR)
		{
			for (; *p; p++)
			{
				//skip comment line
				if (*p == COMMENT && *(p + 1) && *(p + 1) == COMMENT)
				{
					for (; *p && *p != '\n'; p++);
				}

				//skip options line
				// [§xbox|§360]
				if (*p == '[')
				{
					for (; *p && *p != ']'; p++);
					p++;
				}

				if (IGNORE_SPACE_TAB_LF_CR)
				{
					if (*p == '\n' || *p == '\r' || *p == '\t' || *p == ' ')
						continue;
				}

				if (*p == expectedTag)
					return p;
				else
					return 0;
			}

			return 0;
		}

		static char32_t* EndReached(char32_t* p, char32_t expectedTag)
		{
			for (; *p; p++)
			{
				//skip comment line
				if (*p == COMMENT && *(p + 1) && *(p + 1) == COMMENT)
				{
					for (; *p && *p != '\n'; p++);
				}

				//skip options line
				// [§xbox|§360]
				if (*p == '[')
				{
					for (; *p && *p != ']'; p++);
					p++;
				}

				if (*p == '\n' || *p == '\r' || *p == '\t' || *p == ' ')
					continue;

				if (*p == expectedTag)
					return p;
				else
					return (char32_t*)-1;
			}

			return 0;
		}

		//reads string until it ecnounters endTag
		static char32_t* ParseTextEndTag(char32_t* p, char32_t endTag)
		{
			for (; *p; p++)
			{
				//skip escape sequence
				//there are also hex escapes
				//\xAA45, no need to handle them tho
				if (*p == '\\')
					p += 2;

				if (*p == endTag)
					return p;
			}

			return 0;
		}
	};

	class Util
	{
	public:
		static bool StrEqu(Str str1, char* str2)
		{
			return str1.toString() == std::string(str2);
		}

		static bool StrEquI(Str str1, char* str2)
		{
			std::string str = str1.toString();
			return str.compare(str2) == std::string::npos;
		}
	};

	class Node
	{
	protected:
		Document* _doc;
	public:
		std::vector<std::shared_ptr<Node>> children;
		virtual bool Parse();

		Node(Document* doc)
		{
			_doc = doc;
		}

		virtual Object* ToObject()
		{
			return 0;
		}

		virtual KeyValue* ToKeyValue()
		{
			return 0;
		}
	};

	class KeyValue : public Node
	{
	public:
		Str Key;
		Str Value;

		KeyValue(Document* doc) : Node(doc) {}

		KeyValue* ToKeyValue()
		{
			return this;
		}
	};

	class Object : public Node
	{
	public:
		Str name;
		Object(Document* doc);
		Object* ToObject();
		bool Parse();
		std::shared_ptr<Node> GetKeyByName(char* name);
	};

	class Document
	{
	private:
		friend class Object;
		friend class Node;

		struct BOM
		{
			char* str;
			size_t len;
		};
		BOM BOMS[ENCODING::MAX];

		char32_t* utf32text = 0;
		char32_t* p;
	public:
		std::shared_ptr<Node> root;

		Document()
		{
			BOMS[UTF8] = { "\xEF\xBB\xBF", 3 };
			BOMS[UTF16_LE] = { "\xFF\xFE", 2 };
			BOMS[UTF16_BE] = { "\xFE\xFF", 2 };
			BOMS[UTF32_LE] = { "\xFF\xFE\x00\x00", 4 };
			BOMS[UTF32_BE] = { "\x00\x00\xFE\xFF", 4 };
		}

		~Document()
		{
			if (utf32text)
				delete[] utf32text;
		}

		std::shared_ptr<Node> BreadthFirstSearch(char* name, bool caseInsensitive = false)
		{
			std::deque<std::shared_ptr<Node>> q;
			q.push_back(root);
			while (!q.empty())
			{
				std::shared_ptr<Node> f = q.front();
				q.pop_front();

				if (f->ToKeyValue())
				{
					if (!caseInsensitive && Util::StrEqu(f->ToKeyValue()->Key, name))
						return f;

					if (caseInsensitive && Util::StrEquI(f->ToKeyValue()->Key, name))
						return f;
				}
				if (f->ToObject())
				{
					if (!caseInsensitive && Util::StrEqu(f->ToObject()->name, name))
						return f;

					if (caseInsensitive && Util::StrEquI(f->ToObject()->name, name))
						return f;
				}

				for (auto& child : f->children)
					q.push_back(child);
			}

			return 0;
		}

		std::vector<std::shared_ptr<Node>> BreadthFirstSearchMultiple(char* name, bool caseInsensitive = false)
		{
			std::vector<std::shared_ptr<Node>> vec;
			std::deque<std::shared_ptr<Node>> q;
			q.push_back(root);
			while (!q.empty())
			{
				std::shared_ptr<Node> f = q.front();
				q.pop_front();

				if (f->ToKeyValue())
				{
					if (!caseInsensitive && Util::StrEqu(f->ToKeyValue()->Key, name))
						vec.push_back(f);

					if (caseInsensitive && Util::StrEquI(f->ToKeyValue()->Key, name))
						vec.push_back(f);
				}
				if (f->ToObject())
				{
					if (!caseInsensitive && Util::StrEqu(f->ToObject()->name, name))
						vec.push_back(f);

					if (caseInsensitive && Util::StrEquI(f->ToObject()->name, name))
						vec.push_back(f);
				}

				for (auto& child : f->children)
					q.push_back(child);
			}

			return vec;
		}

		//returns true when format is correct
		//parse from file
		bool Load(char* path, ENCODING encoding)
		{
			auto f = fopen(path, "rb");
			if (!f)
				return false;

			fseek(f, 0, SEEK_END);
			auto size = ftell(f);
			if (!size)
				return false;

			size_t null_terminator_bytes = 0;
			if (encoding == ENCODING::UTF16_BE ||
				encoding == ENCODING::UTF16_LE)
				null_terminator_bytes = 2;

			if (encoding == ENCODING::UTF8)
				null_terminator_bytes = 1;

			if (encoding == ENCODING::UTF32_BE ||
				encoding == ENCODING::UTF32_LE)
				null_terminator_bytes = 4;

			char* _text = new char[size + null_terminator_bytes];
			fseek(f, 0, SEEK_SET);
			if (fread(_text, 1, size, f) != size)
				return false;

			for (int i = 0; i < null_terminator_bytes; i++)
				_text[size + i] = 0;

			fclose(f);

			//convert to utf32
			//which makes everything easy to handle
			//but increases buffer size
			cnv_to_utf32(_text, encoding);
			delete[] _text;

			p = trimSpace(utf32text);
			if (!p)
				return false;

			return ParseDeep();
		}
	private:
		char* skipBOM(char* p, ENCODING encoding)
		{
			if (!memcmp(p, BOMS[encoding].str, BOMS[encoding].len))
				return p + BOMS[encoding].len;
			else
				return p;
		}

		void cnv_to_utf32(char* text, ENCODING encoding)
		{
			if (encoding == ENCODING::UTF16_LE)
			{
				char* bom = skipBOM(text, ENCODING::UTF16_LE);
				cnv_utf16_le(bom);
			}

			if (encoding == ENCODING::UTF16_BE)
			{
				char* bom = skipBOM(text, ENCODING::UTF16_BE);
				cnv_utf16_be(bom);
			}

			if (encoding == ENCODING::UTF8)
			{
				char* bom = skipBOM(text, ENCODING::UTF8);
				cnv_utf8(bom);
			}

			if (encoding == ENCODING::UTF32_LE)
			{
				char* bom = skipBOM(text, ENCODING::UTF32_LE);
				cnv_utf32_le(bom);
			}

			if (encoding == ENCODING::UTF32_BE)
			{
				char* bom = skipBOM(text, ENCODING::UTF32_BE);
				cnv_utf32_be(bom);
			}
		}

		size_t cnv_utf16_be_len(char* text)
		{
			size_t len = 0;
			char16_t* u16text = (char16_t*)text;
			while (*u16text)
			{

				char16_t c = 0;
				char* t = (char*)u16text;
				c |= (char16_t)*(t + 1) << 8;
				c |= *t;

				//4 bytes
				if (c >= 0xD800 && c <= 0xDFFF)
					u16text++;

				len++;
				u16text++;
			}

			return len;
		}

		size_t cnv_utf16_le_len(char* text)
		{
			size_t len = 0;
			char16_t* u16text = (char16_t*)text;
			while (*u16text)
			{
				if (*u16text >= 0xD800 && *u16text <= 0xDFFF)
					u16text++;

				len++;
				u16text++;
			}

			return len;
		}

		void cnv_utf16_be(char* text)
		{
			auto s = cnv_utf16_be_len(text);
			utf32text = new char32_t[s + 1];
			utf32text[s] = 0;
			int i = 0;

			char16_t* u16text = (char16_t*)text;
			char32_t k;

			while (*u16text)
			{

				char16_t c = 0;
				char* t = (char*)u16text;
				c |= (char16_t)*(t + 1) << 8;
				c |= *t;

				//4 bytes
				if (c >= 0xD800 && c <= 0xDFFF)
				{
					char16_t hi = c - 0xD800;

					t = (char*)(u16text + 1);
					c |= (char16_t)*(t + 1) << 8;
					c |= *t;
					char16_t lo = c - 0xDC00;

					k = (char32_t)(hi & 0x3FF) << 10;
					k |= lo & 0x3FF;
					k += 0x10000;
					u16text++;
				}
				else //2 bytes
				{
					k = c;
				}
				utf32text[i] = k;
				i++;
				u16text++;
			}
		}

		void cnv_utf16_le(char* text)
		{
			auto s = cnv_utf16_le_len(text);
			utf32text = new char32_t[s + 1];
			utf32text[s] = 0;
			int i = 0;

			char16_t* u16text = (char16_t*)text;
			char32_t k;

			while (*u16text)
			{
				//4 bytes
				if (*u16text >= 0xD800 && *u16text <= 0xDFFF)
				{
					char16_t hi = *u16text - 0xD800;
					char16_t lo = *(u16text + 1) - 0xDC00;
					k = (char32_t)(hi & 0x3FF) << 10;
					k |= lo & 0x3FF;
					k += 0x10000;
					u16text++;
				}
				else //2 bytes
				{
					k = *u16text;
				}
				utf32text[i] = k;
				i++;
				u16text++;
			}
		}

		size_t cnv_utf8_len(char* text)
		{
			size_t len = 0;
			unsigned char c = (unsigned char)*text;

			while (c)
			{
				if (c >= 0xc0 && c <= 0xdf)
				{
					text++;
				}
				if (c >= 0xe0 && c <= 0xef)
				{
					text += 2;
				}
				if (c >= 0xf0 && c <= 0xf7)
				{
					text += 3;
				}

				len++;
				c = *++text;
			}

			return len;
		}

		void cnv_utf8(char* text)
		{
			auto s = cnv_utf8_len(text);
			utf32text = new char32_t[s + 1];
			utf32text[s] = 0;
			int i = 0;

			unsigned char c = (unsigned char)*text;

			while (c)
			{
				char32_t k = 0;
				if (c >= 0 && c <= 0x7f)
				{
					k = c;
				}
				if (c >= 0xc0 && c <= 0xdf)
				{
					k = (char32_t)(c ^ 0xc0) << 6;
					c = *++text;
					k |= c ^ 0x80;
				}
				if (c >= 0xe0 && c <= 0xef)
				{
					k = (char32_t)(c ^ 0xe0) << 12;
					c = *++text;
					k |= (char32_t)(c ^ 0x80) << 6;
					c = *++text;
					k |= c ^ 0x80;
				}
				if (c >= 0xf0 && c <= 0xf7)
				{
					k = (char32_t)(c ^ 0xf0) << 18;
					c = *++text;
					k |= (char32_t)(c ^ 0x80) << 12;
					c = *++text;
					k |= (char32_t)(c ^ 0x80) << 6;
					c = *++text;
					k |= c ^ 0x80;
				}
				utf32text[i] = k;
				i++;
				c = *++text;
			}
		}

		size_t cnv_utf32_len(char* text)
		{
			size_t len = 0;
			char32_t* p = (char32_t*)text;

			while (*p)
			{
				len++;
				p++;
			}

			return len;
		}

		void cnv_utf32_le(char* text)
		{
			auto s = cnv_utf32_len(text);
			utf32text = new char32_t[s + 1];
			utf32text[s] = 0;
			int i = 0;

			char32_t* p = (char32_t*)text;

			while (*p)
			{
				utf32text[i] = *p;
				i++;
				p++;
			}
		}

		void cnv_utf32_be(char* text)
		{
			auto s = cnv_utf32_len(text);
			utf32text = new char32_t[s + 1];
			utf32text[s] = 0;
			int i = 0;

			char32_t* p = (char32_t*)text;
			char32_t k;

			while (*p)
			{
				char* t = (char*)p;
				k = (char32_t)*(t + 3) << 24;
				k |= (char32_t)*(t + 2) << 16;
				k |= (char32_t)*(t + 1) << 8;
				k |= *t;

				utf32text[i] = k;
				i++;
				p++;
			}
		}

		char32_t* trimSpace(char32_t* p)
		{
			while (*p)
			{
				if (*p == ' ')
					p++;
				else
					return p;
			}

			return 0;
		}

		bool identify(std::shared_ptr<Node>& node)
		{
			auto string_begin = Str::ParseTextExpectedTag(p, STRING, true);
			if (!string_begin)
				return false;

			auto string_end = Str::ParseTextEndTag(string_begin + 1, STRING);
			if (!string_end)
				return false;


			auto object_open = Str::ParseTextExpectedTag(string_end + 1, OBJECT_OPEN, true);
			auto val_start = Str::ParseTextExpectedTag(string_end + 1, STRING, true);

			if (!object_open && !val_start)
				return false;

			if (object_open)
			{
				std::shared_ptr<Object> obj = std::make_shared<Object>(this);
				obj->name._start = string_begin + 1;
				obj->name._end = string_end - 1;
				node = obj;
				p = object_open + 1;
				return true;
			}

			if (val_start)
			{
				auto val_end = Str::ParseTextEndTag(val_start + 1, STRING);
				if (!val_end)
					return false;

				std::shared_ptr<KeyValue> keyVal = std::make_shared<KeyValue>(this);
				keyVal->Key._start = string_begin + 1;
				keyVal->Key._end = string_end - 1;
				keyVal->Value._start = val_start + 1;
				keyVal->Value._end = val_end - 1;
				node = keyVal;
				p = val_end + 1;
				return true;
			}
		}

		bool ParseDeep()
		{
			root = std::make_shared<Node>(this);
			return root->Parse();
		}
	};
}