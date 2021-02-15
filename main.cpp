/*main.cpp*/

// << Anastasiia Evdokimova >>
// U. of Illinois, Chicago
// CS 251: Spring 2020
// 03/23/2020
// Ectra credit project: Covid-19 data analysis program
//
// Covid-19 data analysis program
//


#include <fstream>
#include <sstream>
#include <experimental/filesystem>
#include <locale>
#include <algorithm>
#include "countryMap.h"

using namespace std;
namespace fs = std::experimental::filesystem;


//
// getFilesWithinFolder
//
// Given the path to a folder, e.g. "./daily_reports/", returns 
// a vector containing the full pathnames of all regular files
// in this folder.  If the folder is empty, the vector is empty.
//
vector<string> getFilesWithinFolder(string folderPath)
{
	vector<string> files;

	for (const auto& entry : fs::directory_iterator(folderPath))
	{
		files.push_back(entry.path().string());
	}
	// let's make sure the files are in alphabetical order, so we
	// process them in the correct order by date:
	sort(files.begin(), files.end());

	return files;
}

/*
 * Display help for commands
 * */
void helpMenu() 
{
	cout << "Available commands:" << endl;
	cout << " <name>: enter a country name such as US or China" << endl;
	cout << " countries: list all countries and most recent reports" << endl;
	cout << " top10: list of top 10 countries based on most recent # of confirmed cases" << endl;
	cout << " totals: world-wide totals of confirmed, deaths, recovered" << endl;
	cout << " infection%: shows all countries and % of confirmed cases to total population (based on the most recent report)" << endl;

}

/*
 * Bring all report dates to one format
 * */
void convertDate(string& last_update)
{
	size_t Tpos = last_update.find('T');
	if (Tpos != string::npos) // means date format was like 2020-03-17T23:33:03
	{
		last_update.erase(Tpos); // remove time
		string year;
		year.assign(last_update, (size_t)0, (size_t)4); // I hope we don't need this to support year 10000
		last_update.erase(0, 5); // remove "2020-"
		last_update.append(year.insert(0, "-")); // put -2020 in the end
	}
	else //it was in the old format 2/1/2020 1:52
	{
		size_t mounthDaySeparator, dayYearSeparator;
		mounthDaySeparator = last_update.find('/');
		dayYearSeparator = last_update.find_first_of('/', mounthDaySeparator + 1);
		last_update[mounthDaySeparator] = '-';
		last_update[dayYearSeparator] = '-';
		if (mounthDaySeparator <= 1) // if the mounth number was less than 10
		{
			last_update.insert(0, "0"); // add 0 to the begining
			mounthDaySeparator += 1;
			dayYearSeparator += 1;
		}
		if (dayYearSeparator <= 4) // if day number was less than 10
		{
			last_update.insert(mounthDaySeparator + 1, "0"); // add 0 after at its start
		}
		size_t blankPos = last_update.find(' ');
		last_update.erase(blankPos, last_update.size() - blankPos);
		if (last_update.size() - dayYearSeparator < 4)
			last_update.append("20");
	}
}

/*
 * Read information from report at the path of filename
 * */
void readOneDailyReport(string filename, countryMap& cmap)
{
	ifstream infile(filename);
	string line;
	getline(infile, line);
	string reportDate = "";
	cmap.newReport(); // cleans old top10 and world totals
	while (getline(infile, line))
	{
		if (line[0] == '"')
			// => province is "city, state"
		{
			//
			// we want to delete the " on either end of the value, and
			// delete the ',' embedded within => will get city state:
			//
			line.erase(0, 1);
			// delete the leading "
			size_t pos = line.find(',');
			// find embedded ','
			line.erase(pos, 1);
			// delete ','
			pos = line.find('"');
			// find closing "
			line.erase(pos, 1);
			// delete closing "
		}
		stringstream s(line);
		string province, country, last_update;
		string confirmed, deaths, recovered;
		getline(s, province, ',');
		getline(s, country, ',');
		getline(s, last_update, ',');
		getline(s, confirmed, ',');
		getline(s, deaths, ',');
		getline(s, recovered, ',');
		if (country == "Mainland China")
			// map to name in the earlier reports:
			country = "China";
		if (reportDate == "")
		{
			convertDate(last_update);
			reportDate = last_update; // so we won't have to do this for all entries (reports are daily so they share the date anyways)
		}
		else
		{
			last_update = reportDate;
		}
		unsigned long int conf, df, rec;
		conf = (confirmed == "") ? 0 : stoul(confirmed);
		df = (deaths == "") ? 0 : stoul(deaths);
		rec = (recovered == "") ? 0 : stoul(recovered);
		cmap.insert(country, last_update, conf, df, rec);
	}
}

/*
 * Import world facts from files in folder "world facts"
 * */
void importWorldFacts(string filename, countryMap& cmap)
{
	ifstream infile(filename);
	string line;
	// the format of world facts files is similar, so this function will work for both
	bool isPopulation = false; // to distinguish between the two
	getline(infile, line);
	if (filename.find("populations.csv") != string::npos) isPopulation = true;
	while (getline(infile, line))
	{
		size_t pos = line.find(',');
		line.erase(0, pos + 1); // delete number and first ','
		stringstream s(line);
		string country, data;
		getline(s, country, ',');
		getline(s, data);
		if (isPopulation)
		{
			unsigned long int population = (data == "") ? 0 : stoul(data);
			cmap.worldFactsUpdate(country, population);
		}
		else
		{
			float lifeExpectancy = (data == "") ? 0 : stof(data);
			cmap.worldFactsUpdate(country, lifeExpectancy);
		}
	}
}
//
// main:
//
int main()
{
	cout << "** COVID-19 Data Analysis **" << endl;
	cout << endl;
	cout << "Based on data made available by John Hopkins University" << endl;
	cout << "https://github.com/CSSEGISandData/COVID-19" << endl;
	cout << endl;
	//
	// setup cout to use thousands separator, and 2 decimal places:
	//
	cout.imbue(std::locale(""));
	cout << std::fixed;
	cout << std::setprecision(2);

	//
	// get a vector of the daily reports, one filename for each:
	//
	vector<string> files = getFilesWithinFolder("./daily_reports/");
	countryMap cmap;


	for (string f : files)
	{
		cmap.newReport();
		readOneDailyReport(f, cmap);
	}

	cout << ">> Processed " << files.size() << " daily reports" << endl;

	// after all countries mentiond in reports are in the map, add worldfacts to them
	files = getFilesWithinFolder("./worldfacts/");

	for (string f : files)
	{
		importWorldFacts(f, cmap);
	}

	cout << ">> Processed " << files.size() << " files of world facts" << endl;
	cout << ">> Current data on " << cmap.getSize() << " countries" << endl;


	string command;
	cout << endl << "Enter command (help for list, # to quit)> ";
	getline(cin, command);
	// since # is a quit command, loop until we encounter it
	while (command != "#")
	{
		if (command == "help")
		{
			helpMenu();
		}
		else if (command == "countries")
		{
			cmap.printAllCountries();
		}
		else if (command == "top10")
		{
			cmap.printTop10();
		}
		else if (command == "totals")
		{
			cmap.printTotals();
		}
		else if (command == "infection%")
		{
			cmap.printCountriesByContaminationRate();
		}
		else if (command[0] >= 'A' && command[0] <= 'Z') // all valid countries start with a capital letter anyways =)
		{
			cmap.printCountryInfo(command);
		}
		else
		{
			cout << "country or command not found..." << endl;
		}
		cout << endl << "Enter command> ";
		getline(cin, command);
	}
	return 0;
}
