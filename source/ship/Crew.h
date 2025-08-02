/* Crew.h
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

#include "../Ship.h"

#include <memory>

using namespace std;

class Ship;

class Crew
{
public:
	void Add(int count);
	int Bunks() const;
	// Access how many crew members this ship has or needs.
	int Members() const;
	int Equivalent() const;
	int RequiredCrew() const;
	// Get the reputational value of this ship's crew, which depends
	// on its crew size and "crew equivalent" attribute.
	int ReputationalValue() const;
	Ship *GetShip() { return this->ship.get(); }
	void Onboard(shared_ptr<Ship> &ship) { this->ship = ship; }
	void StepPilot();
	void SetPilotError(int error);
	int PilotError() const { return pilotError; }
	void Recharge(bool hireCrew);
	int WasCaptured(const shared_ptr<Ship> &capturer);
	int PilotOkay() const { return pilotOkay; }
	int ExtraCrew() const { return Members() - RequiredCrew(); }
private:
	shared_ptr<Ship> ship;
	int members = 0;
	int pilotError = 0;
	int pilotOkay = 0;
};
