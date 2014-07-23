/* 
 * OpenTyrian: A modern cross-platform port of Tyrian
 * Copyright (C) 2014  The OpenTyrian Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include "config_file.h"

#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

/* potential size of decimal representation of type */
#define udecsizeof(t) ((CHAR_BIT * sizeof(t) / 3) + 1)
#define sdecsizeof(t) (udecsizeof(t) + 1)

void config_oom( void )
{
	fprintf(stderr, "out of memory\n");
	exit(EXIT_FAILURE);
}

/* string manipulators */

string_t string_init_len( const char *s, size_t n )
{
	string_t string;
	
	if (s == NULL)
	{
		STRING_LONG_TAG(string) = true;
		
		string.long_buf = NULL;
	}
	else
	{
		char is_long = n >= COUNTOF(string.short_buf);
		
		STRING_LONG_TAG(string) = is_long;
		
		char *buffer = is_long ?
			string.long_buf = malloc((n + 1) * sizeof(char)) :
			string.short_buf;
		if (buffer == NULL)
			config_oom();
		
		memcpy(buffer, s, n * sizeof(char));
		buffer[n] = '\0';
	}
	
	return string;
}

void string_deinit( string_t *string )
{
	char is_long = STRING_LONG_TAG(*string);
	
	if (is_long)
	{
		free(string->long_buf);
		string->long_buf = NULL;
	}
}

bool string_equal_len( string_t *string, const char *s, size_t n )
{
	const char *cstr = to_const_cstr(string);
	return strncmp(cstr, s, n) == 0 && cstr[n] == '\0';
}

/* config manipulators */

static void deinit_section( config_section_t *section );
static void deinit_option( config_option_t *option );

void config_init( config_t *config )
{
	assert(config != NULL);
	
	config->sections_count = 0;
	config->sections = NULL;
}

void config_deinit( config_t *config )
{
	assert(config != NULL);
	
	for (unsigned int s = 0; s < config->sections_count; ++s)
	{
		config_section_t *section = &config->sections[s];
		
		deinit_section(section);
	}
	
	free(config->sections);
	config->sections = NULL;
}

/* config section manipulators -- internal */

static void init_section( config_section_t *section, const char *type, size_t type_len, const char *name, size_t name_len )
{
	section->type = string_init_len(type, type_len);
	section->name = string_init_len(name, name_len);
	section->options_count = 0;
	section->options = NULL;
}

static void deinit_section( config_section_t *section )
{
	for (unsigned int o = 0; o < section->options_count; ++o)
	{
		config_option_t *option = &section->options[o];
		
		deinit_option(option);
	}
	
	string_deinit(&section->type);
	string_deinit(&section->name);
	
	free(section->options);
	section->options = NULL;
}

/* config section accessors/manipulators -- by type, name */

config_section_t *config_add_section_len( config_t *config, const char *type, size_t type_len, const char *name, size_t name_len )
{
	assert(config != NULL);
	assert(type != NULL);
	
	config_section_t *sections = realloc(config->sections, (config->sections_count + 1) * sizeof(config_section_t));
	if (sections == NULL)
		return NULL;
	
	config_section_t *section = &sections[config->sections_count];
	
	config->sections_count += 1;
	config->sections = sections;
	
	init_section(section, type, type_len, name, name_len);
	
	return section;
}

config_section_t *config_find_sections( config_t *config, const char *type, config_section_t **save )
{
	assert(config != NULL);
	assert(type != NULL);
	
	config_section_t *sections_end = &config->sections[config->sections_count];
	
	config_section_t *section = save != NULL && *save != NULL ?
		*save :
		&config->sections[0];
	
	for (; section < sections_end; ++section)
		if (strcmp(to_const_cstr(&section->type), type) == 0)
			break;
	
	if (save != NULL)
		*save = section;
	
	return section < sections_end ? section : NULL;
}

