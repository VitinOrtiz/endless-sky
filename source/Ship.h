/* Ship.h
Copyright (c) 2014 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program. If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include "Body.h"

#include "Angle.h"
#include "Armament.h"
#include "CargoHold.h"
#include "Command.h"
#include "EsUuid.h"
#include "FireCommand.h"
#include "Outfit.h"
#include "Paragraphs.h"
#include "Personality.h"
#include "Point.h"
#include "Port.h"
#include "ship/Bay.h"
#include "ship/Crew.h"
#include "ship/EnginePoint.h"
#include "ship/ShipAICache.h"
#include "ShipJumpNavigation.h"

#include <array>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

using namespace std;

class Bay;
class ConditionsStore;
class DamageDealt;
class DataNode;
class DataWriter;
class Effect;
class Flotsam;
class FormationPattern;
class Government;
class Minable;
class Phrase;
class Planet;
class PlayerInfo;
class Projectile;
class StellarObject;
class Swizzle;
class System;
class Visual;



// Class representing a ship (either a model for sale or an instance of it). A
// ship's information can be saved to a file, so that it can later be read back
// in exactly the same state. The same class is used for the player's ship as
// for all other ships, so their capabilities are exactly the same  within the
// limits of what the AI knows how to command them to do.
class Ship : public Body, public enable_shared_from_this<Ship> {
public:
	enum class ThrustKind {
		LEFT = 0,
		RIGHT = 1,
		FORWARD = 2,
		REVERSE = 3,
	};

	enum class CanFireResult {
		INVALID,
		NO_AMMO,
		NO_ENERGY,
		NO_FUEL,
		NO_HULL,
		NO_HEAT,
		NO_ION,
		NO_DISRUPTION,
		NO_SLOWING,
		CAN_FIRE
	};


public:
	Ship() = default;
	// Construct and Load() at the same time.
	Ship(const DataNode &node, const ConditionsStore *playerConditions);

	// Load data for a type of ship:
	void Load(const DataNode &node, const ConditionsStore *playerConditions);
	// When loading a ship, some of the outfits it lists may not have been
	// loaded yet. So, wait until everything has been loaded, then call this.
	void FinishLoading(bool isNewInstance);
	// Check that this ship model and all its outfits have been loaded.
	bool IsValid() const;
	// Save a full description of this ship, as currently configured.
	void Save(DataWriter &out) const;

	const EsUuid &UUID() const noexcept;
	// Explicitly set this ship's ID.
	void SetUUID(const EsUuid &id);

	// Get the name of this particular ship.
	const string &Name() const;

	// Set / Get the name of this model of ship.
	void SetTrueModelName(const string &model);
	const string &TrueModelName() const;
	const string &DisplayModelName() const;
	const string &PluralModelName() const;
	// Get the name of this ship as a variant.
	const string &VariantName() const;
	// Get the variant name to be displayed on the Shipyard tab of the Map screen.
	const string &VariantMapShopName() const;
	// Get the generic noun (e.g. "ship") to be used when describing this ship.
	const string &Noun() const;
	// Get this ship's description.
	string Description() const;
	// Get the shipyard thumbnail for this ship.
	const Sprite *Thumbnail() const;
	// Get this ship's cost.
	int64_t Cost() const;
	int64_t ChassisCost() const;
	int64_t Strength() const;
	// Get the attraction and deterrence of this ship, for pirate raids.
	// This is only useful for the player's ships.
	double Attraction() const;
	double Deterrence() const;

	// Check if this ship is configured in such a way that it would be difficult
	// or impossible to fly.
	vector<string> FlightCheck() const;

	void SetPosition(Point position);
	void SetVelocity(Point velocity);
	// When creating a new ship, you must set the following:
	void Place(Point position = Point(), Point velocity = Point(), Angle angle = Angle(), bool isDeparting = true);
	void SetName(const string &name);
	void SetSystem(const System *system);
	void SetPlanet(const Planet *planet);
	void SetGovernment(const Government *government);
	void SetIsSpecial(bool special = true);
	bool IsSpecial() const;

	// If a ship belongs to the player, the player can give it commands.
	void SetIsYours(bool yours = true);
	bool IsYours() const;
	// A parked ship stays on a planet and requires no daily salary payments.
	void SetIsParked(bool parked = true);
	bool IsParked() const;
	// The player can selectively deploy their carried ships, rather than just all / none.
	void SetDeployOrder(bool shouldDeploy = true);
	bool HasDeployOrder() const;

	// Access the ship's personality, which affects how the AI behaves.
	const Personality &GetPersonality() const;
	void SetPersonality(const Personality &other);
	// Get a random hail message, or set the object used to generate them. If no
	// object is given the government's default will be used.
	const Phrase *GetHailPhrase() const;
	void SetHailPhrase(const Phrase &phrase);
	string GetHail(map<string, string> &&subs) const;
	bool CanSendHail(const PlayerInfo &player, bool allowUntranslated = false) const;

	// Access the ship's AI cache, containing the range and expected AI behavior for this ship.
	const ShipAICache &GetAICache() const;
	// Updates the AI and navigation caches. If the ship's mass hasn't changed,
	// reuses some of the previous values.
	void UpdateCaches(bool massLessChange = false);

	// Set the commands for this ship to follow this timestep.
	void SetCommands(const Command &command);
	void SetCommands(const FireCommand &firingCommand);
	const Command &Commands() const;
	const FireCommand &FiringCommands() const noexcept;
	// Move this ship. A ship may create effects as it moves, in particular if
	// it is in the process of blowing up.
	void Move(vector<Visual> &visuals, list<shared_ptr<Flotsam>> &flotsam);

	// Launch any ships that are ready to launch.
	void Launch(list<shared_ptr<Ship>> &ships, vector<Visual> &visuals);
	// Check if this ship is boarding another ship. If it is, it either plunders
	// it or, if this is a player ship, returns the ship it is plundering so a
	// plunder dialog can be displayed.
	shared_ptr<Ship> Board(bool autoPlunder, bool nonDocking);
	// Scan the target, if able and commanded to. Return a ShipEvent bitmask
	// giving the types of scan that succeeded.
	int Scan(const PlayerInfo &player);
	// Find out what fraction of the scan is complete.
	double CargoScanFraction() const;
	double OutfitScanFraction() const;

	// Fire any primary or secondary weapons that are ready to fire. Determines
	// if any special weapons (e.g. anti-missile, tractor beam) are ready to fire.
	// The firing of special weapons is handled separately.
	void Fire(vector<Projectile> &projectiles, vector<Visual> &visuals, vector<int> *emptySoundsTimer);
	// Return true if any anti-missile or tractor beam systems are ready to fire.
	bool HasAntiMissile() const;
	bool HasTractorBeam() const;
	// Fire an anti-missile at the given missile. Returns true if the missile was killed.
	bool FireAntiMissile(const Projectile &projectile, vector<Visual> &visuals);
	// Fire tractor beams at the given flotsam. Returns a Point representing the net
	// pull on the flotsam from this ship's tractor beams.
	Point FireTractorBeam(const Flotsam &flotsam, vector<Visual> &visuals);

	// Get the system this ship is in. Set to nullptr if the ship is being carried.
	const System *GetSystem() const;
	// Get the system this ship is in. If being carried, gets the parent's system.
	const System *GetActualSystem() const;
	// If the ship is landed, get the planet it has landed on.
	const Planet *GetPlanet() const;

	// Check the status of this ship.
	bool IsCapturable() const;
	bool IsTargetable() const;
	bool IsOverheated() const;
	bool IsDisabled() const;
	bool IsBoarding() const;
	bool IsLanding() const;
	bool IsFleeing() const;
	// Check if this ship is currently able to begin landing on its target.
	bool CanLand() const;
	// What kind of action this is we are trying to do.
	enum class ActionType {AFTERBURNER, BOARD, COMMUNICATION, FIRE, PICKUP, SCAN};
	// Check if some condition is keeping this ship from acting. (That is, it is
	// landing, hyperspacing, cloaking without "cloaked ActionType", disabled, or under-crewed.)
	bool CannotAct(ActionType actionType) const;
	// Get the degree to which this ship is cloaked. 1 means fully cloaked; 0 means fully visible.
	// Depending on its "cloaking ..." attributes the ship will be unable to shoot, will not be seen on radar...
	double Cloaking() const;
	bool IsCloaked() const;
	// The amount of cloaking this ship can do, per frame.
	double CloakingSpeed() const;
	// If this ship should be immune to the next damage caused.
	bool Phases(Projectile &projectile) const;
	// Check if this ship is entering (rather than leaving) hyperspace.
	bool IsEnteringHyperspace() const;
	// Check if this ship is entering or leaving hyperspace.
	bool IsHyperspacing() const;
	int GetHyperspacePercentage() const;
	// Check if this ship is hyperspacing, specifically via a jump drive.
	bool IsUsingJumpDrive() const;
	// Check if this ship is currently able to enter hyperspace to it target.
	bool IsReadyToJump(bool waitingIsReady = false) const;
	// Check if this ship is allowed to land on this planet, accounting for its personality.
	bool IsRestrictedFrom(const Planet &planet) const;
	// Check if this ship is allowed to enter this system, accounting for its personality.
	bool IsRestrictedFrom(const System &system) const;
	// Get this ship's custom swizzle.
	const Swizzle *CustomSwizzle() const;
	const string &CustomSwizzleName() const;

	// Check if the ship is thrusting. If so, the engine sound should be played.
	bool IsThrusting() const;
	bool IsReversing() const;
	bool IsSteering() const;
	// The direction that the ship is steering. If positive, the ship is steering right.
	// If negative, the ship is steering left.
	double SteeringDirection() const;
	// Get the points from which engine flares should be drawn.
	const vector<EnginePoint> &EnginePoints() const;
	const vector<EnginePoint> &ReverseEnginePoints() const;
	const vector<EnginePoint> &SteeringEnginePoints() const;

	// Make a ship disabled or destroyed, or bring back a destroyed ship.
	void Disable();
	void Destroy();
	void SelfDestruct();
	void Restore();
	bool IsDamaged() const;
	// Check if this ship has been destroyed.
	bool IsDestroyed() const;
	// Recharge and repair this ship (e.g. because it has landed).
	void Recharge(int rechargeType = Port::RechargeType::All, bool hireCrew = true);
	// Check if this ship is able to give the given ship enough fuel to jump.
	bool CanRefuel(const Ship &other) const;
	// Check if this ship can transfer sufficient energy to the other ship.
	bool CanGiveEnergy(const Ship &other) const;
	// Give the other ship enough fuel for it to jump.
	double TransferFuel(double amount, Ship *to);
	// Give the other ship some energy.
	double TransferEnergy(double amount, Ship *to);
	// Mark this ship as property of the given ship. Returns the number of crew transferred from the capturer.
	int WasCaptured(const shared_ptr<Ship> &capturer);
	// Clear all orders and targets this ship has (after capture or transfer of control).
	void ClearTargetsAndOrders();

	// Get characteristics of this ship, as a fraction between 0 and 1.
	double Shields() const;
	double Hull() const;
	double Fuel() const;
	double Energy() const;
	// A ship's heat is generally between 0 and 1, but if it receives
	// heat damage the value can increase above 1.
	double Heat() const;
	// Get the ship's "health," where <=0 is disabled and 1 means full health.
	double Health() const;
	// Get the hull fraction at which this ship is disabled.
	double DisabledHull() const;
	// Get the maximum shield and hull values of the ship, accounting for multipliers.
	double MaxShields() const;
	double MaxHull() const;
	// Get the absolute shield, hull, and fuel levels of the ship.
	double ShieldLevel() const;
	double HullLevel() const;
	double FuelLevel() const;
	// Get how disrupted this ship's shields are.
	double DisruptionLevel() const;
	// Get the (absolute) amount of hull that needs to be damaged until the
	// ship becomes disabled. Returns 0 if the ships hull is already below the
	// disabled threshold.
	double HullUntilDisabled() const;
	// Returns the remaining damage timer, for the damage overlay.
	int DamageOverlayTimer() const;
	// Get this ship's jump navigation, which contains information about how
	// much it costs for this ship to jump, how far it can jump, and its possible
	// jump methods.
	const ShipJumpNavigation &JumpNavigation() const;
	// Get the number of jumps this ship can make before running out of fuel.
	// This depends on how much fuel it has and what sort of hyperdrive it uses.
	// This does not show accurate number of jumps remaining beyond 1.
	// If followParent is false, this ship will not follow the parent.
	int JumpsRemaining(bool followParent = true) const;
	bool NeedsFuel(bool followParent = true) const;
	// Checks whether this ship needs energy to function.
	bool NeedsEnergy() const;
	// Get the amount of fuel missing for the next jump (smart refueling)
	double JumpFuelMissing() const;
	// Get the heat level at idle.
	double IdleHeat() const;
	// Get the heat dissipation, in heat units per heat unit per frame.
	double HeatDissipation() const;
	// Get the maximum heat level, in heat units (not temperature).
	double MaximumHeat() const;
	// Calculate the multiplier for cooling efficiency.
	double CoolingEfficiency() const;
	// Calculate the drag on this ship. The drag can be no greater than the mass.
	double Drag() const;
	// Calculate the drag force that this ship experiences. The drag force is the drag
	// divided by the mass, up to a value of 1.
	double DragForce() const;

	// Access how many crew members this ship has or needs.
	//int Crew() const;
	//int RequiredCrew() const;
	// Get the reputational value of this ship's crew, which depends
	// on its crew size and "crew equivalent" attribute.
	//int CrewValue() const;
	//void AddCrew(int count);
	Crew &GetCrew() const;
	// Check if this is a ship that can be used as a flagship.
	bool CanBeFlagship() const;

	// Get this ship's movement characteristics.
	double Mass() const;
	double InertialMass() const;
	double TurnRate() const;
	double Acceleration() const;
	double MaxVelocity(bool withAfterburner = false) const;
	double ReverseAcceleration() const;
	double MaxReverseVelocity() const;
	// These two values are the ship's current maximum acceleration and turn rate, accounting for the effects of slow.
	double TrueAcceleration() const;
	double TrueTurnRate() const;
	// The ship's current speed right now
	double CurrentSpeed() const;

	double ThrustHeldFraction(ThrustKind kind) const;
	uint8_t ThrustHeldFrames(ThrustKind kind) const;

	// This ship just got hit by a weapon. Take damage according to the
	// DamageDealt from that weapon. The return value is a ShipEvent type,
	// which may be a combination of PROVOKED, DISABLED, and DESTROYED.
	// Create any target effects as sparks.
	int TakeDamage(vector<Visual> &visuals, const DamageDealt &damage, const Government *sourceGovernment);
	// Apply a force to this ship, accelerating it. This might be from a weapon
	// impact, or from firing a weapon, for example.
	void ApplyForce(const Point &force, bool gravitational = false);

	// Check if this ship has bays to carry other ships.
	bool HasBays() const;
	// Check how many bays are not occupied at present. This does not check
	// whether one of your escorts plans to use that bay.
	int BaysFree(const string &category) const;
	// Check how many bays this ship has of a given category.
	int BaysTotal(const string &category) const;
	// Check if this ship has a bay free for the given other ship, and the
	// bay is not reserved for one of its existing escorts.
	bool CanCarry(const Ship &ship) const;
	// Check if this is a ship of a type that can be carried.
	bool CanBeCarried() const;
	// Move the given ship into one of the bays, if possible.
	bool Carry(const shared_ptr<Ship> &ship);
	// Empty the bays. If the carried ships are not special ships that are
	// saved in the player data, they will be deleted. Otherwise, they become
	// visible as ships landed on the same planet as their parent.
	void UnloadBays();
	// Get a list of any ships this ship is carrying.
	const vector<Bay> &Bays() const;

	// Get cargo information.
	CargoHold &Cargo();
	const CargoHold &Cargo() const;
	// Display box effects from jettisoning this much cargo.
	void Jettison(const string &commodity, int tons, bool wasAppeasing = false);
	void Jettison(const Outfit *outfit, int count, bool wasAppeasing = false);

	// Get the current attributes of this ship.
	const Outfit &Attributes() const;
	// Get the attributes of this ship chassis before any outfits were added.
	const Outfit &BaseAttributes() const;
	// Get the list of all outfits installed in this ship.
	const map<const Outfit *, int> &Outfits() const;
	// Find out how many outfits of the given type this ship contains.
	int OutfitCount(const Outfit *outfit) const;
	// Add or remove outfits. (To remove, pass a negative number.)
	void AddOutfit(const Outfit *outfit, int count);

	// Get the list of weapons.
	Armament &GetArmament();
	const vector<Hardpoint> &Weapons() const;
	// Check if we are able to fire the given weapon (i.e. there is enough
	// energy, ammo, and fuel to fire it).
	CanFireResult CanFire(const Weapon *weapon) const;
	// Fire the given weapon (i.e. deduct whatever energy, ammo, or fuel it uses
	// and add whatever heat it generates). Assume that CanFire() is true.
	void ExpendAmmo(const Weapon &weapon);

	// Each ship can have a target system (to travel to), a target planet (to
	// land on) and a target ship (to move to, and attack if hostile).
	shared_ptr<Ship> GetTargetShip() const;
	shared_ptr<Ship> GetShipToAssist() const;
	const StellarObject *GetTargetStellar() const;
	// Get ship's target system (it should always be one jump / wormhole pass away).
	const System *GetTargetSystem() const;
	// Mining target.
	shared_ptr<Minable> GetTargetAsteroid() const;
	shared_ptr<Flotsam> GetTargetFlotsam() const;
	const set<const Flotsam *> &GetTractorFlotsam() const;
	// Pattern to use when flying in a formation.
	const FormationPattern *GetFormationPattern() const;

	// Mark this ship as fleeing.
	void SetFleeing(bool fleeing = true);

	// Set this ship's targets.
	void SetTargetShip(const shared_ptr<Ship> &ship);
	void SetShipToAssist(const shared_ptr<Ship> &ship);
	void SetTargetStellar(const StellarObject *object);
	// Set ship's target system (it should always be one jump / wormhole pass away).
	void SetTargetSystem(const System *system);
	// Mining target.
	void SetTargetAsteroid(const shared_ptr<Minable> &asteroid);
	void SetTargetFlotsam(const shared_ptr<Flotsam> &flotsam);
	// Pattern to use when flying in a formation (nullptr to clear formation).
	void SetFormationPattern(const FormationPattern *formation);

	bool CanPickUp(const Flotsam &flotsam) const;

	// Manage escorts. When you set this ship's parent, it will automatically
	// register itself as an escort of that ship, and unregister itself from any
	// previous parent it had.
	void SetParent(const shared_ptr<Ship> &ship);
	shared_ptr<Ship> GetParent() const;
	const vector<weak_ptr<Ship>> &GetEscorts() const;

	// How many AI steps has this ship been lingering?
	int GetLingerSteps() const;
	// The AI wants the ship to linger for one AI step.
	void Linger();

	// Check if this ship looks the same as another, based on model display names and outfits.
	bool Imitates(const Ship &other) const;

	double CarriedMass() const;
	void AddCarriedMass(double);

	const System *CurrentSystem() const { return currentSystem; }

private:
	// Various steps of Ship::Move:

	// Check if this ship has been in a different system from the player for so
	// long that it should be "forgotten." Also eliminate ships that have no
	// system set because they just entered a fighter bay. Clear the hyperspace
	// targets of ships that can't enter hyperspace.
	bool StepFlags();
	// Step ship destruction logic. Returns 1 if the ship has been destroyed, -1 if it is being
	// destroyed, or 0 otherwise.
	int StepDestroyed(vector<Visual> &visuals, list<shared_ptr<Flotsam>> &flotsam);
	void DoGeneration();
	void DoPassiveEffects(vector<Visual> &visuals, list<shared_ptr<Flotsam>> &flotsam);
	void DoJettison(list<shared_ptr<Flotsam>> &flotsam);
	void DoCloakDecision();
	// Step hyperspace enter/exit logic. Returns true if ship is hyperspacing in or out.
	bool DoHyperspaceLogic(vector<Visual> &visuals);
	// Step landing logic. Returns true if the ship is landing or departing.
	bool DoLandingLogic();
	void DoInitializeMovement();
	void StepPilot();
	void DoMovement(bool &isUsingAfterburner);
	void StepTargeting();
	void DoEngineVisuals(vector<Visual> &visuals, bool isUsingAfterburner);


	// Add or remove a ship from this ship's list of escorts.
	void AddEscort(Ship &ship);
	void RemoveEscort(const Ship &ship);
	// Get the hull amount at which this ship is disabled.
	double MinimumHull() const;
	// Create one of this ship's explosions, within its mask. The explosions can
	// either stay over the ship, or spread out if this is the final explosion.
	void CreateExplosion(vector<Visual> &visuals, bool spread = false);
	// Place a "spark" effect, like ionization or disruption.
	void CreateSparks(vector<Visual> &visuals, const string &name, double amount);
	void CreateSparks(vector<Visual> &visuals, const Effect *effect, double amount);

	// Calculate the attraction and deterrence of this ship, for pirate raids.
	// This is only useful for the player's ships.
	double CalculateAttraction() const;
	double CalculateDeterrence() const;

	// Increment the duration a thruster direction has been held.
	void IncrementThrusterHeld(ThrustKind kind);

	// Helper function for jettisoning flotsam.
	void Jettison(shared_ptr<Flotsam> toJettison);


private:
	// Protected member variables of the Body class:
	// Point position;
	// Point velocity;
	// Angle angle;
	// double zoom;
	// int swizzle;
	// const Government *government;

	// Characteristics of the chassis:
	bool isDefined = false;
	const Ship *base = nullptr;
	string trueModelName;
	string displayModelName;
	string pluralModelName;
	string variantName;
	string variantMapShopName;
	string noun;
	Paragraphs description;
	const Sprite *thumbnail = nullptr;
	// Characteristics of this particular ship:
	EsUuid uuid;
	string name;
	bool canBeCarried = false;

	int forget = 0;
	bool isInSystem = true;
	// "Special" ships cannot be forgotten, and if they land on a planet, they
	// continue to exist and refuel instead of being deleted.
	bool isSpecial = false;
	bool isYours = false;
	bool isParked = false;
	bool shouldDeploy = false;
	bool isOverheated = false;
	bool isDisabled = false;
	bool isBoarding = false;
	bool hasBoarded = false;
	bool isFleeing = false;
	bool isThrusting = false;
	bool isReversing = false;
	bool isSteering = false;
	double steeringDirection = 0.;
	bool neverDisabled = false;
	bool isCapturable = true;
	bool isInvisible = false;
	const Swizzle *customSwizzle = nullptr;
	string customSwizzleName;
	double cloak = 0.;
	double cloakDisruption = 0.;
	// Cached values for figuring out when anti-missiles or tractor beams are in range.
	double antiMissileRange = 0.;
	double tractorBeamRange = 0.;
	double weaponRadius = 0.;
	// Cargo and outfit scanning takes time.
	double cargoScan = 0.;
	double outfitScan = 0.;

	double attraction = 0.;
	double deterrence = 0.;

	// Number of AI steps this ship has spent lingering
	int lingerSteps = 0;

	Command commands;
	FireCommand firingCommands;

	Personality personality;
	const Phrase *hail = nullptr;
	ShipAICache aiCache;

	// Installed outfits, cargo, etc.:
	Outfit attributes;
	Outfit baseAttributes;
	bool addAttributes = false;
	const Outfit *explosionWeapon = nullptr;
	map<const Outfit *, int> outfits;
	CargoHold cargo;
	list<shared_ptr<Flotsam>> jettisoned;
	list<pair<shared_ptr<Flotsam>, size_t>> jettisonedFromBay;

	vector<Bay> bays;
	// Cache the mass of carried ships to avoid repeatedly recomputing it.
	double carriedMass = 0.;

	vector<EnginePoint> enginePoints;
	vector<EnginePoint> reverseEnginePoints;
	vector<EnginePoint> steeringEnginePoints;
	Armament armament;

	// Various energy levels:
	double shields = 0.;
	double hull = 0.;
	double fuel = 0.;
	double energy = 0.;
	double heat = 0.;
	// Accrued "ion damage" that will affect this ship's energy over time.
	double ionization = 0.;
	// Accrued "scrambling damage" that will affect this ship's weaponry over time.
	double scrambling = 0.;
	// Accrued "disruption damage" that will affect this ship's shield effectiveness over time.
	double disruption = 0.;
	// Accrued "slowing damage" that will affect this ship's movement over time.
	double slowness = 0.;
	// Accrued "discharge damage" that will affect this ship's shields over time.
	double discharge = 0.;
	// Accrued "corrosion damage" that will affect this ship's hull over time.
	double corrosion = 0.;
	// Accrued "leak damage" that will affect this ship's fuel over time.
	double leakage = 0.;
	// Accrued "burn damage" that will affect this ship's heat over time.
	double burning = 0.;
	// Delays for shield generation and hull repair.
	int shieldDelay = 0;
	int hullDelay = 0;
	// Timer for a disabled ship to repair itself.
	int disabledRecoveryCounter = 0;
	// Number of frames the damage overlay should be displayed, if any.
	int damageOverlayTimer = 0;
	// Acceleration can be created by engines, firing weapons, or weapon impacts.
	Point acceleration;
	// The amount of time in frames that an engine has been on for.
	array<uint8_t, 4> thrustHeldFrames = {};

	//int crew = 0;
	//int pilotError = 0;
	//int pilotOkay = 0;
	Crew crew;

	// Current status of this particular ship:
	const System *currentSystem = nullptr;
	// A Ship can be locked into one of three special states: landing,
	// hyperspacing, and exploding. Each one must track some special counters:
	const Planet *landingPlanet = nullptr;

	ShipJumpNavigation navigation;
	int hyperspaceCount = 0;
	const System *hyperspaceSystem = nullptr;
	bool isUsingJumpDrive = false;
	double hyperspaceFuelCost = 0.;
	Point hyperspaceOffset;

	// The hull may spring a "leak" (venting atmosphere, flames, blood, etc.)
	// when the ship is dying.
	class Leak {
	public:
		Leak(const Effect *effect = nullptr) : effect(effect) {}

		const Effect *effect = nullptr;
		Point location;
		Angle angle;
		int openPeriod = 60;
		int closePeriod = 60;
	};
	vector<Leak> leaks;
	vector<Leak> activeLeaks;

	// Explosions that happen when the ship is dying:
	map<const Effect *, int> explosionEffects;
	unsigned explosionRate = 0;
	unsigned explosionCount = 0;
	unsigned explosionTotal = 0;
	map<const Effect *, int> finalExplosions;

	// Target ships, planets, systems, etc.
	weak_ptr<Ship> targetShip;
	weak_ptr<Ship> shipToAssist;
	const StellarObject *targetPlanet = nullptr;
	const System *targetSystem = nullptr;
	weak_ptr<Minable> targetAsteroid;
	weak_ptr<Flotsam> targetFlotsam;
	set<const Flotsam *> tractorFlotsam;
	const FormationPattern *formationPattern = nullptr;

	// Links between escorts and parents.
	vector<weak_ptr<Ship>> escorts;
	weak_ptr<Ship> parent;

	bool removeBays = false;
};
