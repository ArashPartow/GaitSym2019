/*
 *  StackedBoxCarDriver.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on Thu Feb 21 2013.
 *  Copyright (c) 2013 Bill Sellers. All rights reserved.
 *
 *  Uses a cyclic boxcar function to return a the value at a given time
 *
 */

#include "StackedBoxCarDriver.h"
#include "GSUtil.h"



#include <algorithm>

using namespace std::string_literals;

StackedBoxcarDriver::StackedBoxcarDriver()
{
}

StackedBoxcarDriver::~StackedBoxcarDriver()
{
}

void StackedBoxcarDriver::SetStackSize(size_t StackSize)
{
    m_StackSize = StackSize;
    m_Delays.resize(m_StackSize);
    m_Widths.resize(m_StackSize);
    m_Heights.resize(m_StackSize);
}

// these parameters control the shape of the box car and when it occurs
// CycleTime - this is the period of the boxcar
// Delay     - value from 0 to 1 used to control the phase of the function
// Width     - value from 0 to 1 used to control the width of the function
// Height    - value when the box car is active (otherwise the output is zero)
// Note: Delay and Width values are subtracted from the floor value to guarantee a value from 0 to 1

void StackedBoxcarDriver::SetCycleTime(double CycleTime)
{
    m_CycleTime = CycleTime;
}

void StackedBoxcarDriver::SetDelays(double *Delays)
{
    for (size_t i = 0; i < m_StackSize; i++) m_Delays[i] = Delays[i] - floor(Delays[i]);
}

void StackedBoxcarDriver::SetWidths(double *Widths)
{
    for (size_t i = 0; i < m_StackSize; i++) m_Widths[i] = Widths[i] - floor(Widths[i]);
}

void StackedBoxcarDriver::SetHeights(double *Heights)
{
    for (size_t i = 0; i < m_StackSize; i++) m_Heights[i] = Heights[i];
}


double StackedBoxcarDriver::GetValue(double Time)
{
    if (Time == LastTime()) return LastValue();
    setLastTime(Time);

    double Output = 0;
    double OffTime;
    // get a normalised cycle time (value from 0 to 1)
    double NormalisedCycleTime = (Time / m_CycleTime) - floor(Time / m_CycleTime);


    for (size_t i = 0; i < m_StackSize; i++)
    {
        OffTime = m_Delays[i] + m_Widths[i];
        if (OffTime < 1) // no wrap case
        {
            if (NormalisedCycleTime > m_Delays[i] && NormalisedCycleTime < OffTime) Output += m_Heights[i];
        }
        else // wrap case
        {
            if (NormalisedCycleTime < OffTime - 1 || NormalisedCycleTime > m_Delays[i]) Output += m_Heights[i];
        }
    }

    setLastValue(Clamp(Output));
    return LastValue();
}

// this function initialises the data in the object based on the contents
// of an xml_node node. It uses information from the simulation as required
// to satisfy dependencies
// it returns nullptr on success and a pointer to lastError() on failure
std::string *StackedBoxcarDriver::CreateFromAttributes()
{
    if (Driver::CreateFromAttributes()) return lastErrorPtr();

    std::string buf;
    if (GetAttribute("StackSize"s, &buf) == nullptr) return lastErrorPtr();
    this->SetStackSize(size_t(GSUtil::Int(buf)));

    buf.reserve(m_StackSize * 32);
    std::vector<double> doubleList;
    doubleList.reserve(m_StackSize);

    if (GetAttribute("CycleTime"s, &buf) == nullptr) return lastErrorPtr();
    this->SetCycleTime(GSUtil::Double(buf));
    if (GetAttribute("Delays"s, &buf) == nullptr) return lastErrorPtr();
    this->SetDelays(GSUtil::Double(buf, int(m_StackSize), doubleList.data()));
    if (GetAttribute("Widths"s, &buf) == nullptr) return lastErrorPtr();
    this->SetWidths(GSUtil::Double(buf, int(m_StackSize), doubleList.data()));
    if (GetAttribute("Heights"s, &buf) == nullptr) return lastErrorPtr();
    this->SetHeights(GSUtil::Double(buf, int(m_StackSize), doubleList.data()));

    return nullptr;
}

// this function appends data to a pre-existing xml_node - often created by XMLSave
void StackedBoxcarDriver::AppendToAttributes()
{
    Driver::AppendToAttributes();
    std::string buf;
    buf.reserve(m_StackSize * 32); // should be big enough but it will grow if necessary anyway
    setAttribute("Type"s, "StackedBoxcar"s);
    setAttribute("StackSize", *GSUtil::ToString(m_StackSize, &buf));
    setAttribute("CycleTime", *GSUtil::ToString(m_CycleTime, &buf));
    setAttribute("Delays", *GSUtil::ToString(m_Delays.data(), m_StackSize, &buf));
    setAttribute("Widths", *GSUtil::ToString(m_Widths.data(), m_StackSize, &buf));
    setAttribute("Heights", *GSUtil::ToString(m_Heights.data(), m_StackSize, &buf));
}


