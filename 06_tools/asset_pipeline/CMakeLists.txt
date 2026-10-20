add_executable(recoil_asset_pipeline
    src/main.cpp
)

target_compile_features(recoil_asset_pipeline PUBLIC cxx_std_20)
target_link_libraries(recoil_asset_pipeline PRIVATE recoil_math recoil_scene)

# This tool currently only needs to run on the dev machine, not ship with the game — no
# WIN32 subsystem flag, plain console app. Add new source files above as it gains real
# content. It may initially shell out to / wrap the already-validated mech3ax (Rust) and
# tools/scripts/*.py parsers rather than re-implement zbd parsing from scratch here — see
# CLAUDE.md's tool-escalation ladder before writing any new parser in this tool.
