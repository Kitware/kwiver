"""
ckwg +29
Copyright 2020 by Kitware, Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * Neither name of Kitware, Inc. nor the names of any contributors may be used
   to endorse or promote products derived from this software without specific
   prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

--------------------

Ensures that all files in the repo have a correct copyright header.

Requirements:
    scriptconfig
    ubelt
"""
from os.path import join
import ubelt as ub
import scriptconfig as scfg
import glob
import datetime


COPYRIGHT_TEMPLATE = ub.codeblock("""
ckwg +29
 Copyright {year} by Kitware, Inc.
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

  * Neither name of Kitware, Inc. nor the names of any contributors may be used
    to endorse or promote products derived from this software without specific
    prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
""")


class CopyrightConfig(scfg.Config):
    default = {
        'root': scfg.Value('.', type=str, help='root directory to check'),
        'force': scfg.Value(False, type=bool, help='no changes are made unless this flag is set to True'),
        'ignore': scfg.Value([
            'vital/util/whereami.h',
            'vital/internal/cereal/external/rapidjson',
            ], help='a list of files that dont need a Kitware copyright'),
        'extensions': scfg.Value(['.h'], help='list of file extensions to check'),
    }


def main(**kw):
    """
    Ignore:
        >>> # TODO: make a doctest
        >>> kw = {
        >>>     'root': ub.expandpath('$HOME/code/kwiver'),
        >>>     'force': False,
        >>> }
    """

    import re
    config = CopyrightConfig(cmdline=True, default=kw)

    year = datetime.datetime.now().year
    cxx_template = '/*' + '\n *'.join(COPYRIGHT_TEMPLATE.split('\n')) + '\n */'
    cxx_copyright_text = cxx_template.format(year=year)

    ignore = config['ignore']
    root_dpath = config['root']
    extensions = config['extensions']

    # Determine the files that do need a copyright header
    needs_kitware_copyright = []
    patterns = [join(root_dpath, '**/*' + ext) for ext in extensions]
    fpaths = list(ub.flatten(glob.glob(pat, recursive=True) for pat in patterns))
    for fpath in fpaths:

        is_ignore = any(fpath.endswith(e) for e in ignore)
        if not is_ignore:
            head_size = 2000
            with open(fpath, 'r') as file:
                header = file.read(head_size)

            # Simple has copyright heuristic to determine if the file is
            # copyrighted by kitware or at all. We are assuming any
            # uncopyrighted file is a Kitware file that needs a copyright.
            # It is the user of this script's responsibility to check that.
            header_lower = header.lower()

            case = 'has_none'

            if 'copyright' in header_lower:
                case = 'has_any'

                if 'Kitware, Inc.' in header:
                    case = 'has_kw'

                if re.search(re.escape('//++ ') + '.*copyright', header, flags=re.IGNORECASE):
                    case = 'has_todo'

            if case == 'has_none':
                print('missing external copyright = {!r}'.format(fpath))
                needs_kitware_copyright.append(fpath)
            elif case == 'has_any':
                print('has external copyright = {!r}'.format(fpath))
            elif case == 'has_todo':
                print('has TODO copyright = {!r}'.format(fpath))
            elif case == 'has_kw':
                pass

    if config['force']:
        # Perform the copyright modifications
        print('needs_kitware_copyright = {}'.format(ub.repr2(needs_kitware_copyright, nl=1)))
        for fpath in needs_kitware_copyright:
            with open(fpath, 'r') as file:
                text = file.read()

            new_text = cxx_copyright_text + '\n\n' + text

            with open(fpath, 'w') as file:
                file.write(new_text)


if __name__ == '__main__':
    main()
