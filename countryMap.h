/*countryMap.h*/

// << Anastasiia Evdokimova >>
// U. of Illinois, Chicago
// CS 251: Spring 2020
// 03/23/2020
// Ectra credit project: Covid-19 data analysis program
//
// Covid-19 data analysis program
//

#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <iomanip>


using namespace std;
class countryMap {
private:

	struct countryInfo {
		vector<unsigned long int> confirmedCases; // vectors, since we will have a timeline
		vector<unsigned long int> deaths;
		vector<unsigned long int> recovered;
		vector<string> timeline; // stores report dates
		unsigned long int population;
		float lifeExpectancy;
	};

	struct top10Info {
		string countryName;
		unsigned long int confirmedCases;
		top10Info* next; // linked list time. Lots of inserts (as we fill the map) and used only to get the whole list
	};

	struct contaminationRateSortNode {
		string countryName;
		double contaminationRate;
		unsigned long int population;
		contaminationRateSortNode* next;
	};



	top10Info* top10 = nullptr;
	unsigned long int minTop10 = 0; // store this and check so that we don't have to traverse the top10 with each insert
	int top10Counter = 0; //to know if we should throw away the last item in the top 10 list
	string lastUpdate;

	unsigned long int worldCoronaData[3]; // for world totals

	map<string, countryInfo> cmap;
	unsigned short int size = 0;
	/*
	 * adds new entry to top10 and throws away entries after 10th
	 * */
	void _addToTop10(string countryName, unsigned long int confirmedCases) {
		top10Info* newData = new top10Info;
		newData->countryName = countryName;
		newData->confirmedCases = confirmedCases;
		top10Info* temp = top10;
		top10Info* prev = nullptr;
		while (temp != nullptr && temp->confirmedCases > confirmedCases)
		{
			prev = temp;
			temp = temp->next;
		}
		if (prev == nullptr)
		{
			top10 = newData;
		}
		else
		{
			prev->next = newData;
		}
		newData->next = temp; // if the iteration never begun because it was lower, it will store the first element. If the top10 was empty, this will assign nullptr
		if (top10Counter < 10)
		{
			top10Counter++;
			if (newData->next == nullptr) minTop10 = newData->confirmedCases; // if it is the last, update min
		}
		else
		{
			prev = nullptr;
			while (temp->next != nullptr)
			{
				prev = temp;
				temp = temp->next;
			}
			delete(temp); // it is the last element, so no memory leak should happen
			if (prev == nullptr)
			{
				newData->next = nullptr;
				minTop10 = confirmedCases; // confirmedCases becomes the new min
			}
			else
			{
				prev->next = nullptr; // put the stopper here
				minTop10 = prev->confirmedCases; // new min, since we threw the previous one away
			}
		}
	}

	/*
	 * If the country receives the update, it must be removed from top10 by name
	 * */
	void _removeFromTop10(string countryName)
	{
		top10Info* temp = top10;
		top10Info* prev = nullptr;
		while (temp != nullptr && temp->countryName != countryName)
		{
			prev = temp;
			temp = temp->next;
		}
		if (temp != nullptr)
		{
			if (prev == nullptr)
			{
				top10 = temp->next;
				delete(temp);
			}
			else
			{
				prev->next = temp->next;
				delete(temp);
			}
			top10Counter--; // so that when it makes it back everything is alright
		}
	}

