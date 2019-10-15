// EnergyPlus, Copyright (c) 1996-2019, The Board of Trustees of the University of Illinois,
// The Regents of the University of California, through Lawrence Berkeley National Laboratory
// (subject to receipt of any required approvals from the U.S. Dept. of Energy), Oak Ridge
// National Laboratory, managed by UT-Battelle, Alliance for Sustainable Energy, LLC, and other
// contributors. All rights reserved.
//
// NOTICE: This Software was developed under funding from the U.S. Department of Energy and the
// U.S. Government consequently retains certain rights. As such, the U.S. Government has been
// granted for itself and others acting on its behalf a paid-up, nonexclusive, irrevocable,
// worldwide license in the Software to reproduce, distribute copies to the public, prepare
// derivative works, and perform publicly and display publicly, and to permit others to do so.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted
// provided that the following conditions are met:
//
// (1) Redistributions of source code must retain the above copyright notice, this list of
//     conditions and the following disclaimer.
//
// (2) Redistributions in binary form must reproduce the above copyright notice, this list of
//     conditions and the following disclaimer in the documentation and/or other materials
//     provided with the distribution.
//
// (3) Neither the name of the University of California, Lawrence Berkeley National Laboratory,
//     the University of Illinois, U.S. Dept. of Energy nor the names of its contributors may be
//     used to endorse or promote products derived from this software without specific prior
//     written permission.
//
// (4) Use of EnergyPlus(TM) Name. If Licensee (i) distributes the software in stand-alone form
//     without changes from the version obtained under this License, or (ii) Licensee makes a
//     reference solely to the software portion of its product, Licensee must refer to the
//     software as "EnergyPlus version X" software, where "X" is the version number Licensee
//     obtained under this License and may not use a different name for the software. Except as
//     specifically required in this Section (4), Licensee shall not use in a company name, a
//     product name, in advertising, publicity, or other promotional activities any name, trade
//     name, trademark, logo, or other designation of "EnergyPlus", "E+", "e+" or confusingly
//     similar designation, without the U.S. Department of Energy's prior written consent.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

