#include <script/modules/ObjectModule.hpp>

#include <script/ScriptMachine.hpp>
#include <script/SCMFile.hpp>
#include <engine/GameWorld.hpp>

#include <objects/InstanceObject.hpp>
#include <objects/VehicleObject.hpp>
#include <objects/CharacterObject.hpp>

#include <render/Model.hpp>
#include <engine/Animator.hpp>
#include <engine/GameWorld.hpp>
#include <engine/GameWorld.hpp>
#include <ai/PlayerController.hpp>
#include <ai/DefaultAIController.hpp>

#include <data/CutsceneData.hpp>
#include <data/Skeleton.hpp>
#include <objects/CutsceneObject.hpp>

#include <glm/gtx/string_cast.hpp>

#include <iostream>
#include <algorithm>

glm::vec3 spawnMagic( 0.f, 0.f, 1.f );

void game_create_player(const ScriptArguments& args)
{
	auto id	= args[0].integer;
	glm::vec3 position(args[1].real, args[2].real, args[3].real);
	
	if( position.z < -99.f ) {
		position = args.getVM()->getWorld()->getGroundAtPosition(position);
	}
	
	auto pc = args.getVM()->getWorld()->createPedestrian(1, position + spawnMagic);
	args.getVM()->getWorld()->state.player = new PlayerController(pc);
	
	*args[4].handle = args.getVM()->getWorld()->state.player;
}

void game_set_character_position(const ScriptArguments& args)
{
	auto controller = (CharacterController*)(*args[0].handle);
	glm::vec3 position(args[1].real, args[2].real, args[3].real + 1.f);
	controller->getCharacter()->setPosition(position + spawnMagic);
}

bool game_player_in_area_2d(const ScriptArguments& args)
{
	auto controller = (CharacterController*)(*args[0].handle);
	glm::vec2 min(args[1].real, args[2].real);
	glm::vec2 max(args[3].real, args[4].real);
	auto player = controller->getCharacter()->getPosition();
	if( player.x > min.x && player.y > min.y && player.x < max.x && player.y < max.y ) {
		return true;
	}
	return false;
}

bool game_player_in_area_3d(const ScriptArguments& args)
{
	auto controller = (CharacterController*)(*args[0].handle);
	glm::vec3 min(args[1].real, args[2].real, args[3].real);
	glm::vec3 max(args[4].real, args[5].real, args[6].real);
	auto player = controller->getCharacter()->getPosition();
	if( player.x > min.x &&
		player.y > min.y &&
		player.z > min.z &&
		player.x < max.x &&
		player.y < max.y &&
		player.z < max.z) {
		return true;
		}
		return false;
}

void game_create_character(const ScriptArguments& args)
{
	auto type = args[0].integer;
	auto id	= args[1].integer;
	glm::vec3 position(args[2].real, args[3].real, args[4].real);
	
	if( type == 21 ) {
		
	}
	if( position.z < -99.f ) {
		position = args.getVM()->getWorld()->getGroundAtPosition(position);
	}
	
	// If there is already a chracter less than this distance away, it will be destroyed.
	const float replaceThreshold = 2.f;
	for( auto it = args.getVM()->getWorld()->objects.begin();
		it != args.getVM()->getWorld()->objects.end();
	++it)
		{
			if( glm::distance(position, (*it)->getPosition()) < replaceThreshold )
			{
				std::cout << (*it)->type() << std::endl;
			}
			if( (*it)->type() == GameObject::Character && glm::distance(position, (*it)->getPosition()) < replaceThreshold )
			{
				args.getVM()->getWorld()->destroyObjectQueued(*it);
			}
		}
		
		
		auto character = args.getVM()->getWorld()->createPedestrian(id, position + spawnMagic);
		auto controller = new DefaultAIController(character);
		
		if ( args.getThread()->isMission )
		{
			args.getVM()->getWorld()->state.missionObjects.push_back(character);
		}
		
		*args[5].handle = controller;
}