config_section_t *config_find_section( config_t *config, const char *type, const char *name )
{
	assert(config != NULL);
	assert(type != NULL);
	
	config_section_t *sections_end = &config->sections[config->sections_count];
	
	for (config_section_t *section = &config->sections[0]; section < sections_end; ++section)
	{
		if (strcmp(to_const_cstr(&section->type), type) == 0)
		{
			const char *section_name = to_const_cstr(&section->name);
			if ((section_name == NULL || name == NULL) ? section_name == name : strcmp(to_const_cstr(&section->name), name) == 0)
				return section;
		}
	}
	
	return NULL;
}

config_section_t *config_find_or_add_section( config_t *config, const char *type, const char *name )
{
	assert(config != NULL);
	assert(type != NULL);
	
	config_section_t *section = config_find_section(config, type, name);
	
	if (section != NULL)
		return section;
	
	return config_add_section(config, type, name);
}

/* config option manipulators -- internal */

static void init_option_value( config_option_t *option, const char *value, size_t value_len )
{
	option->values_count = 0;
	option->v.value = string_init_len(value, value_len);
}

static void deinit_option_value( config_option_t *option )
{
	if (option->values_count != 0)
	{
		string_t *values_end = &option->v.values[option->values_count];
		for (string_t *value = &option->v.values[0]; value < values_end; ++value)
			string_deinit(value);
		
		free(option->v.values);
		option->v.values = NULL;
	}
	else
	{
		string_deinit(&option->v.value);
	}
}

static void init_option( config_option_t *option, const char *key, size_t key_len, const char *value, size_t value_len )
{
	option->key = string_init_len(key, key_len);
	init_option_value(option, value, value_len);
}

static void deinit_option( config_option_t *option )
{
	string_deinit(&option->key);
	deinit_option_value(option);
}

static config_option_t *append_option( config_section_t *section, const char *key, size_t key_len, const char *value, size_t value_len )
{
	config_option_t *options = realloc(section->options, (section->options_count + 1) * sizeof(config_section_t));
	if (options == NULL)
		return NULL;
	
	config_option_t *option = &options[section->options_count];
	
	section->options_count += 1;
	section->options = options;
	
	init_option(option, key, key_len, value, value_len);
	
	return option;
}

static config_option_t *get_option_len( config_section_t *section, const char *key, size_t key_len )
{
	assert(section != NULL);
	assert(key != NULL);
	
	config_option_t *options_end = &section->options[section->options_count];
	for (config_option_t *option = &section->options[0]; option < options_end; ++option)
		if (string_equal_len(&option->key, key, key_len))
			return option;
	
	return NULL;
}

/* config option accessors/manipulators -- by key */

config_option_t *config_set_option_len( config_section_t *section, const char *key, size_t key_len, const char *value, size_t value_len )
{
	assert(section != NULL);
	assert(key != NULL);
	
	config_option_t *option = get_option_len(section, key, key_len);
	
	if (option != NULL)
		return config_set_value_len(option, value, value_len);
	
	return append_option(section, key, key_len, value, value_len);
}

config_option_t *config_get_option( const config_section_t *section, const char *key )
{
	assert(section != NULL);
	assert(key != NULL);
	
	config_option_t *options_end = &section->options[section->options_count];
	for (config_option_t *option = &section->options[0]; option < options_end; ++option)
		if (strcmp(to_const_cstr(&option->key), key) == 0)
			return option;
	
	return NULL;
}

config_option_t *config_get_or_set_option_len( config_section_t *section, const char *key, size_t key_len, const char *value, size_t value_len )
{
	assert(section != NULL);
	assert(key != NULL);
	
	config_option_t *option = get_option_len(section, key, key_len);
	
	if (option != NULL)
		return option;
	
	return append_option(section, key, key_len, value, value_len);
}

void config_set_string_option_len( config_section_t *section, const char *key, size_t key_len, const char *value, size_t value_len )
{
	if (config_set_option_len(section, key, key_len, value, value_len) == NULL)
		config_oom();
}

bool config_get_string_option( const config_section_t *section, const char *key, const char **out_value )
{
	assert(section != NULL);
	assert(key != NULL);
	
	config_option_t *option = config_get_option(section, key);
	if (option != NULL)
	{
		const char *value = config_get_value(option);
		if (value != NULL)
		{
			*out_value = value;
			return true;
		}
	}
	
	return false;
}

