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
#ifndef CONFIG_FILE_H
#define CONFIG_FILE_H

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*! \file */

#ifndef COMPILE_TIME_ASSERT
/*!
 * \brief Cause compile error if compile-time computable condition fails.
 * 
 * \param[in] name the unique identifier of the assertion
 * \param[in] cond the condition
 */
#define COMPILE_TIME_ASSERT(name, cond) typedef int assert_ ## name[(cond) * 2 - 1]
#endif

#ifndef COUNTOF
/*!
 * \brief Calculate the number of elements in a fixed-length array.
 * 
 * \param[in] a the fixed-length array
 * \return the number of elements in the array
 */
#define COUNTOF(a) (sizeof(a) / sizeof(*(a)))
#endif

/* string type */

/*!
 * \brief A short-string-optimizing string type.
 * 
 * This struct allows for storing up to 15 characters (plus a terminating \c '\0') inline.  For
 * longer strings memory will be allocated.
 * 
 * The tag for this union is the last character of \p short_buf:
 * \li if \c '\0' then \p short_buf is valid,
 * \li otherwise \p long_buf is valid.
 */
typedef union
{
	/*!
	 * \brief The inline buffer for short strings.
	 */
	char short_buf[16];
	
	/*!
	 * \brief The buffer for long strings.
	 *
	 * May be \c NULL.
	 */
	char *long_buf;
} string_t;

/*! \cond suppress doxygen */
COMPILE_TIME_ASSERT(string_short_buf_sufficient, sizeof(char *) + 1 <= COUNTOF(((string_t *)NULL)->short_buf));
/*! \endcond */

/*! \sa string_init */
extern string_t string_init_len( const char *s, size_t n );

/*!
 * \brief Initialize a string from a C-string.
 * 
 * \param[in] s the C-string
 * \return the string
 */
static inline string_t string_init( const char *s )
{
	assert(s != NULL);
	return string_init_len(s, strlen(s));
}

/*!
 * \brief Release any memory allocated inside a string.
 * 
 * \param[in] string the string
 * \return void
 */
extern void string_deinit( string_t *string );

/*! \cond suppress doxygen */
#define STRING_LONG_TAG(s) ((s).short_buf[COUNTOF((s).short_buf) - 1])
/*! \endcond */

/*!
 * \brief Return a C-string backed by a string.
 * 
 * \param[in] string the string
 * \return the C-string
 */
static inline char *to_cstr( string_t *string )
{
	assert(string != NULL);
	char is_long = STRING_LONG_TAG(*string);
	return is_long ?
		string->long_buf :
		string->short_buf;
}

/*!
 * \brief Return a C-string backed by a string.
 * 
 * \param[in] string the string
 * \return the C-string
 */
static inline const char *to_const_cstr( const string_t *string )
{
	assert(string != NULL);
	char is_long = STRING_LONG_TAG(*string);
	return is_long ?
		string->long_buf :
		string->short_buf;
}

#undef STRING_LOG_TAG

/* config types */

/*!
 * \brief An option consisting of one (an item) or many (a list) values.
 */
typedef struct
{
	/*!
	 * \brief The key of the option.
	 */
	string_t key;
	
	/*!
	 * \brief The number of values in the option if it is a 'list' option.
	 *
	 * If \c 0 then the option \e may be an 'item' option.
	 * 
	 * \see config_option_t::value
	 */
	unsigned int values_count;
	
	/*!
	 * \brief The value or values.
	 * 
	 * The tag for this union is \p value_count:
	 * \li if \c 0 then \p value is valid,
	 * \li otherwise \p values is valid.
	 */
	union
	{
		/*!
		 * \brief The value of an 'item' option or an empty 'list' option.
		 * 
		 * If this field is \c NULL then the option is an empty 'list' option.
		 */
		string_t value;
		
		/*!
		 * \brief The values of a non-empty 'list' option.
		 */
		string_t *values;
	} v;
} config_option_t;

/*!
 * \brief A section consisting of options.
 */
typedef struct
{
	/*!
	 * \brief The type of the section.
	 */
	string_t type;
	
	/*!
	 * \brief The optional name of the section.
	 *
	 * May be \c NULL.
	 */
	string_t name;
	
	/*!
	 * \brief The number of options in the section.
	 */
	unsigned int options_count;
	
	/*!
	 * \brief The options in the section.
	 *
	 * \c NULL if \p options_count is \c 0.
	 */
	config_option_t *options;
} config_section_t;

/*!
 * \brief A configuration consisting of sections.
 */
