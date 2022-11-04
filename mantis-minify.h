/**
 *  minify.h: 
 * 
 * 	example:
 * 		minify::type::string css("file-path", 1);
 * 		minify_css(css)
 * 
 */

#ifndef MANTIS_MINIFY_H
#define MANTIS_MINIFY_H

#include <cstdio>
#include <iostream>

#include "string.h"

static constexpr char const* version = "v0.1";

enum class comment_mode_t {
	keep,
	strip,
	strip_all
};

enum class lang_t {
	css,
	html,
	js,
	json,
	unspecified
};

static constexpr char 
	asterix     =  '*',
	escape      = '\\',
	exclamation =  '!',
	space       =  ' ',
	tab         = '\t',
	newline     = '\n',
	semicolon   =  ';',
	slash       =  '/',
	angle_open  =  '<',
	angle_close =  '>',
	curly_open  =  '{',
	curly_close =  '}';

static constexpr char quote_marks[3] = {
	'\'', // solo
	 '"', // dual
	 '`'  // tick
};

static constexpr bool is_whitespace(
    char const& c
) {
	return (
		c == ' '  ||
		c == '\t' ||
		c == '\n' ||
		c == '\r'
	);
}

static constexpr bool is_inline_whitespace(
    char const& c
) {
	return (
		c == ' '  ||
		c == '\t'
	);
}

static constexpr bool is_quote_mark(
    char const& c
) {
	return (
		c == quote_marks[0] ||
		c == quote_marks[1] ||
		c == quote_marks[2]
	);
}

static constexpr bool is_special_css_selector_char(
	char const& c
) {
	return (
		c == ',' ||
		c == '{' ||
		c == ')' ||
		c == '>' ||
		c == '+' ||
		c == '~' ||
		c == '"' ||
		c == '\''
	);
}

static constexpr bool is_special_css_property_char(
	char const& c
) {
	return (
		c == ',' ||
		c == '{' ||
		c == ')' ||
		c == '>' ||
		c == '+' ||
		c == '~' ||
		c == '}' ||
		c == ':' ||
		c == '!' ||
		c == '#' ||
		c == '*' ||
		c == '=' ||
		c == '(' ||
		c == '[' ||
		c == ']' ||
		c == '<'
	);
}

static constexpr bool is_js_eol_char(
	char const& c
) {
	return !(
		c == '{'  ||
		c == '}'  ||
		c == '('  ||
		c == '<'  ||
		c == '='  ||
		c == '+'  ||
		c == '-'  ||
		c == '*'  ||
		c == '/'  ||
		c == '|'  ||
		c == '&'  ||
		c == '?'  ||
		c == '.'  ||
		c == ','  ||
		c == ':'  ||
		c == '%'  ||
		c == '\\' 
	);
}

static constexpr bool is_special_char(
	char const& c
) {
	return !(
		('0' <= c && c <= '9') ||
		('a' <= c && c <= 'z') ||
		('A' <= c && c <= 'Z') ||
		c == '_'
	);
}

void skip_past_raw_comment(
    minify::type::string const& code,
    std::size_t& comment_depth,
    std::ptrdiff_t& pos_code
) {
	if(pos_code + 3 < code.size()) {
        if(code[pos_code] == slash) {
            if(code[++pos_code] == slash) {
                ++pos_code;
                while(
                    pos_code < code.size() &&
                    code[pos_code] != newline
                )
                    ++pos_code;
                ++pos_code;
            }
            else if(code[pos_code] == asterix) {
                ++comment_depth;
                ++(++pos_code);
                while(pos_code < code.size()) {
                    if(
                        code[pos_code-1] == slash   && 
                        code[pos_code]   == asterix && 
                        ++pos_code
                    )
                        ++comment_depth;
                    else if(
                        code[pos_code-1] == asterix && 
                        code[pos_code]   == slash   && 
                        ++pos_code                  &&
                        !--comment_depth
                    )
                        break;
                    ++pos_code;
                }
            }
        }
    }
}

void skip_past_html_comment(
    minify::type::string const& code,
    std::ptrdiff_t& pos_code
) {
	if(
		pos_code + 6 < code.size() &&
			code[pos_code] == '<' &&
			code[pos_code+1] == '!' &&
			code[pos_code+2] == '-' &&
			code[pos_code+3] == '-'
	) {
		pos_code += 6;

		while(!(
			code[pos_code-2] == '-' &&
			code[pos_code-1] == '-' &&
			code[pos_code]   == '>'
		)) 
			++pos_code;
		++pos_code;
	}
}