const char *config_get_or_set_string_option( config_section_t *section, const char *key, const char *value )
{
	if (!config_get_string_option(section, key, &value))
		config_set_string_option_len(section, key, strlen(key), value, value == NULL ? 0 : strlen(value));
	return value;
}

static const char *bool_values[][2] = 
{
	{ "0", "1" },
	{ "no", "yes" },
	{ "off", "on" },
	{ "false", "true" },
};

void config_set_bool_option( config_section_t *section, const char *key, bool value, bool_style_t style )
{
	if (config_set_option(section, key, bool_values[style][value ? 1 : 0]) == NULL)
		config_oom();
}

bool config_get_bool_option( const config_section_t *section, const char *key, bool *out_value )
{
	assert(section != NULL);
	assert(key != NULL);
	assert(out_value != NULL);
	
	const char *value;
	if (config_get_string_option(section, key, &value))
	{
		for (size_t i = 0; i < COUNTOF(bool_values); ++i)
		{
			for (size_t j = 0; j < COUNTOF(bool_values[i]); ++j)
			{
				if (strcmp(value, bool_values[i][j]) == 0)
				{
					*out_value = j == 0 ? false : true;
					return true;
				}
			}
		}
	}
	
	return false;
}

bool config_get_or_set_bool_option( config_section_t *section, const char *key, bool value, bool_style_t style )
{
	if (!config_get_bool_option(section, key, &value))
		config_set_bool_option(section, key, value, style);
	return value;
}

void config_set_int_option( config_section_t *section, const char *key, int value )
{
	assert(key != NULL);
	
	char buffer[sdecsizeof(int) + 1];
	int buffer_len = snprintf(buffer, sizeof(buffer), "%i", value);
	
	if (config_set_option_len(section, key, strlen(key), buffer, buffer_len) == NULL)
		config_oom();
}

bool config_get_int_option( const config_section_t *section, const char *key, int *out_value )
{
	assert(section != NULL);
	assert(key != NULL);
	assert(out_value != NULL);
	
	const char *value;
	if (config_get_string_option(section, key, &value))
	{
		int i;
		int n;
		if (sscanf(value, "%i%n", &i, &n) > 0 && value[n] == '\0')  /* must be entire string */
		{
			*out_value = i;
			return true;
		}
	}
	
	return false;
}

int config_get_or_set_int_option( config_section_t *section, const char *key, int value )
{
	if (!config_get_int_option(section, key, &value))
		config_set_int_option(section, key, value);
	return value;
}

void config_set_uint_option( config_section_t *section, const char *key, unsigned int value )
{
	assert(key != NULL);
	
	char buffer[udecsizeof(unsigned int) + 1];
	int buffer_len = snprintf(buffer, sizeof(buffer), "%u", value);
	
	if (config_set_option_len(section, key, strlen(key), buffer, buffer_len) == NULL)
		config_oom();
}

bool config_get_uint_option( const config_section_t *section, const char *key, unsigned int *out_value )
{
	assert(section != NULL);
	assert(key != NULL);
	assert(out_value != NULL);
	
	const char *value;
	if (config_get_string_option(section, key, &value))
	{
		unsigned int u;
		int n;
		if (sscanf(value, "%u%n", &u, &n) > 0 && value[n] == '\0')  /* must be entire string */
		{
			*out_value = u;
			return true;
		}
	}
	
	return false;
}

unsigned int config_get_or_set_uint_option( config_section_t *section, const char *key, unsigned int value )
{
	if (!config_get_uint_option(section, key, &value))
		config_set_uint_option(section, key, value);
	return value;
}

/* config option accessors/manipulators -- by reference */

config_option_t *config_set_value_len( config_option_t *option, const char *value, size_t value_len )
{
	assert(option != NULL);
	
	deinit_option_value(option);
	
	init_option_value(option, value, value_len);
	
	return option;
}

