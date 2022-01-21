/*
* Loads all models from the data. Doing this here made it more
* convenient to alter the models for testing and helped prevent 
* circular references with the #includes and reduced compile 
* time.
* However, I'm not happy with the existance of this file, as I
* feel that there is a better method
*/

#include "global.h"
#include "amt.h"
#include "behaviour.h"
#include "settings.h"
#include "localization.h"
#include "particles.h"
#include "animation.h"
#include "menus.h"

using namespace std;
using namespace std::filesystem;

namespace amt {
	vector<unique_ptr<IModel>> GetModelList() { //Models to load
		vector<unique_ptr<IModel>> model;
		model.push_back(unique_ptr<IModel>(new BehaviourModel()));
		model.push_back(unique_ptr<IModel>(new Settings::Model()));
		model.push_back(unique_ptr<IModel>(new Localization::Model()));
		model.push_back(unique_ptr<IModel>(new Particles::Model()));
		model.push_back(unique_ptr<IModel>(new Animation::Model()));
		model.push_back(unique_ptr<IModel>(new Menus::Model()));
		return model;
	}
}
