#include "VV.H"

#include "Utilities.H"

using namespace Foam;

preciceAdapter::VV::VolumeVolume::VolumeVolume
(
    const Foam::fvMesh& mesh
)
:
mesh_(mesh)
{}

bool preciceAdapter::VV::VolumeVolume::configure(const IOdictionary& adapterConfig)
{
    DEBUG(adapterInfo("Configuring the Volume-Volume module..."));

    // Read the VV-specific options from the adapter's configuration file
    if (!readConfig(adapterConfig))
    {
        return false;
    }

    // NOTE: If you want to add a new solver type, which you can manually
    // specify in the configuration, add it here. See also the methods
    // addWriters() and addReaders().
    // Check the solver type and determine it if needed
    cout << "SolverType: |" << solverType_ << "|" << endl;
    if (
        solverType_.compare("compressible") == 0 ||
        solverType_.compare("incompressible") == 0 ||
        solverType_.compare("basic") == 0
    )
    {
        DEBUG(adapterInfo("Known solver type: " + solverType_));
    }
    else if (solverType_.compare("none") == 0)
    {
        DEBUG(adapterInfo("Determining the solver type..."));
        solverType_ = determineSolverType();
    }
    else
    {
        DEBUG(adapterInfo("Unknown solver type. Determining the solver type..."));
        solverType_ = determineSolverType();
    }

    return true;
}

bool preciceAdapter::VV::VolumeVolume::readConfig(const IOdictionary& adapterConfig)
{
    const dictionary VVdict = adapterConfig.subOrEmptyDict("VV");

    // Read the solver type (if not specified, it is determined automatically)
    solverType_ = VVdict.lookupOrDefault<word>("solverType", "");
    DEBUG(adapterInfo("    user-defined solver type : " + solverType_));

    /* TODO: Read the names of any needed fields and parameters.
    * Include the force here?
    */

    // Read the name of the pointDisplacement field (if different)
    nameT_ = VVdict.lookupOrDefault<word>("nameT", "T");
    DEBUG(adapterInfo("    temperature field name : " + nameT_));

    // Read the name of the pointDisplacement field (if different)
    nameTransportProperties_ = VVdict.lookupOrDefault<word>("nameTransportProperties", "transportProperties");
    DEBUG(adapterInfo("    transportProperties name : " + nameTransportProperties_));

    return true;
}

std::string preciceAdapter::VV::VolumeVolume::determineSolverType()
{
    // NOTE: When coupling a different variable, you may want to
    // add more cases here. Or you may provide the solverType in the config.

    std::string solverType;

    // Determine the solver type: Compressible, Incompressible or Basic.
    // Look for the files transportProperties
    bool transportPropertiesExists = false;

    if (mesh_.foundObject<IOdictionary>(nameTransportProperties_))
    {
        transportPropertiesExists = true;
        DEBUG(adapterInfo("Found the transportProperties dictionary."));
    }
    else
    {
        DEBUG(adapterInfo("Did not find the transportProperties dictionary."));
    }

    if (transportPropertiesExists)
    {
        solverType = "basic";
        DEBUG(adapterInfo("This is a basic solver, as transport properties "
        "are provided, while turbulence or transport properties are not "
        "provided."));
    }
    else
    {
        adapterInfo("Could not determine the solver type, or this is not a "
        "compatible solver: neither transport, nor turbulence properties "
        "are provided.",
        "error");
    }

    return solverType;
}

void preciceAdapter::VV::VolumeVolume::addWriters(std::string dataName, Interface * interface)
{
    if (dataName.find("Temperature") == 0)
    {
        interface->addCouplingDataWriter
        (
            dataName,
            new Temperature(mesh_, nameT_)
        );
        DEBUG(adapterInfo("Added writer: Temperature."));
    }
    else
    {
        adapterInfo("Unknown data type - cannot add " + dataName +".", "error");
    }

    // NOTE: If you want to couple another variable, you need
    // to add your new coupling data user as a coupling data
    // writer here (and as a reader below).
    // The argument of the dataName.compare() needs to match
    // the one provided in the adapter's configuration file.
}

void preciceAdapter::VV::VolumeVolume::addReaders(std::string dataName, Interface * interface)
{

    if (dataName.find("Temperature") == 0)
    {
        interface->addCouplingDataReader
        (
            dataName,
            new Temperature(mesh_, nameT_)
        );
        DEBUG(adapterInfo("Added reader: Temperature."));
    }
    else
    {
        adapterInfo("Unknown data type - cannot add " + dataName +".", "error");
    }

    // NOTE: If you want to couple another variable, you need
    // to add your new coupling data user as a coupling data
    // reader here (and as a writer above).
    // The argument of the dataName.compare() needs to match
    // the one provided in the adapter's configuration file.
}
