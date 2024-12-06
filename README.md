# Known issues 

1. The custom build step for obtaining the satellite DLL from the VSCT will mess with MSBuild's caching, i.e. updates to the command table will not trigger a rebuild.