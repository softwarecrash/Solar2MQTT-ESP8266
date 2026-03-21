Import("env")

env.Append(CPPDEFINES=[
    ("SWVERSION", env.StringifyMacro(env.GetProjectOption("custom_prog_version"))),
    ("HWBOARD", env.StringifyMacro(env["PIOENV"])),
])

if env.GetProjectOption("custom_hardwareserial") == "true":
    env.Append(CPPDEFINES=[
        ("isUART_HARDWARE",  env.StringifyMacro(env.GetBuildType())),
    ])

env.Replace(PROGNAME="Solar2MQTT-ESP8266_%s_%s" % (str(env["PIOENV"]), env.GetProjectOption("custom_prog_version")))