void cpy_comment(
	minify::type::string const& code,
	std::size_t& comment_depth,
	std::ptrdiff_t const& pos_begin,
	std::ptrdiff_t& pos_code,
	minify::type::string& cpy,
	std::ptrdiff_t& cpy_pos
) {
	skip_past_raw_comment(
		code, 
		comment_depth,
		pos_code
	);

	memcpy(
		&cpy[cpy_pos], 
		&code[pos_begin], 
		pos_code-pos_begin
	);
	cpy_pos += pos_code-pos_begin-1;
}

void cpy_html_comment(
	minify::type::string const& code,
	std::ptrdiff_t const& pos_begin,
	std::ptrdiff_t& pos_code,
	minify::type::string& cpy,
	std::ptrdiff_t& cpy_pos
) {
	skip_past_html_comment(
		code, 
		pos_code
	);

	memcpy(
		&cpy[cpy_pos], 
		&code[pos_begin], 
		pos_code-pos_begin
	);
	cpy_pos += pos_code-pos_begin-1;
}

void skip_past_zeros(
    minify::type::string const& code,
    std::ptrdiff_t& pos_code
) {
	while(
		pos_code < code.size() && 
		code[pos_code] == '0'
	)
		++pos_code;
}

void skip_past_whitespace(
	lang_t const& lang,
    minify::type::string const& code,
    std::size_t& comment_depth,
    std::ptrdiff_t& pos_code,
	comment_mode_t const& comment_mode
) {
	while(
		pos_code < code.size()
	) {
		if(is_whitespace(code[pos_code]))
			++pos_code;
		else if(
			comment_mode != comment_mode_t::keep &&
			lang != lang_t::html &&
			pos_code+3 < code.size()    &&
			code[pos_code]   == slash   && (
				code[pos_code+1] == asterix ||
				code[pos_code+1] == slash
			) && (
				comment_mode == comment_mode_t::strip_all ||
				code[pos_code+2] != exclamation
			)
		)
			skip_past_raw_comment(
				code, 
				comment_depth,
				pos_code
			);
		else if(
			comment_mode != comment_mode_t::keep &&
			lang == lang_t::html &&
			pos_code+4 < code.size() &&
			code[pos_code] == '<'    &&
			code[pos_code+1] == '!'  &&
			code[pos_code+2] == '-'  &&
			code[pos_code+3] == '-'
		)
			skip_past_html_comment(
				code, 
				pos_code
			);
		else
			break;
	}

	if(
		pos_code < code.size() &&
		code[pos_code] == '0'
	) {
		++pos_code;
		skip_past_zeros(
			code,
			pos_code
		);

		if(
			pos_code < code.size() && 
			code[pos_code] != '.'
		)
			--pos_code;
	}
}

void skip_past_inline_whitespace(
    minify::type::string const& code,
    std::size_t& comment_depth,
    std::ptrdiff_t& pos_code,
	comment_mode_t const& comment_mode
) {
	while(
		pos_code < code.size()
	) {
		if(is_inline_whitespace(code[pos_code]))
			++pos_code;
		else if(
			comment_mode != comment_mode_t::keep &&
			pos_code+3 < code.size()    &&
			code[pos_code]   == slash   &&
			code[pos_code+1] == asterix && (
				comment_mode == comment_mode_t::strip_all ||
				code[pos_code+2] != exclamation
			)
		)
			skip_past_raw_comment(
				code, 
				comment_depth,
				pos_code
			);
		else
			break;
	}

	if(
		pos_code < code.size() &&
		code[pos_code] == '0'
	) {
		++pos_code;
		skip_past_zeros(
			code,
			pos_code
		);

		if(
			pos_code < code.size() && 
			code[pos_code] != '.'
		)
			--pos_code;
	}
}

void skip_to_quote_end(
	char const& quote_char,
    minify::type::string const& code,
    std::ptrdiff_t& pos_code
) {
	while(
		++pos_code < code.size() && 
		code[pos_code] != quote_char
	)
		if(code[pos_code] == escape)
			++pos_code;
}

void skip_past_quote(
	char const& quote_char,
    minify::type::string const& code,
    std::ptrdiff_t& pos_code
) {
	skip_to_quote_end(
		quote_char, 
		code, 
		pos_code
	);
	++pos_code;
}

void cpy_between(
	minify::type::string const& code,
	std::ptrdiff_t const& pos_begin,
	std::ptrdiff_t const& pos_end,
	minify::type::string& cpy,
	std::ptrdiff_t& cpy_pos
) {
	memcpy(
		&cpy[cpy_pos], 
		&code[pos_begin], 
		pos_end-pos_begin + 1
	);
	cpy_pos += pos_end-pos_begin;
}