config_option_t *config_add_value_len( config_option_t *option, const char *value, size_t value_len )
{
	assert(option != NULL);
	assert(value != NULL);
	
	/* convert 'item' to 'list' */
	if (option->values_count == 0 && to_const_cstr(&option->v.value) != NULL)
	{
		string_t option_value = option->v.value;
		
		string_t *values = malloc(2 * sizeof(string_t));
		if (values == NULL)
			return NULL;
		
		option->v.values = values;
		option->v.values[0] = option_value;
		option->v.values[1] = string_init_len(value, value_len);
		option->values_count = 2;
	}
	else
	{
		string_t *values = realloc(option->v.values, (option->values_count + 1) * sizeof(string_t));
		if (values == NULL)
			return NULL;
		
		option->v.values = values;
		option->v.values[option->values_count] = string_init_len(value, value_len);
		option->values_count += 1;
	}
	
	return option;
}

config_option_t *config_remove_value( config_option_t *option, unsigned int i )
{
	assert(option != NULL);
	
	if (!config_is_value_list(option))
	{
		if (i > 0)
			return NULL;
		
		config_set_value_len(option, NULL, 0);
	}
	else
	{
		if (i >= option->values_count)
			return NULL;
		
		string_deinit(&option->v.values[i]);
		memmove(&option->v.values[i], &option->v.values[i + 1], (option->values_count - i - 1) * sizeof(string_t));
		
		if (option->values_count - 1 == 0)
		{
			option->v.value = string_init_len(NULL, 0);
			option->values_count = 0;
		}
		else
		{
			string_t *values = realloc(option->v.values, (option->values_count - 1) * sizeof(string_t));
			if (values == NULL)
				return NULL;
			
			option->v.values = values;
			option->values_count -= 1;
		}
	}
	
	return option;
}

const char *config_get_value( const config_option_t *option )
{
	if (option == NULL || option->values_count != 0)
		return NULL;
	
	return to_const_cstr(&option->v.value);
}

/* config parser */

static bool is_whitespace( char c )
{
	return c == '\t' || c == ' ';
}

static bool is_end( char c )
{
	return c == '\0' || c == '\n' || c == '\r';
}

static bool is_whitespace_or_end( char c )
{
	return is_whitespace(c) || is_end(c);
}

typedef enum
{
	INVALID_DIRECTIVE = 0,
	SECTION_DIRECTIVE,
	ITEM_DIRECTIVE,
	LIST_DIRECTIVE,
} directive_t;

static directive_t match_directive( const char *buffer, size_t *index )
{
	size_t i = *index;
	
	while (is_whitespace(buffer[i]))
		++i;
	
	directive_t directive;
	
	if (strncmp("section", &buffer[i], 7) == 0)
	{
		directive = SECTION_DIRECTIVE;
		i += 7;
	}
	else if (strncmp("item", &buffer[i], 4) == 0)
	{
		directive = ITEM_DIRECTIVE;
		i += 4;
	}
	else if (strncmp("list", &buffer[i], 4) == 0)
	{
		directive = LIST_DIRECTIVE;
		i += 4;
	}
	else
	{
		return INVALID_DIRECTIVE;
	}
	
	if (!is_whitespace_or_end(buffer[i]))
		return INVALID_DIRECTIVE;
	
	*index = i;
	
	return directive;
}

static bool match_nonquote_field( const char *buffer, size_t *index, size_t *length )
{
	size_t i = *index;
	
	for (; ; ++i)
	{
		char c = buffer[i];
		
		if (is_whitespace_or_end(c))
		{
			break;
		}
		else if (c <= ' ' || c > '~' || c == '#' || c == '\'' || c == '"')
		{
			return false;
		}
	}
	
	*length = i - *index;
	*index = i;
	
	return *length > 0;
}

