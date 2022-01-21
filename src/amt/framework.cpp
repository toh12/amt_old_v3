/*
* Implements the only non-template function from the framework.
*/

#include "framework.h"

using namespace std;
using namespace std::filesystem;
using namespace apcl;

namespace amt {
	vector<Hero> Hero::LoadHeroList() {
		vector<Hero> retval;
		ifstream ifs{ HeroInfoFile };
		if (!ifs) 
			throw FileException("Failed to open the hero info file: " + HeroInfoFile.string());
		string line;
		size_t n, m;
		if(!getline(ifs, line)) 
			throw FileException("Failed to read the hero info file header.");
		while (getline(ifs, line)) {
			if (line.empty()) break;
			Hero hero;
			n = line.find(',');
			m = line.find(',', n + 1);
			hero.name = line.substr(0U, n);
			hero.id = line.substr(n+1, m-(n+1));
			hero.numSkins = stoi(line.substr(m + 1));
			if (hero.id.length() != 2) throw 
				FileException("Invalid class ID length in the hero info file, each class ID must consist of exactly 2 digits.");
			if (hero.numSkins < 1 || hero.numSkins > 19) 
				throw FileException("Invalid number of hero skins in the hero info file, each hero must have between 1 and 19 skins.");
			retval.push_back(move(hero));
		}
		return retval;
	}
}