typedef struct
{
	/*!
	 * \brief The number of sections in the configuration.
	 */
	unsigned int sections_count;
	
	/*!
	 * \brief The sections in the configuration.
	 *
	 * \c NULL if \p sections_count is \c 0.
	 */
	config_section_t *sections;
} config_t;

/* config manipulators */

/*!
 * \brief Initialize a configuration.
 * 
 * \param[in] config the configuration
 * \return void
 */
extern void config_init( config_t *config );

/*!
 * \brief Release any memory allocated inside a configuration.
 * 
 * \param[in] config the configuration
 * \return void
 */
extern void config_deinit( config_t *config );

/*!
 * \brief Parse a configuration from a file.
 * 
 * \param[in] config the uninitalized configuration
 * \param[in] file the file handle
 * \return whether parsing succeeded
 */
extern bool config_parse( config_t *config, FILE *file );

/*!
 * \brief Write a configuration to a file.
 * 
 * \param[in] config the configuration
 * \param[in] file the file handle
 * \return void
 */
extern void config_write( const config_t *config, FILE *file );

/* config section accessors/manipulators -- by type, name */

/*! \sa config_add_section */
extern config_section_t *config_add_section_len( config_t *config, const char *type, size_t type_len, const char *name, size_t name_len );

/*!
 * \brief Add a section to a configuration.
 * 
 * \param[in] config the configuration to contain the section
 * \param[in] type the type of the section
 * \param[in] name the name of the section; may be \c NULL
 * \return the added section; \c NULL if out of memory
 */
static inline config_section_t *config_add_section( config_t *config, const char *type, const char *name)
{
	assert(type != NULL);
	return config_add_section_len(config, type, strlen(type), name, name == NULL ? 0 : strlen(name));
}

// TODO: extern config_t *config_remove_section( config_t *config, unsigned int i );

/*!
 * \brief Iterate sections by type.
 * 
 * \param[in] config the configuration containing the sections
 * \param[in] type the type of the section
 * \param[in,out] save the saved state of the iterator; initialize \c *save to \c NULL before
 *                     iteration
 * \return the section; \c NULL if iteration finished
 */
extern config_section_t *config_find_sections( config_t *config, const char *type, config_section_t **save );

/*!
 * \brief Find a section by type and name.
 * 
 * \param[in] config the configuration containing the section
 * \param[in] type the type of the section
 * \param[in] name the name of the section
 * \return the section; \c NULL if it does not exist
 */
extern config_section_t *config_find_section( config_t *config, const char *type, const char *name );

/*!
 * \brief Find a section by type and name, creating the section if it did not exist.
 * 
 * \param[in] config the configuration containing the section
 * \param[in] type the type of the section
 * \param[in] name the name of the section; may be \c NULL
 * \return the section; \c NULL if out of memory
 */
extern config_section_t *config_find_or_add_section( config_t *config, const char *type, const char *name );

/* config option accessors/manipulators -- by key */

/*! \sa config_set_option */
extern config_option_t *config_set_option_len( config_section_t *section, const char *key, size_t key_len, const char *value, size_t value_len );

/*!
 * \brief Set a value of an 'item' option by key, creating the option if necessary.
 * 
 * \param[in] section the section containing the option
 * \param[in] key the option key
 * \param[in] value the item value; \c NULL to set an emtpy 'list' option instead of an 'item'
 *                  option (can be used to delete an 'item' option)
 * \return the option; \c NULL if out of memory
 */
static inline config_option_t *config_set_option( config_section_t *section, const char *key, const char *value)
{
	assert(key != NULL);
	return config_set_option_len(section, key, strlen(key), value, value == NULL ? 0 : strlen(value));
}

/*!
 * \brief Get an option by key.
 * 
 * \param[in] section the section containing the option
 * \param[in] key the option key
 * \return the option; \c NULL if it does not exist
 */
extern config_option_t *config_get_option( const config_section_t *section, const char *key );

/*! \sa config_get_or_set_option */
extern config_option_t *config_get_or_set_option_len( config_section_t *section, const char *key, size_t key_len, const char *value, size_t value_len );

/*!
 * \brief Get an option by key, creating an 'item' option if the option did not exist.
 * 
 * \param[in] section the section containing the option
 * \param[in] key the option key
 * \param[in] value the default item value; \c NULL to set an empty 'list' option instead of an
 *                  'item' option
 * \return the option; \c NULL if out of memory
 */
static inline config_option_t *config_get_or_set_option( config_section_t *section, const char *key, const char *value )
{
	assert(key != NULL);
	return config_get_or_set_option_len(section, key, strlen(key), value, value == NULL ? 0 : strlen(value));
}