void game_destroy_character(const ScriptArguments& args)
{
	auto controller = static_cast<CharacterController*>(*args[0].handle);
	
	if ( controller )
	{
		args.getVM()->getWorld()->destroyObjectQueued(controller->getCharacter());
	}
}

void game_create_vehicle(const ScriptArguments& args)
{
	auto id	= args[0].integer;
	glm::vec3 position(args[1].real, args[2].real, args[3].real);
	position += spawnMagic;
	
	// If there is already a vehicle less than this distance away, it will be destroyed.
	const float replaceThreshold = 1.f;
	for( auto it = args.getVM()->getWorld()->objects.begin();
		it != args.getVM()->getWorld()->objects.end();
	++it)
		{
			if( (*it)->type() == GameObject::Vehicle && glm::distance(position, (*it)->getPosition()) < replaceThreshold )
			{
				args.getVM()->getWorld()->destroyObjectQueued(*it);
			}
		}
		
		auto vehicle = args.getVM()->getWorld()->createVehicle(id, position);
		
		if ( args.getThread()->isMission )
		{
			args.getVM()->getWorld()->state.missionObjects.push_back(vehicle);
		}
		
		*args[4].handle = vehicle;
}

void game_destroy_vehicle(const ScriptArguments& args)
{
	auto vehicle = static_cast<VehicleObject*>(*args[0].handle);
	
	args.getVM()->getWorld()->destroyObjectQueued(vehicle);
}

void game_get_vehicle_position(const ScriptArguments& args)
{
	auto vehicle = static_cast<VehicleObject*>(*args[0].handle);
	
	if( vehicle )
	{
		auto vp = vehicle->getPosition();
		*args[1].globalReal = vp.x;
		*args[2].globalReal = vp.y;
		*args[3].globalReal = vp.z;
	}
}

bool game_character_in_vehicle(const ScriptArguments& args)
{
	auto controller = static_cast<CharacterController*>(*args[0].handle);
	auto vehicle = static_cast<VehicleObject*>(*args[1].handle);
	
	if( controller == nullptr || vehicle == nullptr )
	{
		return false;
	}
	
	return controller->getCharacter()->getCurrentVehicle() == vehicle;
}

bool game_character_in_model(const ScriptArguments& args)
{
	auto vdata = args.getVM()->getWorld()->findObjectType<VehicleData>(args[1].integer);
	if( vdata )
	{
		auto controller = (CharacterController*)(*args[0].handle);
		auto character = controller->getCharacter();
		auto vehicle = character->getCurrentVehicle();
		if ( vehicle ) {
			
			return vehicle->model && vdata->modelName == vehicle->model->name;
		}
	}
	return false;
}

bool game_character_in_any_vehicle(const ScriptArguments& args)
{
	auto controller = static_cast<CharacterController*>(*args[0].handle);
	
	auto vehicle = controller->getCharacter()->getCurrentVehicle();
	return vehicle != nullptr;
}

bool game_player_in_area_2d_in_vehicle(const ScriptArguments& args)
{
	auto character = static_cast<CharacterController*>(*args[0].handle);
	glm::vec2 position(args[1].real, args[2].real);
	glm::vec2 radius(args[3].real, args[4].real);
	bool show = args[5].integer;
	
	if( character->getCharacter()->getCurrentVehicle() == nullptr )
	{
		return false;
	}
	
	auto vp = character->getCharacter()->getCurrentVehicle()->getPosition();
	glm::vec2 distance = glm::abs(position - glm::vec2(vp));
	
	if(distance.x <= radius.x && distance.y <= radius.y)
	{
		return true;
	}
	
	return false;
}

bool game_character_near_point_in_vehicle(const ScriptArguments& args)
{
	auto controller = static_cast<CharacterController*>(*args[0].handle);
	glm::vec3 center(args[1].real, args[2].real, args[3].real);
	glm::vec3 size(args[4].real, args[5].real, args[6].real);
	bool unkown	= !!args[7].integer;
	
	auto vehicle = controller->getCharacter()->getCurrentVehicle();
	if( vehicle ) {
		auto distance = center - controller->getCharacter()->getPosition();
		distance /= size;
		if( glm::length( distance ) < 1.f ) return true;
	}
	
	return false;
}