static bool parse_quote_field( char *buffer, size_t *index, size_t *length )
{
	size_t i = *index;
	size_t o = *index;
	
	char quote = buffer[i];
	
	for (; ; )
	{
		char c = buffer[++i];
		
		if (c == quote)
		{
			++i;
			break;
		}
		else if (c == '\\')
		{
			c = buffer[++i];
			if (c == quote)
			{
				buffer[o++] = quote;
			}
			else
			{
				switch (c)
				{
				case 't':
					buffer[o++] = '\t';
					break;
				case 'n':
					buffer[o++] = '\n';
					break;
				case 'r':
					buffer[o++] = '\r';
					break;
				case '\\':
					buffer[o++] = '\\';
					break;
				case 'x':
					/* parse two hex digits */
					c = buffer[++i];
					char m = (c >= '0' && c <= '9') ? '0' :
					         (c >= 'a' && c <= 'f') ? 'a' - 10 :
					         (c >= 'A' && c <= 'F') ? 'A' - 10 : 0;
					if (m == 0)
						return false;
					char h = c - m;
					c = buffer[++i];
					m = (c >= '0' && c <= '9') ? '0' :
					    (c >= 'a' && c <= 'f') ? 'a' - 10 :
					    (c >= 'A' && c <= 'F') ? 'A' - 10 : 0;
					if (m == 0)
						return false;
					buffer[o++] = (h << 4) | (c - m);
					break;
				default:
					return false;
				}
			}
		}
		else if (c >= ' ' && c <= '~')
		{
			buffer[o++] = c;
		}
		else
		{
			return false;
		}
	}
	
	*length = o - *index;
	*index = i;
	
	return true;
}

static bool parse_field( char *buffer, size_t *index, size_t *start, size_t *length )
{
	size_t i = *index;
	
	while (is_whitespace(buffer[i]))
		++i;
	
	*start = i;
	
	if (buffer[i] == '"' || buffer[i] == '\'')
	{
		if (!parse_quote_field(buffer, &i, length))
			return false;
	}
	else
	{
		if (!match_nonquote_field(buffer, &i, length))
			return false;
	}
	
	if (!is_whitespace_or_end(buffer[i]))
		return INVALID_DIRECTIVE;
	
	*index = i;
	
	return true;
}

bool config_parse( config_t *config, FILE *file )
{
	assert(config != NULL);
	assert(file != NULL);
	
	config_init(config);
	
	config_section_t *section = NULL;
	config_option_t *option = NULL;
	
	size_t buffer_cap = 128;
	char *buffer = malloc(buffer_cap * sizeof(char));
	if (buffer == NULL)
		config_oom();
	size_t buffer_end = 1;
	buffer[buffer_end - 1] = '\0';
	
	for (size_t line = 0, next_line = 0; ; line = next_line)
	{
		/* find begining of next line */
		while (next_line < buffer_end)
		{
			char c = buffer[next_line];
			
			if (c == '\0' && next_line == buffer_end - 1)
			{
				if (line > 0)
				{
					/* shift to front */
					memmove(&buffer[0], &buffer[line], buffer_end - line);
					buffer_end -= line;
					next_line -= line;
					line = 0;
				}
				else if (buffer_end > 1)
				{
					/* need larger capacity */
					buffer_cap *= 2;
					char *new_buffer = realloc(buffer, buffer_cap * sizeof(char));
					if (new_buffer == NULL)
						config_oom();
					buffer = new_buffer;
				}
				
				size_t read = fread(&buffer[buffer_end - 1], sizeof(char), buffer_cap - buffer_end, file);
				if (read == 0)
					break;
				
				buffer_end += read;
				buffer[buffer_end - 1] = '\0';
			}
			else
			{
				++next_line;
				
				if (c == '\n' || c == '\r')
					break;
			}
		}
		
		/* if at end of file */
		if (next_line == line)
			break;
		
		size_t i = line;
		
		directive_t directive = match_directive(buffer, &i);
		
		switch (directive)
		{
		case INVALID_DIRECTIVE:
			continue;
		case SECTION_DIRECTIVE:
			{
				size_t type_start;
				size_t type_length;
				
				if (!parse_field(buffer, &i, &type_start, &type_length))
					continue;
				
				size_t name_start;
				size_t name_length;
				
				bool has_name = parse_field(buffer, &i, &name_start, &name_length);
				
				section = config_add_section_len(config,
						&buffer[type_start], type_length,
						has_name ? &buffer[name_start] : NULL, has_name ? name_length : 0);
				if (section == NULL)
					config_oom();
				option = NULL;
			}
			break;
		case ITEM_DIRECTIVE:
		case LIST_DIRECTIVE:
			{
				if (section == NULL)
					continue;
				
				size_t key_start;
				size_t key_length;
				
				if (!parse_field(buffer, &i, &key_start, &key_length))
					continue;
				
				size_t value_start;
				size_t value_length;
				
				if (!parse_field(buffer, &i, &value_start, &value_length))
					continue;
				
				if (directive == ITEM_DIRECTIVE)
				{
					option = config_set_option_len(section,
							&buffer[key_start], key_length,
							&buffer[value_start], value_length);
				}
				else
				{
					if (option == NULL || !string_equal_len(&option->key, &buffer[key_start], key_length))
						option = config_get_or_set_option_len(section,
								&buffer[key_start], key_length,
								NULL, 0);
					if (option != NULL)
						option = config_add_value_len(option,
								&buffer[value_start], value_length);
				}
				if (option == NULL)
					config_oom();
			}
			break;
		}
		
		assert(i <= next_line);
	}
	
	free(buffer);
	
	return config;
}

