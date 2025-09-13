#define DF_DEBUG_LEVEL 8
#include "classes/dataframe.cpp"
#include "vector.cpp"

int main(int argc, char** argv) {
	df_dataframe_t df = {
		{"index", df_range_int32(1, 6)},	// INT32[5] of {1, 2, 3, 4, 5}
		{"date", df_range_date("2000-1-1", "2005-1-1", "1 years")},	// DATE[5] of {2000-01-01 00:00, ...}
		{"text", df_column_text_t{"kazuki", "dataframe", "c++", "minecraft", "hello world"}},	// typed-column, having faster construct
		{"pi", {3.14, 314, "3.14", 3.1415f, 3.1415}}	// default consturct able to use any type if able to convert, and typed by first object type
	};

	// print dataframe
	std::cout << " === Data Frame ===\n";
	std::cout << df << "\n";

	// set ["text"][4] as text, << will cast any type to dest type
	df["text"][4] << "next";

	// range each row
	for (df_row_t& row : df.rows()) {		// df_row_t is a lazy row, it wouldn't iterate useless columns
		std::cout << row["date"] << "   " << row["text"] << "   " << row["pi"] << "\n";

        row.begin();
		std::cout << row << "\n";	// print row
	}

	// get row at 4
	df.row(4)["date"] << "2025-09-10 12:00";	// set ["date"][4] as 2025-09-10 12:00 (auto convert to its type: DATE);

	// print dataframe again
	std::cout << df << "\n";
	return 0;
}