void cpy_quote(
	char const& quote_char,
    minify::type::string const& code,
    std::ptrdiff_t const& pos_begin,
    std::ptrdiff_t& pos_code,
    minify::type::string& cpy,
    std::ptrdiff_t& cpy_pos
) {
	skip_past_quote(
		quote_char,
		code,
		pos_code
	);

	cpy_between(
		code,
		pos_begin,
		pos_code-1,
		cpy,
		cpy_pos
	);
}

void minify_css(
	minify::type::string const& css,
	ptrdiff_t& pos_css,
	minify::type::string& minified,
	ptrdiff_t& pos_minified,
	bool const& minify_comments = 1,
	comment_mode_t const& comment_mode = comment_mode_t::strip,
	bool const& minify_capacity = 0
) {
	char quote_type;
	std::size_t comment_depth = 0,
	            curly_bracket_depth = 0;
	std::ptrdiff_t pos_begin;
	minify::type::string cstr;

	if(minified.capacity() <= css.length())
		minified.strict_resize(css.length()+1);

	skip_past_whitespace(
		lang_t::css,
		css, 
		comment_depth,
		pos_css, 
		comment_mode
	);

	while(pos_css < css.size()) {
		if(is_quote_mark(css[pos_css])) {
			cpy_quote(
				quote_type = css[pos_css],
				css, 
				pos_begin  = pos_css,
				pos_css, 
				minified, 
				++pos_minified
			);

			if(
				pos_css < css.size() &&
				is_whitespace(css[pos_css])
			)
				skip_past_whitespace(
					lang_t::css,
					css, 
					comment_depth,
					++pos_css, 
					comment_mode
				);
		}
		else if((
				!curly_bracket_depth && (
					is_special_css_selector_char(css[pos_css]) ||
					css[pos_css] == ':' ||
					css[pos_css] == '*'
				)
			) || (
				curly_bracket_depth &&
				is_special_css_property_char(css[pos_css])
			)
		) {
			minified[++pos_minified] = css[pos_css];

			if(css[pos_css] == curly_open)
				++curly_bracket_depth;
			else if(css[pos_css] == curly_close)
				--curly_bracket_depth;

			if(
				++pos_css < css.size() &&
				is_whitespace(css[pos_css])
			)
				skip_past_whitespace(
					lang_t::css,
					css, 
					comment_depth,
					pos_css, 
					comment_mode
				);
		}
		else if(is_whitespace(css[pos_css])) {
			skip_past_whitespace(
				lang_t::css,
				css, 
				comment_depth,
				pos_css, 
				comment_mode
			);

			if((
				!curly_bracket_depth &&
				!is_special_css_selector_char(css[pos_css])
			) || (
				curly_bracket_depth && 
				!is_special_css_property_char(css[pos_css])
			))
				minified[++pos_minified] = space;
		}
		else if(
			pos_css+3 < css.size()    &&
			css[pos_css]   == slash   && (
				css[pos_css+1] == asterix ||
				css[pos_css+1] == slash
			)
		) {
			if(
				comment_mode != comment_mode_t::keep && ( 
					comment_mode == comment_mode_t::strip_all ||
					css[pos_css+2] != exclamation
				)
			)
				skip_past_whitespace(
					lang_t::css,
					css, 
					comment_depth,
					pos_css, 
					comment_mode
				);
			else if(!minify_comments) {
				cpy_comment(
					css,
					comment_depth,
					pos_begin = pos_css,
					pos_css,
					minified,
					++pos_minified
				);

				skip_past_inline_whitespace(
					css, 
					comment_depth,
					pos_css, 
					comment_mode
				);

				if(
					pos_css < css.size() &&
					css[pos_css] == newline
				) {
					minified[++pos_minified] = newline;
					++pos_css;

					skip_past_whitespace(
						lang_t::css,
						css, 
						comment_depth,
						pos_css, 
						comment_mode
					);
				}
			}
			else {
				minified[++pos_minified] = css[pos_css];
				++pos_css;
			}
		}
		else if(css[pos_css] == semicolon) {
			skip_past_whitespace(
				lang_t::css,
				css, 
				comment_depth,
				++pos_css, 
				comment_mode
			);
			
			if(
				pos_css < css.size() &&
				css[pos_css] != curly_close
			)
				minified[++pos_minified] = semicolon;
		}
		else if(css[pos_css] == angle_open) {
			cstr.substr(css, pos_css, 7);
			if(cstr == "</style") {
				css[pos_css] = '\0';
				break; //return;
			}

			minified[++pos_minified] = angle_open;
			++pos_css;
		}
		else {
			minified[++pos_minified] = css[pos_css];
			++pos_css;
		}
	}

	minified[++pos_minified] = '\0';
	minified.length(pos_minified);

	if(minify_capacity)
		minified.strict_resize(pos_minified);
}

