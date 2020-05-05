/*
 *  Global.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 11/11/2018.
 *  Copyright 2018 Bill Sellers. All rights reserved.
 *
 */

#include "Global.h"
#include "GSUtil.h"
#include "Simulation.h"
#ifdef USE_QT
#include "Preferences.h"
#endif

#include <string>
#include <algorithm>
using namespace std::string_literals;

Global::Global()
{
    m_SpringConstant = m_ERP / (m_CFM * m_StepSize);
    m_DampingConstant = (1.0 - m_ERP) / m_CFM;
}

//Global::Global(const Global &global)
//{
//    *this = global;
//}

Global::~Global()
{
}

// assignment operator function
//Global& Global::operator=(const Global &global)
//{
//    if (this != &global)
//    {
//        m_FitnessType = global.m_FitnessType;
//        m_StepType = global.m_StepType;
//        m_AllowConnectedCollisions = global.m_AllowConnectedCollisions;
//        m_AllowInternalCollisions = global.m_AllowInternalCollisions;
//        m_Gravity = global.m_Gravity;
//        m_BMR = global.m_BMR;
//        m_CFM = global.m_CFM;
//        m_ContactMaxCorrectingVel = global.m_ContactMaxCorrectingVel;
//        m_ContactSurfaceLayer = global.m_ContactSurfaceLayer;
//        m_DampingConstant = global.m_DampingConstant;
//        m_ERP = global.m_ERP;
//        m_MechanicalEnergyLimit = global.m_MechanicalEnergyLimit;
//        m_MetabolicEnergyLimit = global.m_MetabolicEnergyLimit;
//        m_SpringConstant = global.m_SpringConstant;
//        m_StepSize = global.m_StepSize;
//        m_TimeLimit = global.m_TimeLimit;
//        m_WarehouseDecreaseThresholdFactor = global.m_WarehouseDecreaseThresholdFactor;
//        m_WarehouseFailDistanceAbort = global.m_WarehouseFailDistanceAbort;
//        m_WarehouseUnitIncreaseDistanceThreshold = global.m_WarehouseUnitIncreaseDistanceThreshold;
//        m_LinearDamping = global.m_LinearDamping;
//        m_AngularDamping = global.m_AngularDamping;
//        m_CurrentWarehouseFile = global.m_CurrentWarehouseFile;
//        m_DistanceTravelledBodyIDName = global.m_DistanceTravelledBodyIDName;
//        m_OutputModelStateFile = global.m_OutputModelStateFile;
//        m_MeshSearchPath = global.m_MeshSearchPath;
//        setSize1(global.size1());
//        setSize2(global.size2());
//        setSize3(global.size3());
//        setColour1(global.colour1());
//        setColour2(global.colour2());
//        setColour3(global.colour3());
//    }
//    return *this;
//}

double Global::SpringConstant() const
{
    return m_SpringConstant;
}

void Global::setSpringConstant(double SpringConstant)
{
    m_SpringConstant = SpringConstant;
}

double Global::DampingConstant() const
{
    return m_DampingConstant;
}

void Global::setDampingConstant(double DampingConstant)
{
    m_DampingConstant = DampingConstant;
}

std::string Global::MeshSearchPath() const
{
    return m_MeshSearchPath;
}

void Global::setMeshSearchPath(const std::string &MeshSearchPath)
{
    m_MeshSearchPath = MeshSearchPath;
}

double Global::LinearDamping() const
{
    return m_LinearDamping;
}

void Global::setLinearDamping(double LinearDamping)
{
    m_LinearDamping = LinearDamping;
}

double Global::AngularDamping() const
{
    return m_AngularDamping;
}

void Global::setAngularDamping(double AngularDamping)
{
    m_AngularDamping = AngularDamping;
}

