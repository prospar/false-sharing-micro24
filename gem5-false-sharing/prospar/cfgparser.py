import os
import configparser
from typing import Tuple
import sys

_CONFIG: str = "fs.ini"
_tup_sections: Tuple[str, str,str] = ("PROJECT", "SYSTEM","USER")
_tup_allowedProjKeys: Tuple[str, str, str, str] = ("BENCHMARKS_ROOT",
                                                   "FS_GEM5_ROOT",
                                                       "EXP_OUTPUT_ROOT",
                                                       "VIRTUALENV_ROOT")
_tup_allowedSysKeys:Tuple[str]=("PROTOCOL",)
_tup_allowedUserKeys: Tuple[str, str] = ("USER", "EMAIL")

def raise_error(*args, stack=False):
    """Raise errors and exit.

    stack is a 'keyword-only' argument, meaning that it can only be used as a
    keyword rather than a positional argument.
    """
    if stack:
        import traceback
        traceback.print_stack()
    stmt = "[error] "
    for s in args:
        stmt += s + " "
    sys.exit(stmt)


def sanity_check(config):
    '''Check for missing sections.'''
    sections = config.sections()
    if len(sections) != len(_tup_sections):
        raise_error(f"CONFIG file contains incorrect number of sections!")

    for _section in sections:
        if _section == "PROJECT":
            for _key in config[_section]:
                if _key.upper() not in _tup_allowedProjKeys:
                    raise_error(f"Invalid key {_key} in PROJECT section in "
                                "CONFIG file!")
        elif _section == "SYSTEM":
            for _key in config[_section]:
                if _key.upper() not in _tup_allowedSysKeys:
                    raise_error(f"Invalid key {_key} in SYSTEM section in "
                                "CONFIG file!")
        elif _section == "USER":
            for _key in config[_section]:
                if _key.upper() not in _tup_allowedUserKeys:
                    raise_error(f"Invalid key {_key} in USER section in "
                                "CONFIG file!")



_config = configparser.ConfigParser()
config_path: str = _CONFIG
# print(f"Config file path: {config_path}")
if not os.path.exists(config_path):
    raise_error(f"CONFIG file missing in {config_path} directory.")

_config.read(config_path)
sanity_check(_config)


for sec in _config.sections():
    print(f"declare -A {sec}")
    for key, val in _config.items(sec):
        print(f"{sec}[{key}]='{val}'")
