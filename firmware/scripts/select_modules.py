# Builds everything under src/ except the view and provider folders, plus the picked ones.
# The VIEW and PROVIDER environment variables override custom_view and custom_provider in
# platformio.ini.
import os
import sys

Import("env")

src = env.subst("$PROJECT_SRC_DIR")
selected = {}
for kind, variable in (("view", "VIEW"), ("provider", "PROVIDER")):
    name = os.environ.get(variable) or env.GetProjectOption("custom_" + kind)
    kind_dir = os.path.join(src, kind)
    if not os.path.isdir(os.path.join(kind_dir, name)):
        choices = sorted(d for d in os.listdir(kind_dir) if os.path.isdir(os.path.join(kind_dir, d)))
        sys.stderr.write(
            "%s=%s: no src/%s/%s/ (available: %s)\n" % (variable, name, kind, name, ", ".join(choices))
        )
        env.Exit(1)
    selected[kind] = name

print("Building view=%(view)s provider=%(provider)s" % selected)
env.Replace(
    SRC_FILTER=[
        "+<*>",
        "-<view/>",
        "-<provider/>",
        "+<view/%s/>" % selected["view"],
        "+<provider/%s/>" % selected["provider"],
    ]
)