// this function initialises the data in the object based on the contents
// of an xml_node node. It uses information from the simulation as required
// to satisfy dependencies
// it returns nullptr on success and a pointer to lastError() on failure
std::string *Global::createFromAttributes()
{
    if (NamedObject::createFromAttributes()) return lastErrorPtr();

    std::string buf;
    std::string buf2;
    double m_DoubleList[3];
    size_t i;

    // gravity
    if (findAttribute("GravityVector", &buf) == nullptr) return lastErrorPtr();
    GSUtil::Double(buf, 3, m_DoubleList);
    m_Gravity.Set(m_DoubleList);

    // set the simulation integration step size
    if (findAttribute("IntegrationStepSize", &buf) == nullptr) return lastErrorPtr();
    m_StepSize = GSUtil::Double(buf);

    // can specify ERP & CFM; SpringConstant & DampingConstant; SpringConstant & ERP; SpringConstant & CFM; DampingConstant & ERP; DampingConstant & CFM
    if (findAttribute("ERP", &buf) && findAttribute("CFM", &buf2))
    {
        m_ERP = GSUtil::Double(buf);
        m_CFM = GSUtil::Double(buf2);
        m_SpringConstant = m_ERP / (m_CFM * m_StepSize);
        m_DampingConstant = (1.0 - m_ERP) / m_CFM;
    }
    else if (findAttribute("ERP", &buf) && findAttribute("SpringConstant", &buf2))
    {
        m_ERP = GSUtil::Double(buf);
        m_SpringConstant = GSUtil::Double(buf2);
        m_DampingConstant = m_StepSize * (m_SpringConstant / m_ERP - m_SpringConstant);
        m_CFM = 1.0/(m_StepSize * m_SpringConstant + m_DampingConstant);
    }
    else if (findAttribute("ERP", &buf) && findAttribute("DampingConstant", &buf2))
    {
        m_ERP = GSUtil::Double(buf);
        m_DampingConstant = GSUtil::Double(buf2);
        m_SpringConstant = m_DampingConstant / (m_StepSize / m_ERP - m_StepSize);
        m_CFM = 1.0/(m_StepSize * m_SpringConstant + m_DampingConstant);
    }
    else if (findAttribute("CFM", &buf) && findAttribute("DampingConstant", &buf2))
    {
        m_CFM = GSUtil::Double(buf);
        m_DampingConstant = GSUtil::Double(buf2);
        m_SpringConstant = (1.0 / m_CFM - m_DampingConstant) / m_StepSize;
        m_ERP = m_StepSize * m_SpringConstant/(m_StepSize * m_SpringConstant + m_DampingConstant);
    }
    else if (findAttribute("CFM", &buf) && findAttribute("SpringConstant", &buf2))
    {
        m_CFM = GSUtil::Double(buf);
        m_SpringConstant = GSUtil::Double(buf2);
        m_DampingConstant = 1.0 / m_CFM - m_StepSize * m_SpringConstant;
        m_ERP = m_StepSize * m_SpringConstant/(m_StepSize * m_SpringConstant + m_DampingConstant);
    }
    else if (findAttribute("DampingConstant", &buf) && findAttribute("SpringConstant", &buf2))
    {
        m_DampingConstant = GSUtil::Double(buf);
        m_SpringConstant = GSUtil::Double(buf2);
        m_CFM = 1.0/(m_StepSize * m_SpringConstant + m_DampingConstant);
        m_ERP = m_StepSize * m_SpringConstant/(m_StepSize * m_SpringConstant + m_DampingConstant);
    }
    else
    {
        setLastError("Error: GLOBAL needs one of these pairs ERP & CFM; SpringConstant & DampingConstant; SpringConstant & ERP; SpringConstant & CFM; DampingConstant & ERP; DampingConstant & CFM"s);
        return lastErrorPtr();
    }

    if (findAttribute("ContactMaxCorrectingVel", &buf) == nullptr) return lastErrorPtr();
    m_ContactMaxCorrectingVel = GSUtil::Double(buf);

    if (findAttribute("ContactSurfaceLayer", &buf) == nullptr) return lastErrorPtr();
    m_ContactSurfaceLayer = GSUtil::Double(buf);


    // get the stepper required
    // WorldStep, accurate but slow
    // QuickStep, faster but less accurate
    findAttribute("StepType", &buf);
    for (i = 0; i < stepTypeCount; i++)
    {
        if (strcmp(buf.c_str(), stepTypeStrings(i)) == 0)
        {
            m_StepType = StepType(i);
            break;
        }
    }
    if (i > 1)
    {
        setLastError("GLOBAL: Unrecognised StepType=\""s + buf + "\""s);
        return lastErrorPtr();
    }

    // allow internal collisions
    if (findAttribute("AllowInternalCollisions", &buf) == nullptr) return lastErrorPtr();
    m_AllowInternalCollisions = GSUtil::Bool(buf);

    // allow collisions for objects connected by a joint
    findAttribute("AllowConnectedCollisions", &buf);
    if (buf.size()) m_AllowConnectedCollisions = GSUtil::Bool(buf);

    if (findAttribute("LinearDamping"s, &buf)) this->setLinearDamping(GSUtil::Double(buf));
    if (findAttribute("AngularDamping"s, &buf)) this->setAngularDamping(GSUtil::Double(buf));

    // now some run parameters

    if (findAttribute("BMR", &buf) == nullptr) return lastErrorPtr();
    m_BMR = GSUtil::Double(buf);

    if (findAttribute("TimeLimit", &buf) == nullptr) return lastErrorPtr();
    m_TimeLimit = GSUtil::Double(buf);
    if (findAttribute("MechanicalEnergyLimit", &buf) == nullptr) return lastErrorPtr();
    m_MechanicalEnergyLimit = GSUtil::Double(buf);
    if (findAttribute("MetabolicEnergyLimit", &buf) == nullptr) return lastErrorPtr();
    m_MetabolicEnergyLimit = GSUtil::Double(buf);
    if (findAttribute("DistanceTravelledBodyID", &buf) == nullptr) return lastErrorPtr();
    m_DistanceTravelledBodyIDName = buf;
    if (findAttribute("FitnessType", &buf) == nullptr) return lastErrorPtr();
    for (i = 0; i < fitnessTypeCount; i++)
    {
        if (strcmp(buf.c_str(), fitnessTypeStrings(i)) == 0)
        {
            m_FitnessType = FitnessType(i);
            break;
        }
    }
    if (i > 5)
    {
        setLastError("Error GLOBAL: Unrecognised FitnessType=\""s + buf + "\""s);
        return lastErrorPtr();
    }

    if (m_FitnessType == DistanceTravelled && m_DistanceTravelledBodyIDName.size() == 0)
    {
        setLastError("Error GLOBAL: must specify DistanceTravelledBodyIDName with FitnessType=\"DistanceTravelled\""s);
        return lastErrorPtr();
    }

    findAttribute("MeshSearchPath", &buf);
    if (buf.size()) m_MeshSearchPath = buf;

    findAttribute("WarehouseFailDistanceAbort", &buf);
    if (buf.size()) m_WarehouseFailDistanceAbort = GSUtil::Double(buf);

    findAttribute("WarehouseUnitIncreaseDistanceThreshold", &buf);
    if (buf.size()) m_WarehouseUnitIncreaseDistanceThreshold = GSUtil::Double(buf);

    findAttribute("WarehouseDecreaseThresholdFactor", &buf);
    if (buf.size()) m_WarehouseDecreaseThresholdFactor = GSUtil::Double(buf);

    findAttribute("CurrentWarehouse", &buf);
    if (buf.size()) m_CurrentWarehouseFile = buf;

    return nullptr;
}

