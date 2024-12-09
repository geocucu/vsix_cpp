# ================ INPUT ================
$new_proj_name = "gc_msvc_toolbar"

$boilerplate_name = "vsix_cpp" 

# C++ Project Type (Ignore): 8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942
# C# Project Type (Ignore): FAE04EC0-301F-11D3-BF4B-00C04F79EFBC

# ======== guids.h ========

function generate_guidh_triplet {
  param (
    [string]$name
  )

  $new = [guid]::NewGuid()
  $new_str = $new.ToString()
  $parts = $new_str -split '-'

  $new_define1 = @"
GUID_$name { 0x$($parts[0]), 0x$($parts[1]), 0x$($parts[2]), { 0x$($parts[3][0..1] -join ''), 0x$($parts[3][2..3] -join ''), 0x$($parts[4][0..1] -join ''), 0x$($parts[4][2..3] -join ''), 0x$($parts[4][4..5] -join ''), 0x$($parts[4][6..7] -join ''), 0x$($parts[4][8..9] -join ''), 0x$($parts[4][10..11] -join '') } }
"@

  $new_define2 = @"
DEFINE_GUID(CLSID_$name,
0x$($parts[0]), 0x$($parts[1]), 0x$($parts[2]), 0x$($parts[3][0..1] -join ''), 0x$($parts[3][2..3] -join ''), 0x$($parts[4][0..1] -join ''), 0x$($parts[4][2..3] -join ''), 0x$($parts[4][4..5] -join ''), 0x$($parts[4][6..7] -join ''), 0x$($parts[4][8..9] -join ''), 0x$($parts[4][10..11] -join '') );
"@

  return @{
    new           = $new_str
    new_define1   = $new_define1
    new_define2   = $new_define2
  }
}

$guids_pkg = generate_guidh_triplet -name "pkg"
$guids_cmdset = generate_guidh_triplet -name "cmdset"
$guids_img = generate_guidh_triplet -name "img"

Set-Content -Path "res/guids.h" -Value @"
// { $($guids_pkg.new) }
#define $($guids_pkg.new_define1)
#ifdef DEFINE_GUID
$($guids_pkg.new_define2)
#endif 

// { $($guids_cmdset.new) }
#define $($guids_cmdset.new_define1)
#ifdef DEFINE_GUID
$($guids_cmdset.new_define2)
#endif 

// { $($guids_img.new) }
#define $($guids_img.new_define1)
#ifdef DEFINE_GUID
$($guids_img.new_define2)
#endif 
"@

# ======== Everything else ========

$guid_replacements = @{
  "GUID_pkg" = @{
    "old" = "d6cbff0b-ce7e-4205-9e41-869492cfb46e"
    "new" = $guids_pkg.new
    "files" = @("source.extension.manifest", "vsix_cpp.idl", "vsix_cpp.pkgdef")
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

# == Clean up files that are not committed anyway (mostly for dev) ==

# .user
Get-ChildItem -Path $PWD -Recurse -File -Filter "*.user" | ForEach-Object {
  try {
    Remove-Item -Path $_.FullName -Force -ErrorAction Stop
    Write-Host "Deleted: $($_.FullName)"
  } catch {
    Write-Host "Error deleting: $($_.FullName). Error: $($_.Exception.Message)" -ForegroundColor Red
  }
}

# bin/
$binDir = Join-Path -Path $PWD -ChildPath "bin"

if (Test-Path $binDir) {
  try {
    Remove-Item -Path $binDir -Recurse -Force -ErrorAction Stop
    Write-Host "Deleted: $binDir"
  } catch {
    Write-Host "Error deleting: $binDir. Error: $($_.Exception.Message)" -ForegroundColor Red
  }
}

Write-Host "Cleaned up rubbish..."

# == Rename files ==
foreach ($dir in $subdirs) {
  if (Test-Path $dir) {
    Get-ChildItem -Path $dir -Recurse -File | Where-Object {
      # Match only the file name, not the full path
      $_.Name -match [regex]::Escape($boilerplate_name)
    } | ForEach-Object {
      $new_name = Join-Path -Path $_.DirectoryName -ChildPath ($_.Name -replace [regex]::Escape($boilerplate_name), $new_proj_name)

      try {
        Rename-Item -Path $_.FullName -NewName $new_name -ErrorAction Stop
        Write-Host "Renamed: $($_.FullName) ---> $new_name"
      } catch {
        Write-Host "Error renaming file: $($_.FullName). Error: $($_.Exception.Message)" -ForegroundColor Red
      }
    }
  }
}

Write-Host "Done! Feel free to delete me (generate_new.ps1)"