bool game_character_dead(const ScriptArguments& args)
{
	auto controller = static_cast<CharacterController*>(*args[0].handle);
	
	if ( controller )
	{
		return !controller->getCharacter()->isAlive();
	}
	return true;
}

bool game_vehicle_dead(const ScriptArguments& args)
{
	auto controller = static_cast<VehicleObject*>(*args[0].handle);
	return controller == nullptr;
}

bool game_character_in_zone(const ScriptArguments& args)
{
	auto controller = static_cast<CharacterController*>(*args[0].handle);
	std::string zname(args[1].string);
	
	auto zfind = args.getVM()->getWorld()->gameData.zones.find(zname);
	if( zfind != args.getVM()->getWorld()->gameData.zones.end() ) {
		auto player = controller->getCharacter()->getPosition();
		auto& min = zfind->second.min;
		auto& max = zfind->second.max;
		if( player.x > min.x && player.y > min.y && player.z > min.z &&
			player.x < max.x && player.y < max.y && player.z < max.z ) {
			return true;
			}
	}
	
	return false;
}

void game_create_character_in_vehicle(const ScriptArguments& args)
{
	auto vehicle = static_cast<VehicleObject*>(*args[0].handle);
	auto type = args[1].integer;
	auto id = args[2].integer;
	
	auto character = args.getVM()->getWorld()->createPedestrian(id, vehicle->getPosition() + spawnMagic);
	auto controller = new DefaultAIController(character);
	
	character->setCurrentVehicle(vehicle, 0);
	vehicle->setOccupant(0, character);
	
	*args[3].handle = controller;
}

void game_set_character_heading(const ScriptArguments& args)
{
	auto controller = (CharacterController*)(*args[0].handle);
	controller->getCharacter()->setHeading(args[1].real);
}

void game_get_character_heading(const ScriptArguments& args)
{
	auto vehicle = static_cast<VehicleObject*>(*args[0].handle);
	
	if ( vehicle )
	{
		*args[1].globalReal = 0.f;
	}
}

void game_set_vehicle_heading(const ScriptArguments& args)
{
	auto vehicle = (VehicleObject*)(*args[0].handle);
	vehicle->setHeading(args[1].real);
}

void game_set_object_heading(const ScriptArguments& args)
{
	auto inst = (InstanceObject*)(*args[0].handle);
	inst->setHeading(args[1].real);
}

bool game_vehicle_stopped(const ScriptArguments& args)
{
	auto vehicle = static_cast<VehicleObject*>(*args[0].handle);
	
	if( vehicle )
	{
		return std::abs( vehicle->physVehicle->getCurrentSpeedKmHour() ) <= 0.01f;
	}
	return false;
}

void game_make_object_important(const ScriptArguments& args)
{
	auto inst = (InstanceObject*)(*args[0].handle);
	std::cout << "Unable to pin object " << inst << ". Object pinning unimplimented" << std::endl;
}

bool game_character_in_area_on_foot(const ScriptArguments& args)
{
	/// @todo
	return false;
}

bool game_character_stoped_in_volume_in_vehicle(const ScriptArguments& args)
{
	auto controller = static_cast<CharacterController*>(*args[0].handle);
	
	if( controller && controller->getCharacter()->getCurrentVehicle() != nullptr )
	{
		glm::vec3 min(args[1].real, args[2].real, args[3].real);
		glm::vec3 max(args[4].real, args[5].real, args[6].real);
		
		glm::vec3 pp = controller->getCharacter()->getCurrentVehicle()->getPosition();
		
		if( pp.x >= min.x && pp.y >= min.y && pp.z >= min.z &&
			pp.x <= max.x && pp.y <= max.y && pp.z <= max.z )
		{
			return controller->getCharacter()->getCurrentVehicle()->physVehicle->getCurrentSpeedKmHour() < 0.1f;
		}
	}
	
	return false;
}