	/*
	 * Helper for printCountryInfo. Prints the timeline for the chosen parameter
	 * */
	void _printTimeline(map<string, countryInfo>::iterator it, char whatToPrint)
	{
		unsigned int firstEntryIndex = 0;
		// for deaths and recoveries, we need to find the first non-zero index
		if (whatToPrint == 'd')
		{
			cout << "Deaths:" << endl;
			if (it->second.deaths.back() == 0) return; // nothing to print
			while (it->second.deaths[firstEntryIndex] == 0) ++firstEntryIndex;
		}
		else if (whatToPrint == 'r')
		{
			if (it->second.recovered.back() == 0) return; // nothing to print then
			while (it->second.recovered[firstEntryIndex] == 0) ++firstEntryIndex;
			cout << "Recovered:" << endl;
		}
		else
		{
			cout << "Confirmed:" << endl;
		}
		// if we need to only display the first and last 7 elements, this will help to determine where to start the loop
		unsigned int sizeController = ((it->second.timeline.size() - firstEntryIndex) > 14) ? it->second.timeline.size() - 7 : firstEntryIndex;
		if (sizeController != firstEntryIndex) // for those longer than 14, this loop will display first 7 elements
		{
			for (unsigned int i = firstEntryIndex; i < firstEntryIndex + 7; i++)
			{
				cout << it->second.timeline[i] << " (day " << i + 1 << "): "; // +1 since indexes start with 0
				switch (whatToPrint)
				{
				case 'c': cout << it->second.confirmedCases[i] << endl; break;
				case 'd': cout << it->second.deaths[i] << endl; break;
				case 'r': cout << it->second.recovered[i] << endl; break;
				}
			}
			cout << " ." << endl << " ." << endl << " ." << endl;
		}
		// for timelines shorter than 14, this will start from 0. For others - from the 7th element from the end
		
		for (unsigned int i = sizeController; i < it->second.timeline.size(); i++)
		{
			cout << it->second.timeline[i] << " (day " << i + 1 << "): ";
			switch (whatToPrint)
			{
			case 'c': cout << it->second.confirmedCases[i] << endl; break;
			case 'd': cout << it->second.deaths[i] << endl; break;
			case 'r': cout << it->second.recovered[i] << endl; break;
			}
		}
	}

	/*
	 * Helper function for displayCountryInfo. Returns a string with the date when the first death occured
	 * */
	string _getFirstDeathDate(map<string, countryInfo>::iterator it)
	{
		string returnValue = "none";
		for (unsigned int i = 0; i < it->second.deaths.size(); i++)
		{
			if (it->second.deaths[i] > 0)
			{
				returnValue = it->second.timeline[i]; // sizes of deaths/timeline vectors are the same and they are correlated
				break;
			}
		}
		return returnValue;
	}

	/*
	 * returns true if a is less or equal to b
	 * */
	bool _doubleCompare(double a, double b)
	{
		return (a - b) < 0.0000000001; // we are working with really small numbers here
	}

	/*
	 * Helper for printCountriesByContaminationRate. Inserts country in order
	 * */
	void _insertInOrder(contaminationRateSortNode*& head, string countryName, double contaminationRate, unsigned long int population)
	{
		contaminationRateSortNode* temp = head, * prev = nullptr, * newNode = new contaminationRateSortNode;
		newNode->countryName = countryName;
		newNode->contaminationRate = contaminationRate;
		newNode->population = population;
		while (temp != nullptr && _doubleCompare(contaminationRate, temp->contaminationRate)) // additional function is required because we are comparing doubles
		{
			prev = temp;
			temp = temp->next;
		}
		if (prev == nullptr) // the list had 0 or 1 element
		{
			prev = head;
			head = newNode;
			newNode->next = prev;
		}
		else
		{
			prev->next = newNode;
			newNode->next = temp;
		}
	}

	/*
	 * Deallocates memory for top10 linked list
	 * */
	void _cleanTop10()
	{
		top10Info* temp;
		while (top10 != nullptr)
		{
			temp = top10;
			top10 = top10->next;
			delete(temp);
		}
	}

public:

	/*
	 * Not much to do here, since map is a built-in class
	 * */
	countryMap()
	{
		size = 0;
		top10 = nullptr;
		minTop10 = 0; // store this and check so that we don't have to traverse the top10 with each insert
		top10Counter = 0;
	}

	~countryMap()
	{
		_cleanTop10();
	}

	/*
	 * This should be called for each daily report - so that one country won't make it to top10 several times
	 * and reset world totals
	 * */
	void newReport() {
		_cleanTop10();
		minTop10 = 0;
		top10Counter = 0;
		for (int i = 0; i < 3; i++)
			worldCoronaData[i] = 0;
	}

