// ************************************************************************** //
//                                                                            //
//                                                                            //
//   parser.cpp                                                               //
//                                                                            //
//   By: Mateo <teorodrip@protonmail.com>                                     //
//                                                                            //
//   Created: 2019/01/10 17:57:13 by Mateo                                    //
//   Updated: 2019/01/11 19:54:30 by Mateo                                    //
//                                                                            //
// ************************************************************************** //

#include "../includes/parser.hpp"
#include <stdlib.h>

excel_parser::excel_parser()
{
	this->flags = 0x0;
}

excel_parser::excel_parser(std::string file_path)
{
	this->file_path = file_path;
	this->flags = 0x0;
}

void excel_parser::handle_fatal_error(const std::string message)
{
	std::cerr << "Fatal error: " << message << "\n";
	exit(EXIT_FAILURE);
}

bool excel_parser::issdigit(char *str)
{
	if (!*str)
		return (false);
	while (*str)
		{
			if (!std::isdigit(*str))
				return (false);
			str++;
		}
	return (true);
}

void excel_parser::init()
{
	if ((book = xlsxioread_open(file_path.c_str())) == NULL)
		{
			std::cerr << "Error opening: " << file_path << "\n";
			exit(EXIT_FAILURE);
		}
	//list available sheets
  xlsxioreadersheetlist sheetlist;
  const XLSXIOCHAR* sheetname;
  if ((sheetlist = xlsxioread_sheetlist_open(book)) != NULL)
		{
			while ((sheetname = xlsxioread_sheetlist_next(sheetlist)) != NULL)
				sheet_names.push_back(sheetname);
			xlsxioread_sheetlist_close(sheetlist);
		}
	else
		{
			std::cerr << "Error: listing the sheets\n";
			exit(EXIT_FAILURE);
		}
}

void excel_parser::parse_book()
{
	xlsxioreadersheet sheet;
	ticker_json_t j_quarter;//before half ticker
	ticker_json_t j_year;//after half ticker

	for (std::string sheet_name : sheet_names)
		{
			if ((sheet = xlsxioread_sheet_open(book, sheet_name.c_str(),
																				 XLSXIOREAD_SKIP_EMPTY_ROWS)) != NULL)
				{
					while (!(this->flags & F_END_PARSING) && xlsxioread_sheet_next_row(sheet))
						{
							parse_row(sheet, &j_quarter, &j_year);
						}
					xlsxioread_sheet_close(sheet);
					//uplad jsnons here
					delete(j_quarter.j);
					delete(j_year.j);
				}
		}
}

void excel_parser::parse_row(const xlsxioreadersheet sheet, ticker_json_t *j_quarter,
														 ticker_json_t *j_year)
{
	ticker_json_t *j;
	size_t cell_j;
	char *cell_value;
	std::string current_field;

	cell_j = 0;
	j = (this->flags & F_HALF) ? j_year : j_quarter;
	while ((cell_value = xlsxioread_sheet_next_cell(sheet)) != NULL)
		{
			if (cell_j == 0)
				{
					if (!strcmp(cell_value, TICKER_START) && !(flags & F_START))
						{
							this->flags = 0x0;
							init_ticker(sheet, j);
							continue;
						}
					else if (!strcmp(cell_value, HALF_TICKER) && (flags & F_START) &&
									 !(flags & F_HALF))
						{
							this->flags |= F_HALF;
							j = j_year;
							init_date(sheet, j);
							if (!xlsxioread_sheet_next_row(sheet)) //epty row after half ticker
								handle_fatal_error("In the format of the sheet after HALF_TICKER");
							continue;
						}
					else if (!strcmp(cell_value, HALF_TICKER) && (flags & F_START) &&
									 (flags & F_HALF) && !(flags & F_END))
						{
							this->flags = F_END;
							if (!xlsxioread_sheet_next_row(sheet)) //epty row after end ticker
								handle_fatal_error("In the format of the sheet after END_TICKER");
							continue;
						}
					else if (*cell_value == '\0')
						{
							if (this->flags & F_END)
								{
									this->flags = F_END_PARSING;
									break;
								}
							if (!xlsxioread_sheet_next_row(sheet) && !xlsxioread_sheet_next_row(sheet)) //jump empty row
								handle_fatal_error("In the format of the sheet after END_TICKER");
							continue;
						}
					else
						current_field = cell_value;
				}
			else if (*cell_value == '\0')
				break;
			else
				(j->j[cell_j - 1])[current_field] = cell_value;
			cell_j++;
		}
}

//this will create the array of json for each date of the ticker
void excel_parser::init_date(const xlsxioreadersheet sheet, ticker_json_t *j)
{
	char *cell_value;

	j->len = (size_t)-1; //the first cell does not count
	if (xlsxioread_sheet_next_row(sheet) && //Company name row
			xlsxioread_sheet_next_row(sheet)) //dates row
		{
			while ((cell_value = xlsxioread_sheet_next_cell(sheet)) != NULL && *cell_value != '\0')
				j->len++;
			j->j = new nlohmann::json[j->len];
		}
	else
		handle_fatal_error("In the format of the sheet while reading dates");
	if (!xlsxioread_sheet_next_row(sheet)) //epty row after dates
		handle_fatal_error("In the format of the sheet while reading dates");
}

void excel_parser::init_ticker(const xlsxioreadersheet sheet, ticker_json_t *j)
{
	char *cell_value;

	//index must be in the next line
	if (xlsxioread_sheet_next_row(sheet) &&
			(cell_value = xlsxioread_sheet_next_cell(sheet)) &&
			this->issdigit(cell_value))
		{
			this->ticker_index = (size_t)std::stol(cell_value);
			this->flags |= F_START;
			init_date(sheet, j);
		}
	else
		handle_fatal_error("In the format of the sheet while reading ticker");
}