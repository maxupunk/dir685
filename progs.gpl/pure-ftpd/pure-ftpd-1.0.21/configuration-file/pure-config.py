#! /usr/bin/python

#  Original pure-config.py by Frank Denis <j@pureftpd.org>
#  Copyright 2001 by Joshua Rodman <joshua_rodman@yahoo.com>,
#  Modifications Copyright 2001-2006 by Matthias Andree
#  
#                All Rights Reserved
# 
# Permission to use, copy, modify, and distribute this software
# in original or modified form for any purpose and without fee
# is hereby granted, provided that the above copyright notice
# appear in all copies.
# 
# Joshua Rodman DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
# SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
# AND FITNESS, IN NO EVENT SHALL Joshua Rodman BE LIABLE FOR
# ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
# WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
# ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
# PERFORMANCE OF THIS SOFTWARE. 

"""Pure-FTPd configuration parser.

   Parse pure-ftpd configuration file to options.
   If run directly, execute the program.

"""
import re
import os
import sys

# autoconf stuff
if '${exec_prefix}/sbin'[0] == '/':
    pureftpd = '${exec_prefix}/sbin/pure-ftpd'
else:
    pureftpd = '/usr/local/pureftpd/sbin/pure-ftpd'

argv = sys.argv[1:]
try:
    conffile = argv.pop(0)
except IndexError:
    print "Usage:", sys.argv[0], "<configuration file> [extra options]"
    sys.exit(1)

comment = re.compile("[ ]*#+.*")

# option_tuple members are lists as follows:
# 0 - case insensitive regex to match
# 1 - flag to use
# remaining - 'None' are placeholders for items matched by the regex
#             the matched items and literal strings are concatenated
#             into a literal option which follows the flag

option_tuple = (
    ["IPV4Only[\s]+yes",                   "-4"                  ],
    ["IPV6Only[\s]+yes",                   "-6"                  ],    
    ["FileSystemCharset\s+(\S+)",          "-8", None            ],
    ["ClientCharset\s+(\S+)",              "-9", None            ],    
    ["ChrootEveryone[\s]+yes",             "-A"                  ],
    ["TrustedGID[\s]+([\d]+)",             "-a", None            ],
    ["BrokenClientsCompatibility[\s]+yes", "-b"                  ],
    ["MaxClientsNumber\s+(\d+)",           "-c", None            ],
    ["Daemonize\s+yes",                    "-B"                  ],
    ["MaxClientsPerIP\s+(\d+)",            "-C", None            ],
    ["VerboseLog\s+yes",                   "-d"                  ],
    ["DisplayDotFiles\s+yes",              "-D"                  ],
    ["AnonymousOnly\s+yes",                "-e"                  ],
    ["MaxDiskUsage\s+(\d+)",               "-k", None            ],
    ["NoAnonymous\s+yes",                  "-E"                  ],
    ["SyslogFacility\s+(\S+)",             "-f", None            ],
    ["FortunesFile\s+(\S+)",               "-F", None            ],
    ["DontResolve\s+yes",                  "-H"                  ],
    ["MaxIdleTime\s+(\d+)",                "-I", None            ],
    ["LDAPConfigFile\s+(\S+)",             "-l", "ldap:",   None ],
    ["MySQLConfigFile\s+(\S+)",            "-l", "mysql:",  None ],
    ["PGSQLConfigFile\s+(\S+)",            "-l", "pgsql:",  None ],
    ["PureDB\s+(\S+)",                     "-l", "puredb:", None ],
    ["ExtAuth\s+(\S+)",                    "-l", "extauth:",None ],    
    ["PAMAuthentication\s+yes",            "-l", "pam"           ],
    ["UnixAuthentication\s+yes",           "-l", "unix"          ],
    ["LimitRecursion\s+(\d+)\s+(\d+)",     "-L", None, ":", None ],
    ["AnonymousCanCreateDirs\s+yes",       "-M"                  ],
    ["MaxLoad\s+(\d+)",                    "-m", None            ],
    ["NATmode\s+yes",                      "-N"                  ],
    ["CallUploadScript\s+yes",             "-o"                  ],
    ["PassivePortRange\s+(\d+)\s+(\d+)",   "-p", None, ":", None ],
    ["ForcePassiveIP\s+(\S+)",             "-P", None            ],
    ["AnonymousRatio\s+(\d+)\s+(\d+)",     "-q", None, ":", None ],
    ["UserRatio\s+(\d+)\s+(\d+)",          "-Q", None, ":", None ],
    ["AntiWarez\s+yes",                    "-s"                  ],
    ["Bind\s+(\S+)",                       "-S", None            ],
    ["AnonymousBandwidth\s+([:0-9]+)",     "-t", None            ],
    ["UserBandwidth\s+([:0-9]+)",          "-T", None            ],
    ["Quota\s+([:0-9]+)",                  "-n", None            ],
    ["Umask\s+(\d+):(\d+)",                "-U", None, ":", None ],
    ["MinUID\s+(\d+)",                     "-u", None            ],
    ["AllowUserFXP\s+yes",                 "-w"                  ],
    ["AllowAnonymousFXP\s+yes",            "-W"                  ],
    ["ProhibitDotFilesWrite\s+yes",        "-x"                  ],
    ["ProhibitDotFilesRead\s+yes",         "-X"                  ],
    ["AllowDotFiles\s+yes",                "-z"                  ],
    ["AutoRename\s+yes",                   "-r"                  ],
    ["AnonymousCantUpload\s+yes",          "-i"                  ],
    ["TrustedIP\s+(\S+)",                  "-V", None            ],
    ["LogPID\s+yes",                       "-1"                  ],
    ["AltLog\s+(\S+)",                     "-O", None            ],
    ["NoChmod\s+yes",                      "-R"                  ],
    ["KeepAllFiles\s+yes",                 "-K"                  ],
    ["CreateHomeDir\s+yes",                "-j"                  ],
    ["NoRename\s+yes",                     "-G"                  ],
    ["CustomerProof\s+yes",                "-Z"                  ],
    ["NoTruncate\s+yes",                   "-0"                  ],    
    ["PIDFile\s+(\S+)",                    "-g", None            ],
    ["PerUserLimits\s+([:0-9]+)",          "-y", None            ],
    ["TLSAuth\s+yes",                      "-Y", None            ])

for option in option_tuple:
    option[0] = re.compile(option[0], re.IGNORECASE)


def build_argument(match, controls):
    """construct an argument string
    match:    a match object containing argument values
    controls: a list controlling the format of the argument

    Return parameter as string"""

    argument = ""
    value = 1

    for control in controls:
        if control:
            # concat the literal string
            argument = argument + control
        else:
            # concat a value
            argument = argument + match.group(value)
            value = value + 1

    return argument


def parse(file_obj):
    """parse/extract a pure-ftpd configuration
    file_obj: a readlines() capable object containing the configurion

    Returns list of arguments as appropriate for exec()"""

    args = [pureftpd]

    for line in file_obj.readlines():
        if comment.search(line):
            continue

        for option in option_tuple:
            match = option[0].search(line)
            if match:
                args.append(option[1])

                if len(option) <= 2:
                    # The option takes no argument
                    continue

                argument = build_argument(match, option[2:])
                args.append(argument)

    return args

def parse_filename(filename = conffile):
    """parse/extract a pure-ftpd from a named file
    filename: text path to file

    Returns list of arguments as appropriate for exec()"""

    file_obj = open(filename)
    return parse(file_obj)

if __name__ == '__main__':
    args = parse_filename()

    ## debug
    #print str(args)
    print "Running:", args + argv
    os.execv(pureftpd, args + argv)