// this function copies the data in the object to an xml_node node that it creates internally.
// doc is used to allocate the memory so deletion should be automatic
void Global::saveToAttributes()
{
    this->setTag("GLOBAL"s);
    this->clearAttributeMap();
    this->appendToAttributes();
}

void Global::appendToAttributes()
{
    NamedObject::appendToAttributes();
    std::string buf;

    // generated using regexp r'.*m_(.*);' to r'setAttribute("\1", *GSUtil::ToString(m_\1, &buf));'
    setAttribute("AllowConnectedCollisions", *GSUtil::ToString(m_AllowConnectedCollisions, &buf));
    setAttribute("AllowInternalCollisions", *GSUtil::ToString(m_AllowInternalCollisions, &buf));
    setAttribute("BMR", *GSUtil::ToString(m_BMR, &buf));
    setAttribute("CFM", *GSUtil::ToString(m_CFM, &buf));
    setAttribute("ContactMaxCorrectingVel", *GSUtil::ToString(m_ContactMaxCorrectingVel, &buf));
    setAttribute("ContactSurfaceLayer", *GSUtil::ToString(m_ContactSurfaceLayer, &buf));
    setAttribute("DistanceTravelledBodyID", m_DistanceTravelledBodyIDName);
    setAttribute("ERP", *GSUtil::ToString(m_ERP, &buf));
    setAttribute("FitnessType", fitnessTypeStrings(m_FitnessType));
    setAttribute("LinearDamping", *GSUtil::ToString(m_LinearDamping, &buf));
    setAttribute("AngularDamping", *GSUtil::ToString(m_AngularDamping, &buf));
    setAttribute("GravityVector", *GSUtil::ToString(m_Gravity, &buf));
    setAttribute("IntegrationStepSize", *GSUtil::ToString(m_StepSize, &buf));
    setAttribute("MechanicalEnergyLimit", *GSUtil::ToString(m_MechanicalEnergyLimit, &buf));
    setAttribute("MetabolicEnergyLimit", *GSUtil::ToString(m_MetabolicEnergyLimit, &buf));
    setAttribute("StepType", stepTypeStrings(m_StepType));
    setAttribute("TimeLimit", *GSUtil::ToString(m_TimeLimit, &buf));
    setAttribute("MeshSearchPath", m_MeshSearchPath);
    setAttribute("CurrentWarehouse", m_CurrentWarehouseFile);
    setAttribute("WarehouseDecreaseThresholdFactor", *GSUtil::ToString(m_WarehouseDecreaseThresholdFactor, &buf));
    setAttribute("WarehouseFailDistanceAbort", *GSUtil::ToString(m_WarehouseFailDistanceAbort, &buf));
    setAttribute("WarehouseUnitIncreaseDistanceThreshold", *GSUtil::ToString(m_WarehouseUnitIncreaseDistanceThreshold, &buf));
}