void minify_css(
	minify::type::string const& css,
	minify::type::string& minified,
	bool const& minify_comments = 1,
	comment_mode_t const& comment_mode = comment_mode_t::strip,
	bool const& minify_capacity = 0
) {
	ptrdiff_t pos_css = 0, pos_minified = -1;
	minify_css(
		css, 
		pos_css,
		minified, 
		pos_minified,
		minify_comments,
		comment_mode,
		minify_capacity
	);
}

void minify_css(
	minify::type::string& css,
	bool const& minify_comments = 1,
	comment_mode_t const& comment_mode = comment_mode_t::strip,
	bool const& minify_capacity = 0
) {
	ptrdiff_t pos_css = 0, pos_minified = -1;
	minify_css(
		css, 
		pos_css,
		css, 
		pos_minified,
		minify_comments,
		comment_mode,
		minify_capacity
	);
}

void minify_js(
	minify::type::string const& js,
	ptrdiff_t& pos_js,
	minify::type::string& minified,
	ptrdiff_t& pos_minified,
	bool const& minify_comments = 1,
	comment_mode_t const& comment_mode = comment_mode_t::strip,
	bool const& minify_capacity = 0
) {
	bool inside_comment = 0, treat_round_close_as_eol = 1;
	char quote_type;
	std::size_t comment_depth = 0, round_bracket_depth = 0;
	std::ptrdiff_t pos_begin;
	minify::type::string cstr;

	if(minified.capacity() <= js.length())
		minified.strict_resize(js.length()+1);

	skip_past_whitespace(
		lang_t::js,
		js, 
		comment_depth,
		pos_js, 
		comment_mode
	);

	while(pos_js < js.size()) {
		if(is_quote_mark(js[pos_js])) {
			cpy_quote(
				quote_type = js[pos_js],
				js, 
				pos_begin  = pos_js,
				pos_js, 
				minified, 
				++pos_minified
			);

			if(
				pos_js < js.size() &&
				is_inline_whitespace(js[pos_js])
			)
				skip_past_inline_whitespace(
					js, 
					comment_depth,
					++pos_js, 
					comment_mode
				);
		}
		else if(is_inline_whitespace(js[pos_js])) {
			skip_past_inline_whitespace(
				js, 
				comment_depth,
				pos_js, 
				comment_mode
			);

			if(!is_special_char(js[pos_js]))
				minified[++pos_minified] = space;
		}
		else if(
			inside_comment &&
			is_whitespace(js[pos_js])
		) {
			skip_past_whitespace(
				lang_t::js,
				js, 
				comment_depth,
				pos_js, 
				comment_mode
			);

			if(!is_special_char(js[pos_js]))
				minified[++pos_minified] = space;
		}
		else if(
			js[pos_js] == semicolon || (
				!inside_comment &&
				js[pos_js] == newline
			)
		) {
			skip_past_whitespace(
				lang_t::js,
				js, 
				comment_depth,
				++pos_js, 
				comment_mode
			);
			
			if(
				pos_js < js.size() &&
				js[pos_js] != curly_close
			)
				minified[++pos_minified] = semicolon;
		}
		else if(
			pos_js+3 < js.size()    &&
			js[pos_js]   == slash   && (
				js[pos_js+1] == asterix ||
				js[pos_js+1] == slash
			)
		) {
			if(
				comment_mode == comment_mode_t::strip_all || ( 
					comment_mode == comment_mode_t::strip &&
					js[pos_js+2] != exclamation
				)
			)
				skip_past_whitespace(
					lang_t::js,
					js, 
					comment_depth,
					pos_js, 
					comment_mode
				);
			else if(!minify_comments) {
				cpy_comment(
					js,
					comment_depth,
					pos_begin = pos_js,
					pos_js,
					minified,
					++pos_minified
				);

				skip_past_inline_whitespace(
					js, 
					comment_depth,
					pos_js, 
					comment_mode
				);

				if(
					pos_js < js.size() &&
					js[pos_js] == newline
				) {
					minified[++pos_minified] = newline;
					++pos_js;

					skip_past_whitespace(
						lang_t::js,
						js, 
						comment_depth,
						pos_js, 
						comment_mode
					);
				}
			}
			else {
				inside_comment = 1;
				minified[++pos_minified] = js[pos_js];
				++pos_js;
			}
		}
		else if(
			pos_js+1 < js.size() &&
			js[pos_js]   == asterix &&
			js[pos_js+1] == slash
		) {
			inside_comment = 0;
			minified[++pos_minified] = js[pos_js];
			minified[++pos_minified] = js[++pos_js];

			skip_past_whitespace(
				lang_t::js,
				js, 
				comment_depth,
				++pos_js, 
				comment_mode
			);
		}
		else if(
			!is_js_eol_char(js[pos_js]) || (
				!treat_round_close_as_eol && 
				js[pos_js] == ')'
			)
		) {
			if(js[pos_js] == angle_open) {
				cstr.substr(js, pos_js, 8);
				if(cstr == "</script") 
					return; //break;
			}
			else if(
				js[pos_js] == ')' &&
				!--round_bracket_depth
			)
					treat_round_close_as_eol = 1;

			minified[++pos_minified] = js[pos_js];

			skip_past_whitespace(
				lang_t::js,
				js, 
				comment_depth,
				++pos_js, 
				comment_mode
			);
		}
		else if(
			!treat_round_close_as_eol && 
			js[pos_js] == '('
		) {
			++round_bracket_depth;
			minified[++pos_minified] = '(';
			++pos_js;
		}
		else if(
			pos_js+1 < js.size() &&
			js[pos_js]   == 'i'  && 
			js[pos_js+1] == 'f'
		) {
			treat_round_close_as_eol = 0;
			round_bracket_depth = 0;
			minified[++pos_minified] = js[pos_js];
			minified[++pos_minified] = js[++pos_js];
			++pos_js;
		}
		else if(
			pos_js+2 < js.size() &&
			js[pos_js]   == 'f'  && 
			js[pos_js+1] == 'o'  &&
			js[pos_js+2] == 'r'
		) {
			treat_round_close_as_eol = 0;
			round_bracket_depth = 0;
			minified[++pos_minified] = js[pos_js];
			minified[++pos_minified] = js[++pos_js];
			minified[++pos_minified] = js[++pos_js];
			++pos_js;
		}
		else if(
			pos_js+4 < js.size() &&
			js[pos_js]   == 'w'  && 
			js[pos_js+1] == 'h'  &&
			js[pos_js+2] == 'i'  &&
			js[pos_js+3] == 'l'  &&
			js[pos_js+4] == 'e'
		) {
			treat_round_close_as_eol = 0;
			round_bracket_depth = 0;
			minified[++pos_minified] = js[pos_js];
			minified[++pos_minified] = js[++pos_js];
			minified[++pos_minified] = js[++pos_js];
			minified[++pos_minified] = js[++pos_js];
			minified[++pos_minified] = js[++pos_js];
			++pos_js;
		}
		else if(
			pos_js+7 < js.size() &&
			js[pos_js]   == 'f'  && 
			js[pos_js+1] == 'u'  &&
			js[pos_js+2] == 'n'  &&
			js[pos_js+3] == 'c'  &&
			js[pos_js+4] == 't'  &&
			js[pos_js+5] == 'i'  &&
			js[pos_js+6] == 'o'  &&
			js[pos_js+7] == 'n'
		) {
			treat_round_close_as_eol = 0;
			round_bracket_depth = 0;
			minified[++pos_minified] = js[pos_js];
			minified[++pos_minified] = js[++pos_js];
			minified[++pos_minified] = js[++pos_js];
			minified[++pos_minified] = js[++pos_js];
			minified[++pos_minified] = js[++pos_js];
			++pos_js;
		}
		else if(
			pos_js+3 < js.size() &&
			js[pos_js]   == 'e'  && 
			js[pos_js+1] == 'l'  &&
			js[pos_js+2] == 's'  &&
			js[pos_js+3] == 'e'
		) {
			minified[++pos_minified] = js[pos_js];
			minified[++pos_minified] = js[++pos_js];
			minified[++pos_minified] = js[++pos_js];
			minified[++pos_minified] = js[++pos_js];

			skip_past_whitespace(
				lang_t::js,
				js, 
				comment_depth,
				++pos_js, 
				comment_mode
			);

			if(pos_js < js.size()) {
				if(js[pos_js] == '(') {
					treat_round_close_as_eol = 0;
					round_bracket_depth = 0;
				}
				else
					minified[++pos_minified] = space;
			}
		}
		else {
			minified[++pos_minified] = js[pos_js];
			++pos_js;
		}
	}

	minified[++pos_minified] = '\0';
	minified.length(pos_minified);

	if(minify_capacity)
		minified.strict_resize(pos_minified);
}