	/*
	 * Insert the new element into the map. If the element already existed, updates it and returnes false
	 * Also accumulates data for top10 and world totals
	 * */
	bool insert(string countryName, string date, unsigned long int confirmedCases, unsigned long int deaths, unsigned long int recovered) {
		bool returnValue = true;
		map<string, countryInfo>::iterator it;
		it = cmap.find(countryName);
		/* I will leave it here in case other country names get simplified (like Mainland China -> China)
		if (it == cmap.end()) // try searching for names that contain this
		{
			// if it finds something that fully contains the name, now it will point to it
			for (it = cmap.begin(); it != cmap.end(); ++it)
			{
			  if (it->first.find(countryName) != string::npos)
			  {
				  returnValue = false;
				  break;
			  }
			}
		}
		*/
		if (it == cmap.end()) // the first time this entry is encountered
		{
			countryInfo newInfo;
			newInfo.confirmedCases.push_back(confirmedCases);
			newInfo.deaths.push_back(deaths);
			newInfo.recovered.push_back(recovered);
			newInfo.timeline.push_back(date);
			newInfo.population = 0;
			newInfo.lifeExpectancy = 0;
			cmap[countryName] = newInfo;
			size++;
			// cout << "Added new country! " << countryName << " Cases: " << confirmedCases << endl;
		}
		else
		{
			if (it->second.timeline.back() == date)
			{
				it->second.confirmedCases.back() += confirmedCases;
				it->second.deaths.back() += deaths;
				it->second.recovered.back() += recovered;
				// cout << "Updated country for today! " << countryName << " Total cases: " << it->second.confirmedCases.back() << endl;
			}
			else
			{
				/* Used it to deal with Mainland China and similar cases before found out the one case solution in project pdf
				if (returnValue == false) // the only way it can be false at this point is if some key in the map fully contains countryName
				{
					// if so, we need to change the key to a simplier (new) one
					auto mappedData = cmap.extract(it); // ty C++17 for easy key change
					mapData.key() = countryName;
					cmap.insert(move(mapData));
					it = cmap.find(countryName);
				}
				*/
				it->second.confirmedCases.push_back(confirmedCases);
				it->second.deaths.push_back(deaths);
				it->second.recovered.push_back(recovered);
				it->second.timeline.push_back(date);
				// cout << "New day for old country! " << countryName << " Total cases: " << it->second.confirmedCases.back() << endl;
			}
			returnValue = false;
		}
		/*
		 * Update totals here because confirmedCases might be changed later
		 * */
		worldCoronaData[0] += confirmedCases;
		worldCoronaData[1] += deaths;
		worldCoronaData[2] += recovered;
		if (!returnValue)
		{
			confirmedCases = it->second.confirmedCases.back(); // update confirmedCases so they can compete for top10
			_removeFromTop10(countryName);
		}
		if (top10Counter < 10 || confirmedCases > minTop10) // the country is eligible for top10
		{
			_addToTop10(countryName, confirmedCases);
		}
		lastUpdate = date; // the map will know when the last update was
		return returnValue;
	}

	/*
	 * Prints top 10 countries by number of confirmed cases
	 * */
	void printTop10() {
		int counter = 1;
		top10Info* temp = top10;
		while (temp != nullptr)
		{
			cout << counter << ". " << temp->countryName << ": " << temp->confirmedCases << endl;
			temp = temp->next;
			counter++;
		}
	}
	

	/*
	 * Displays the latest data for all countries in alphabetic order
	 * */
	void printAllCountries()
	{
		for (auto it = cmap.begin(); it != cmap.end(); ++it)
		{
			if (it->second.timeline.back() == lastUpdate)
			cout << it->first << ": " << it->second.confirmedCases.back() << ", " << it->second.deaths.back() << ", " << it->second.recovered.back() << endl;
			else
			cout << it->first << ": " << 0 << ", " << 0 << ", " << 0 << endl;
		}
	}

