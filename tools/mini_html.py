Import("env")
import os
import glob
from pathlib import Path
import sys
import pip
import subprocess
import pkg_resources

def ensure_module_version(package_name, required_version):
    try:
        installed_version = pkg_resources.get_distribution(package_name).version
        if installed_version == required_version:
            print(f"{package_name} {required_version} is already installed.")
            return
        else:
            print(f"{package_name} version {installed_version} found – replacing with {required_version}.")
    except pkg_resources.DistributionNotFound:
        print(f"{package_name} is not installed – installing version {required_version}.")

    # Install the required version (will upgrade or downgrade as needed)
    subprocess.check_call(['pip', 'install', '--upgrade', f'{package_name}=={required_version}'])


ensure_module_version("minify_html", "0.15.0")
import minify_html

filePath = 'src/webpages/'
try:
  print("==========================")
  print("Generating webpage")
  print("==========================")
  print("Preparing html.h file from source")
  print("  -insert header") 
  cpp_output = "#pragma once\n\n#include <Arduino.h>  // PROGMEM\n\n"
  print("  -insert html")

  for x in glob.glob(filePath+"*.html"):
   print("prozessing file:" + Path(x).stem)
   print(Path(x).stem)
   cpp_output += "static const char "+Path(x).stem+"[] PROGMEM = R\"rawliteral("
   f = open(x, "r")
   if env.GetProjectOption("build_type") == "debug":
        cpp_output += f.read()  
   else:
      #cpp_output += f.read()  #disable compression until fixed that the compressor remove %VARIABLE%
       cpp_output += minify_html.minify(f.read(), minify_js=True)

   f.close()
   cpp_output += ")rawliteral\";\n"

   f = open ("./src/html.h", "w")
   f.write(cpp_output)
   f.close()
   print("==========================\n")

except SyntaxError as e:
  print(e)