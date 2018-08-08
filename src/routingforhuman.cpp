/*
 * Copyright (C) 2016-2018 Rinigus https://github.com/rinigus
 * 
 * This file is part of OSM Scout Server.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifdef USE_OSMSCOUT

#include "routingforhuman.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

RoutingForHuman::RoutingForHuman(QObject *parent) : QObject(parent)
{

}

QString RoutingForHuman::CrossingWaysDescriptionToString(const osmscout::RouteDescription::CrossingWaysDescription& crossingWaysDescription)
{
    std::set<std::string>                          names;
    osmscout::RouteDescription::NameDescriptionRef originDescription=crossingWaysDescription.GetOriginDesccription();
    osmscout::RouteDescription::NameDescriptionRef targetDescription=crossingWaysDescription.GetTargetDesccription();

    if (originDescription) {
        std::string nameString=originDescription->GetDescription();

        if (!nameString.empty()) {
            names.insert(nameString);
        }
    }

    if (targetDescription) {
        std::string nameString=targetDescription->GetDescription();

        if (!nameString.empty()) {
            names.insert(nameString);
        }
    }

    for (std::list<osmscout::RouteDescription::NameDescriptionRef>::const_iterator name=crossingWaysDescription.GetDescriptions().begin();
         name!=crossingWaysDescription.GetDescriptions().end();
         ++name) {
        std::string nameString=(*name)->GetDescription();

        if (!nameString.empty()) {
            names.insert(nameString);
        }
    }

    if (names.size()>0) {
        std::ostringstream stream;

        for (std::set<std::string>::const_iterator name=names.begin();
             name!=names.end();
             ++name) {
            if (name!=names.begin()) {
                stream << ", ";
            }
            stream << "" << *name << "";
        }

        return QString::fromStdString(stream.str());
    }
    else {
        return "";
    }
}


void RoutingForHuman::MoveToTurnCommand(osmscout::RouteDescription::DirectionDescription::Move move,
                                        QString &txt,
                                        QString &type)
{
    switch (move) {
    case osmscout::RouteDescription::DirectionDescription::sharpLeft:
        txt = tr("Turn sharp left"); type = "turn-sharp-left"; return;
    case osmscout::RouteDescription::DirectionDescription::left:
        txt = tr("Turn left"); type = "turn-left"; return;
    case osmscout::RouteDescription::DirectionDescription::slightlyLeft:
        txt = tr("Turn slightly left"); type = "turn-slight-left"; return;
    case osmscout::RouteDescription::DirectionDescription::straightOn:
        txt = tr("Straight on"); type = "straight"; return;
    case osmscout::RouteDescription::DirectionDescription::slightlyRight:
        txt = tr("Turn slightly right"); type = "turn-slight-right"; return;
    case osmscout::RouteDescription::DirectionDescription::right:
        txt = tr("Turn right"); type = "turn-right"; return;
    case osmscout::RouteDescription::DirectionDescription::sharpRight:
        txt = tr("Turn sharp right"); type = "turn-sharp-right"; return;
    }

    assert(false);
}


void RoutingForHuman::DumpStartDescription(QJsonObject& action,
                                           const osmscout::RouteDescription::StartDescriptionRef& startDescription,
                                           const osmscout::RouteDescription::NameDescriptionRef& nameDescription)
{
    QString txt;

    if (nameDescription &&
            nameDescription->HasName()) {
        QString post = tr("Drive along %along%").replace("%along%", QString::fromStdString(nameDescription->GetDescription()));
        txt = tr("Start at %start%. Drive along %along%").
                replace("%along%", QString::fromStdString(nameDescription->GetDescription())).
                replace("%start%", QString::fromStdString(startDescription->GetDescription()));

        action.insert("verbal_post_transition_instruction", post);
    }
    else
        txt = tr("Start at %start%").
                replace("%start%", QString::fromStdString(startDescription->GetDescription()));

    action.insert("instruction", txt);
    action.insert("type", QString("start"));
}


void RoutingForHuman::DumpTargetDescription(QJsonObject& action,
                                            const osmscout::RouteDescription::TargetDescriptionRef& targetDescription)
{
    action.insert("instruction",
                  tr("Target reached: %target%").
                  replace("%target%", QString::fromStdString(targetDescription->GetDescription())) );
    action.insert("type", QString("destination"));
}


void RoutingForHuman::DumpTurnDescription(QJsonObject& action,
                                          const osmscout::RouteDescription::TurnDescriptionRef& /*turnDescription*/,
                                          const osmscout::RouteDescription::CrossingWaysDescriptionRef& crossingWaysDescription,
                                          const osmscout::RouteDescription::DirectionDescriptionRef& directionDescription,
                                          const osmscout::RouteDescription::NameDescriptionRef& nameDescription)
{
    QString crossingWaysString;
    QString nameDescString;
    QString pre;
    QString comm;
    QString type;

    if (crossingWaysDescription)
        crossingWaysString=CrossingWaysDescriptionToString(*crossingWaysDescription);

    if (nameDescription &&
            nameDescription->HasName())
        nameDescString = QString::fromStdString(nameDescription->GetDescription());

    if (directionDescription)
        MoveToTurnCommand(directionDescription->GetCurve(), comm, type);
    else
        comm = tr("Turn");

    if (!nameDescString.isEmpty())
        comm = tr("%turncommand% into %where%").
                replace("%turncommand%", comm).
                replace("%where%", nameDescString);

    if (!crossingWaysString.isEmpty())
        pre = tr("At crossing (%crossingway%), %turncommand%").
                replace("%crossingway%", crossingWaysString).
                replace("%turncommand%", comm);

    action.insert("instruction", comm);
    action.insert("type", type);
    if (!pre.isEmpty()) action.insert("verbal_pre_transition_instruction", pre);
}

