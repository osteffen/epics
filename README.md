epics
=====

My personal EPICS distribution for Debian-like systems (Debian, Ubuntu, Raspbian).

It contains:

	* EPICS Base 3.15 (bazaar version May 21, 2014)
	* Modules
	  * seq
	  * ipac
	  * asyn
	  * stream device
	* Extensions
	  * EasyCA

All sources are taken from the EPICs website.
The corresponding RELEASE files where adapted, no source files were changed.
Credit goes to the corresponding authors, I just put the sources together in one place.

EPICS and the here used modules are published under the [EPICS Open License](http://www.aps.anl.gov/epics/license/open.php).

Please see:
  * The [EPICS website](http://www.aps.anl.gov/epics)
  * The [EPICs modules download page](http://www.aps.anl.gov/epics/download/modules/index.php)

Usage
-----
1. Clone into /opt/epics. Other destination paths are not supportet right now.
2. cd /opt/epics
3. make -jN, where N = number of processes to use. You will be asked to enter your password for sudo to install (possibly) missing dependencies.
4. source thisEPICS.sh to set up paths.
