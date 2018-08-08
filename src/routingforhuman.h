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

#ifndef ROUTINGFORHUMAN_H
#define ROUTINGFORHUMAN_H

#include <osmscout/RoutingService.h>
#include <osmscout/RoutePostprocessor.h>

#include <QObject>
#include <QString>

////////////////////////////////////////////////////////////
/// \brief The RoutingForHuman class
///
/// Provides routing instructions for humans
///
class RoutingForHuman : public QObject
{
    Q_OBJECT
public:
    explicit RoutingForHuman(QObject *parent = 0);
    virtual ~RoutingForHuman() {}

public:

    QString CrossingWaysDescriptionToString(const osmscout::RouteDescription::CrossingWaysDescription& crossingWaysDescription);

    void MoveToTurnCommand(osmscout::RouteDescription::DirectionDescription::Move move,
                                  QString &txt,
                                  QString &type);

    void DumpStartDescription(QJsonObject& action,
                                     const osmscout::RouteDescription::StartDescriptionRef& startDescription,
                                     const osmscout::RouteDescription::NameDescriptionRef& nameDescription);

    void DumpTargetDescription(QJsonObject& action,
                                      const osmscout::RouteDescription::TargetDescriptionRef& targetDescription);

    void DumpTurnDescription(QJsonObject& action,
                                    const osmscout::RouteDescription::TurnDescriptionRef& /*turnDescription*/,
                                    const osmscout::RouteDescription::CrossingWaysDescriptionRef& crossingWaysDescription,
                                    const osmscout::RouteDescription::DirectionDescriptionRef& directionDescription,
                                    const osmscout::RouteDescription::NameDescriptionRef& nameDescription);

    void DumpRoundaboutEnterDescription(QJsonObject& action,
                                               const osmscout::RouteDescription::RoundaboutEnterDescriptionRef& /*roundaboutEnterDescription*/,
                                               const osmscout::RouteDescription::CrossingWaysDescriptionRef& crossingWaysDescription);

    void DumpRoundaboutLeaveDescription(QJsonObject& action,
                                               const osmscout::RouteDescription::RoundaboutLeaveDescriptionRef& roundaboutLeaveDescription,
                                               const osmscout::RouteDescription::NameDescriptionRef& nameDescription);

    void DumpMotorwayEnterDescription(QJsonObject& action,
                                             const osmscout::RouteDescription::MotorwayEnterDescriptionRef& motorwayEnterDescription,
                                             const osmscout::RouteDescription::CrossingWaysDescriptionRef& crossingWaysDescription);

    void DumpMotorwayChangeDescription(QJsonObject& action,
                                              const osmscout::RouteDescription::MotorwayChangeDescriptionRef& motorwayChangeDescription,
                                              const osmscout::RouteDescription::MotorwayJunctionDescriptionRef& motorwayJunctionDescription);

    void DumpMotorwayLeaveDescription(QJsonObject& action,
                                             const osmscout::RouteDescription::MotorwayLeaveDescriptionRef& motorwayLeaveDescription,
                                             const osmscout::RouteDescription::MotorwayJunctionDescriptionRef& motorwayJunctionDescription,
                                             const osmscout::RouteDescription::DirectionDescriptionRef& directionDescription,
                                             const osmscout::RouteDescription::NameDescriptionRef& nameDescription);

    void DumpNameChangedDescription(QJsonObject& action,
                                           const osmscout::RouteDescription::NameChangedDescriptionRef& nameChangedDescription);
};

#endif // ROUTINGFORHUMAN_H