/* config writer */

static void write_field( const string_t *field, FILE *file )
{
	fputc('\'', file);
	
	char buffer[128];
	size_t o = 0;
	
	for (const char *ci = to_const_cstr(field); *ci != '\0'; ++ci)
	{
		char c = *ci;
		
		size_t l;
		switch (c)
		{
			case '\t':
			case '\n':
			case '\r':
			case '\'':
			case '\\':
				l = 2;
				break;
			default:
				l = (c >= ' ' && c <= '~') ? 1 : 4;
				break;
		}
		
		if (o + l > COUNTOF(buffer))
		{
			fwrite(buffer, sizeof(*buffer), o, file);
			o = 0;
		}
		
		switch (l)
		{
			case 1:
				buffer[o++] = c;
				break;
			case 2:
				switch (c)
				{
					case '\t':
						buffer[o++] = '\\';
						buffer[o++] = 't';
						break;
					case '\n':
						buffer[o++] = '\\';
						buffer[o++] = 'n';
						break;
					case '\r':
						buffer[o++] = '\\';
						buffer[o++] = 'r';
						break;
					case '\'':
					case '\\':
						buffer[o++] = '\\';
						buffer[o++] = c;
						break;
				}
				break;
			case 4:
				buffer[o++] = '\\';
				buffer[o++] = 'x';
				char n = (c >> 4) & 0x0f;
				buffer[o++] = (n < 10 ? '0' : ('a' - 10)) + n;
				n = c & 0x0f;
				buffer[o++] = (n < 10 ? '0' : ('a' - 10)) + n;
				break;
		}
	}
	
	if (o > 0)
		fwrite(buffer, sizeof(*buffer), o, file);
	
	fputc('\'', file);
}

void config_write( const config_t *config, FILE *file )
{
	assert(config != NULL);
	assert(file != NULL);
	
	for (unsigned int s = 0; s < config->sections_count; ++s)
	{
		config_section_t *section = &config->sections[s];
		
		fputs("section ", file);
		write_field(&section->type, file);
		if (to_cstr(&section->name) != NULL)
		{
			fputc(' ', file);
			write_field(&section->name, file);
		}
		fputc('\n', file);
		
		for (unsigned int o = 0; o < section->options_count; ++o)
		{
			config_option_t *option = &section->options[o];
			
			if (option->values_count == 0 && to_const_cstr(&option->v.value) != NULL)
			{
				fputs("\titem ", file);
				write_field(&option->key, file);
				fputc(' ', file);
				write_field(&option->v.value, file);
				fputc('\n', file);
			}
			else
			{
				string_t *values_end = &option->v.values[option->values_count];
				for (string_t *value = &option->v.values[0]; value < values_end; ++value)
				{
					fputs("\tlist ", file);
					write_field(&option->key, file);
					fputc(' ', file);
					write_field(value, file);
					fputc('\n', file);
				}
			}
		}
		
		fputc('\n', file);
	}
}
