# ================ INPUT ================
$new_proj_name = "gc_msvc_toolbar"

$boilerplate_name = "vsix_cpp" 

# C++ Project Type (Ignore): 8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942
# C# Project Type (Ignore): FAE04EC0-301F-11D3-BF4B-00C04F79EFBC
$guid_replacements = @{
  "GUID_pkg" = @{
    "old" = "d6cbff0b-ce7e-4205-9e41-869492cfb46e"
    "new" = [guid]::NewGuid().ToString()
    "files" = @("source.extension.manifest", "vsix_cpp.idl", "vsix_cpp.pkgdef", "res/guids.h")
  }
  "GUID_cmdset" = @{
    "old" = "4082e768-130c-49a3-b9f6-ec28d9aea2d5"
    "new" = [guid]::NewGuid().ToString()
    "files" = @("res/guids.h")
  }
  "GUID_img" = @{
    "old" = "ed56039a-5fbe-47f5-bfe0-e347a9f30c18"
    "new" = [guid]::NewGuid().ToString()
    "files" = @("res/guids.h")
  }
  "GUID_idl" = @{
    "old" = "4b55dd76-7653-40e5-ad65-702e1626b258"
    "new" = [guid]::NewGuid().ToString()
    "files" = @("vsix_cpp.idl")
  }
  "GUID_cpp_proj" = @{
    "old" = "20DA4D05-A1D6-42AF-9731-82C9A888AB15"
    "new" = [guid]::NewGuid().ToString()
    "files" = @("vsix_cpp.sln", "vsix_cpp.vcxproj", "vsix.csproj")
  }
  "GUID_vsix_proj" = @{
    "old" = "99EF908D-86C3-40FC-AE94-3298B65481C5"
    "new" = [guid]::NewGuid().ToString()
    "files" = @("vsix_cpp.sln", "vsix.csproj")
  }
}

$projname_replacement_files = @(
  "source.extension.vsixmanifest",
  "vsix_cpp.def",
  "vsix_cpp.idl",
  "vsix_cpp.pkgdef",
  "vsix_cpp.sln",
  "vsix_cpp.vcxproj",
  "vsix_cpp.vcxproj.filters",
  "vsix.csproj",
  "res/vsct.rc",
  "res/vsix_cpp.rc",
  "src/module.cpp"
)

$subdirs = @(
  (Get-Location).Path,  # Working dir
  (Join-Path -Path (Get-Location).Path -ChildPath "src"),  # src/ and subdirs
  (Join-Path -Path (Get-Location).Path -ChildPath "res")   # res/ and subdirs
)

# == GUID replacements == 
foreach ($key in $guid_replacements.Keys) {
  $guid_old = $guid_replacements[$key]["old"]
  $guid_new = $guid_replacements[$key]["new"]
  $files = $guid_replacements[$key]["files"]

  foreach ($file in $files) {
    if (Test-Path $file) {
      $content = Get-Content -Path $file -Raw
      $content = $content -replace [regex]::Escape($guid_old), $guid_new
      Set-Content -Path $file -Value $content
    }
  }
}

Write-Host "GUID replacements done..."

# == Proj name replacements ==
foreach ($file in $projname_replacement_files) {
  if (Test-Path $file) {
    $content = Get-Content -Path $file -Raw
    $content = $content -replace $boilerplate_name, $new_proj_name
    Set-Content -Path $file -Value $content
  }
}

Write-Host "Project name replacements done..."

# == Rename files ==
foreach ($dir in $subdirs) {
  if (Test-Path $dir) {
    Get-ChildItem -Path $dir -Recurse -File | Where-Object {
      # Match only the file name, not the full path
      $_.Name -match [regex]::Escape($boilerplate_name)
    } | ForEach-Object {
      $new_name = Join-Path -Path $_.DirectoryName -ChildPath ($_.Name -replace [regex]::Escape($boilerplate_name), $new_proj_name)
      Rename-Item -Path $_.FullName -NewName $new_name
    }
  }
}

Write-Host "Done! Feel free to delete me (generate_new.ps1)"
