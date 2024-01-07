<< RESTORE APPLICABLE STUFF FROM UPSTRTEAM README >>

# Python Requirements
Python requirements for runtime are defined in the ``requirements.txt`` file,
and for development additionally the ``requirements/dev.txt`` file.

When developing on KWIVER, requirements from both of these files should be
installed into the appropriate [virtual] environment:

    $ pip install -r requirements.txt -r requirements/dev.txt

## Generation
The requirement files here are generated with the ``pip-compile`` tool from the
python package ``pip-tools``.

To regenerate the ``requirements/dev.txt`` file with the version-pinned python
package dependencies, from the repository root, run:

    $ pip-compile
    $ pip-compile requirements/dev.in

This should result in the generation of the ``requirements/dev.txt`` file.