// C++ Headers
#include <cmath>

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/CTElectricGenerator.hh>
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataPlant.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutAirNodeManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus {

namespace CTElectricGenerator {

    //__________________________________________________________________________;
    // BLAST inherited generators:
    // CTElectricGenerator (COMBUSTION Turbine)

    // MODULE INFORMATION:
    //       AUTHOR         Dan Fisher
    //       DATE WRITTEN   Sept 2000
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS MODULE:
    // This module simulates the performance of the COMBUSTION turbine
    // Generators.

    // METHODOLOGY EMPLOYED:
    // Once the Electric power manager determines that the CT Generator
    // is available, it calls SimCTGenerator which in turn calls the
    // appropriate COMBUSTION turbine Generator model.
    // All CT Generator models are based on a polynomial fit of Generator
    // performance data.

    int NumCTGenerators(0); // number of CT Generators specified in input
    bool GetCTInput(true);  // then TRUE, calls subroutine to read input file.

    Array1D<CTGeneratorSpecs> CTGenerator; // dimension to number of machines

    void SimCTGenerator(int const EP_UNUSED(GeneratorType), // type of Generator
                        std::string const &GeneratorName,   // user specified name of Generator
                        int &GeneratorIndex,
                        bool const RunFlag,  // simulate Generator when TRUE
                        Real64 const MyLoad, // generator demand
                        bool const FirstHVACIteration)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Dan Fisher
        //       DATE WRITTEN   Sept. 2000
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE: This is the CT Generator driver.  It
        // gets the input for the models, initializes simulation variables, call
        // the appropriate model and sets up reporting variables.

        int genNum; // Generator number counter

        // Get Generator data from input file
        if (GetCTInput) {
            GetCTGeneratorInput();
            GetCTInput = false;
        }

        // SELECT and CALL MODELS
        if (GeneratorIndex == 0) {
            genNum = UtilityRoutines::FindItemInList(GeneratorName, CTGenerator);
            if (genNum == 0) ShowFatalError("SimCTGenerator: Specified Generator not one of Valid COMBUSTION Turbine Generators " + GeneratorName);
            GeneratorIndex = genNum;
        } else {
            genNum = GeneratorIndex;
            if (genNum > NumCTGenerators || genNum < 1) {
                ShowFatalError("SimCTGenerator: Invalid GeneratorIndex passed=" + General::TrimSigDigits(genNum) +
                               ", Number of CT Engine Generators=" + General::TrimSigDigits(NumCTGenerators) + ", Generator name=" + GeneratorName);
            }
            if (CTGenerator(genNum).CheckEquipName) {
                if (GeneratorName != CTGenerator(genNum).Name) {
                    ShowFatalError("SimCTGenerator: Invalid GeneratorIndex passed=" + General::TrimSigDigits(genNum) + ", Generator name=" + GeneratorName +
                                   ", stored Generator Name for that index=" + CTGenerator(genNum).Name);
                }
                CTGenerator(genNum).CheckEquipName = false;
            }
        }

        InitCTGenerators(genNum, RunFlag, MyLoad, FirstHVACIteration);
        CalcCTGeneratorModel(genNum, RunFlag, MyLoad, FirstHVACIteration);
        UpdateCTGeneratorRecords(RunFlag, genNum);
    }

    void SimCTPlantHeatRecovery(std::string const &EP_UNUSED(CompType), // unused1208
                                std::string const &CompName,
                                int const EP_UNUSED(CompTypeNum), // unused1208
                                int &CompNum,
                                bool const EP_UNUSED(RunFlag),
                                bool &InitLoopEquip,
                                Real64 &EP_UNUSED(MyLoad),
                                Real64 &MaxCap,
                                Real64 &MinCap,
                                Real64 &OptCap,
                                bool const EP_UNUSED(FirstHVACIteration) // TRUE if First iteration of simulation
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         BGriffith
        //       DATE WRITTEN   March 2008
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Fill data needed in PlantLoopEquipments

        if (GetCTInput) {
            GetCTGeneratorInput();
            GetCTInput = false;
        }

        if (InitLoopEquip) {
            CompNum = UtilityRoutines::FindItemInList(CompName, CTGenerator);
            if (CompNum == 0) {
                ShowFatalError("SimCTPlantHeatRecovery: CT Generator Unit not found=" + CompName);
                return;
            }
            MinCap = 0.0;
            MaxCap = 0.0;
            OptCap = 0.0;
            return;
        } // End Of InitLoopEquip
    }

    // End CT Generator Module Driver Subroutines
    //******************************************************************************

    // Beginning of CT Generator Module Get Input subroutines
    //******************************************************************************

    void GetCTGeneratorInput()
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR:          Dan Fisher
        //       DATE WRITTEN:    April 2000

        // PURPOSE OF THIS SUBROUTINE:
        // This routine will get the input
        // required by the CT Generator models.

        int NumAlphas;                  // Number of elements in the alpha array
        int NumNums;                    // Number of elements in the numeric array
        int IOStat;                     // IO Status when calling get input subroutine
        Array1D_string AlphArray(12);   // character string data
        Array1D<Real64> NumArray(12);   // numeric data
        static bool ErrorsFound(false); // error flag

        DataIPShortCuts::cCurrentModuleObject = "Generator:CombustionTurbine";
        NumCTGenerators = inputProcessor->getNumObjectsFound(DataIPShortCuts::cCurrentModuleObject);

        if (NumCTGenerators <= 0) {
            ShowSevereError("No " + DataIPShortCuts::cCurrentModuleObject + " equipment specified in input file");
            ErrorsFound = true;
        }

        // ALLOCATE ARRAYS
        CTGenerator.allocate(NumCTGenerators);

        // LOAD ARRAYS WITH CT CURVE FIT Generator DATA
        for (int genNum = 1; genNum <= NumCTGenerators; ++genNum) {
            inputProcessor->getObjectItem(DataIPShortCuts::cCurrentModuleObject,
                                          genNum,
                                          AlphArray,
                                          NumAlphas,
                                          NumArray,
                                          NumNums,
                                          IOStat,
                                          _,
                                          DataIPShortCuts::lAlphaFieldBlanks,
                                          DataIPShortCuts::cAlphaFieldNames,
                                          DataIPShortCuts::cNumericFieldNames);
            UtilityRoutines::IsNameEmpty(AlphArray(1), DataIPShortCuts::cCurrentModuleObject, ErrorsFound);

            CTGenerator(genNum).Name = AlphArray(1);

            CTGenerator(genNum).RatedPowerOutput = NumArray(1);
            if (NumArray(1) == 0.0) {
                ShowSevereError("Invalid " + DataIPShortCuts::cNumericFieldNames(1) + '=' + General::RoundSigDigits(NumArray(1), 2));
                ShowContinueError("Entered in " + DataIPShortCuts::cCurrentModuleObject + '=' + AlphArray(1));
                ErrorsFound = true;
            }

            // Not sure what to do with electric nodes, so do not use optional arguments
            CTGenerator(genNum).ElectricCircuitNode = NodeInputManager::GetOnlySingleNode(
                AlphArray(2), ErrorsFound, DataIPShortCuts::cCurrentModuleObject, AlphArray(1), DataLoopNode::NodeType_Electric, DataLoopNode::NodeConnectionType_Electric, 1, DataLoopNode::ObjectIsNotParent);

            CTGenerator(genNum).MinPartLoadRat = NumArray(2);
            CTGenerator(genNum).MaxPartLoadRat = NumArray(3);
            CTGenerator(genNum).OptPartLoadRat = NumArray(4);

            // Load Special CT Generator Input

            CTGenerator(genNum).PLBasedFuelInputCurve = CurveManager::GetCurveIndex(AlphArray(3)); // convert curve name to number
            if (CTGenerator(genNum).PLBasedFuelInputCurve == 0) {
                ShowSevereError("Invalid " + DataIPShortCuts::cAlphaFieldNames(3) + '=' + AlphArray(3));
                ShowContinueError("Entered in " + DataIPShortCuts::cCurrentModuleObject + '=' + AlphArray(1));
                ErrorsFound = true;
            }

            CTGenerator(genNum).TempBasedFuelInputCurve = CurveManager::GetCurveIndex(AlphArray(4)); // convert curve name to number
            if (CTGenerator(genNum).TempBasedFuelInputCurve == 0) {
                ShowSevereError("Invalid " + DataIPShortCuts::cAlphaFieldNames(4) + '=' + AlphArray(4));
                ShowContinueError("Entered in " + DataIPShortCuts::cCurrentModuleObject + '=' + AlphArray(1));
                ErrorsFound = true;
            }

            CTGenerator(genNum).ExhaustFlowCurve = CurveManager::GetCurveIndex(AlphArray(5)); // convert curve name to number
            if (CTGenerator(genNum).ExhaustFlowCurve == 0) {
                ShowSevereError("Invalid " + DataIPShortCuts::cAlphaFieldNames(5) + '=' + AlphArray(5));
                ShowContinueError("Entered in " + DataIPShortCuts::cCurrentModuleObject + '=' + AlphArray(1));
                ErrorsFound = true;
            }

            CTGenerator(genNum).PLBasedExhaustTempCurve = CurveManager::GetCurveIndex(AlphArray(6)); // convert curve name to number
            if (CTGenerator(genNum).PLBasedExhaustTempCurve == 0) {
                ShowSevereError("Invalid " + DataIPShortCuts::cAlphaFieldNames(6) + '=' + AlphArray(6));
                ShowContinueError("Entered in " + DataIPShortCuts::cCurrentModuleObject + '=' + AlphArray(1));
                ErrorsFound = true;
            }

            CTGenerator(genNum).TempBasedExhaustTempCurve = CurveManager::GetCurveIndex(AlphArray(7)); // convert curve name to number
            if (CTGenerator(genNum).TempBasedExhaustTempCurve == 0) {
                ShowSevereError("Invalid " + DataIPShortCuts::cAlphaFieldNames(7) + '=' + AlphArray(7));
                ShowContinueError("Entered in " + DataIPShortCuts::cCurrentModuleObject + '=' + AlphArray(1));
                ErrorsFound = true;
            }

            CTGenerator(genNum).QLubeOilRecoveredCurve = CurveManager::GetCurveIndex(AlphArray(8)); // convert curve name to number
            if (CTGenerator(genNum).QLubeOilRecoveredCurve == 0) {
                ShowSevereError("Invalid " + DataIPShortCuts::cAlphaFieldNames(8) + '=' + AlphArray(8));
                ShowContinueError("Entered in " + DataIPShortCuts::cCurrentModuleObject + '=' + AlphArray(1));
                ErrorsFound = true;
            }

            CTGenerator(genNum).UACoef(1) = NumArray(5);
            CTGenerator(genNum).UACoef(2) = NumArray(6);

            CTGenerator(genNum).MaxExhaustperCTPower = NumArray(7);
            CTGenerator(genNum).DesignMinExitGasTemp = NumArray(8);
            CTGenerator(genNum).DesignAirInletTemp = NumArray(9);
            CTGenerator(genNum).FuelHeatingValue = NumArray(10);
            CTGenerator(genNum).DesignHeatRecVolFlowRate = NumArray(11);

            if (CTGenerator(genNum).DesignHeatRecVolFlowRate > 0.0) {
                CTGenerator(genNum).HeatRecActive = true;
                CTGenerator(genNum).HeatRecInletNodeNum = NodeInputManager::GetOnlySingleNode(
                    AlphArray(9), ErrorsFound, DataIPShortCuts::cCurrentModuleObject, AlphArray(1), DataLoopNode::NodeType_Water, DataLoopNode::NodeConnectionType_Inlet, 1, DataLoopNode::ObjectIsNotParent);
                if (CTGenerator(genNum).HeatRecInletNodeNum == 0) {
                    ShowSevereError("Missing Node Name, Heat Recovery Inlet, for " + DataIPShortCuts::cCurrentModuleObject + '=' + AlphArray(1));
                    ErrorsFound = true;
                }
                CTGenerator(genNum).HeatRecOutletNodeNum = NodeInputManager::GetOnlySingleNode(
                    AlphArray(10), ErrorsFound, DataIPShortCuts::cCurrentModuleObject, AlphArray(1), DataLoopNode::NodeType_Water, DataLoopNode::NodeConnectionType_Outlet, 1, DataLoopNode::ObjectIsNotParent);
                if (CTGenerator(genNum).HeatRecOutletNodeNum == 0) {
                    ShowSevereError("Missing Node Name, Heat Recovery Outlet, for " + DataIPShortCuts::cCurrentModuleObject + '=' + AlphArray(1));
                    ErrorsFound = true;
                }
                BranchNodeConnections::TestCompSet(DataIPShortCuts::cCurrentModuleObject, AlphArray(1), AlphArray(9), AlphArray(10), "Heat Recovery Nodes");
                PlantUtilities::RegisterPlantCompDesignFlow(CTGenerator(genNum).HeatRecInletNodeNum, CTGenerator(genNum).DesignHeatRecVolFlowRate);
            } else {
                CTGenerator(genNum).HeatRecActive = false;
                CTGenerator(genNum).HeatRecInletNodeNum = 0;
                CTGenerator(genNum).HeatRecOutletNodeNum = 0;
                if (!DataIPShortCuts::lAlphaFieldBlanks(9) || !DataIPShortCuts::lAlphaFieldBlanks(10)) {
                    ShowWarningError("Since Design Heat Flow Rate = 0.0, Heat Recovery inactive for " + DataIPShortCuts::cCurrentModuleObject + '=' + AlphArray(1));
                    ShowContinueError("However, Node names were specified for Heat Recovery inlet or outlet nodes");
                }
            }

            // Fuel Type Case Statement
            {
                auto const SELECT_CASE_var(AlphArray(11));
                if (is_blank(SELECT_CASE_var)) { // If blank then the default is Natural Gas
                    CTGenerator(genNum).FuelType = "Gas";

                } else if ((SELECT_CASE_var == "GAS") || (SELECT_CASE_var == "NATURALGAS") || (SELECT_CASE_var == "NATURAL GAS")) {
                    CTGenerator(genNum).FuelType = "Gas";

                } else if (SELECT_CASE_var == "DIESEL") {
                    CTGenerator(genNum).FuelType = "Diesel";

                } else if (SELECT_CASE_var == "GASOLINE") {
                    CTGenerator(genNum).FuelType = "Gasoline";

                } else if ((SELECT_CASE_var == "FUEL OIL #1") || (SELECT_CASE_var == "FUELOIL#1") || (SELECT_CASE_var == "FUEL OIL") ||
                           (SELECT_CASE_var == "DISTILLATE OIL")) {
                    CTGenerator(genNum).FuelType = "FuelOil#1";

                } else if ((SELECT_CASE_var == "FUEL OIL #2") || (SELECT_CASE_var == "FUELOIL#2") || (SELECT_CASE_var == "RESIDUAL OIL")) {
                    CTGenerator(genNum).FuelType = "FuelOil#2";

                } else if ((SELECT_CASE_var == "PROPANE") || (SELECT_CASE_var == "LPG") || (SELECT_CASE_var == "PROPANEGAS") ||
                           (SELECT_CASE_var == "PROPANE GAS")) {
                    CTGenerator(genNum).FuelType = "Propane";

                } else if (SELECT_CASE_var == "OTHERFUEL1") {
                    CTGenerator(genNum).FuelType = "OtherFuel1";

                } else if (SELECT_CASE_var == "OTHERFUEL2") {
                    CTGenerator(genNum).FuelType = "OtherFuel2";

                } else {
                    ShowSevereError("Invalid " + DataIPShortCuts::cAlphaFieldNames(11) + '=' + AlphArray(11));
                    ShowContinueError("Entered in " + DataIPShortCuts::cCurrentModuleObject + '=' + AlphArray(1));
                    ErrorsFound = true;
                }
            }

            CTGenerator(genNum).HeatRecMaxTemp = NumArray(12);

            // begin CR7021
            if (DataIPShortCuts::lAlphaFieldBlanks(12)) {
                CTGenerator(genNum).OAInletNode = 0;
            } else {
                CTGenerator(genNum).OAInletNode = NodeInputManager::GetOnlySingleNode(AlphArray(12),
                                                                                      ErrorsFound,
                                                                                      DataIPShortCuts::cCurrentModuleObject,
                                                                                      AlphArray(1),
                                                                                      DataLoopNode::NodeType_Air,
                                                                                      DataLoopNode::NodeConnectionType_OutsideAirReference,
                                                                                      1,
                                                                                      DataLoopNode::ObjectIsNotParent);
                if (!OutAirNodeManager::CheckOutAirNodeNumber(CTGenerator(genNum).OAInletNode)) {
                    ShowSevereError(DataIPShortCuts::cCurrentModuleObject + ", \"" + CTGenerator(genNum).Name +
                                    "\" Outdoor Air Inlet Node Name not valid Outdoor Air Node= " + AlphArray(12));
                    ShowContinueError("...does not appear in an OutdoorAir:NodeList or as an OutdoorAir:Node.");
                    ErrorsFound = true;
                }
            }
        }

        if (ErrorsFound) {
            ShowFatalError("Errors found in processing input for " + DataIPShortCuts::cCurrentModuleObject);
        }

        for (int genNum = 1; genNum <= NumCTGenerators; ++genNum) {
            SetupOutputVariable("Generator Produced Electric Power",
                                OutputProcessor::Unit::W,
                                CTGenerator(genNum).ElecPowerGenerated,
                                "System",
                                "Average",
                                CTGenerator(genNum).Name);
            SetupOutputVariable("Generator Produced Electric Energy",
                                OutputProcessor::Unit::J,
                                CTGenerator(genNum).ElecEnergyGenerated,
                                "System",
                                "Sum",
                                CTGenerator(genNum).Name,
                                _,
                                "ElectricityProduced",
                                "COGENERATION",
                                _,
                                "Plant");

            SetupOutputVariable("Generator " + CTGenerator(genNum).FuelType + " Rate",
                                OutputProcessor::Unit::W,
                                CTGenerator(genNum).FuelEnergyUseRate,
                                "System",
                                "Average",
                                CTGenerator(genNum).Name);
            SetupOutputVariable("Generator " + CTGenerator(genNum).FuelType + " Energy",
                                OutputProcessor::Unit::J,
                                CTGenerator(genNum).FuelEnergy,
                                "System",
                                "Sum",
                                CTGenerator(genNum).Name,
                                _,
                                CTGenerator(genNum).FuelType,
                                "COGENERATION",
                                _,
                                "Plant");

            //    general fuel use report (to match other generators)
            SetupOutputVariable("Generator Fuel HHV Basis Rate",
                                OutputProcessor::Unit::W,
                                CTGenerator(genNum).FuelEnergyUseRate,
                                "System",
                                "Average",
                                CTGenerator(genNum).Name);
            SetupOutputVariable("Generator Fuel HHV Basis Energy",
                                OutputProcessor::Unit::J,
                                CTGenerator(genNum).FuelEnergy,
                                "System",
                                "Sum",
                                CTGenerator(genNum).Name);

            SetupOutputVariable("Generator " + CTGenerator(genNum).FuelType + " Mass Flow Rate",
                                OutputProcessor::Unit::kg_s,
                                CTGenerator(genNum).FuelMdot,
                                "System",
                                "Average",
                                CTGenerator(genNum).Name);

            SetupOutputVariable("Generator Exhaust Air Temperature",
                                OutputProcessor::Unit::C,
                                CTGenerator(genNum).ExhaustStackTemp,
                                "System",
                                "Average",
                                CTGenerator(genNum).Name);

            if (CTGenerator(genNum).HeatRecActive) {
                SetupOutputVariable("Generator Exhaust Heat Recovery Rate",
                                    OutputProcessor::Unit::W,
                                    CTGenerator(genNum).QExhaustRecovered,
                                    "System",
                                    "Average",
                                    CTGenerator(genNum).Name);
                SetupOutputVariable("Generator Exhaust Heat Recovery Energy",
                                    OutputProcessor::Unit::J,
                                    CTGenerator(genNum).ExhaustEnergyRec,
                                    "System",
                                    "Sum",
                                    CTGenerator(genNum).Name,
                                    _,
                                    "ENERGYTRANSFER",
                                    "HEATRECOVERY",
                                    _,
                                    "Plant");

                SetupOutputVariable("Generator Lube Heat Recovery Rate",
                                    OutputProcessor::Unit::W,
                                    CTGenerator(genNum).QLubeOilRecovered,
                                    "System",
                                    "Average",
                                    CTGenerator(genNum).Name);
                SetupOutputVariable("Generator Lube Heat Recovery Energy",
                                    OutputProcessor::Unit::J,
                                    CTGenerator(genNum).LubeOilEnergyRec,
                                    "System",
                                    "Sum",
                                    CTGenerator(genNum).Name,
                                    _,
                                    "ENERGYTRANSFER",
                                    "HEATRECOVERY",
                                    _,
                                    "Plant");

                SetupOutputVariable("Generator Produced Thermal Rate",
                                    OutputProcessor::Unit::W,
                                    CTGenerator(genNum).QTotalHeatRecovered,
                                    "System",
                                    "Average",
                                    CTGenerator(genNum).Name);
                SetupOutputVariable("Generator Produced Thermal Energy",
                                    OutputProcessor::Unit::J,
                                    CTGenerator(genNum).TotalHeatEnergyRec,
                                    "System",
                                    "Sum",
                                    CTGenerator(genNum).Name);

                SetupOutputVariable("Generator Heat Recovery Inlet Temperature",
                                    OutputProcessor::Unit::C,
                                    CTGenerator(genNum).HeatRecInletTemp,
                                    "System",
                                    "Average",
                                    CTGenerator(genNum).Name);
                SetupOutputVariable("Generator Heat Recovery Outlet Temperature",
                                    OutputProcessor::Unit::C,
                                    CTGenerator(genNum).HeatRecOutletTemp,
                                    "System",
                                    "Average",
                                    CTGenerator(genNum).Name);
                SetupOutputVariable("Generator Heat Recovery Mass Flow Rate",
                                    OutputProcessor::Unit::kg_s,
                                    CTGenerator(genNum).HeatRecMdot,
                                    "System",
                                    "Average",
                                    CTGenerator(genNum).Name);
            }
        }
    }

    void CalcCTGeneratorModel(int const genNum, // Generator number
                              bool const RunFlag,     // TRUE when Generator operating
                              Real64 const MyLoad,    // Generator demand
                              bool const FirstHVACIteration)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Dan Fisher
        //       DATE WRITTEN   Sept. 2000
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // simulate a vapor compression Generator using the CT model

        // METHODOLOGY EMPLOYED:
        // curve fit of performance data.  This model was originally
        // developed by Dale Herron for the BLAST program

        Real64 const ExhaustCP(1.047); // Exhaust Gas Specific Heat (J/kg-K)
        Real64 const KJtoJ(1000.0);    // convert Kjoules to joules
        static std::string const RoutineName("CalcCTGeneratorModel");

        // min allowed operating frac full load
        Real64 MinPartLoadRat = CTGenerator(genNum).MinPartLoadRat;

        // max allowed operating frac full load
        Real64 MaxPartLoadRat = CTGenerator(genNum).MaxPartLoadRat;

        // Generator nominal capacity (W)
        Real64 RatedPowerOutput = CTGenerator(genNum).RatedPowerOutput;

        // MAX EXHAUST FLOW PER W POWER OUTPUT COEFF
        Real64 MaxExhaustperCTPower = CTGenerator(genNum).MaxExhaustperCTPower;

        // design turbine inlet temperature (C)
        Real64 DesignAirInletTemp = CTGenerator(genNum).DesignAirInletTemp;

        int HeatRecInNode; // Heat Recovery Fluid Inlet Node Num
        Real64 HeatRecInTemp; // Heat Recovery Fluid Inlet Temperature (C)

        Real64 HeatRecMdot;      // Heat Recovery Fluid Mass FlowRate (kg/s)
        Real64 HeatRecCp;        // Specific Heat of the Heat Recovery Fluid (J/kg-K)

        if (CTGenerator(genNum).HeatRecActive) {
            HeatRecInNode = CTGenerator(genNum).HeatRecInletNodeNum;
            HeatRecInTemp = DataLoopNode::Node(HeatRecInNode).Temp;

            HeatRecCp = FluidProperties::GetSpecificHeatGlycol(DataPlant::PlantLoop(CTGenerator(genNum).HRLoopNum).FluidName,
                                              HeatRecInTemp,
                                              DataPlant::PlantLoop(CTGenerator(genNum).HRLoopNum).FluidIndex,
                                              RoutineName);
            if (FirstHVACIteration && RunFlag) {
                HeatRecMdot = CTGenerator(genNum).DesignHeatRecMassFlowRate;
            } else {
                HeatRecMdot = DataLoopNode::Node(HeatRecInNode).MassFlowRate;
            }
        } else {
            HeatRecInTemp = 0.0;
            HeatRecCp = 0.0;
            HeatRecMdot = 0.0;
        }

        // If no loop demand or Generator OFF, return
        if (!RunFlag) {
            CTGenerator(genNum).ElecPowerGenerated = 0.0;
            CTGenerator(genNum).ElecEnergyGenerated = 0.0;
            CTGenerator(genNum).HeatRecInletTemp = HeatRecInTemp;
            CTGenerator(genNum).HeatRecOutletTemp = HeatRecInTemp;
            CTGenerator(genNum).HeatRecMdot = 0.0;
            CTGenerator(genNum).QLubeOilRecovered = 0.0;
            CTGenerator(genNum).QExhaustRecovered = 0.0;
            CTGenerator(genNum).QTotalHeatRecovered = 0.0;
            CTGenerator(genNum).LubeOilEnergyRec = 0.0;
            CTGenerator(genNum).ExhaustEnergyRec = 0.0;
            CTGenerator(genNum).TotalHeatEnergyRec = 0.0;
            CTGenerator(genNum).FuelEnergyUseRate = 0.0;
            CTGenerator(genNum).FuelEnergy = 0.0;
            CTGenerator(genNum).FuelMdot = 0.0;
            CTGenerator(genNum).ExhaustStackTemp = 0.0;
            return;
        }

        // CALCULATE POWER GENERATED AND PLR
        // Generator output (W)
        Real64 ElecPowerGenerated = min(MyLoad, RatedPowerOutput);
        ElecPowerGenerated = max(ElecPowerGenerated, 0.0);

        // Generator operating part load ratio
        Real64 PLR = min(ElecPowerGenerated / RatedPowerOutput, MaxPartLoadRat);
        PLR = max(PLR, MinPartLoadRat);
        ElecPowerGenerated = PLR * RatedPowerOutput;

        // SET OFF-DESIGN AIR TEMPERATURE DIFFERENCE
        // (ATAIR) Difference between ambient actual and ambient design temperatures
        Real64 AmbientDeltaT;
        if (CTGenerator(genNum).OAInletNode == 0) {
            AmbientDeltaT = DataEnvironment::OutDryBulbTemp - DesignAirInletTemp;
        } else {
            AmbientDeltaT = DataLoopNode::Node(CTGenerator(genNum).OAInletNode).Temp - DesignAirInletTemp;
        }

        // Use Curve fit to determine Fuel Energy Input.  For electric power generated in Watts, the fuel
        // energy input is calculated in J/s.  The PLBasedFuelInputCurve selects ratio of fuel flow (J/s)/power generated (J/s).
        // The TempBasedFuelInputCurve is a correction based on deviation from design inlet air temperature conditions.
        // The first coefficient of this fit should be 1.0 to ensure that no correction is made at design conditions.
        // (EFUEL) rate of Fuel Energy Required to run COMBUSTION turbine (W)
        Real64 FuelUseRate = ElecPowerGenerated * CurveManager::CurveValue(CTGenerator(genNum).PLBasedFuelInputCurve, PLR) *
                      CurveManager::CurveValue(CTGenerator(genNum).TempBasedFuelInputCurve, AmbientDeltaT);

        // Use Curve fit to determine Exhaust Flow.  This curve shows the ratio of exhaust gas flow (kg/s) to electric power
        // output (J/s).  The units on ExhaustFlowCurve are (kg/J).  When multiplied by the rated power of the unit,
        // it gives the exhaust flow rate in kg/s
        // (FEX) Exhaust Gas Flow Rate cubic meters per second???
        Real64 ExhaustFlow = RatedPowerOutput * CurveManager::CurveValue(CTGenerator(genNum).ExhaustFlowCurve, AmbientDeltaT);

        // Use Curve fit to determine Exhaust Temperature.  This curve calculates the exhaust temperature (C) by
        // multiplying the exhaust temperature (C) for a particular part load as given by PLBasedExhaustTempCurve
        // a correction factor based on the deviation from design temperature, TempBasedExhaustTempCurve

        Real64 QExhaustRec; // recovered exhaust heat (W)
        Real64 ExhaustStackTemp;  // turbine stack temp. (C)
        if ((PLR > 0.0) && ((ExhaustFlow > 0.0) || (MaxExhaustperCTPower > 0.0))) {

            // (TEX) Exhaust Gas Temperature in C
            Real64 ExhaustTemp = CurveManager::CurveValue(CTGenerator(genNum).PLBasedExhaustTempCurve, PLR) *
                          CurveManager::CurveValue(CTGenerator(genNum).TempBasedExhaustTempCurve, AmbientDeltaT);

            // (UACGC) Heat Exchanger UA to Capacity
            Real64 UA = CTGenerator(genNum).UACoef(1) * std::pow(RatedPowerOutput, CTGenerator(genNum).UACoef(2));

            // design engine stack saturated steam temp. (C)
            Real64 DesignMinExitGasTemp = CTGenerator(genNum).DesignMinExitGasTemp;

            ExhaustStackTemp = DesignMinExitGasTemp + (ExhaustTemp - DesignMinExitGasTemp) /
                                                          std::exp(UA / (max(ExhaustFlow, MaxExhaustperCTPower * RatedPowerOutput) * ExhaustCP));

            QExhaustRec = max(ExhaustFlow * ExhaustCP * (ExhaustTemp - ExhaustStackTemp), 0.0);
        } else {
            ExhaustStackTemp = CTGenerator(genNum).DesignMinExitGasTemp;
            QExhaustRec = 0.0;
        }

        // Use Curve fit to determine Heat Recovered Lubricant heat.  This curve calculates the lube heat recovered (J/s) by
        // multiplying the total power generated by the fraction of that power that could be recovered in the lube oil at that
        // particular part load.
        // recovered lube oil heat (W)
        Real64 QLubeOilRec = ElecPowerGenerated * CurveManager::CurveValue(CTGenerator(genNum).QLubeOilRecoveredCurve, PLR);

        // Check for divide by zero
        Real64 HeatRecOutTemp;   // Heat Recovery Fluid Outlet Temperature (C)
        if ((HeatRecMdot > 0.0) && (HeatRecCp > 0.0)) {
            HeatRecOutTemp = (QExhaustRec + QLubeOilRec) / (HeatRecMdot * HeatRecCp) + HeatRecInTemp;
        } else {
            HeatRecMdot = 0.0;
            HeatRecOutTemp = HeatRecInTemp;
            QExhaustRec = 0.0;
            QLubeOilRec = 0.0;
        }

        // Now verify the maximum temperature was not exceeded
        // Heat Recovery Flow Rate if minimal heat recovery is accomplished
        Real64 MinHeatRecMdot = 0.0;

        Real64 HRecRatio;        // When Max Temp is reached the amount of recovered heat has to be reduced.

        if (HeatRecOutTemp > CTGenerator(genNum).HeatRecMaxTemp) {
            if (CTGenerator(genNum).HeatRecMaxTemp != HeatRecInTemp) {
                MinHeatRecMdot = (QExhaustRec + QLubeOilRec) / (HeatRecCp * (CTGenerator(genNum).HeatRecMaxTemp - HeatRecInTemp));
                if (MinHeatRecMdot < 0.0) MinHeatRecMdot = 0.0;
            }

            // Recalculate Outlet Temperature, with adjusted flowrate
            if ((MinHeatRecMdot > 0.0) && (HeatRecCp > 0.0)) {
                HeatRecOutTemp = (QExhaustRec + QLubeOilRec) / (MinHeatRecMdot * HeatRecCp) + HeatRecInTemp;
                HRecRatio = HeatRecMdot / MinHeatRecMdot;
            } else {
                HeatRecOutTemp = HeatRecInTemp;
                HRecRatio = 0.0;
            }
            QLubeOilRec *= HRecRatio;
            QExhaustRec *= HRecRatio;
        }

        // Calculate Energy
        // Generator output (J)
        Real64 ElectricEnergyGen = ElecPowerGenerated * DataHVACGlobals::TimeStepSys * DataGlobals::SecInHour;

        // Amount of Fuel Energy Required to run COMBUSTION turbine (J)
        Real64 FuelEnergyUsed = FuelUseRate * DataHVACGlobals::TimeStepSys * DataGlobals::SecInHour;

        // recovered lube oil heat (J)
        Real64 LubeOilEnergyRec = QLubeOilRec * DataHVACGlobals::TimeStepSys * DataGlobals::SecInHour;

        // recovered exhaust heat (J)
        Real64 ExhaustEnergyRec = QExhaustRec * DataHVACGlobals::TimeStepSys * DataGlobals::SecInHour;

        CTGenerator(genNum).ElecPowerGenerated = ElecPowerGenerated;
        CTGenerator(genNum).ElecEnergyGenerated = ElectricEnergyGen;

        CTGenerator(genNum).HeatRecInletTemp = HeatRecInTemp;
        CTGenerator(genNum).HeatRecOutletTemp = HeatRecOutTemp;

        CTGenerator(genNum).HeatRecMdot = HeatRecMdot;
        CTGenerator(genNum).QExhaustRecovered = QExhaustRec;
        CTGenerator(genNum).QLubeOilRecovered = QLubeOilRec;
        CTGenerator(genNum).QTotalHeatRecovered = QExhaustRec + QLubeOilRec;
        CTGenerator(genNum).FuelEnergyUseRate = std::abs(FuelUseRate);
        CTGenerator(genNum).ExhaustEnergyRec = ExhaustEnergyRec;
        CTGenerator(genNum).LubeOilEnergyRec = LubeOilEnergyRec;
        CTGenerator(genNum).TotalHeatEnergyRec = ExhaustEnergyRec + LubeOilEnergyRec;
        CTGenerator(genNum).FuelEnergy = std::abs(FuelEnergyUsed);

        // Heating Value of Fuel in (kJ/kg)
        Real64 FuelHeatingValue = CTGenerator(genNum).FuelHeatingValue;

        CTGenerator(genNum).FuelMdot = std::abs(FuelUseRate) / (FuelHeatingValue * KJtoJ);

        CTGenerator(genNum).ExhaustStackTemp = ExhaustStackTemp;
    }

    void InitCTGenerators(int const genNum,         // Generator number
                          bool const RunFlag,             // TRUE when Generator operating
                          Real64 const EP_UNUSED(MyLoad), // Generator demand
                          bool const FirstHVACIteration)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Dan Fisher
        //       DATE WRITTEN   Oct 2000
        //       MODIFIED       na
        //       RE-ENGINEERED  Brent Griffith, Sept 2010 plant upgrades, generalize fluid props

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for initializations of the CT generators.

        static std::string const RoutineName("InitICEngineGenerators");
        static bool MyOneTimeFlag(true); // Initialization flag

        bool errFlag;

        // Do the one time initializations
        if (MyOneTimeFlag) {
            MyOneTimeFlag = false;
        }

        if (CTGenerator(genNum).MyPlantScanFlag && allocated(DataPlant::PlantLoop) && CTGenerator(genNum).HeatRecActive) {
            errFlag = false;
            PlantUtilities::ScanPlantLoopsForObject(CTGenerator(genNum).Name,
                                    DataPlant::TypeOf_Generator_CTurbine,
                                    CTGenerator(genNum).HRLoopNum,
                                    CTGenerator(genNum).HRLoopSideNum,
                                    CTGenerator(genNum).HRBranchNum,
                                    CTGenerator(genNum).HRCompNum,
                                    errFlag,
                                    _,
                                    _,
                                    _,
                                    _,
                                    _);
            if (errFlag) {
                ShowFatalError("InitCTGenerators: Program terminated due to previous condition(s).");
            }
            CTGenerator(genNum).MyPlantScanFlag = false;
        }

        if (CTGenerator(genNum).MySizeAndNodeInitFlag && (!CTGenerator(genNum).MyPlantScanFlag) && CTGenerator(genNum).HeatRecActive) {
            int HeatRecInletNode = CTGenerator(genNum).HeatRecInletNodeNum;
            int HeatRecOutletNode = CTGenerator(genNum).HeatRecOutletNodeNum;

            // size mass flow rate
            Real64 rho = FluidProperties::GetDensityGlycol(DataPlant::PlantLoop(CTGenerator(genNum).HRLoopNum).FluidName,
                                   DataGlobals::InitConvTemp,
                                   DataPlant::PlantLoop(CTGenerator(genNum).HRLoopNum).FluidIndex,
                                   RoutineName);

            CTGenerator(genNum).DesignHeatRecMassFlowRate = rho * CTGenerator(genNum).DesignHeatRecVolFlowRate;

            PlantUtilities::InitComponentNodes(0.0,
                               CTGenerator(genNum).DesignHeatRecMassFlowRate,
                               HeatRecInletNode,
                               HeatRecOutletNode,
                               CTGenerator(genNum).HRLoopNum,
                               CTGenerator(genNum).HRLoopSideNum,
                               CTGenerator(genNum).HRBranchNum,
                               CTGenerator(genNum).HRCompNum);

            CTGenerator(genNum).MySizeAndNodeInitFlag = false;
        } // end one time inits

        // Do the Begin Environment initializations
        if (DataGlobals::BeginEnvrnFlag && CTGenerator(genNum).MyEnvrnFlag && CTGenerator(genNum).HeatRecActive) {
            int HeatRecInletNode = CTGenerator(genNum).HeatRecInletNodeNum;
            int HeatRecOutletNode = CTGenerator(genNum).HeatRecOutletNodeNum;
            // set the node Temperature, assuming freeze control
            DataLoopNode::Node(HeatRecInletNode).Temp = 20.0;
            DataLoopNode::Node(HeatRecOutletNode).Temp = 20.0;
            // set the node max and min mass flow rates
            PlantUtilities::InitComponentNodes(0.0,
                               CTGenerator(genNum).DesignHeatRecMassFlowRate,
                               HeatRecInletNode,
                               HeatRecOutletNode,
                               CTGenerator(genNum).HRLoopNum,
                               CTGenerator(genNum).HRLoopSideNum,
                               CTGenerator(genNum).HRBranchNum,
                               CTGenerator(genNum).HRCompNum);

            CTGenerator(genNum).MyEnvrnFlag = false;
        } // end environmental inits

        if (!DataGlobals::BeginEnvrnFlag) {
            CTGenerator(genNum).MyEnvrnFlag = true;
        }

        if (CTGenerator(genNum).HeatRecActive) {
            if (FirstHVACIteration) {
                Real64 mdot;
                if (RunFlag) {
                    mdot = CTGenerator(genNum).DesignHeatRecMassFlowRate;
                } else {
                    mdot = 0.0;
                }
                PlantUtilities::SetComponentFlowRate(mdot,
                                     CTGenerator(genNum).HeatRecInletNodeNum,
                                     CTGenerator(genNum).HeatRecOutletNodeNum,
                                     CTGenerator(genNum).HRLoopNum,
                                     CTGenerator(genNum).HRLoopSideNum,
                                     CTGenerator(genNum).HRBranchNum,
                                     CTGenerator(genNum).HRCompNum);

            } else {
                PlantUtilities::SetComponentFlowRate(CTGenerator(genNum).HeatRecMdot,
                                     CTGenerator(genNum).HeatRecInletNodeNum,
                                     CTGenerator(genNum).HeatRecOutletNodeNum,
                                     CTGenerator(genNum).HRLoopNum,
                                     CTGenerator(genNum).HRLoopSideNum,
                                     CTGenerator(genNum).HRBranchNum,
                                     CTGenerator(genNum).HRCompNum);
            }
        }
    }

    void UpdateCTGeneratorRecords(bool const EP_UNUSED(RunFlag), // TRUE if Generator operating
                                  int const genNUm                  // Generator number
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR:          Dan Fisher
        //       DATE WRITTEN:    October 1998

        if (CTGenerator(genNUm).HeatRecActive) {
            int HeatRecOutletNode = CTGenerator(genNUm).HeatRecOutletNodeNum;
            DataLoopNode::Node(HeatRecOutletNode).Temp = CTGenerator(genNUm).HeatRecOutletTemp;
        }
    }

    void GetCTGeneratorResults(int const EP_UNUSED(GeneratorType), // type of Generator
                               int const genNum,
                               Real64 &GeneratorPower,  // electrical power
                               Real64 &GeneratorEnergy, // electrical energy
                               Real64 &ThermalPower,    // heat power
                               Real64 &ThermalEnergy    // heat energy
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         B Griffith
        //       DATE WRITTEN   March 2008
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // get some results for load center's aggregation

        GeneratorPower = CTGenerator(genNum).ElecPowerGenerated;
        GeneratorEnergy = CTGenerator(genNum).ElecEnergyGenerated;
        ThermalPower = CTGenerator(genNum).QTotalHeatRecovered;
        ThermalEnergy = CTGenerator(genNum).TotalHeatEnergyRec;
    }

} // namespace CTElectricGenerator

} // namespace EnergyPlus
