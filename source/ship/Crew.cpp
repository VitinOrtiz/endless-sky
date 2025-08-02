#include "crew.h"

#include "../Messages.h"
#include "../Preferences.h"
#include "../Random.h"

#include <string>

using namespace std;

void Crew::Add(int count)
{
	members = min<int>(members + count, Bunks());
}



int Crew::Bunks() const
{
	return ship->Attributes().Get("bunks");
}



int Crew::Members() const
{
	return members;
}



int Crew::Equivalent() const
{
	return ship->Attributes().Get("crew equivalent");
}



int Crew::RequiredCrew() const
{
	if(ship->Attributes().Get("automaton"))
		return 0;
	else
		// Drones do not need crew, but all other ships need at least one.
		return max<int>(1, ship->Attributes().Get("required crew"));
}



int Crew::ReputationalValue() const
{
	if(ship->Attributes().Get("use crew equivalent as crew"))
		return Equivalent();
	else
		return max<int>(Members(), RequiredCrew()) + Equivalent();
}



void Crew::StepPilot()
{
	int requiredCrew = RequiredCrew();

	if(pilotError)
		--pilotError;
	else if(pilotOkay)
		--pilotOkay;
	else if(ship->IsDisabled())
	{
		// If the ship is disabled, don't show a warning message due to missing crew.
	}
	else if(requiredCrew && static_cast<int>(Random::Int(requiredCrew)) >= Members())
	{
		pilotError = 30;
		if(ship->IsYours() || ship->GetPersonality().IsEscort())
		{
			if(!ship->weak_from_this().lock())
				Messages::Add("Your ship is moving erratically because you do not have enough crew to pilot it."
					, Messages::Importance::Low);
			else if(Preferences::Has("Extra fleet status messages"))
				Messages::Add("The " + ship->Name() + " is moving erratically because there are not enough crew to pilot it."
					, Messages::Importance::Low);
		}
	}
	else
		pilotOkay = 30;
}



void Crew::SetPilotError(int error)
{
	pilotError = error;
}



void Crew::Recharge(bool hireCrew)
{
	if(hireCrew)
		members = min<int>(max(Members(), RequiredCrew()), Bunks());
	pilotError = 0;
	pilotOkay = 0;
}



int Crew::WasCaptured(const shared_ptr<Ship> &capturer)
{
	// Transfer some crew over. Only transfer the bare minimum unless even that
	// is not possible, in which case, share evenly.
	int totalRequired = capturer->GetCrew().RequiredCrew() + RequiredCrew();
	int transfer = RequiredCrew() - Members();
	if(transfer > 0)
	{
		if(totalRequired > capturer->GetCrew().Members() + Members())
			transfer = max<int>(Members() ? 0 : 1, (capturer->GetCrew().Members() * transfer) / totalRequired);
		capturer.get()->GetCrew().Add(-transfer);
		ship.get()->GetCrew().Add(transfer);
	}
	return transfer;
}