bool game_character_in_area_9(const ScriptArguments& args)
{
	return false;
}

bool game_player_in_taxi(const ScriptArguments& args)
{
	auto controller = static_cast<CharacterController*>(*args[0].handle);
	
	auto vehicle = controller->getCharacter()->getCurrentVehicle();
	return vehicle && (vehicle->vehicle->classType & VehicleData::TAXI) == VehicleData::TAXI;
}

void game_get_speed(const ScriptArguments& args)
{
	auto vehicle = static_cast<VehicleObject*>(*args[0].handle);
	if( vehicle )
	{
		*args[1].globalReal = vehicle->physVehicle->getCurrentSpeedKmHour();
	}
}

void game_enter_as_passenger(const ScriptArguments& args)
{
	auto controller = (CharacterController*)(*args[0].handle);
	auto vehicle = (VehicleObject*)(*args[1].handle);
	/// @todo find next lowest free seat.
	controller->setNextActivity(new Activities::EnterVehicle(vehicle,1));
}

bool game_vehicle_flipped(const ScriptArguments& args)
{
	auto vehicle = static_cast<VehicleObject*>(*args[0].handle);
	
	if( vehicle )
	{
		return vehicle->isFlipped();
	}
	
	return false;
}

bool game_vehicle_in_air(const ScriptArguments& args)
{
	/// @todo IS vehicle in air.
	auto vehicle = (VehicleObject*)(*args[0].handle);
	return false;	
}

bool game_character_near_car_2d(const ScriptArguments& args)
{
	auto controller = (CharacterController*)(*args[0].handle);
	auto vehicle = (VehicleObject*)(*args[1].handle);
	glm::vec2 radius(args[2].real, args[3].real);
	bool drawMarker = !!args[4].integer;
	
	auto charVehicle = controller->getCharacter()->getCurrentVehicle();
	if( charVehicle ) {
		auto dist = charVehicle->getPosition() - vehicle->getPosition();
		if( dist.x <= radius.x && dist.y <= radius.y ) {
			return true;
		}
	}
	
	return false;
}

void game_set_vehicle_colours(const ScriptArguments& args)
{
	auto vehicle = (VehicleObject*)(*args[0].handle);
	
	auto& colours = args.getVM()->getWorld()->gameData.vehicleColours;
	vehicle->colourPrimary = colours[args[1].integer];
	vehicle->colourSecondary = colours[args[2].integer];
}

void game_create_object_world(const ScriptArguments& args)
{
	int id = 0;
	switch(args[0].type) {
		case TInt8:
			id = (std::int8_t)args[0].integer;
			break;
		case TInt16:
			id = (std::int16_t)args[0].integer;
			break;
	}
	
	if( id < 0 ) {
		auto& modelname = args.getVM()->getFile()->getModels()[-id];
		id = args.getVM()->getWorld()->findModelDefinition(modelname);
		if( id == (uint16_t)-1 ) {
			std::cerr << "Failed to find model: " << modelname << std::endl;
		}
	}
	
	auto& object = args.getVM()->getWorld()->objectTypes[id];
	glm::vec3 position(args[1].real, args[2].real, args[3].real);
	
	auto inst = args.getVM()->getWorld()->createInstance(object->ID, position);
	
	*args[4].handle = inst;
}
bool game_is_boat(const ScriptArguments& args)
{
	/*auto vehicle = (VehicleObject*)(*args[0].handle);
	 *	if( vehicle )
	 *	{
	 *		return vehicle->vehicle->type == VehicleData::BOAT;
}*/
	return false;
}