void RoutingForHuman::DumpRoundaboutEnterDescription(QJsonObject& action,
                                                     const osmscout::RouteDescription::RoundaboutEnterDescriptionRef& /*roundaboutEnterDescription*/,
                                                     const osmscout::RouteDescription::CrossingWaysDescriptionRef& crossingWaysDescription)
{
    QString comm = tr("Enter roundabout");
    QString pre;

    if (crossingWaysDescription) {
        pre = tr("At crossing %crossway%, enter roundabout").
                replace("%crossway%",
                        CrossingWaysDescriptionToString(*crossingWaysDescription));
    }

    action.insert("instruction", comm);
    action.insert("type", QString("roundabout-enter"));
    if (!pre.isEmpty()) action.insert("verbal_pre_transition_instruction", pre);
}

void RoutingForHuman::DumpRoundaboutLeaveDescription(QJsonObject& action,
                                                     const osmscout::RouteDescription::RoundaboutLeaveDescriptionRef& roundaboutLeaveDescription,
                                                     const osmscout::RouteDescription::NameDescriptionRef& nameDescription)
{
    int count = roundaboutLeaveDescription->GetExitCount();
    QString cmd;

    cmd = tr("Leave roundabout") + " (" + QString::number(roundaboutLeaveDescription->GetExitCount()) + ". " + tr("exit") + ")";

    if (nameDescription &&
            nameDescription->HasName())
        cmd = tr("Leave roundabout (%num% exit) into street %street%").
                replace("%num%", QString::number(roundaboutLeaveDescription->GetExitCount())).
                replace("%street%", QString::fromStdString(nameDescription->GetDescription()));
    else
        cmd = tr("Leave roundabout (%num% exit)").
                replace("%num%", QString::number(roundaboutLeaveDescription->GetExitCount()));

    action.insert("instruction", cmd);
    action.insert("type", QString("roundabout-exit"));
    action.insert("roundabout_exit_count", count);
}