void minify_js(
	minify::type::string const& js,
	minify::type::string& minified,
	bool const& minify_comments = 1,
	comment_mode_t const& comment_mode = comment_mode_t::strip,
	bool const& minify_capacity = 0
) {
	ptrdiff_t pos_js = 0, pos_minified = -1;
	minify_js(
		js, 
		pos_js,
		minified, 
		pos_minified,
		minify_comments,
		comment_mode,
		minify_capacity
	);
}

void minify_js(
	minify::type::string& js,
	bool const& minify_comments = 1,
	comment_mode_t const& comment_mode = comment_mode_t::strip,
	bool const& minify_capacity = 0
) {
	ptrdiff_t pos_js = 0, pos_minified = -1;
	minify_js(
		js, 
		pos_js,
		js, 
		pos_minified,
		minify_comments,
		comment_mode,
		minify_capacity
	);
}

void minify_html(
	minify::type::string const& html,
	ptrdiff_t& pos_html,
	minify::type::string& minified,
	ptrdiff_t& pos_minified,
	bool const& minify_comments = 1,
	comment_mode_t const& comment_mode = comment_mode_t::strip,
	bool const& minify_capacity = 0
) {
	bool between_close_and_open = 0,
	     inside_tag = 0;
	char quote_type;
	std::size_t comment_depth = 0;
	std::ptrdiff_t pos_begin;
	minify::type::string cstr;

	if(minified.capacity() <= html.length())
		minified.strict_resize(html.length()+1);

	skip_past_whitespace(
		lang_t::html,
		html, 
		comment_depth,
		pos_html, 
		comment_mode
	);

	while(pos_html < html.size()) {
		if(is_quote_mark(html[pos_html])) {
			cpy_quote(
				quote_type = html[pos_html],
				html, 
				pos_begin  = pos_html,
				pos_html, 
				minified, 
				++pos_minified
			);

			if(
				pos_html < html.size() &&
				is_whitespace(html[pos_html])
			)
				skip_past_whitespace(
					lang_t::html,
					html, 
					comment_depth,
					++pos_html, 
					comment_mode
				);
		}
		else if(is_whitespace(html[pos_html])) {
			skip_past_whitespace(
				lang_t::html,
				html, 
				comment_depth,
				pos_html, 
				comment_mode
			);

			if(inside_tag) {
				if(!is_special_char(html[pos_html]))
					minified[++pos_minified] = space;
			}
			else if(
				pos_html < html.size() && 
				html[pos_html] != angle_close && !(
					html[pos_html] == angle_open &&
					pos_html+1 < html.size() && 
					html[pos_html+1] == slash
				)
			)
				minified[++pos_minified] = space;
		}
		else if(
			pos_html+6 < html.size() &&
			html[pos_html]   == '<' &&
			html[pos_html+1] == '!' &&
			html[pos_html+2] == '-' &&
			html[pos_html+3] == '-'
		) {
			if(comment_mode != comment_mode_t::keep)
				skip_past_whitespace(
					lang_t::html,
					html, 
					comment_depth,
					pos_html, 
					comment_mode
				);
			else if(!minify_comments)
				cpy_html_comment(
					html,
					pos_begin = pos_html,
					pos_html,
					minified,
					++pos_minified
				);
		}
		else if(html[pos_html] == angle_close ) {
			inside_tag = 0;
			minified[++pos_minified] = angle_close;

			if(
				++pos_html < html.size() &&
				is_whitespace(html[pos_html])
			) {
				skip_past_whitespace(
					lang_t::html,
					html, 
					comment_depth,
					pos_html, 
					comment_mode
				);

				if(between_close_and_open && ((
						pos_html < html.size() && 
						html[pos_html] != angle_open
					) || (
						pos_html+1 < html.size() &&
						html[pos_html] == angle_open && //probably redundant
						html[pos_html+1] != slash
					))
				)
					minified[++pos_minified] = space;
			}
		}
		else if(html[pos_html] == angle_open) {
			inside_tag = 1;

			minified[++pos_minified] = html[pos_html];
			++pos_html;

			if(
				pos_html < html.size() && 
				html[pos_html] == slash
			) {
				between_close_and_open = 1;

				minified[++pos_minified] = html[pos_html];
				++pos_html;
			}
			else {
				between_close_and_open = 0;
				if(
					pos_html < html.size() &&
					html[pos_html] == 's'
				) {
					if(cstr.substr(html, pos_html+2, 5) == "cript") 
						minify_js(
							html,
							pos_html,
							minified,
							pos_minified,
							minify_comments,
							comment_mode,
							minify_capacity);
					else if(cstr.substr(html, pos_html+2, 4) == "tyle")
						minify_css(
							html,
							pos_html,
							minified,
							pos_minified,
							minify_comments,
							comment_mode,
							minify_capacity);
				}
			}
		}
		else if(
			inside_tag &&
			is_special_char(html[pos_html])
		) {
			minified[++pos_minified] = html[pos_html];

			skip_past_whitespace(
					lang_t::html,
					html, 
					comment_depth,
					++pos_html, 
					comment_mode
				);
		}
		else {
			minified[++pos_minified] = html[pos_html];
			++pos_html;
		}
	}

	minified[++pos_minified] = '\0';
	minified.length(pos_minified);

	if(minify_capacity)
		minified.strict_resize(pos_minified);
}