/*! \sa config_set_string_option */
extern void config_set_string_option_len( config_section_t *section, const char *key, size_t key_len, const char *value, size_t value_len );

/*!
 * \brief Set a string value of an 'item' option by key, creating the option if necessary.
 * 
 * \param[in] section the section containing the option
 * \param[in] key the option key
 * \param[in] value the item value
 * \return void
 */
static inline void config_set_string_option( config_section_t *section, const char *key, const char *value )
{
	assert(key != NULL);
	config_set_string_option_len(section, key, strlen(key), value, value == NULL ? 0 : strlen(value));
}

/*!
 * \brief Get a string value of an 'item' option by key.
 * 
 * \param[in] section the section containing the option
 * \param[in] key the option key
 * \param[out] out_value the item value if a valid option exists; otherwise unset
 * \return whether \p out_value was set
 */
extern bool config_get_string_option( const config_section_t *section, const char *key, const char **out_value );

/*!
 * \brief Get a string value of an 'item' option by key, setting the option if it was invalid or
 *        creating the option if it did not exist.
 * 
 * \param[in] section the section containing the option
 * \param[in] key the option key
 * \param[in] value the default item value
 * \return the value
 */
extern const char *config_get_or_set_string_option( config_section_t *section, const char *key, const char *value );

/*!
 * \brief The styles of boolean values.
 */
typedef enum
{
	ZERO_ONE = 0,
	NO_YES = 1,
	OFF_ON = 2,
	FALSE_TRUE = 3,
} bool_style_t;

/*!
 * \brief Set a boolean value of an 'item' option by key, creating the option if necessary.
 * 
 * \param[in] section the section containing the option
 * \param[in] key the option key
 * \param[in] value the item value
 * \param[in] style the style of boolean value
 * \return void
 */
extern void config_set_bool_option( config_section_t *section, const char *key, bool value, bool_style_t style );

/*!
 * \brief Get a boolean value of an 'item' option by key.
 * 
 * \param[in] section the section containing the option
 * \param[in] key the option key
 * \param[out] out_value the item value if a valid option exists; otherwise unset
 * \return whether \p out_value was set
 */
extern bool config_get_bool_option( const config_section_t *section, const char *key, bool *out_value );

/*!
 * \brief Get a boolean value of an 'item' option by key, setting the option if it was invalid or
 *        creating the option if it did not exist.
 * 
 * \param[in] section the section containing the option
 * \param[in] key the option key
 * \param[in] value the default item value
 * \param[in] style the style of boolean value
 * \return the value
 */
extern bool config_get_or_set_bool_option( config_section_t *section, const char *key, bool value, bool_style_t style );

/*!
 * \brief Set an integer value of an 'item' option by key, creating the option if necessary.
 * 
 * \param[in] section the section containing the option
 * \param[in] key the option key
 * \param[in] value the item value
 * \return void
 */
extern void config_set_int_option( config_section_t *section, const char *key, int value );

/*!
 * \brief Get an integer value of an 'item' option by key.
 * 
 * \param[in] section the section containing the option
 * \param[in] key the option key
 * \param[out] out_value the item value if a valid option exists; otherwise unset
 * \return whether \p out_value was set
 */
extern bool config_get_int_option( const config_section_t *section, const char *key, int *out_value );

/*!
 * \brief Get an integer value of an 'item' option by key, setting the option if it was invalid or
 *        creating the option if it did not exist.
 * 
 * \param[in] section the section containing the option
 * \param[in] key the option key
 * \param[in] value the default item value
 * \return the value
 */
extern int config_get_or_set_int_option( config_section_t *section, const char *key, int value );

/*!
 * \brief Set an unsigned integer value of an 'item' option by key, creating the option if
 *        necessary.
 * 
 * \param[in] section the section containing the option
 * \param[in] key the option key
 * \param[in] value the item value
 * \return void
 */
extern void config_set_uint_option( config_section_t *section, const char *key, unsigned int value );

/*!
 * \brief Get an unsigned integer value of an 'item' option by key.
 * 
 * \param[in] section the section containing the option
 * \param[in] key the option key
 * \param[out] out_value the item value if a valid option exists; otherwise unset
 * \return whether \p out_value was set
 */
extern bool config_get_uint_option( const config_section_t *section, const char *key, unsigned int *out_value );

/*!
 * \brief Get an unsigned integer value of an 'item' option by key, setting the option if it was
 *        invalid or creating the option if it did not exist.
 * 
 * \param[in] section the section containing the option
 * \param[in] key the option key
 * \param[in] value the default item value
 * \return the value
 */
