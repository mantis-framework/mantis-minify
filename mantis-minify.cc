#include "mantis-minify.h"

int main(int argc_int, char ** argv)
{
	std::size_t argc = argc_int, p = 0;
	minify::type::string param;

	exec_name.assign(argv[0]);

	while(++p < argc)
	{
		if(!strlen(argv[p]) || argv[p][0] != '-')
			break;

		param.assign(argv[p]);

		if(
			param == "-o" ||
			param == "--output-path"
		) {
			output_type = output_t::file;
			specified_output_path.assign(argv[++p]);

			minify::type::string dir;
			for(std::ptrdiff_t i=specified_output_path.length()-1; i>=0; --i) {
				if(
					specified_output_path[i] == 
					#if defined _WIN32 || defined _WIN64
						'\\'
					#else
						'/'
					#endif
				) {
					dir.substr(
						specified_output_path,
						0,
						i
					);
					break;
				}
			}


			if(dir.length() && !dir_exists(dir.c_str())) {
				std::cout << "directory '" << specified_output_path
				          << "' does not exist, would you like to create it?"
				          << std::endl;
				std::string response;
				std::cin >> response;
				if(
					response == "y"   ||
					response == "Y"   ||
					response == "yes" ||
					response == "Yes"
				){
					if(!create_dirs(specified_output_path)) {
						std::cout << exec_name << ": error: failed to create '"
						          << specified_output_path << "'" << std::endl;
						return 0;
					}
				}
				else
					return 0;
			}
		}
		else if(
			param == "-d"    ||
			param == "--dir" ||
			param == "--directory"
		) {
			output_type = output_t::directory;
			specified_output_path.assign(argv[++p]);

			if(
				argv[p][specified_output_path.length()-1] != '/' &&
				argv[p][specified_output_path.length()-1] != '\\'
			) {
				specified_output_path.strict_resize(
					specified_output_path.capacity()+1
				);
				specified_output_path[
					specified_output_path.length()
				] = 
					#if defined _WIN32 || defined _WIN64
						'\\'; 
					#else
						'/';
					#endif
				specified_output_path.length(
					specified_output_path.length()+1
				);
			}

			if(specified_output_path.length() && !dir_exists(specified_output_path.c_str())) {
				std::cout 
					<< "directory '" << specified_output_path << "' does not exit\n"
					<< "would you like to create it?\r\n";
				std::string response;
				std::cin >> response;
				if(
					response == "y"   ||
					response == "Y"   ||
					response == "yes" ||
					response == "Yes"
				){
					if(!create_dirs(specified_output_path)) {
						std::cout << exec_name << ": error: failed to create '"
						          << specified_output_path << "'" << std::endl;
						return 0;
					}
				}
				else
					return 0;
			}
		}
		else if(
			param == "-css" ||
			param == "--css"
		)
			lang = lang_t::css;
		else if(
			param == "-html" ||
			param == "--html"
		)
			lang = lang_t::html;
		else if(
			param == "-js" ||
			param == "--js"
		)
			lang = lang_t::js;
		else if(
			param == "-json" ||
			param == "--json"
		)
			lang = lang_t::json;
		else if(
			param == "-k" ||
			param == "--keep-comments"
		)
			comment_mode = comment_mode_t::keep;
		else if(
			param == "--doc-comments"
		)
			comment_mode = comment_mode_t::strip;
		else if(
			param == "--raw-comments"
		)
			minify_comments = 0;
		else if(
			param == "-v" || 
			param == "--version"
		) {
			std::cout << exec_name << " " << version << std::endl;

			return 0;
		}
		else if(
			param == "-h" || 
			param == "--help"
		)
		{
			std::cout 
				<< exec_name << "\n\n"

				<< "=> features:\n"
				<< "  - lightweight\n"
				<< "  - zero dependencies\n"
				<< "  - high performance\n"
				<< "  - low memory footprint\n"
				<< "  - multi threaded\n"
				<< "  - cross platform\n\n"

				<< "=> usage:\n"
				<< "  | " << exec_name << " sources\n"
				<< "  | " << exec_name << " (options) sources\n\n"

				<< "=> examples:\n"
				<< "  | " << exec_name << " *.css\n"
				<< "  | " << exec_name << " --output-path bundled.min.css *.css\n\n"

				<< "=> options:\n"
				<< "  -css,  --css\n"
				<< "    minify css\n"
				<< "  -html, --html\n"
				<< "    minify html\n"
				<< "  -js,   --js\n"
				<< "    minify js\n"
				<< "  -json, --json\n"
				<< "    minify json\n"
				<< "  -d, --directory, --dir <DIR>\n"
				<< "    output to <DIR> as *.min.css\n"
				<< "  -o, --output-path <PATH>\n"
				<< "    output combined to <PATH>\n"
				<< "  -k, --keep-comments\n"
				<< "    keep all comments\n"
				<< "      --doc-comments\n"
				<< "    keep /*! .. */ comments\n"
				<< "      --raw-comments\n"
				<< "    unminified comments\n"
				<< "  -v, --version\n"
				<< "    version installed\r\n";

			return 0;
		}
		else
		{
			std::cout 
				<< "error: " << exec_name << ": "
				<< "unrecognised option: " << param << "\n"
				<< "see '" << exec_name << "--help'\r\n";

			return 0;
		}
	}

	if(p >= argc) {
		std::cout << "no files specified, nothing to do" << std::endl;
		return 0;
	}

	do {
		to_minify.push_back(argv[p]);
	}while(++p < argc);

	minified_vec = std::vector<char*>(to_minify.size());

	std::size_t no_cores = std::thread::hardware_concurrency();
	std::vector<std::thread> thrds;

	for(std::size_t c=0; c<no_cores; ++c) 
		thrds.push_back(std::thread(minify_thrd));
	for(std::size_t c=0; c<no_cores; ++c)
		thrds[c].join();


	if(output_type == output_t::terminal) {
		for(std::size_t m=0; m<minified_vec.size(); ++m)
			std::cout << minified_vec[m];
		std::cout << std::endl;
	}
	else if(output_type == output_t::file) {
		FILE* f = fopen(specified_output_path.c_str(), "w");
		for(std::size_t m=0; m<minified_vec.size(); ++m)
			fputs(minified_vec[m], f);
		fclose(f);
	}

	for(std::size_t m=0; m<minified_vec.size(); ++m)
		free(minified_vec[m]);
	minified_vec.clear();

	return 0;
}
