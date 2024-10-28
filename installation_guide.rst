.. _getting_started:

Getting Started Guide
#####################

Follow this guide to:

- Set up a command-line Zephyr development environment on Ubuntu, macOS, or
  Windows (instructions for other Linux distributions are discussed in
  :ref:`installation_linux`)
- Get the source code
- Build, flash, and run a sample application

.. _host_setup:

Select and Update OS
********************

Click the operating system you are using.

.. tabs::

   .. group-tab:: Ubuntu

      This guide covers Ubuntu version 20.04 LTS and later.

      .. code-block:: bash

         sudo apt update
         sudo apt upgrade

   .. group-tab:: macOS

      On macOS Mojave or later, select *System Preferences* >
      *Software Update*. Click *Update Now* if necessary.

      On other versions, see `this Apple support topic
      <https://support.apple.com/en-us/HT201541>`_.

   .. group-tab:: Windows

      Select *Start* > *Settings* > *Update & Security* > *Windows Update*.
      Click *Check for updates* and install any that are available.

.. _install-required-tools:

Install dependencies
********************

Next, you'll install some host dependencies using your package manager.

The current minimum required version for the main dependencies are:

.. list-table::
   :header-rows: 1

   * - Tool
     - Min. Version

   * - `CMake <https://cmake.org/>`_
     - 3.20.5

   * - `Python <https://www.python.org/>`_
     - 3.10

   * - `Devicetree compiler <https://www.devicetree.org/>`_
     - 1.4.6

.. tabs::

   .. group-tab:: Ubuntu

      .. _install_dependencies_ubuntu:

      #. If using an Ubuntu version older than 22.04, it is necessary to add extra
         repositories to meet the minimum required versions for the main
         dependencies listed above. In that case, download, inspect and execute
         the Kitware archive script to add the Kitware APT repository to your
         sources list.
         A detailed explanation of ``kitware-archive.sh`` can be found here
         `kitware third-party apt repository <https://apt.kitware.com/>`_:

         .. code-block:: bash

            wget https://apt.kitware.com/kitware-archive.sh
            sudo bash kitware-archive.sh

      #. Use ``apt`` to install the required dependencies:

         .. code-block:: bash

            sudo apt install --no-install-recommends git cmake ninja-build gperf \
              ccache dfu-util device-tree-compiler wget \
              python3-dev python3-pip python3-setuptools python3-tk python3-wheel xz-utils file \
              make gcc gcc-multilib g++-multilib libsdl2-dev libmagic1

      #. Verify the versions of the main dependencies installed on your system by entering:

         .. code-block:: bash

            cmake --version
            python3 --version
            dtc --version

         Check those against the versions in the table in the beginning of this section.
         Refer to the :ref:`installation_linux` page for additional information on updating
         the dependencies manually.

   .. group-tab:: macOS

      .. _install_dependencies_macos:

      #. Install `Homebrew <https://brew.sh/>`_:

         .. code-block:: bash

            /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

      #. After the Homebrew installation script completes, follow the on-screen
         instructions to add the Homebrew installation to the path.

         * On macOS running on Apple Silicon, this is achieved with:

           .. code-block:: bash

              (echo; echo 'eval "$(/opt/homebrew/bin/brew shellenv)"') >> ~/.zprofile
              source ~/.zprofile

         * On macOS running on Intel, use the command for Apple Silicon, but replace ``/opt/homebrew/`` with ``/usr/local/``.

      #. Use ``brew`` to install the required dependencies:

         .. code-block:: bash

            brew install cmake ninja gperf python3 python-tk ccache qemu dtc libmagic wget openocd

      #. Add the Homebrew Python folder to the path, in order to be able to
         execute ``python`` and ``pip`` as well ``python3`` and ``pip3``.

           .. code-block:: bash

              (echo; echo 'export PATH="'$(brew --prefix)'/opt/python/libexec/bin:$PATH"') >> ~/.zprofile
              source ~/.zprofile

   .. group-tab:: Windows

      .. note::

         Due to issues finding executables, the Zephyr Project doesn't
         currently support application flashing using the `Windows Subsystem
         for Linux (WSL)
         <https://msdn.microsoft.com/en-us/commandline/wsl/install_guide>`_
         (WSL).

         Therefore, we don't recommend using WSL when getting started.

      These instructions must be run in a ``cmd.exe`` command prompt terminal window.
      In modern version of Windows (10 and later) it is recommended to install the Windows Terminal
      application from the Microsoft Store. The required commands differ on PowerShell.

      These instructions rely on `Chocolatey`_. If Chocolatey isn't an option,
      you can install dependencies from their respective websites and ensure
      the command line tools are on your :envvar:`PATH` :ref:`environment
      variable <env_vars>`.

      |p|

      .. _install_dependencies_windows:

      #. `Install chocolatey`_.

      #. Open a ``cmd.exe`` terminal window as **Administrator**. To do so, press the Windows key,
         type ``cmd.exe``, right-click the :guilabel:`Command Prompt` search result, and choose
         :guilabel:`Run as Administrator`.

      #. Disable global confirmation to avoid having to confirm the
         installation of individual programs:

         .. code-block:: bat

            choco feature enable -n allowGlobalConfirmation

      #. Use ``choco`` to install the required dependencies:

         .. code-block:: bat

            choco install cmake --installargs 'ADD_CMAKE_TO_PATH=System'
            choco install ninja gperf python311 git dtc-msys2 wget 7zip

         .. warning::

            As of November 2023, Python 3.12 is not recommended for Zephyr development on Windows,
            as some required Python dependencies may be difficult to install.

      #. Close the terminal window.

.. _Chocolatey: https://chocolatey.org/
.. _Install chocolatey: https://chocolatey.org/install

.. _get_the_code:
.. _clone-zephyr:
.. _install_py_requirements:
.. _gs_python_deps:

Get Zephyr and install Python dependencies
******************************************