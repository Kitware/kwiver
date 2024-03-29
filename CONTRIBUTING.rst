======================
Contributing to KWIVER
======================

Pull Requests
=============

Integration Branches
--------------------

There are two primary integration branches named ``release`` and ``master``.
Generally, the ``release`` branch contains that last versioned stable release
plus a few patches in preparation for the next patch release.  The ``master``
branch contains new features and API changes since the last release and is
preparing for the next major or minor versioned release.

If your PR is a bug fix, unit testing improvement, or documentation enhancement
that applies to the last release, please branch off of ``release`` and submit
your PR to the ``release`` branch. If your PR is a new feature or bug fix
that applies to ``master`` but not to ``release`` then submit your PR to the
``master`` branch.  Any PR accepted in ``release`` is also accepted into
``master``, but not vice versa.

Release Notes
-------------

When making a PR, the topic branch should almost always include a commit which
updates the relevant release notes text file found in `<doc/release-notes>`_.
The relevant release notes file differs depending on whether you are targeting
``release`` or ``master``.  For the ``release`` branch notes should be added
to ``release.txt``.  For the ``master`` branch notes should be added to
``master.txt``.  Note that the ``master.txt`` file should not exist if you
have branched off of ``release``, but both files will exist if you have
branched off of ``master``.

The changes to the release notes files should, at a very high level, describe
the changes that the PR is introducing.  Usually you would add one or more
bullet points to describe the changes.  Bullet points should be entered in
the appropriate section.  There are sections for Updates (enhancements) as
well as Fixes.  There are subsection for different components of the code.
Most changes on the ``release`` branch go under Fixes, and most changes on
the ``master`` branch go under Updates.

If the code change on the topic branch impacts an existing release note
then the release note should be updated.  If the PR is to fix a bug
on the master branch that was introduced since the last release, then this
should **not** be documented in the Fixes section of the release notes
because the bug itself was never released.  This is one of the few cases
where release notes updates are not required on a PR.

Branch Naming
-------------

Topic branches should be named starting with a ``dev/`` prefix to distinguish
them from integration branches like ``master`` and ``release``.

Code Review
-----------

Pull requests are reviewed by one or more of the core KWIVER maintainers
using the Github tools for discussions.  Maintainers should not merge
a PR until it conforms to the requirements described here (e.g.
coding style, release notes, etc.) and it is confirmed that the code
has sufficient unit tests and does not break any existing unit tests.

Commits
-------

While not a strict requirement, it is beneficial for commits to record a set of
separate, logical changes.  Ideally, each commit should compile and have
correct code style (this is partly enforced by our workflow tools, and such
enforcement may be expanded in the future).  "Fixup" commits only serve to
clutter the history and make it harder to understand what changed.  Judicious
use of amend and rebase is encouraged.

Additionally, commits should follow the accepted conventions for git commit
messages.  In short:

- Use proper spelling and grammar; avoid "twitter speak".
- Use complete sentences (including the first line) and appropriate
  capitalization.  Use imperative mood.
- Try to limit the subject line to 50 characters.  *Don't* end with a period.
- Limit body lines (when necessary; occasionally just a subject is enough) to
  72 characters (unless this is impossible e.g. due to a long URL).

For further reading, see:

- https://chris.beams.io/posts/git-commit/
- https://www.freecodecamp.org/news/writing-good-commit-messages-a-practical-guide/
- https://medium.com/@steveamaza/how-to-write-a-proper-git-commit-message-e028865e5791


Coding Style
============

When developing KWIVER, please keep to the prevailing style of the code.

CMake, C++
----------

Please refer to the `style guide <doc/code-style.rst>`_ for a detailed
description and explanation of the coding style we follow.

KWIVER uses ``uncrustify`` to assist with style consistency.
A `configuration <.uncrustify.cfg>`_ is provided, however, please note that it
is considered experimental as of uncrustify 0.71, which is known to mis-handle
some edge cases and is not currently able to enforce all aspects of our desired
style. It is strongly recommended that you build uncrustify from from sources
(it requires only a C++ compiler and CMake, both of which you should already
have if you are doing KWIVER development), as feature additions and bug fixes
are frequent and older versions, especially those packaged by LTS
distributionsm, are often prone to misformatting.

Python
------

* Follow PEP8

* When catching exceptions, catch the type then use ``sys.exc_info()`` so
  that it works in Python versions from 2.4 to 3.3

* No metaclasses; they don't work with the same syntax in Python2 and Python3


Testing
=======

Generally, all new code should come with tests. The goal is sustained 95%
coverage and higher (due to impossible-to-generically-create corner cases such
as files which are readable, but error out in the middle). Tests should be
grouped into a single executable for each class, group of cooperating classes
(e.g., types tests), or higher-level use case. In C++, use Google Test and
follow the examples of other tests to integrate with the testing infrastructure
automatically. In Python, name functions so that they start with ``test_`` and
they will be picked up automatically.