void RoutingForHuman::DumpMotorwayEnterDescription(QJsonObject& action,
                                                   const osmscout::RouteDescription::MotorwayEnterDescriptionRef& motorwayEnterDescription,
                                                   const osmscout::RouteDescription::CrossingWaysDescriptionRef& crossingWaysDescription)
{
    QString crossingWaysString;
    QString motorwayString;
    QString comm;
    QString pre;

    if (crossingWaysDescription)
        crossingWaysString=CrossingWaysDescriptionToString(*crossingWaysDescription);

    if (motorwayEnterDescription->GetToDescription() &&
            motorwayEnterDescription->GetToDescription()->HasName())
        motorwayString = QString::fromStdString(motorwayEnterDescription->GetToDescription()->GetDescription());

    if (motorwayString.isEmpty())
        comm = tr("Enter motorway");
    else
        comm = tr("Enter motorway %motorway%").
                replace("%motorway%", motorwayString);

    if (!crossingWaysString.isEmpty())
        pre = tr("At crossing %crossing%, enter motorway %motorway%").
                replace("%crossing%", crossingWaysString).
                replace("%motorway%", motorwayString);

    action.insert("instruction", comm);
    action.insert("type", QString("merge"));
    if (!pre.isEmpty()) action.insert("verbal_pre_transition_instruction", pre);
}

void RoutingForHuman::DumpMotorwayChangeDescription(QJsonObject& action,
                                                    const osmscout::RouteDescription::MotorwayChangeDescriptionRef& motorwayChangeDescription,
                                                    const osmscout::RouteDescription::MotorwayJunctionDescriptionRef& motorwayJunctionDescription)
{
    QString motoName;
    QString motoRef;
    QString from;
    QString to;

    QString comm;
    QString pre;

    if (motorwayJunctionDescription &&
            motorwayJunctionDescription->GetJunctionDescription())
    {
        motoName = QString::fromStdString(motorwayJunctionDescription->GetJunctionDescription()->GetName());
        motoRef = QString::fromStdString(motorwayJunctionDescription->GetJunctionDescription()->GetRef());
    }

    if (motorwayChangeDescription->GetFromDescription() &&
            motorwayChangeDescription->GetFromDescription()->HasName())
        from = QString::fromStdString(motorwayChangeDescription->GetFromDescription()->GetDescription());

    if (motorwayChangeDescription->GetToDescription() &&
            motorwayChangeDescription->GetToDescription()->HasName())
        to = QString::fromStdString(motorwayChangeDescription->GetToDescription()->GetDescription());

    if (from.isEmpty() && to.isEmpty())
        comm = tr("Change motorway");
    else if (from.isEmpty())
        comm = tr("Change motorway to %to%").
                replace("%to%", to);
    else if (to.isEmpty())
        comm = tr("Change motorway from %from%").
                replace("%from%", from);
    else
        comm = tr("Change motorway from %from% to %to%").
                replace("%to%", to).
                replace("%from%", from);

    if (!motoName.isEmpty() && !motoRef.isEmpty())
        pre = tr("At %motoName% (exit %motoRef%), %command%").
                replace("%motoName%", motoName).
                replace("%motoRef%", motoRef).
                replace("%command%", comm);

    else if (!motoName.isEmpty())
        pre = tr("At %motoName%, %command%").
                replace("%motoName%", motoName).
                replace("%command%", comm);

    action.insert("instruction", comm);
    action.insert("type", QString("motorway-change"));
    if (!pre.isEmpty()) action.insert("verbal_pre_transition_instruction", pre);
}