	/*
	 * Updates population data
	 * */
	void worldFactsUpdate(string countryName, unsigned long int population)
	{
		auto it = cmap.find(countryName);
		if (it != cmap.end()) it->second.population = population;
	}
	/*
	 * Updates life Expectancy data
	 * */
	void worldFactsUpdate(string countryName, float lifeExpectancy)
	{
		auto it = cmap.find(countryName);
		if (it != cmap.end()) it->second.lifeExpectancy = lifeExpectancy;
	}

	/*
	 *Returns map size AKA the total number of countries
	 * */
	unsigned short int getSize()
	{
		return size;
	}

	/*
	 * Displays information for one country
	 * */
	void printCountryInfo(string countryName)
	{
		auto it = cmap.find(countryName);
		if (it != cmap.end())
		{
			if (it->second.timeline.back() == lastUpdate)
			{
			cout << "Population: " << it->second.population << endl;
			cout << "Life expectancy: " << it->second.lifeExpectancy << " years" << endl;
			cout << "Latest data:" << endl;
			cout << " confirmed: " << it->second.confirmedCases.back() << endl;
			cout << " deaths: " << it->second.deaths.back() << endl;
			cout << " recovered: " << it->second.recovered.back() << endl;
			cout << "First confirmed case: " << it->second.timeline[0] << endl;
			cout << "First recorded death: " << _getFirstDeathDate(it) << endl;
			string s;
			char command = ' ';
			while (command != 'n')
			{
				cout << "Do you want to see a timeline? Enter c/d/r/n> ";
				getline(cin, s);
				command = (s == "") ? ' ' : s[0];
				if (command == 'c' || command == 'd' || command == 'r')
				{
					_printTimeline(it, command);
					break;
				}
			}
			}
			else
			{
				cout << "Population: " << 0 << endl;
			cout << "Life expectancy: " << 0 << " years" << endl;
			cout << "Latest data:" << endl;
			cout << " confirmed: " << 0 << endl;
			cout << " deaths: " << 0 << endl;
			cout << " recovered: " << 0 << endl;
			cout << "First confirmed case: " << 0 << endl;
			cout << "First recorded death: " << 0 << endl;
			string s;
			char command = ' ';
			while (command != 'n')
			{
				cout << "Do you want to see a timeline? Enter c/d/r/n> ";
				getline(cin, s);
				command = (s == "") ? ' ' : s[0];
				if (command == 'c' || command == 'd' || command == 'r')
				{
					_printTimeline(it, command);
					break;
				}
			}
			}
		}
		else
		{
			cout << "country or command not found..." << endl;
		}
	}

	/*
	 * Displays world totals
	 * */
	void printTotals()
	{
		cout << "As of " << cmap.begin()->second.timeline.back() << ", the world-wide totals are: " << endl;
		cout << " confirmed: " << worldCoronaData[0] << endl;
		cout << setprecision(2) << " deaths: " << worldCoronaData[1] << " (" << (float)worldCoronaData[1] / worldCoronaData[0] * 100 << "%)" << endl;
		cout << setprecision(2) << " recovered: " << worldCoronaData[2] << " (" << (float)worldCoronaData[2] / worldCoronaData[0] * 100 << "%)" << endl;
	}

	/*
	 * Prints countries sorted by the amount of deaths related to the cases
	 * */
	void printCountriesByContaminationRate()
	{
		double contaminationRate;
		// map with contaminationRate as a key could have worked if there weren't any countries with same contaminationRate
		contaminationRateSortNode* head = nullptr, * temp;
		for (auto it = cmap.begin(); it != cmap.end(); ++it)
		{
			if (it->second.population != 0) // if it is 0, there was no population data, so I won't insert it
			{
				contaminationRate = (double)it->second.confirmedCases.back() / it->second.population * 100;
				_insertInOrder(head, it->first, contaminationRate, it->second.population);
			}
		}
		unsigned int i = 0;
		while (head != nullptr)
		{
			temp = head;
			cout << setprecision(6); // we are dealing with small numbers here
			cout << ++i << ". " << head->countryName << ", " << head->contaminationRate << "% out of " << head->population << " people" << endl;
			// immediately frees the linked list memory
			head = head->next;
			delete(temp);
		}
		cout << setprecision(2); // set it back so other functions won't have to
	}
};