void minify_html(
	minify::type::string const& html,
	minify::type::string& minified,
	bool const& minify_comments = 1,
	comment_mode_t const& comment_mode = comment_mode_t::strip,
	bool const& minify_capacity = 0
) {
	ptrdiff_t pos_html = 0, pos_minified = -1;
	minify_html(
		html,
		pos_html,
		minified,
		pos_minified,
		minify_comments,
		comment_mode,
		minify_capacity
	);
}

void minify_html(
	minify::type::string& html,
	bool const& minify_comments = 1,
	comment_mode_t const& comment_mode = comment_mode_t::strip,
	bool const& minify_capacity = 0
) {
	ptrdiff_t pos_html = 0, pos_minified = -1;
	minify_html(
		html, 
		pos_html,
		html, 
		pos_minified,
		minify_comments,
		comment_mode,
		minify_capacity
	);
}

void minify_json(
	minify::type::string const& json,
	minify::type::string& minified,
	bool const& minify_comments = 1,
	comment_mode_t const& comment_mode = comment_mode_t::strip,
	bool const& minify_capacity = 0
) {
	char quote_type;
	std::size_t comment_depth = 0;
	std::ptrdiff_t pos_begin,
		pos_json = 0,
		pos_minified = -1;
	minify::type::string cstr;

	if(minified.capacity() < json.length())
		minified.strict_resize(json.length()+1);

	while(pos_json < json.length()) {
		if(is_quote_mark(json[pos_json]))
			cpy_quote(
				quote_type = json[pos_json],
				json, 
				pos_begin  = pos_json,
				pos_json, 
				minified, 
				++pos_minified
			);
		else if(is_whitespace(json[pos_json]))
			skip_past_whitespace(
				lang_t::json,
				json, 
				comment_depth,
				pos_json, 
				comment_mode
			);
		else if(
			pos_json+3 < json.size() &&
			json[pos_json] == slash  && (
				json[pos_json+1] == asterix ||
				json[pos_json+1] == slash
			)
		) {
			if(
				comment_mode == comment_mode_t::strip_all || (
					comment_mode == comment_mode_t::strip && 
					json[pos_json+2] != exclamation
				)
			)
				skip_past_whitespace(
					lang_t::json,
					json, 
					comment_depth,
					pos_json, 
					comment_mode
				);
			else if(!minify_comments) {
				cpy_comment(
					json,
					comment_depth,
					pos_begin = pos_json,
					pos_json,
					minified,
					++pos_minified
				);

				skip_past_inline_whitespace(
					json, 
					comment_depth,
					pos_json, 
					comment_mode
				);

				if(
					pos_json < json.size() &&
					json[pos_json] == newline
				) {
					minified[++pos_minified] = newline;
					++pos_json;

					skip_past_whitespace(
						lang_t::json,
						json, 
						comment_depth,
						pos_json, 
						comment_mode
					);
				}
			}
			else {
				minified[++pos_minified] = json[pos_json];
				++pos_json;
			}
		}
		else {
			pos_begin = pos_json;
			while(
				pos_json < json.size() && 
				!is_quote_mark(json[pos_json]) &&
				!is_whitespace(json[pos_json]) && !(
					pos_json+3 < json.size() &&
					json[pos_json] == slash  &&
					json[pos_json+1] == asterix
				)
			)
				++pos_json;

			cpy_between(
				json,
				pos_begin,
				pos_json-1,
				minified,
				++pos_minified
			);
		}
	}

	minified[++pos_minified] = '\0';
	minified.length(pos_minified);

	if(minify_capacity)
		minified.strict_resize(pos_minified);
}