void RoutingForHuman::DumpMotorwayLeaveDescription(QJsonObject& action,
                                                   const osmscout::RouteDescription::MotorwayLeaveDescriptionRef& motorwayLeaveDescription,
                                                   const osmscout::RouteDescription::MotorwayJunctionDescriptionRef& motorwayJunctionDescription,
                                                   const osmscout::RouteDescription::DirectionDescriptionRef& directionDescription,
                                                   const osmscout::RouteDescription::NameDescriptionRef& nameDescription)
{
    QString motoName;
    QString motoRef;
    QString from;
    QString move;
    QString into;
    QString type = "motorway-leave";

    QString comm;
    QString pre;

    if (motorwayJunctionDescription &&
            motorwayJunctionDescription->GetJunctionDescription())
    {
        motoName = QString::fromStdString(motorwayJunctionDescription->GetJunctionDescription()->GetName());
        motoRef = QString::fromStdString(motorwayJunctionDescription->GetJunctionDescription()->GetRef());
    }

    if (motorwayLeaveDescription->GetFromDescription() &&
            motorwayLeaveDescription->GetFromDescription()->HasName())
        from = QString::fromStdString(motorwayLeaveDescription->GetFromDescription()->GetDescription());

    if (directionDescription)
        MoveToTurnCommand(directionDescription->GetCurve(), move, type);

    if (directionDescription &&
            ( directionDescription->GetCurve()!=osmscout::RouteDescription::DirectionDescription::slightlyLeft ||
            directionDescription->GetCurve()!=osmscout::RouteDescription::DirectionDescription::straightOn ||
            directionDescription->GetCurve()!=osmscout::RouteDescription::DirectionDescription::slightlyRight) )
        type = "motorway-leave";

    if (nameDescription &&
            nameDescription->HasName())
        into = QString::fromStdString(nameDescription->GetDescription());

    if (from.isEmpty() && move.isEmpty() && into.isEmpty())
        comm = tr("Leave motorway");
    else if (from.isEmpty() && move.isEmpty())
        comm = tr("Leave motorway into %into%").
                replace("%into%", into);
    else if (from.isEmpty() && into.isEmpty())
        comm = tr("Leave motorway. %move%").
                replace("%move%", move);
    else if (move.isEmpty() && into.isEmpty())
        comm = tr("Leave motorway %from%").
                replace("%from%", from);
    else if (from.isEmpty())
        comm = tr("Leave motorway. %move% into %into%").
                replace("%into%", into).
                replace("%move%", move);
    else if (move.isEmpty())
        comm = tr("Leave motorway %from%").
                replace("%into%", into).
                replace("%from%", from);
    else if (into.isEmpty())
        comm = tr("Leave motorway %from%. %move%").
                replace("%move%", move).
                replace("%from%", from);
    else
        comm = tr("Leave motorway %from%. %move% into %into%").
                replace("%into%", into).
                replace("%move%", move).
                replace("%from%", from);


    if (!motoName.isEmpty() && !motoRef.isEmpty())
        pre = tr("At %motoName% (exit %motoRef%), %command%").
                replace("%motoName%", motoName).
                replace("%motoRef%", motoRef).
                replace("%command%", comm);

    else if (!motoName.isEmpty())
        pre = tr("At %motoName%, %command%").
                replace("%motoName%", motoName).
                replace("%command%", comm);

    action.insert("instruction", comm);
    action.insert("type", type);
    if (!pre.isEmpty()) action.insert("verbal_pre_transition_instruction", pre);
}

void RoutingForHuman::DumpNameChangedDescription(QJsonObject& action,
                                                 const osmscout::RouteDescription::NameChangedDescriptionRef& nameChangedDescription)
{
    QString comm;

    if ( nameChangedDescription->GetOriginDesccription() )
        comm = tr("Way changes name from %from% to %to%").
                replace("%from%", QString::fromStdString(nameChangedDescription->GetOriginDesccription()->GetDescription())).
                replace("%to%", QString::fromStdString(nameChangedDescription->GetTargetDesccription()->GetDescription()));
    else
        comm = tr("Way changes name to %to%").
                replace("%to%", QString::fromStdString(nameChangedDescription->GetTargetDesccription()->GetDescription()));

    action.insert("instruction", comm);
    action.insert("type", QString("straight"));
}

#endif
