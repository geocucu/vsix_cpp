# Known issues 

1. The custom build step for obtaining the satellite DLL from the VSCT will mess with MSBuild's caching, i.e. updates to the command table will not trigger a rebuild.
2. The caching done by Visual Studio "Experimental" is broken outright. Best bet to avoid wild chases is manually uninstalling from `Extensions->Manage Extensions...` every single time (`Reset Visual Studio 2022 Experimental Instance` doesn't work) 