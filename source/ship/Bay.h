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
#include "../Effect.h"
#include "../Point.h"
#include "Ship.h"

#include <memory>
#include <string>
#include <vector>

class Bay {
public:
	Bay(double x, double y, std::string category) :
		position(x * .5, y * .5), category(std::move(category)) {}
	Bay(Bay &&) = default;
	// Copying a bay does not copy the carrier or the fighter inside it.
	Bay(const Bay &b) : position(b.position), category(b.category), side(b.side),
		facing(b.facing), launchEffects(b.launchEffects) {}
	~Bay() = default;

	Bay &operator=(Bay &&) = default;
	Bay &operator=(const Bay &b) { return *this = Bay(b); }

	// Side
	static const uint8_t INSIDE = 0;
	static const uint8_t OVER = 1;
	static const uint8_t UNDER = 2;

	void InsertLaunchEffects(int count, Effect *e)
	{
		launchEffects.insert(launchEffects.end(), count, e);
	}
	void EmplaceLaunchEffects(Effect *e)
	{
		launchEffects.emplace_back(e);
	}
	void Deploy(bool includingDamaged) const;

	Point position;
	// Master ship
	std::shared_ptr<Ship> carrier;
	// Slave ship
	std::shared_ptr<Ship> fighter;

	std::string category;
	uint8_t side = Bay::INSIDE;

	// The angle at which the fighter ship will depart, relative to the carrying ship.
	Angle facing;

	// The launch effect(s) to be simultaneously played when the bay's ship launches.
	std::vector<Effect *> launchEffects;

};