void game_change_nearest_model(const ScriptArguments& args)
{
	glm::vec3 position(args[0].real, args[1].real, args[2].real);
	float radius = args[3].real;
	int newid = 0, oldid = 0;
	
	/// @todo fix this being a problem.
	switch(args[4].type) {
		case TInt8:
			oldid = (std::int8_t)args[4].integer;
			break;
		case TInt16:
			oldid = (std::int16_t)args[4].integer;
			break;
	}
	
	switch(args[5].type) {
		case TInt8:
			newid = (std::int8_t)args[5].integer;
			break;
		case TInt16:
			newid = (std::int16_t)args[5].integer;
			break;
	}
	
	if( std::abs(newid) > 178 || std::abs(oldid) > 178 ) {
		/// @todo implement this path,
		return;
	}
	
	std::string newmodel;
	std::string oldmodel;
	
	if(newid < 0) newid = -newid;
	if(oldid < 0) oldid = -oldid;
	
	newmodel = args.getVM()->getFile()->getModels()[newid];
	oldmodel = args.getVM()->getFile()->getModels()[oldid];
	std::transform(newmodel.begin(), newmodel.end(), newmodel.begin(), ::tolower);
	std::transform(oldmodel.begin(), oldmodel.end(), oldmodel.begin(), ::tolower);
	
	auto newobjectid = args.getVM()->getWorld()->findModelDefinition(newmodel);
	auto nobj = args.getVM()->getWorld()->findObjectType<ObjectData>(newobjectid);
	
	/// @todo Objects need to adopt the new object ID, not just the model.
	for(auto o : args.getVM()->getWorld()->objects) {
		if( o->type() == GameObject::Instance ) {
			if( !o->model ) continue;
			if( o->model->name != oldmodel ) continue;
			float d = glm::distance(position, o->getPosition());
			if( d < radius ) {
				args.getVM()->getWorld()->gameData.loadDFF(newmodel + ".dff", false);
				InstanceObject* inst = static_cast<InstanceObject*>(o);
				inst->changeModel(nobj);
				inst->model = args.getVM()->getWorld()->gameData.models[newmodel];
			}
		}
	}
}

void game_get_vehicle_colours(const ScriptArguments& args)
{
	auto vehicle = static_cast<VehicleObject*>(*args[0].handle);
	
	if ( vehicle )
	{
		/// @TODO use correct values.
		*args[1].globalInteger = 0;
		*args[2].globalInteger = 0;
	}
}


