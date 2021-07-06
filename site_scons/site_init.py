"""Configure/setup global scons options for DAOS"""

import shutil
import sys
import os

def set_auto_clean():
    """Replicate using --config=force on the command line"""

    if (GetOption('config') == 'auto' and
        not GetOption('clean') and
        not GetOption('help')):

        # If --config=auto is set then do not wipe the config cache here.
        if '--config=auto' in sys.argv:
            return

        if os.path.exists('.sconsign.dblite'):
            os.unlink('.sconsign.dblite')

        if os.path.exists('.sconf_temp'):
            shutil.rmtree('.sconf_temp')

set_auto_clean()