void minify_json(
	minify::type::string& json,
	bool const& minify_comments = 1,
	comment_mode_t const& comment_mode = comment_mode_t::strip,
	bool const& minify_capacity = 0
) {
	minify_json(
		json, 
		json, 
		minify_comments,
		comment_mode,
		minify_capacity
	);
}

#include <sys/stat.h>
bool path_exists(minify::type::string const& path) {
	struct stat info;

	if(stat(path.c_str(), &info ) != 0) 
		return 0; //does not exist
	else
		return 1; //exists
}
bool dir_exists(minify::type::string const& path) {
	struct stat info;

	if(stat( path.c_str(), &info ) != 0) 
		return 0; //path doesn't exist
	else if(info.st_mode & S_IFDIR) 
		return 1; //dir
	else 
		return 0; //file
}
bool file_exists(minify::type::string const& path) {
	struct stat info;

	if(stat( path.c_str(), &info ) != 0) 
		return 0; //path nonexistent
	else if(info.st_mode & S_IFDIR) 
		return 0; //dir
	else 
		return 1; //file
}

int make_dir(minify::type::string const& dir) {
	#if defined _WIN32 || defined _WIN64
		return _mkdir(
			dir.c_str()
		); //windows 
	#else 
		return mkdir(
			dir.c_str(), 
			S_IRWXU | 
			S_IRWXG | 
			S_IROTH | 
			S_IXOTH
		); //*[bsd|nix]
	#endif
}

