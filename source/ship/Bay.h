/* Bay.h
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

#include "../Angle.h"
#include "../Point.h"
#include "../Ship.h"
#include "../Visual.h"
#include "../net/Subscriber.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

class Effect;
class Subscriber;

class Bay : Subscriber {
public:
	// Bay position
	static const uint8_t INSIDE = 0;
	static const uint8_t OVER = 1;
	static const uint8_t UNDER = 2;

public:
	Bay() = default;
	Bay(Bay &&) = default;
	// Copying a bay does not copy the ship inside it.
	Bay(const Bay &b) : position(b.Position()), category(b.Category()), side(b.Side()),
		facing(b.facing), launchEffects(b.launchEffects) {}
	~Bay() = default;

	Bay &operator=(Bay &&) = default;
	Bay &operator=(const Bay &b) { return *this = Bay(b); }

	void Load(const std::string &key, const DataNode &child);
	// When loading a ship, some of the outfits it lists may not have been
	// loaded yet. So, wait until everything has been loaded, then call this.
	void FinishLoading(bool isNewInstance);
	void Save(DataWriter &out) const;

	void Launch(std::vector<Visual> &visuals);

	Point Position() const { return position; }
	void SetPosition(const Point &position);

	string Category() const { return category; }
	void SetCategory(const std::string category);

	uint8_t Side() const { return side; }
	void SetSide(const uint8_t side);

	Angle Facing() const { return facing; }
	void SetFacing(const Angle &facing);

	// Manage fighters. When you set this fighter's carrier, it will automatically
	// register itself as an fighter of that carrier, and unregister itself from any
	// previous carrier it had.
	Ship *Carrier() const { return carrier.get(); }
	void SetCarrier(const std::shared_ptr<Ship> &carrier);

	// Add or remove a fighter from this ship's list of fighters.
	Ship *Fighter() const { return fighter.get(); }
	// Adjust the positions and velocities of any visible carried fighters or
	// drones. If any are visible, return true.
	bool PositionFighter() const;

	// Check if this ship has a bay free for the given other ship, and the
	// bay is not reserved for one of its existing fighters.
	bool CanCarry(const Ship &ship) const;

	// Move the given fighter into the bays, if possible.
	bool Carry(const std::shared_ptr<Ship> &ship);

	// Empty the bay. If the carried ship is not special ship that is
	// saved in the player data, they will be deleted. Otherwise, they become
	// visible as ships landed on the same planet as their carrier.
	void Unload();

	// Transfer as many of the given outfits from the source ship to the target
	// ship as the source ship can remove and the target ship can handle. Returns the
	// items and amounts that were actually transferred (so e.g. callers can determine
	// how much material was transferred, if any).
	std::map<const Outfit *, int> TransferAmmo(const std::map<const Outfit *, int> &stockpile, Ship &from, Ship &to);

	void Deploy(bool includingDamaged) const;
	bool HasDeployOrders() const;

	void Receive(const Broker::Message *message, const string &topic) override;

private:
	Point position;
	string category;
	uint8_t side = 0;

	// The angle at which the carried ship will depart, relative to the carrying ship.
	Angle facing;

	std::shared_ptr<Ship> carrier;
	std::shared_ptr<Ship> fighter;

	// The launch effect(s) to be simultaneously played when the bay's ship launches.
	std::vector<const Effect *> launchEffects;
};