extern unsigned int config_get_or_set_uint_option( config_section_t *section, const char *key, unsigned int value );

/* config option accessors/manipulators -- by reference */

/*! \sa config_set_value */
extern config_option_t *config_set_value_len( config_option_t *option, const char *value, size_t value_len );

/*!
 * \brief Set the value of an 'item' option.
 * 
 * \param[in] option the option
 * \param[in] value the value
 * \return the option; \c NULL if out of memory
 */
static inline config_option_t *config_set_value( config_option_t *option, const char *value )
{
	return config_set_value_len(option, value, value == NULL ? 0 : strlen(value));
}

/*! \sa config_add_value */
extern config_option_t *config_add_value_len( config_option_t *option, const char *value, size_t value_len );

/*!
 * \brief Add a value to a 'list' option.
 * 
 * \param[in] option the option
 * \param[in] value the value
 * \return the option; \c NULL if out of memory
 */
static inline config_option_t *config_add_value( config_option_t *option, const char *value )
{
	assert(value != NULL);
	return config_add_value_len(option, value, strlen(value));
}

/*!
 * \brief Remove a value from a 'list' option.
 * 
 * \param[in] option the option
 * \param[in] i the index of the value
 * \return the option; \c NULL if out of memory or invalid \p index
 */
extern config_option_t *config_remove_value( config_option_t *option, unsigned int i );

/*!
 * \brief Get the value of an 'item' option.
 * 
 * \param[in] option the option
 * \return the value; \c NULL if \p option was \c NULL or was a 'list' option
 */
extern const char *config_get_value( const config_option_t *option );

/*!
 * \brief Get the value that indicates whether the option is a 'list' option.
 * 
 * \param[in] option the option
 * \return whether the option is a 'list' option
 */
static inline bool config_is_value_list( const config_option_t *option )
{
	assert(option != NULL);
	return option->values_count > 0 ||
		to_const_cstr(&option->v.value) == NULL;
}

/*!
 * \brief Get the number of values assigned to the option.
 * 
 * \param[in] option the option
 * \return \c 1 if the option is an 'item' option; the number of elements if the option is a 'list'
 *         option
 */
static inline unsigned int config_get_value_count( const config_option_t *option )
{
	assert(option != NULL);
	return option->values_count == 0 ?
		(to_const_cstr(&option->v.value) == NULL ? 0 : 1) :
		option->values_count;
}

/*!
 * \brief Iterate over the values assigned to the option.
 * 
 * \param[out] string_value the value variable to declare
 * \param[in] option the option
 */
#define foreach_option_value( string_value, option ) \
	for (config_option_t *_option = option; _option != NULL; _option = NULL) \
	for (string_t *_values_begin = (option)->values_count == 0 ? &(option)->v.value : &(option)->v.values[0], \
	              *_values_end = (option)->values_count == 0 ? _values_begin + 1 : &(option)->v.values[(option)->values_count], \
	              *_value = _values_begin; _value < _values_end; ++_value) \
	for (const char *(string_value) = to_const_cstr(_value); (string_value) != NULL; (string_value) = NULL)

/*!
 * \brief Iterate over the values assigned to the option.
 * 
 * \param[out] i the index variable to declare
 * \param[out] string_value the value variable to declare
 * \param[in] option the option
 */
#define foreach_option_i_value( i, string_value, option ) \
	for (unsigned int (i) = 0; (i) == 0; (i) = ~0) \
	for (config_option_t *_option = option; _option != NULL; _option = NULL) \
	for (string_t *_values_begin = (option)->values_count == 0 ? &(option)->v.value : &(option)->v.values[0], \
	              *_values_end = (option)->values_count == 0 ? _values_begin + 1 : &(option)->v.values[(option)->values_count], \
	              *_value = _values_begin; _value < _values_end; ++_value, (i) = _value - _values_begin) \
	for (const char *(string_value) = to_const_cstr(_value); (string_value) != NULL; (string_value) = NULL)

/*!
 * \brief Remove a value from an option during iteration.  Should be followed by \c continue.
 */
#define foreach_remove_option_value() \
	{ \
		extern void config_oom( void ); \
		unsigned int _value_i = _value - _values_begin; \
		if (config_remove_value(_option, _value_i) == NULL) \
			config_oom(); \
		_values_begin = (option)->values_count == 0 ? &(option)->v.value : &(option)->v.values[0]; \
		_values_end = (option)->values_count == 0 ? _values_begin + 1 : &(option)->v.values[(option)->values_count]; \
		_value = _values_begin + _value_i - 1; \
	}

#endif