ObjectModule::ObjectModule()
: ScriptModule("Object")
{
	bindFunction(0x0053, game_create_player, 5, "Create Player" );
	
	bindFunction(0x0055, game_set_character_position, 4, "Set Player Position" );
	bindFunction(0x0056, game_player_in_area_2d, 6, "Is Player In Area 2D" );
	bindFunction(0x0057, game_player_in_area_3d, 8, "Is Player In Area 3D" );
	
	bindFunction(0x009A, game_create_character, 6, "Create Character" );
	bindFunction(0x009B, game_destroy_character, 1, "Destroy Character" );
	
	bindFunction(0x00A5, game_create_vehicle, 5, "Create Vehicle" );
	bindFunction(0x00A6, game_destroy_vehicle, 1, "Destroy Vehicle" );
	
	bindFunction(0x00AA, game_get_vehicle_position, 4, "Get Vehicle Position" );
	
	bindFunction(0x00DB, game_character_in_vehicle, 2, "Is Character in Vehicle" );
	bindFunction(0x00DC, game_character_in_vehicle, 2, "Is Player in Vehicle" );
	
	bindFunction(0x00DE, game_character_in_model, 2, "Is Player In Model" );
	
	bindFunction(0x00E0, game_character_in_any_vehicle, 1, "Is Player In Any Vehicle" );
	
	bindFunction(0x00E5, game_player_in_area_2d_in_vehicle, 6, "Is Player in 2D Area in Vehicle" );
	
	bindUnimplemented( 0x00E4, game_locate_character_on_foot_2d, 6, "Locate Player on foot 2D" );
	
	bindUnimplemented( 0x00F5, game_locate_character_in_sphere, 8, "Locate Player In Sphere" );
	
	bindFunction(0x0100, game_character_near_point_in_vehicle, 8, "Is Character near point in car" );
	
	bindFunction(0x0118, game_character_dead, 1, "Is Character Dead" );
	bindFunction(0x0119, game_vehicle_dead, 1, "Is Vehicle Dead" );
	
	bindFunction(0x0121, game_character_in_zone, 2, "Is Player In Zone" );
	
	bindFunction(0x0129, game_create_character_in_vehicle, 4, "Create Character In Car" );
	
	bindFunction(0x0171, game_set_character_heading, 2, "Set Player Heading" );
	
	bindFunction(0x0173, game_set_character_heading, 2, "Set Character Heading" );
	bindFunction(0x0174, game_get_character_heading, 2, "Get Vehicle Heading" );
	
	bindFunction(0x0175, game_set_vehicle_heading, 2, "Set Vehicle heading" );
	
	bindFunction(0x0177, game_set_object_heading, 2, "Set Object heading" );
	
	bindFunction(0x019C, game_character_in_area_on_foot, 8, "Is Player in Area on Foot" );
	
	bindFunction(0x01A0, game_character_stoped_in_volume_in_vehicle, 8, "Is Player Stopped in cube in vehicle" );
	bindFunction(0x01AA, game_character_stoped_in_volume_in_vehicle, 8, "Is Char Stopped in cube in vehicle" );
	
	bindUnimplemented( 0x01BB, game_object_coordinates, 4, "Get Object Coordinates" );
	
	bindUnimplemented( 0x01BE, game_turn_character, 4, "Turn Character To Face Point" );
	
	bindFunction(0x01C1, game_vehicle_stopped, 1, "Is Vehicle Stopped" );
	
	bindUnimplemented( 0x01C3, game_release_vehicle, 1, "Mark Car Unneeded" );
	
	bindFunction(0x01C7, game_make_object_important, 1, "Don't remove object" );
	
	bindFunction(0x01D4, game_enter_as_passenger, 2, "Character Enter Vehicle as Passenger" );
	
	bindFunction(0x01F3, game_vehicle_in_air, 1, "Is Vehicle In Air" );
	bindFunction(0x01F4, game_vehicle_flipped, 1, "Is Car Flipped" );
	
	bindFunction(0x0204, game_character_near_car_2d, 5, "Is Char near Car in Car 2D" );
	
	bindUnimplemented( 0x020A, game_lock_vehicle_doors, 2, "Lock Car Doors" );
	
	bindFunction(0x0229, game_set_vehicle_colours, 3, "Set Vehicle Colours" );
	
	bindFunction(0x029B, game_create_object_world, 5, "Create Object no offset" );
	bindFunction(0x029C, game_is_boat, 1, "Is Vehicle Boat" );
	
	bindFunction(0x02B3, game_character_in_area_9, 9, "Is Player In Area" );
	
	bindFunction(0x02DE, game_player_in_taxi, 1, "Is Player In Taxi" );
	
	bindFunction(0x02E3, game_get_speed, 2, "Get Vehicle Speed" );
	
	bindUnimplemented( 0x02FB, game_create_crane, 10, "Create Crusher Crane" );
	
	bindUnimplemented( 0x034D, game_rotate_object, 4, "Rotate Object" );
	bindUnimplemented( 0x034E, game_slide_object, 8, "Slide Object" );
	
	bindUnimplemented( 0x035D, game_set_object_targetable, 1, "Set Object Targetable" );
	
	bindUnimplemented( 0x0363, game_set_close_object_visible, 6, "Set Closest Object Visibility" );
	
	bindUnimplemented( 0x0368, game_create_ev_crane, 10, "Create ev Crane" );
	
	bindFunction(0x03B6, game_change_nearest_model, 6, "Change Nearest Instance Model" );
	
	bindUnimplemented( 0x03BA, game_clear_area_vehicles, 6, "Clear Cars From Area" );
	
	bindFunction(0x03F3, game_get_vehicle_colours, 3, "Get Vehicle Colours" );
	
	bindFunction(0x0442, game_character_in_vehicle, 2, "Is Player in This Vehicle" );
	bindFunction(0x0448, game_character_in_vehicle, 2, "Is Character in This Vehicle" );
}