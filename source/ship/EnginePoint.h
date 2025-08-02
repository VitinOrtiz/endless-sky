/* EnginePoint.h
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

#include "../Angle.h"
#include "../Point.h"

#include <memory>
#include <vector>

using namespace std;

class EnginePoint : public Point {
public:
	EnginePoint() = default;
	EnginePoint(Point pos, double zoom) : Point(pos), zoom(zoom) {}

	uint8_t side = 0;
	static const uint8_t UNDER = 0;
	static const uint8_t OVER = 1;

	uint8_t steering = 0;
	static const uint8_t NONE = 0;
	static const uint8_t LEFT = 1;
	static const uint8_t RIGHT = 2;

	double Zoom() const { return zoom; }
	void SetZoom(double zoom) { this->zoom = zoom; }

	Angle Facing() const { return facing; }
	void SetFacing(const Angle &facing) { this->facing = facing; }

	Angle Gimbal() const { return gimbal; }
	void SetGimbal(const Angle &gimbal) { this->gimbal = gimbal; }
private:
	double zoom;
	Angle facing;
	Angle gimbal;
};