Global::FitnessType Global::fitnessType() const
{
    return m_FitnessType;
}

void Global::setFitnessType(FitnessType fitnessType)
{
    m_FitnessType = fitnessType;
}

Global::StepType Global::stepType() const
{
    return m_StepType;
}

void Global::setStepType(Global::StepType stepType)
{
    m_StepType = stepType;
}

bool Global::AllowConnectedCollisions() const
{
    return m_AllowConnectedCollisions;
}

void Global::setAllowConnectedCollisions(bool AllowConnectedCollisions)
{
    m_AllowConnectedCollisions = AllowConnectedCollisions;
}

bool Global::AllowInternalCollisions() const
{
    return m_AllowInternalCollisions;
}

void Global::setAllowInternalCollisions(bool AllowInternalCollisions)
{
    m_AllowInternalCollisions = AllowInternalCollisions;
}

pgd::Vector Global::Gravity() const
{
    return m_Gravity;
}

void Global::setGravity(const pgd::Vector &gravity)
{
    m_Gravity = gravity;
}

void Global::setGravity(double gravityX, double gravityY, double gravityZ)
{
    m_Gravity.Set(gravityX, gravityY, gravityZ);
}

double Global::BMR() const
{
    return m_BMR;
}

void Global::setBMR(double BMR)
{
    m_BMR = BMR;
}

double Global::CFM() const
{
    return m_CFM;
}

void Global::setCFM(double CFM)
{
    m_CFM = CFM;
}

double Global::ContactMaxCorrectingVel() const
{
    return m_ContactMaxCorrectingVel;
}

void Global::setContactMaxCorrectingVel(double ContactMaxCorrectingVel)
{
    m_ContactMaxCorrectingVel = ContactMaxCorrectingVel;
}

double Global::ContactSurfaceLayer() const
{
    return m_ContactSurfaceLayer;
}

void Global::setContactSurfaceLayer(double ContactSurfaceLayer)
{
    m_ContactSurfaceLayer = ContactSurfaceLayer;
}

double Global::ERP() const
{
    return m_ERP;
}

void Global::setERP(double ERP)
{
    m_ERP = ERP;
}

double Global::MechanicalEnergyLimit() const
{
    return m_MechanicalEnergyLimit;
}

void Global::setMechanicalEnergyLimit(double MechanicalEnergyLimit)
{
    m_MechanicalEnergyLimit = MechanicalEnergyLimit;
}

double Global::MetabolicEnergyLimit() const
{
    return m_MetabolicEnergyLimit;
}

void Global::setMetabolicEnergyLimit(double MetabolicEnergyLimit)
{
    m_MetabolicEnergyLimit = MetabolicEnergyLimit;
}

double Global::StepSize() const
{
    return m_StepSize;
}

void Global::setStepSize(double StepSize)
{
    m_StepSize = StepSize;
}

double Global::TimeLimit() const
{
    return m_TimeLimit;
}

void Global::setTimeLimit(double TimeLimit)
{
    m_TimeLimit = TimeLimit;
}

double Global::WarehouseDecreaseThresholdFactor() const
{
    return m_WarehouseDecreaseThresholdFactor;
}

void Global::setWarehouseDecreaseThresholdFactor(double WarehouseDecreaseThresholdFactor)
{
    m_WarehouseDecreaseThresholdFactor = WarehouseDecreaseThresholdFactor;
}

double Global::WarehouseFailDistanceAbort() const
{
    return m_WarehouseFailDistanceAbort;
}

void Global::setWarehouseFailDistanceAbort(double WarehouseFailDistanceAbort)
{
    m_WarehouseFailDistanceAbort = WarehouseFailDistanceAbort;
}

double Global::WarehouseUnitIncreaseDistanceThreshold() const
{
    return m_WarehouseUnitIncreaseDistanceThreshold;
}

void Global::setWarehouseUnitIncreaseDistanceThreshold(double WarehouseUnitIncreaseDistanceThreshold)
{
    m_WarehouseUnitIncreaseDistanceThreshold = WarehouseUnitIncreaseDistanceThreshold;
}

std::string Global::CurrentWarehouseFile() const
{
    return m_CurrentWarehouseFile;
}

void Global::setCurrentWarehouseFile(const std::string &CurrentWarehouse)
{
    m_CurrentWarehouseFile = CurrentWarehouse;
}

std::string Global::DistanceTravelledBodyIDName() const
{
    return m_DistanceTravelledBodyIDName;
}

void Global::setDistanceTravelledBodyIDName(const std::string &DistanceTravelledBodyIDName)
{
    m_DistanceTravelledBodyIDName = DistanceTravelledBodyIDName;
}