int create_dirs(minify::type::string const& path) {
	minify::type::string dir;

	for(std::ptrdiff_t i=0; i<path.length(); ++i) {
		if(path[i] == '/' || path[i] == '\\') {
			dir.substr(path, 0, i);
			if(!dir_exists(dir) && make_dir(dir))
				return 0;
		}
	}

	return 1;
}

/*#include <unistd.h>
#include <fcntl.h>
#ifdef _WIN32
	#include <direct.h>
#endif

bool create_file(minify::type::string const& path) {
	if(path.length())
	{
		close(creat(path.c_str(), O_CREAT));
		chmod(path.c_str(), 0644);
	}

	return 0;
}*/

void get_filename(
	minify::type::string& path,
	minify::type::string& file
) {
	for(std::size_t i=path.length()-1; i!=std::string::npos; --i) {
		if(path[i] == '/' || path[i] == '\\') {
			file.substr(
				path,
				i+1, 
				path.length()-i-1);
			break;
		}
	}

}

void append_filename(
	minify::type::string& path,
	minify::type::string& dir
) {
	for(std::size_t i=path.length()-1; i!=std::string::npos; --i) {
		if(path[i] == '/' || path[i] == '\\') {
			dir.append_substr(
				path,
				i+1, 
				path.length()-i-1);
			break;
		}
	}

}

enum class output_t {
	file,
	directory,
	terminal
};

#include <atomic>
#include <mutex>
#include <thread>
#include <vector>

static bool minify_comments = 1;
static comment_mode_t comment_mode = comment_mode_t::strip_all;
static lang_t lang = lang_t::unspecified;
static output_t output_type = output_t::terminal;
static std::atomic<std::size_t> counter(-1);
static std::mutex mtx;
static std::vector<char const*> to_minify;
static std::vector<char*> minified_vec;
static minify::type::string exec_name, specified_output_path;

void minify_thrd() {
	std::size_t i=0;
	minify::type::string code,
	                     input_path,
	                     output_path;
	std::ptrdiff_t const op_length = specified_output_path.length();
	FILE* f;

	if(output_type == output_t::directory)
		output_path.assign(specified_output_path.c_str());

	while((i = ++counter) < to_minify.size()) {
		input_path.assign(to_minify[i]);

		if(file_exists(input_path)) {
			code.load_file(to_minify[i]);

			switch(lang) {
				case lang_t::css:
					minify_css(
						code, 
						minify_comments, 
						comment_mode
					);
					break;
				case lang_t::html:
					minify_html(
						code, 
						minify_comments, 
						comment_mode
					);
					break;
				case lang_t::js:
					minify_js(
						code, 
						minify_comments, 
						comment_mode
					);
					break;
				case lang_t::json:
					minify_json(
						code, 
						minify_comments, 
						comment_mode
					);
					break;
				default:
					std::cout << "no language specified" << std::endl;
					break;
			}

			if(
				output_type == output_t::terminal ||
				output_type == output_t::file
			) {
				mtx.lock();
				minified_vec[i] = (char*) malloc(sizeof(char)*(code.length()+1));
				memcpy(&minified_vec[i][0], &code[0], code.length());
				minified_vec[i][code.length()] = '\0';
				mtx.unlock();
			}
			else if(output_type == output_t::directory) {
				append_filename(input_path, output_path);

				f = fopen(output_path.c_str(), "w");
				fputs(code.c_str(), f);
				fclose(f);
				f = nullptr;

				output_path.length(op_length);
			}
		}
		else
		{
			std::cout << "error: " << exec_name << ": ";
			std::cout << "source file '" << input_path << "' does not exist" << std::endl;
		}
	}
}

#endif //MANTIS_MINIFY_H