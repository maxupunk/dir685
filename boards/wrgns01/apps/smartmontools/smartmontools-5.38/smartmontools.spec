Release:  1
Summary:	smartmontools - for monitoring S.M.A.R.T. disks and devices
Summary(cs):	smartmontools - pro monitorov�n� S.M.A.R.T. disk� a za��zen�
Summary(de):	smartmontools - zur �berwachung von S.M.A.R.T.-Platten und-Ger�ten
Summary(es):	smartmontools - para el seguimiento de discos y dispositivos S.M.A.R.T.
Summary(fr):	smartmontools - pour le suivi des disques et instruments S.M.A.R.T.
Summary(pt):	smartmontools - para monitorar discos e dispositivos S.M.A.R.T.
Summary(it):	smartmontools - per monitare dischi e dispositivi S.M.A.R.T.
Summary(pl):	Monitorowanie i kontrola dysk�w u�ywaj�� S.M.A.R.T.
Name:		smartmontools
Version:	5.38
License:	GPL
Group:		Applications/System
Group(de):	Applikationen/System
Group(es):	Aplicaciones/Sistema
Group(fr):	Applications/Syst�me
Group(pt):	Aplicativos/Sistema
Group(it):      Applicazioni/Sistemi
Source0:	%{name}-%{version}.tar.gz
URL:            http://smartmontools.sourceforge.net/
Prereq:		/sbin/chkconfig
BuildRoot:	%{_tmppath}/%{name}-%{version}-root
Obsoletes:	smartctl
Obsoletes:      smartd
Obsoletes:	ucsc-smartsuite
Obsoletes:      smartsuite
Packager:       Bruce Allen <smartmontools-support@lists.sourceforge.net>

%define mandrake    %(test ! -f /etc/mandrake-release ; echo $?)
%define suse        %(test ! -f /etc/SuSE-release ; echo $?)
%define redhat      %(test ! -f /etc/redhat-release ; echo $?)
%define fedora      %(test ! -f /etc/fedora-release ; echo $?)
%if %{fedora}                                                                                                                                                             
   %define redhat 1                                                                                                                                                       
%endif

# Source code can be found at:
# http://ftp1.sourceforge.net/smartmontools/smartmontools-%{version}-%{release}.tar.gz

# CVS ID of this file is:
# $Id: smartmontools.spec,v 1.172 2008/03/04 22:09:47 ballen4705 Exp $

# Copyright (C) 2002-8 Bruce Allen <smartmontools-support@lists.sourceforge.net>
# Home page: http://smartmontools.sourceforge.net/
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation; either version 2, or (at your option) any later
# version.
# 
# You should have received a copy of the GNU General Public License (for
# example COPYING); if not, write to the Free Software Foundation, Inc., 675
# Mass Ave, Cambridge, MA 02139, USA.
#
# This code was originally developed as a Senior Thesis by Michael Cornwell
# at the Concurrent Systems Laboratory (now part of the Storage Systems
# Research Center), Jack Baskin School of Engineering, University of
# California, Santa Cruz. http://ssrc.soe.ucsc.edu/


%description
smartmontools controls and monitors storage devices using the
Self-Monitoring, Analysis and Reporting Technology System (S.M.A.R.T.)
built into ATA and SCSI Hard Drives.  This is used to check the
reliability of the hard drive and to predict drive failures.  The suite
is derived from the smartsuite package, and contains two utilities.  The
first, smartctl, is a command line utility designed to perform simple
S.M.A.R.T. tasks.  The second, smartd, is a daemon that periodically
monitors smart status and reports errors to syslog.  The package is
compatible with the ATA/ATAPI-5 specification.  Future releases will be
compatible with the ATA/ATAPI-6 andATA/ATAPI-7 specifications.  The
package is intended to incorporate as much "vendor specific" and
"reserved" information as possible about disk drives.  man smartctl and
man smartd will provide more information.  This RPM file is compatible
with all RedHat releases back to at least 6.2 and should work OK on any
modern linux distribution.  The most recent versions of this package and
additional information can be found at the URL:
http://smartmontools.sourceforge.net/

%description -l cs
smartmontools ��d� a monitoruj� za��zen� pro ukl�d�n� dat za pou�it�
technologie automatick�ho monitorov�n�, anal�zy a hl�en�
(Self-Monitoring, Analysis and Reporting Technology System -
S.M.A.R.T.) vestav�n�ho do pevn�ch disk� ATA a SCSI. Pou��v� se ke
kontrole pou�itelnosti pevn�ho disku a p�edv�d�n� hav�ri� disk�.
N�stroje jsou odvozeny od bal��ku smartsuite a obsahuj� dva programy.
Prvn�, smartctl, je n�stroj pro prov�d�n� jednoduch�ch S.M.A.R.T. �loh
na p��kazov� ��dce. Druh�, smartd, je d�mon, kter� periodicky
monitoruje stav a hl�s� chyby do syst�mov�ho protokolu. Bal��ek je
kompatibiln� se specifikac� ATA/ATAPI-5. Dal�� verze budou
kompatibiln� se specifikacemi ATA/ATAPI-6 a ATA/ATAPI-7. Bal��ek je
navr�en tak, aby pokryl co nejv�ce polo�ek s informacemi "z�visl� na
v�robci" a "rezervov�no". V�ce informac� z�sk�te pomoc� man smartctl a
man smartd. Tento RPM bal��ek je kompatibiln� se v�emi verzemi RedHatu
a m�l by fungovat na v�ech modern�ch distribuc�ch Linuxu. Aktu�ln�
verzi najdete na URL http://smartmontools.sourceforge.net/

%description -l de
Die smartmontools steuern und �berwachen Speicherger�te mittels des
S.M.A.R.T.-Systems (Self-Monitoring, Analysis and Reporting Technology,
Technologie zur Selbst-�berwachung, Analyse und Berichterstellung), das
in ATA- und SCSI-Festplatten eingesetzt wird.  Sie werden benutzt, um
die Zuverl�ssigkeit der Festplatte zu pr�fen und Plattenfehler
vorherzusagen.  Die Suite wurde vom smartsuite-Paket abgeleitet und
enth�lt zwei Dienstprogramme.  Das erste, smartctl, ist ein
Kommandozeilentool, das einfache S.M.A.R.T. Aufgaben ausf�hrt.  Das
zweite, smartd, ist ein Daemon, der periodisch den S.M.A.R.T.-Status
�berwacht und Fehler ins Syslog protokolliert.  Das Paket ist zur
ATA/ATAPI-5 Spezifikation kompatibel. Zuk�nftige Versionen werden auch
die ATA/ATAPI-6 und ATA/ATAPI-7 Spezifikationen umsetzen.  Das Paket
versucht, so viele "herstellerspezifische" und "reservierte" Information
�ber Plattenlaufwerke wie m�glich bereitzustellen.  man smartctl und man
smartd liefern mehr Informationen �ber den Einsatz.  Dieses RPM ist zu
allen RedHat-Versionen ab sp�testens 6.2 kompatibel und sollte unter
jedem modernen Linux arbeiten.  Die aktuellsten Versionen dieses Pakets
und zus�tzliche Informationen sind zu finden unter der URL:
http://smartmontools.sourceforge.net/

%description -l es
smartmontools controla y hace el seguimiento de dispositivos de
almacenamiento usando el Self-Monitoring, Analysis and Reporting
Technology System (S.M.A.R.T.) incorporado en discos duros ATA y SCSI. 
Es usado para asegurar la fiabilidad de discos duros y predecir averias. 
El conjunto de programas proviene del conjunto smartsuite y contiene dos
utilidades.  La primera, smartctl, es una utilidad command-line hecha
para hacer operaciones S.M.A.R.T. sencillas.  La segunda, smartd, es un
programa que periodicamente chequea el estatus smart e informa de
errores a syslog.  Estos programas son compatibles con el sistema
ATA/ATAPI-5.  Futuras versiones seran compatibles con los sistemas
ATA/ATAPI-6 y ATA/ATAPI-7.  Este conjunto de programas tiene el
proposito de incorporar la mayor cantidad posible de informacion
reservada y especifica de discos duros.  Los comandos 'man smartctl' y
'man smartd' contienen mas informacion.  Este fichero RPM es compatible
con todas las versiones de RedHat a partir de la 6.2 y posiblemente
funcionaran sin problemas en cualquier distribucion moderna de linux. 
La version mas reciente de estos programas ademas de informacion
adicional pueden encontrarse en: http://smartmontools.sourceforge.net/

%description -l fr
smartmontools contr�le et fait le suivi de p�riph�riques de stockage
utilisant le syst�me Self-Monitoring, Analysis and Reporting
Technology (S.M.A.R.T) int�gr�dans les disques durs ATA et SCSI.  Ce
syst�me est utilis� pour v�rifier la fiabilit� du disque dur et pr�dire
les d�faillances du lecteur.  La suite logicielle d�rive du paquet
smartsuite et contient deux utilitaires.  Le premier, smartctl,
fonctionne en ligne de commande et permet de r�aliser des t�ches
S.M.A.R.T. simples.  Le second, smartd, est un d�mon qui fait
p�riodiquement le suivi du statut smart et transmet les erreurs au
syslog.  Ce paquet est compatible avec la sp�cification ATA/ATAPI-5. 
Les prochaines versions seront compatibles avec les sp�cifications
ATA/ATAPI-6 et ATA/ATAPI-7.  Ce paquet tente d'incorporer le plus
d'informations possible sur les disques durs qu'elles soient sp�cifiques
au constructeur ("vendor specific") ou r�serv�es ("reserved").  man
smartctl et man smartd donnent plus de renseignements.  Ce fichier RPM
est compatible avec toutes les versions de RedHat v6.2 et ult�rieures,
et devrait fonctionner sur toutes les distributions r�centes de Linux. 
Les derni�res versions de ce paquet et des informations suppl�mentaires
peuvent �tre trouv�es � l'adresse URL:
http://smartmontools.sourceforge.net/

%description -l pt
smartmontools controla e monitora dispositivos de armazenamento
utilizando o recurso Self-Monitoring, Analysis and Reporting Technology
System (S.M.A.R.T.) integrado nos discos r�gidos ATA e SCSI, cuja
finalidade � verificar a confiabilidade do disco r�gido e prever falhas
da unidade.  A suite � derivada do pacote smartsuite, e cont�m dois
utilit�rios.  O primeiro, smartctl, � um utilit�rio de linha de comando
projetado para executar tarefas simples de S.M.A.R.T.  O segundo,
smartd, � um daemon que monitora periodicamente estados do smart e
reporta erros para o syslog.  O pacote � compat�vel com a especifica��o
ATA/ATAPI-5.  Futuras vers�es ser�o compat�veis com as especifica��es
ATA/ATAPI-6 e ATA/ATAPI-7.  O pacote pretende incorporar o maior n�mero
poss�vel de informa��es "espec�ficas do fabricante" e "reservadas" sobre
unidades de disco.  man smartctl e man smartd cont�m mais informa��es. 
Este arquivo RPM � compat�vel com todas as vers�es do RedHat a partir da
6.2 e dever� funcionar perfeitamente em qualquer distribui��o moderna do
Linux.  As mais recentes vers�es deste pacote e informa��es adicionais
podem ser encontradas em http://smartmontools.sourceforge.net/

%description -l it
smartmontools controlla e monitora dischi che usano il "Self-Monitoring,
Analysis and Reporting Technology System" (S.M.A.R.T.), in hard drive
ATA e SCSI. Esso � usato per controllare l'affidabilit� dei drive e
predire i guasti. La suite � derivata dal package smartsuite e contiene
due utility. La prima, smartctl, � una utility a linea di comando
progettata per eseguire semplici task S.M.A.R.T.. La seconda, smartd, �
un daemon che periodicamente monitora lo stato di smart e riporta errori
al syslog. Il package � compatibile con le specifiche ATA/ATAPI-6 e
ATA/ATAPI-7. Il package vuole incorporare tutte le possibili
informazioni riservate e "vendor specific" sui dischi. man smartctl e
man smartd danno pi� informazioni. Questo file RPM � compatibile con
tutte le release di RedHat, almeno dalla 6.2 e dovrebbe funzionare bene
su ogni moderna distribuzione di linux. Le versioni pi� recenti di
questo package e informazioni addizionali possono essere trovate al sito
http://smartmontools.sourceforge.net/

%description -l pl
Pakiet zawiera dwa programy (smartctl oraz smartd) do kontroli i
monitorowania system�w przechowywania danych za pomoc� S.M.A.R.T -
systemu wbudowanego w wi�kszo�� nowych dysk�w ATA oraz SCSI. Pakiet
pochodzi od oprogramowania smartsuite i wspiera dyski ATA/ATAPI-5.

# The following sections are executed by the SRPM file
%prep

%setup -q

%build
  %configure
  make

%install
  rm -rf $RPM_BUILD_ROOT
  rm -rf %{_buildroot}
  %makeinstall
  rm -f examplescripts/Makefile*
  %if %{suse}
    mkdir -p $RPM_BUILD_ROOT%{_defaultdocdir}
    mv $RPM_BUILD_ROOT/usr/share/doc/%{name}-%{version} $RPM_BUILD_ROOT%{_defaultdocdir}/%{name}
    ln -s ../../etc/rc.d/init.d/smartd $RPM_BUILD_ROOT%{_sbindir}/rcsmartd
  %endif

%files
  %defattr(-,root,root)
  %attr(755,root,root) %{_sbindir}/smartd
  %attr(755,root,root) %{_sbindir}/smartctl
  %if %{suse}
    %attr(755,root,root) %{_sbindir}/rcsmartd
  %endif
  %attr(755,root,root) /etc/rc.d/init.d/smartd
  %attr(644,root,root) %{_mandir}/man8/smartctl.8*
  %attr(644,root,root) %{_mandir}/man8/smartd.8*
  %attr(644,root,root) %{_mandir}/man5/smartd.conf.5*
  %doc AUTHORS CHANGELOG COPYING INSTALL NEWS README TODO WARNINGS smartd.conf examplescripts
  %config(noreplace) %{_sysconfdir}/smartd.conf

%clean
  rm -rf $RPM_BUILD_ROOT
  rm -rf %{_buildroot}
  rm -rf %{_builddir}/%{name}-%{version}

# The following are executed only by the binary RPM at install/uninstall

# since this installs the gzipped documentation files, remove
# non-gzipped ones of the same name.

# run before installation.  Passed "1" the first time package installed, else a larger number
%pre
if [ -f /usr/share/man/man8/smartctl.8 ] ; then
	echo "You MUST delete (by hand) the outdated file /usr/share/man/man8/smartctl.8 to read the new manual page for smartctl."	
fi
if [ -f /usr/share/man/man8/smartd.8 ] ; then
	echo "You MUST delete (by hand) the outdated file /usr/share/man/man8/smartd.8 to read the new manual page for smartd."	
fi
if [ -f /usr/share/man/man5/smartd.conf.5 ] ; then
        echo "You MUST delete (by hand) the outdated file /usr/share/man/man5/smartd.conf.5 to read the new manual page for smartd.conf"
fi

if [ ! -f /etc/smartd.conf ]; then
	echo "Note that you can use a configuration file /etc/smartd.conf to control the"
	echo "startup behavior of the smartd daemon.  See man 8 smartd for details."
fi

# run after installation.  Passed "1" the first time package installed, else a larger number
%post
# if smartd is already running, restart it with the new daemon
if [ -f /var/lock/subsys/smartd ]; then
        /etc/rc.d/init.d/smartd restart 1>&2
	echo "Restarted smartd services"
else
# else tell the user how to start it
        echo "Run \"/etc/rc.d/init.d/smartd start\" to start smartd service now."
fi

# Now see if we should tell user to set service to start on boot	
/sbin/chkconfig --list smartd > /dev/null 2> /dev/null
printmessage=$?

if [ $printmessage -ne 0 ] ; then
	echo "Run \"/sbin/chkconfig --add smartd\", to start smartd service on system boot"
else
	echo "smartd will continue to start up on system boot"
fi


# run before uninstallation.  Passed zero when the last version uninstalled, else larger
%preun

# if uninstalling the final copy, stop and remove any links	
if [ "$1" = "0" ]; then
  if [ -f /var/lock/subsys/smartd ]; then
    /etc/rc.d/init.d/smartd stop 1>&2
    echo "Stopping smartd services"
  fi

# see if any links remain, and kill them if they do
  /sbin/chkconfig --list smartd > /dev/null 2> /dev/null
  notlinked=$?
	
  if [ $notlinked -eq 0 ]; then
    /sbin/chkconfig --del smartd
    echo "Removing chkconfig links to smartd boot-time startup scripts"
  fi
fi

# run after uninstallation. Passed zero when the last version uninstalled, else larger
# %postun

%define date	%(echo `LC_ALL="C" date +"%a %b %d %Y"`)

# Maintainers / Developers Key:
# [BA] Bruce Allen
# [EB] Erik Inge Bols�
# [SB] Stanislav Brabec
# [PC] Peter Cassidy
# [YD] Yuri Dario
# [CD] Capser Dik
# [CF] Christian Franke
# [GF] Guilhem Fr�zou
# [DG] Douglas Gilbert
# [GG] Guido Guenther
# [GK] Geoff Keating
# [DK] David Kirkby
# [KM] Kai M�kisarai
# [EM] Eduard Martinescu
# [FM] Fr�d�ric L. W. Meunier
# [KS] Keiji Sawada
# [DS] David Snyder
# [SS] Sergey Svishchev
# [PW] Phil Williams
# [LW] Leon Woestenberg
# [RZ] Richard Zybert
# [SZ] Sf Zhou


%changelog
* Wed Dec 20 2006 Bruce Allen  <smartmontools-support@lists.sourceforge.net>
  [CF] Windows: Added alternate method for (limited) monitoring of
       3ware controllers by parsing the output of CLI or 3DM.
       Either "tw_cli" can be run internally ("/dev/tw_cli/cx/py"),
       or data can be read from standard input ("/dev/tw_cli/stdin")
       or clipboard ("/dev/tw_cli/clip").
  [DG] Remove linux specific libata detect code; rely on general SAT
       code. smartd should now generate a sensible log message for
       ATA devices behind a SAT layer on all architectures.
  [BA] Increased max line length MAXLINELEN for /etc/smartd.conf from
       128 to 256 characters to handle long strings in
       /dev/disk/by-id.  Thanks to Martin Krafft.
  [PW] Drive database: added missing drives from Seagate Momentus 5400.2
       family
  [BA] Finished Christian's fix (next item below) by removing
       LINUX_86_64 hack from configure.in.
  [CF] Fixed inclusion of PRI?64 macros from inttypes.h.
  [CF] Windows: Added WRITE LOG to support selective self tests.
  [CF] Fix selective self test log revision number if '-T permissive'
       is specified (Tested with Samsung HD401LJ).
  [CF] Windows: Fixed int64 printf format for MinGW runtime.
  [PW] Drive database: added Seagate Barracuda 7200.10 family, Seagate
       Momentus 42 family, Maxtor DiamondMax 60 ATA 66 family, Maxtor
       DiamondMax 60 ATA 100 family, and Western Digital Caviar Serial
       ATA family
  [PW] Drive database: added missing drives from Seagate Barracuda
       7200.9 family, Seagate Barracuda 7200.7 family, Seagate Momentus
       7200.1 family, Toshiba 2.5" HDD family (80 GB and above), Western
       Digital Caviar RE Serial ATA family, Hitachi Deskstar 7K80 family,
       and Maxtor DiamondMax 4320 Ultra ATA family
  [BA] Linux: compile fix for SuSE.  Check for existence
       of linux/compiler.h and include in os_linux.h if
       present.  Thanks to SB.
  [BA] smartd: DEVICESCAN will now pick up SATA/SAT devices
       attached to a SCSI device tree via SAT translation.
       Note: this is a bit of a hack.  I will document it once
       I know if this needs to be Linux only or can have more
       general application.
  [BA] Added a couple SATA commands to the tables -- thanks DG!
       Phil -- how about going through and systematically adding
       these new commands to atacmdnames.cpp?
  [BA] Linux s86_64: get rid of some compiler warnings on
       x86_64 Linux systems.
  [CF] Windows: Added missing support for READ_LOG, ABORT_SELFTEST
       and CHECK_POWER_STATE for 3ware 9000 controllers. Thanks to
       Greg de Valois for implementing this new ioctl in the driver.
  [PW] Drive database: added Seagate NL35 SATA family.  Thanks to Kai
       Harrekilde-Petersen for providing a patch.
  [DG] [SCSI, Windows] add SPT interface for NT and later. New device
       names are "pd<n>", "sd<l>" and "tape<n>".
  [PW] Drive database: added Western Digital Scorpio family, Fujitsu MHV
       family, Maxtor MaXLine Pro 500 family, and Maxtor DiamondMax 11
       family
  [PW] Drive database: added missing drives from Toshiba 2.5" HDD
       (30-60 GB) family, Maxtor DiamondMax 10 family, Seagate Barracuda
       7200.8 family, Fujitsu MHT family, and Maxtor DiamondMax Plus 8
       family
  [SB] Added examplescripts/Example4 using powersave-notify.
  [SB] More temperature monitoring examples in smartd.conf with DEVICESCAN.
  [SB] Minor improvements of SuSE part of init script.
  [CF] Drive database: added Samsung P80 series, P120 series, SP8004H
       and T series.
  [GG] Add CCISS (Compaq Smart Array Controller) support with contributions
       from Praveen Chidambaram, Douglas Gilbert, Guido Guenther and Fr�d�ric
       BOITEUX
  [PW] Drive database: added Hitachi Deskstar T7K250 and Hitachi
       Deskstar 7K500 series.  Thanks to L. J. Wu for providing a
       patch
  [PW] Drive database: added Maxtor MaXLine III family, Seagate U7
       family, Seagate ST34321A, FUJITSU MHM2060AT, FUJITSU MHT2040AS,
       Western Digital Caviar SE16 family, IBM Travelstar 4GT family,
       QUANTUM FIREBALLP KA9.1, QUANTUM FIREBALL SE4.3A, TOSHIBA
       MK1032GAX, TOSHIBA MK4026GAX
  [PW] Drive database: added missing drives from Western Digital Caviar
       SE (Serial ATA) and WD Raptor families
  [CF] Windows: Added support for 3ware 9000 controllers using extended
       SMART functionality in new 3ware driver. This includes DEVICESCAN
       support for at most 2 controllers. Thanks to Greg de Valois from
       AMCC/3ware for new driver features, development support and
       hardware for testing.
  [SZ] smartd: Support HighPoint RocketRAID controller under GNU/linux
  [DG] [SCSI] First cut for '-l background' to show background scan
       results log
  [SZ] smartctl: Support HighPoint RocketRAID controller under GNU/linux
  [KS] C++ compile fixes for Solaris with a few cleanups.
  [BA] C++ compile fixes for Darwin (thanks to CF)
  [CF] Removed old *.c files (now in CVS Attic).
  [CF] Added changes for C++ to platform independent and Windows
       related files.
  [BA] Tagged last .c Version with PRE_MOVE_TO_CPP. Copied *.c,v
       to *.cpp,v in CVS repository to preserve history of source
       files. Removed sm5_Darwin repository.
  [CF] smartctl: Added -n option to skip checks when disk is in
       low-power mode.
  [CF] Windows: Added alternate system call for power state check
       because the PASS THROUGH calls may spin up the disk.
  [CF] smartd: Modified power state logging to report state changes
       instead of standby condition.
  [CF] smartd: Ignore -n directive on scheduled self tests.
  [DG] [SCSI] Make start stop cycle counter log page decoding
       more robust
  [DG] Modify smartctl (but not smartd) to detect probable ATA
       devices behind a SAT layer. In the absence of an explicit
       device type, change to device type 'sat'.
  [DG] Add indication that controller (device) type has been
       explicitly set. Preparation for automatic detection of
       'sat' device type unless user specifies a device type.
  [SS] NetBSD: Deliver strings from ata_identify_device properly 
       on little- and big-endian platforms.
  [BA] Added published ANSI ATA-7 spec to list of recognized ATA
       versions.
  [BA] Code janitor: added missing header strings for '-V' option.
  [DG] [SATA] Extend 'sat' device type to allow either 12 or 16 byte
       variant of the SAT ATA PASS THROUGH SCSI command. Syntax is
       '-d sat,<n>' where <n> can be 0, 12 or 16 . The ',<n>' part
       is optional. Currently defaults to 16 byte variant but that
       could be made platform or even device dependent.
  [DG] [SATA] Add new 'sat' device type for SATA disks behind a
       SCSI to ATA Translation (SAT) Layer (SATL). Uses the ATA
       PASS THROUGH (16) SCSI command thence the generic SCSI
       passthrough for each platform.
  [CF] Windows: Added script and make targets to create installer
       with NSIS (http://nsis.sourceforge.net/)
  [CF] Updated hostname and links for new SourceForge CVS service.
  [CF] smartd: Added '-W' directive to track temperature changes
       and warn if temperature limits are reached.
  [CF] Windows: Added IOCTL_ATA_PASS_THROUGH (Win2003, XP SP2)
       for commands unsupported by SMART_IOCTL. Added device
       specific options to select subset and ordering of the ATA
       IOCTLs actually used. These options are specified as
       modifiers of the device name (/dev/hd[a-j]:[saic]+)
  [CF] Windows: Added support for drives 4-7 (/dev/hd[e-h]) via
       SMARTVSE.VXD on Win9x/ME. Thanks to Dariusz Rzonca for
       patch and testing.
  [DG] [SCSI/SATA linux] from lk 2.6.17 SATA disk identification in
       libata will change. Expand LibAta detection to see old
       identifier and new variant (VPD page 0x83).
  [BA] Identified Attribute 190 for Western Digital disks.  This
       stores temperature in Celsius, just like Attribute 194.
       But it has a failure threshold set to correspond to the
       maximum design operating temperature of the disk, which
       is 55 Celsius on the WD800JD drives that I studied.
       So if this Attribute has 'failed
       in the past' this means that the maximum disk operating
       temperature has been exceeded.
  [GK] Darwin: Add support for AHCI drivers found in Intel-based Macs.

* Wed Apr 12 2006 Bruce Allen  <smartmontools-support@lists.sourceforge.net>
  [BA] Linux: smartd/smartctl issue syntax hints to user if 3ware
       disk controller present with EITHER 3ware OR AMCC vendor
       name, and user syntax incorrect.
  [BA] Update copyright dates to 2006.
  [DG] [SCSI] Loosen sanity check on Seagate/Hitachi factory information
       log page so it is not skipped on recent Seagate SCSI disks.
  [CF] Added command 'smartd -q showtests' to list test schedules.
  [CF] Added command 'smartctl -P showall MODEL [FIRMWARE]' to list
       database entries for specific drives and firmware.
  [PW] Automatically set -v 9,minutes and -v 194,unknown for Maxtor
       DiamondMax D540X-4G drives.
  [DG] [SCSI] suppress various outputs when data fails sanity checks.
       Correct 'last n error events' log page indexing.
  [DG] [SCSI] changed smartctl exit status to reflect any problems in
       the most recent 20 self test logs [Leandro Santi]
  [DG] [SCSI] Fix process return value when scsiGetSmartData() fails
       in smartctl [Leandro Santi]
  [BA] Updated docs and error message to reflect Linux libata
       support for smartmontools starting with the 2.6.15 kernel
       series. Also init script support for the 'tinysofa' release.
  [DG] [SCSI] Mask dpofua bit when changing mode pages. Fix failure
       of 'smartctl -l error'.
  [EM] Fixed a problem with FreeBSD and 3Ware 'twe' devices
  [CF] Fixed a regexp in knowndrives table, added regexp syntax check
       via 'smartctl -P showall'.
  [CF] Cygwin & Windows: Fixed memory leak in function calling
       IOCTL_IDE_PASS_THROUGH. Thanks to Fred Schmidt for the problem
       report.
  [CF] Cygwin: added cygrunsrv support and commands "install", "remove"
       and "status" to smartd.initd.
  [SS] Fix runtime problems on big-engian NetBSD platforms (patch provided
       by Martin Husemann)
  [CF] Cygwin smartd: Open smartd.conf in textmode to allow use of
       Windows editors.
  [CF] Cygwin smartd: Added option '--service' to allow smartd running
       as windows service via cygrunsrv. Useful in conjunction with new
       syslogd support added in Cygwin 1.5.15.
  [CF] Windows: Added patch to avoid output of non-ascii timezone names.
  [EM] Incorporate various patches to provide TWE support and support for 
       multiple 3Ware cards, Power Check Support, and FreeBSD 6.x support.
       Thanks to Rudolf Cejka, Frank Behrens, and Jung-uk Kim.
  [DG] Silence gcc 4.0.1 compile warning concerning the difference in
       "signedness" in pointer assignments. Changes to SCSI code
       and os_linux.c .
  [PW] Additions to knowndrives table: added missing drive from Quantum
       Fireball Plus LM series, added QUANTUM BIGFOOT TS10.0A, added
       ExcelStor J680 and J880, added Western Digital Caviar RE Serial ATA
       series, added missing drives from Western Digital Caviar SE series,
       added Seagate Momentus 4200.2 series, added missing drives from
       Maxtor DiamondMax 10 series, added Fujitsu MHG and MHH series, and
       added Hitachi Travelstar 5K100 series.
  [PW] Additions to knowndrives table: added Fujitsu MHU2100AT, added
       Fujitsu M1623TAU, added missing drives from Seagate Barracuda
       7200.8 series, added Seagate Momentus 5400.2 series, and added
       QUANTUM FIREBALL CR8.4A.
  [PW] Additions to knowndrives table: added missing drive from Maxtor
       MaxLine II series, added Maxtor DiamondMax 2880 Ultra ATA series,
       added Maxtor DiamondMax 17 VL series, added Hitachi Deskstar 7K80
       series, and added Hitachi Deskstar 7K400 series.
  [CF] Windows: Fixed unsupported 'smartctl -X' on Win2000/XP by using
       IOCTL_IDE_PASS_THROUGH instead.

* Tue Apr 20 2005 Bruce Allen  <smartmontools-support@lists.sourceforge.net>
  [CF] Cygwin & Windows smartd: Increased SCSI DEVICESCAN range
       from ASPI adapter 0-3 to 0-9. Added diagnostic messages.
  [CF] Windows smartd: Added ability to run .bat files via '-M exec'
       directive.
  [CF] Cygwin smartd: Added FreeConsole() after fork() to avoid hang
       of terminated shell console window.
  [DG] [SCSI] Add code so 'smartctl -A' outputs the number of elements
       in the grown defect list. When this number is increasing a
       disk has problems. N.B. Similar logic should be added to smartd.
  [CF] Windows smartd: Fixed event handling to allow start of another
       smartd process when service is already running. Useful for testing
       service configuration changes in debug mode.
  [PW] Added following drives to knowndrives table: Western Digital Raptor
       family, Seagate Barracuda 7200.8 family, Maxtor DiamondMax 2160
       Ultra ATA and DiamondMax 10 families, Hitachi Travelstar E7K60
       family, Seagate Medalist 17240, 13030, 10231, 8420, and 4310,
       TOSHIBA MK4018GAP and MK6022GAX, ExcelStor Technology J360, and
       Western Digital Caviar AC14300.
  [PW] Added missing Fujitsu MHTxxxxAT and Seagate Barracuda 7200.7 drives
       to knowndrives table.
  [PW] Added QUANTUM FIREBALLP LM10.2 to knowndrives table.  Thanks to
       Mike Fleetwood for submitting the patch.
  [KS] Solaris/SPARC: fixed not to disable automatic offline test and
       automatic save attributes incorrectly.  Thanks to Roy Badami.
  [BA] Linux: smartd init script now recognizes 'trustix' distro.
  [DG] [SCSI] Medium and hardware errors were slipping through
       unreported. Fix linux SCSI sense reporting via SG_IO ioctl.
  [DG] [SCSI] Change lba of first failure in selftest output to
       decimal (was hex) to conform with ATA output.
  [GK] smartd: Detect most self-test failures even if the hour counter
       has wrapped.
  [BA] smartctl: list 'marvell' as option if user give invalid
       -d argument
  [CF] Windows: fixed SCSI timeout handling to allow long timeouts
       for selftests.
  [CF] Fixed buffer overflow issues in printone() and safe_vsnprintf()
       which results in crash on -V option (at least on Windows).
  [DG] [SCSI] Add explicit timeouts to INQUIRY and REQUEST SENSE (that
       were missed in an earlier patch). Could have impacted freebsd.
  [DG] When linux detects a sata_via_libata disk suggest that user try
       '-d ata' (rather then '-d libata). Anticipate kernel change.
  [YD] Added OS/2 and eComStation platform support.
  [PW] Added Seagate U4 family, Fujitsu MHJ and MHK families, Seagate
       Barracuda 5400.1, QUANTUM FIREBALLP KX27.3, QUANTUM FIREBALLP KA10.1,
       and ExcelStor J340 to knowndrives table.
  [DG] [SCSI] After report of Hitachi IC35L073UCDY10 disks locking up
       on log page 0x7 (last n error events), check log page (and some
       others) is supported (via log page 0x0) before probing.
  [CF] Added safe_v?snprintf() for platforms using v?snprintf()
       with non standard behaviour on overflow (Windows, old Linux)
  [CF] smartd: Added message if check power mode spins up disk.
  [CF] Windows: Added support for READ_LOG on WinNT4 using undocumented
       pseudo SCSI command via IOCTL_SCSI_PASS_THROUGH.
  [CF] smartd: Added ',q' option for '-n' directive to suppress 'skipping
       checks' log message. This prevents a laptop disk from spinning up
       due to this message. Thanks to Rob MacLachlan and Manfred Schwarb
       for pointing out problem & solution.
  [CF] Windows: Added function get_os_version_str() to show OS flavor in
       copyright message.
  [CF] Windows: Added function ata_identify_is_cached() to check for outdated
       SMART enabled bit in identify data.
  [CF] Windows: Added fix to prevent linkage of smartd specific win32 modules
       to smartctl.
  [PW] Added Fujitsu MPG3153AH, Hitachi Endurastar J4K20/N4K20 (formerly
       DK23FA-20J), Seagate Momentus family, and Maxtor Fireball 3 family
       to knowndrives table.
  [PW] Added missing Maxtor DiamondMax 16, Seagate Barracuda ATA IV, and
       Western Digital Caviar WDxxxAA/WDxxxBA drives to knowndrives table.
  [CF] Windows: Added ATA check power mode for smartd -n directive.
  [CF] Windows: Fixed use of new service status flag which causes hang
       of smartd service on WinNT4.
  [CF] Windows: Fixed error checking of IOCTL_IDE_PASS_THROUGH (used
       for READ_LOG on 2000/XP). Added some diagnostic messages on
       -r ataioctl[,2]. Thanks to Manfred Schwarb for bug report and testing.
  [BA] Fixed code bug that made it impossible to enable SMART on
       disks with failing health status.  This would happen if the
       os_*.c author made STATUS and STATUS_CHECK work the same way.
       I have corrected this at a higher level; we now handle the
       case where STATUS and STATUS_CHECK are identical without
       issues. 
  [LW] Make os_linux.c/marvell_command_interface() always return 0 on STATUS.
       Needed for a disk having bad SMART status.
  [CF] smartctl: Added drive family printing.
  [CF] autogen.sh: Allow automake 1.9, added message if automake
       version is unknown.
  [BA] smartctl: use locale-specific separators for printing disk
       capacity.  Also use AC_CHECK_HEADERS not AC_CHECK_HEADER in
       configure.in.
  [BA] clean-up of #include structure so that -V options to smartd
       and smartctl work correctly.  Please, don't #include header
       files into other header files.

* Fri Sep 10 2004 Bruce Allen  <smartmontools-support@lists.sourceforge.net>
  [BA] smartctl: ATA disks, if SMART ATTRIBUTE THRESHOLDS page has ID
       errors with some Attributes having NULL IDs, print Attribute
       info anyway (but issuing a warning to the user).
  [DG] [SCSI] Decode Last n error events log page; decode track following
       and positioning errors [Hitachi]
  [EM] FreeBSD: another tweak, __packed__ introduced in Version 5.0040
  [EM] Cleaner tweak of fixes for FreeBSD 4.x.
  [EM] Fix compilation errors under FreeBSD 4.x, as it is still using
       and old GCC
  [EM] Remove 3ware/FreeBSD specific files and just include pieces we need
  [DG] Add logic in smartd to detect 3ware, Marvell controllers and SATA
       disks behind an ATA-SCSI simulator (in Linux). If specific device
       types are not given and they are picked in a general SCSI device
       scan then warn and skip.
  [GG] insert correct path to smartd into smartd's init script
  [BA] Changed all default paths in documentation to reflect /usr/local as
       default path prefix.  This affects on-line man pages, primarily.
  [DS] Added support for OpenBSD.
  [BA] Added another environment variable SMART_FULLMESSAGE set by
       the smartd mailing feature, and modified examplescripts/Example1
       to illustrate it.
  [BA] Fixed potentially misleading messages of the form:
       XXX failed: success
  [DG] emit warning if SATA disk detected using libata in Linux; then exit
  [PW] Added Seagate U10 family, Hitachi Travelstar 7K60, Fujitsu MHR2020AT,
       and QUANTUM FIREBALLP AS20.5 to knowndrives table.
  [DG] Detect 3ware and Marvell controllers from SCSI INQUIRY vendor string
       and suggest usage of appropriate '-d' argument in smartctl.
  [LW] Tested the RELEASE_5_33_WITH_MARVELL_SUPPORT branch on
       actual Marvell 88SX5041 hardware, with success.
       Merged into HEAD.
  [BA] Fixed nasty DEVICESCAN bug
  [BA] Checked in RELEASE_5_33_WITH_MARVELL_SUPPORT branch with
       some Marvell support.
  [BA] Additional modifications of Ed's controller scheme.  Fixed
       broken 3ware support under linux, problems with scanning
       devices in smartd, and other small problems.
  [EM] Minor change to FreeBSD inclusion of 'twe' include files.  Add 
       code to check if they exising in /usr/include/sys to use those
       in preference to ones added here
  [EM] Very preliminary support attempt for 3Ware controllers under 
       FreeBSD. Also, switched 'escalade_type/escalade_port' to
       'controler_type/controller_port' and moved away from 
       'tryata/tryscsi' to using new 'controller*' variables to 
       determine which controller type (ATA/SCSI/3Ware) to use.
  [GK] Added initscript support for Darwin.
  [CF] Windows smartd: Added ability to run smartd as a windows service,
       including new commands "smartd install ..." and "smartd remove"
       to install and remove the service registry entry.
  [BA] smartd: warn user if -s regexp regular expression contains
       characters other than 0123456789.*()|+?[-]{}:=SLCO since such
       characters are 'suspicous' and may indicate a poorly formed
       regexp.  Extended regular expression gurus: can this list be
       reduced somewhat?
  [CF] Fixed bug in Windows smartd: Missing close of config file when
       configuration is reloaded by smartd daemon.
  [CF] Windows smartd: Added mail warning feature using the "Blat"
       (http://blat.sourceforge.net/) mailer as a default.
  [PW] Added Maxtor DiamondMax Plus 5120 Ultra ATA 33 series and TOSHIBA
       MK3017GAP to knowndrives table.
  [CF] Added fixes to build smartmontools on old Linux systems
       (libc < 6, Kernel 2.0.x).
  [BA] Added ATA minor version identity strings for latest ATA specification
       updates: ATA/ATAPI-7 T13 1532D revision 4a and ATA/ATAPI-6 published,
       ANSI INCITS 361-2002
  [PW] Added Hitachi Travelstar 5K80 family and Fujitsu MHTxxxxAH family to
       knowndrives table.
  [EM] Fix up compilation under FreeBSD < 5.x
  [PW] Added QUANTUM FIREBALL EX3.2A and missing Western Digital Caviar SE
       drives to knowndrives table.
  [BA] Modified Hitachi Travelstar 80GN family regexp in drive database.
       Thanks to [GK/CF] for problem & solution.
  [GK] Added os_darwin.[ch]
  [PW] Added the following drives to the knowndrives table: IBM Travelstar
       48GH, 30GN, and 15GN family; IBM Deskstar 37GP and 34GXP family;
       Western Digital WDC WD272AA; Maxtor DiamondMax D540X-4D family;
       TOSHIBA MK2016GAP, MK2018GAP, MK2018GAS, MK2023GAS; and
       QUANTUM FIREBALL ST3.2A
  [BA] smartd/smarctl now print build HOST/OS information as part
       of startup slogan.  This should make it slightly easier to
       read bug reports from users.
  [RZ] Fixed the DEVICESCAN to do what it was supposed to do - give
       error message unless scanning is in progress.  
  [BA] Update documentation to describe 3ware character devices. Better
       error detection for missing/malfunctioning devices behind 3ware
       controllers. Now pack 3ware ioctl structures explicitly.
  [BA] For ATA devices that support LBA mode, print capacity as part
       of smartctl --info
  [RZ] Made DEVICESCAN quiet about non-existing devices unless debug
       is on.
  [DG] treat "unit attention" SCSI warning as try again in some contexts
       (test unit ready and mode sense)
  [BA] on drives that store max/min rather than min/max, get order
       correct in printing temp.
  [BA] fixed typo in 'smartctl -h' output.  Thanks to Gabor Z. Papp.
  [BA] linux: clean-up to 3ware/AMCC support; dynamically create
       or fix /dev/tw[ae][0-15] device node entries if they don't
       exist or are incorrect. One can now use the character devices
       /dev/twe[0-15] OR /dev/sd? for 3ware 6000/7000/8000 series
       cards.  One must use /dev/twa[0-15] for 3ware 9000 series cards.
       Note that selective self-tests now work via /dev/tw[ae] devices.
       Next step: documentation.
  [BA] linux: experimental "support" for 3ware/AMCC 9000 series
       controllers that use the 3w-9xxx driver.  This will be in a
       state of flux for a few days.  Note that this requires the
       character interface /dev/twa[0-15].
  [DG] linux: extend general SCSI OS interface to use the SG_IO ioctl. If
       not available, use the older SCSI_IOCTL_SEND_COMMAND ioctl.
  [KS] Solaris/x86: fixed system identification problem in configure
       script.  Thanks to Stuart Swales.

* Mon Jul 5 2004 Bruce Allen  <smartmontools-support@lists.sourceforge.net>
  [BA] Update link to revised/updated IBM Deskstar Firmware
  [CF] Cygwin & Windows: Added missing ASPI manager initialization
       with GetASPI32SupportInfo(). Thanks to Nikolai SAOUKH for pointing
       this out and providing a patch.
  [BA] modified smartd init script to work on whitebox (thanks to
       Michael Falzon)
  [BA] removed (reverted) additional Attribute definitions from
       http://smart.friko.pl/attributes.php.  All (or most?) of these
       appear to be return code values for the WD Digital Life Guard Utility.
  [PW] Added Seagate Medalist 17242, 13032, 10232, 8422, and 4312 to
       knowndrives table.  Added missing Seagate U Series 5 drives.
  [PW] Added the following QUANTUM models to knowndrives table:
       FIREBALL EX6.4A, FIREBALLP AS10.2, FIREBALLP AS40.0, FIREBALL CR4.3A,
       FIREBALLP LM15, FIREBALLP LM30, and FIREBALLlct20 30
  [PW] Added missing Western Digital Protege drives to knowndrives table.
  [PW] Added Maxtor DiamondMax 40 ATA 66 series and DiamondMax 40 VL Ultra
       ATA 100 series to knowndrives table.
  [PW] Added the following Hitachi/IBM drives to knowndrives table:
       HITACHI_DK14FA-20B, Travelstar 40GNX series, Travelstar 4LP series,
       and Travelstar DK23XXB series.  Added the missing Travelstar 80GN
       drives.
  [PW] Added Fujitsu MPB series and MPG series to knowndrives table.  Added
       the missing Fujitsu MHSxxxxAT drives.
  [KS] Solaris: added workaround for dynamic change of time-zone.
  [KS] Solaris: fixed problem that autogen.sh cannot detect absence of
       auto* tools.
  [BA] smartd: added time-zone bug information to man page. 
       Reverted CF code for _WIN32 case. 
  [CF] Cygwin & Windows: Added better error messages on IDE/ATA device
       open error.
  [BA] added additional Attribute definitions from
       http://smart.friko.pl/attributes.php
  [BA] smartd: reworked TimeZone bug workaround so it is only invoked
       for glibc.  Note: this might not be right -- a similar bug may
       exist in other platform's libcs.
  [DG] SCSI smartmontools documentation updated [2004/5/6]. See:
       http://smartmontools.sourceforge.net/smartmontools_scsi.html
  [CF] Windows: Fixed reset of TZ=GMT in glibc timezone bug workaround.

* Tue May 4 2004 Bruce Allen  <smartmontools-support@lists.sourceforge.net>
  [DG] move SCSI device temperature and start-stop log page output
       (smartctl) into --attributes section (was in --info section).
  [GG] change default installation location to /usr/local
  [CF] Cygwin smartd: Fixed crash on access of SCSI devices after fork().
  [PW] Added TOSHIBA MK4018GAS and the following Maxtor drive families
       to knowndrives table: DiamondMax D540X-4G, Fireball 541DX,
       DiamondMax 3400 Ultra ATA, DiamondMax Plus 6800 Ultra ATA 66.
  [PW] Added missing Maxtor DiamondMax 16, DiamondMax D540X-4K, and
       DiamondMax Plus 45 Ulta ATA 100 drives to knowndrives table.
  [PW] Added ExcelStor J240, Hitachi Travelstar 80GN family, Fujitsu
       MHTxxxxAT family, and IBM Deskstar 25GP and 22GXP families to
       knowndrives table.
  [CF] Cygwin smartd: Added workaround for missing SIGQUIT via keyboard:
       To exit smartd in debug mode, type CONTROL-C twice.
  [BA] smartctl: printing of the selective self-test log is now
       controlled by a new option: -l selective
  [BA] Added entries for Samsung firmware versions -25 to -39 based
       on latest info about firmware bug fixes.
  [PW] Added Seagate U Series X family, Seagate U8 family, and Seagate
       Medalist 8641 family to knowndrives table.
  [CF] smartd: Added exit values 5/6 for missing/unreadable config file.
  [BA] smartd: now monitor the Current Pending Sector count (Attribute 197)
       and the Offline Pending Sector Count (Attribute 198).  Log a
       warning (and send an email, if so configured) if the raw count
       is nonzero.  These are controlled by new Directives: -C and -U.
       Currently they are enabled by default.
  [CF] Added option -c FILE, --configfile=FILE to smartd to specify
       an alternate configuration FILE or '-' for standard input.
  [KS] configure.in now searches for -lnsl and -lsocket for Solaris.
  [CF] Win32/native smartd: Added thread to combine several syslog output
       lines into one single event log entry.
  [CF] Win32 smartd: Added DEVICESCAN for SCSI/ASPI devices.
  [GG] Use gethostbyname() the get the DNS domain since getdomainname() 
       returns the NIS domain when sending mails from smartd.
  [GG] smartd.init.in: pass smartd_opts to smartd on startup, read distribution
       specific configuration files if found
  [SS] smartctl: added NetBSD support for Selective Self-tests.
  [BA] smartd.conf example configuration file now has all examples
       commented out except for 'DEVICESCAN'.
  [CF] Win32/native smartd: Added ability to display warning "emails"
       as message box by "-m msgbox" directive. With "-m sysmsgbox",
       a system modal (always on top) message box is shown.
  [BA] smartctl: printing of self-test log for disks that support
       Selective self-testing now shows the status of the (optional)
       read-scan after the selective self test.  Also, changed format
       in printing self-test log to print failing LBA in base 10 not
       base 16 (more compatible with kernel error messages).  Also,
       in printing SMART error log, print timestamps in format
       days+hours+minutes+seconds.
  [CF] Win32 smartd: Added ability to log to stdout/stderr
       (-l local1/2). Toggling debug console still works
       if stdout is redirected.
  [BA] smartctl: selective self-test log, print current status
       in a  more detailed way.  Allow writing of selective self-test
       log provided that no other self-test is underway.
  [BA] Linux: eliminated dependency on kernel tree hdreg.h.
  [BA] smartctl: -l selftest option now prints Selective self-test
       log in addition to the normal self-test log.
       Added additional options (-t pending, -t afterselect) to
       control remaining Selective Self-test capabilities.  Tested
       with several Maxtor disks. Modified error message printing
       so that munged option messages print at the end not the
       start of output.
  [CF] Added daemon support to Win32 native version of smartd.
       The daemon can be controlled by commands similar to initd
       scripts: "smartd status|stop|reload|restart|sigusr1|sigusr2".
  [CF] Added minor support for option "-l local[0-7]" to Win32 native
       (not Cygwin) version of smartd. If specified, the log output
       is written to file "./smartd[1-7]?.log" instead of event log.
  [BA] Added Selective Self-test to smartctl (-t selective,M-N).
       Currently only supported under Linux; Solaris, NetBSD, FreeBSD
       and Windows developers must add WRITE LOG functionality to
       os_*.c
  [BA] Added workaround for an annoying glibc bug: if you change
       timezones, (eg, flying with a laptop from USA to Europe)
       localtime() does not notice this in a running
       executable, so time that appears in the system log (syslog!)
       will be incorrect.  See
       http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=48184
       for additional examples of this bug.
  [DG] Set explicit timeouts for SCSI commands (most default to 6 seconds).
       Previously a 0 second timeout was meant to be interpreted as a 
       default timeout but the FreeBSD port had a problem in this area.
  [CF] Fixed un-thread-safe exit signal handler for Win32
  [BA] Fixed un-thread-safe exit signal handler pointed out
       by CF.
  [BA] Changed configure script to eliminate warnings under
       Solaris from sys/int_type.h conflicts with int64.h
       Added header files for umask to smartd.c.
  [BA] Man page format change from Werner LEMBERG.  " " changed to \&
  [CF] Added os_win32/syslogevt.* event message file tool for Win32
       smartd (native+cygwin). May also be useful for other cygwin
       programs writing to syslog().
  [CF] Added Win32 version of smartd
  [CF] Merged RELEASE_5_26_WIN32_BRANCH
  [BA] Made some changes to man page markup suggested by
       Richard Verhoeven to work around bugs in man2html.
       Tested not to break anything under Linux and Solaris.
  [CF] Moved PrintOut() from utility.c to smart{ctl,d}.c to avoid
       syslog() output of smartctl.
  [BA] Grew worried that some time-zone names could be very long (eg,
       Mitteleuropaische Zeit) and put date string lengths into a
       single macro in utility.c
  [EM] Updated os_freebsd.c to handle older versions of FreeBSD in a 
       more appropriate/obvious fashion.
  [EM] Modified autogen.sh as FreeBSD installs automake 1.7 as 
       'automake17' and NOT 'automake-1.7'

* Sat Mar 6 2004 Bruce Allen  <smartmontools-support@lists.sourceforge.net>
  [PW] Added QUANTUM FIREBALLlct15 30, QUANTUM FIREBALLlct20 40, and
       Maxtor 6Y060P0 (DiamondMax Plus 9 60GB) to knowndrives table.
  [PW] Added Maxtor MaXLine II family to knowndrives table (thanks to
       Brett Russ for submitting the patch).
  [BA] Added remaining read/write commands to detailed list of
       error log commands that have text descriptions of problem
       printed.  For commands that support it, print number of failed
       sectors at problem LBA.
  [BA] Made SuSE section of smartd init script more SuSE 9 compatible.
       Thanks to Hans-Peter Jansen.
  [CF] Windows smartd: Added IDE/ATA device scan
       Added windows device names to smartctl.8.in, smartd.8.in
  [BA] smartctl/smartd: user-provided '-F samsung' and '-F samsung2'
       command line options/Directives did NOT over-ride preset values
       unless user specified '-P ignore'.  Now they will always over-ride
       preset values from the database.
  [BA] Added error decoding for a few more READ and WRITE commands.
  [PW] Added Maxtor MaXLine Plus II, Western Digital Caviar SE (Serial ATA)
       series, Hitachi Deskstar 7K250 series, and Ultra ATA 66 models of
       the Maxtor DiamondMax Plus 40 series to knowndrives table.
  [BA] Added Maxtor Diamondmax 250 GB drives to database.  Note that
       these model numbers are not listed in Maxtor documentation, but
       they exist.
  [BA] Removed the 'contact developers' phrase from the Samsung disk
       warning messages.
  [PW] Added TOSHIBA MK2017GAP, IBM Deskstar 14GXP and 16GP series,
       Fujitsu MPC series, Seagate Barracuda ATA III family, and missing
       Seagate Barracuda U Series drives to knowndrives table
  [BA] smartd: wrong loglevel for message: Configuration file
       /etc/smartd.conf parsed.  Changed to LOG_INFO from LOG_CRIT.
       Thanks to  Emmanuel CHANTREAU for the report.
  [CF] Checked in development version of windows code base.

* Tue Feb 24 2004 Bruce Allen  <smartmontools-support@lists.sourceforge.net>
  [BA] smartd: configure script did not set correct directory to search for
       smartd.conf based on --prefix argument to ./configure.  Thanks to
       GG for identifying the problem and fix.
  [BA] make clean now removes man pages (generated from *.in) files as well
       as object files.
  [EM] Correct copying of sense data in FreeBSD SCSI implementation. Thanks
       to Sergey Svishchev for noticing the bug.
  [BA] On solaris, wrong warning message if no ATA support.  Warning message
       concerns 3ware controller, not ATA.
  [SS] Added SCSI support for NetBSD.
  [BA] on big-endian linux machines, fixed interpretation of HDIO_GET_IDENTITY
       to correctly identify ATAPI bit (was byte swapped).  This should
       eliminate some SYSLOG noise if user queries a packet device (eg, CD
       ROM or DVD reader).
  [PW] Removed warning for IBM Deskstar 40GV & 75GXP series drives with
       A5AA/A6AA firmware.  Thanks to Gerald Schnabel.
  [PW] Added Toshiba TOS MK3019GAXB SUN30G to knowndrives table
  [PW] Added Western Digital Caviar AC12500, AC24300, AC25100, AC36400,
       and AC38400 to knowndrives table
  [BA] When printing ATA error log, print the LBA at which READ
       or WRITE commands failed.
  [BA] Changed syntax of error message in smartctl
  [BA] Added versioning info (-V options to smartd/smartctl) for
       Solaris ATA module.

* Thu Feb 12 2004 Bruce Allen  <smartmontools-support@lists.sourceforge.net>
  [KS] Added ATA/IDE support for Solaris/SPARC (ATA/IDE not yet for
       Solaris/x86).
  [BA] 3ware controllers: documented that one can monitor any of the
       physical disks from any of the 3ware /dev/sd? logical devices.
       Better warnings if querying a disk that does not exist.
  [PW] Added Hitachi Travelstar DK23DA series, Maxtor DiamondMax Plus 40
       series, Western Digital Caviar WDxxxAA, WDxxxBA, and WDxxxAB series
       to knowndrives table
  [BA] missing 'pragma pack' on ATA IDENIFY DEVICE structure may have
       caused odd or incorrect results on 64-bit machines.
  [BA] smartctl/smartd allow inspection of self-test and error logs even
       if disk firmware claims that these don't exist.  This is needed
       for some Maxtor disks whose firmware does not indicate log support
       even though the disk DOES support it.
  [BA] Improved porting instructions and documentation in os_generic.c
  [PW] Add Western Digital Caviar WD136AA and SAMSUNG SP40A2H (RR100-07
       firmware) to knowndrives table.
  [EM] FreeBSD:	remove extra definition of FreeNonZero
  [BA] smartctl: the -q silent option was printing output for some
       error conditions.  Fixed.  Will rename relevant variables to help
       avoid these errors in the future.
  [SS] NetBSD port added.
  [BA] more sensible error messages for devfs and devfs-like systems.
       Instead of saying that the DIRECTORY does not exist, say that
       the DEVICE does not exist.
  [BA] smartd: added -n Directive, to prevent disk spin-up depending
       upon the power mode (SLEEP, STANDBY, or IDLE).
  [PW] Added Maxtor DiamondMax 20 VL series, Fujitsu MPF series,
       Maxtor DiamondMax 36 series, Maxtor DiamondMax 4320 series, and
       Maxtor DiamondMax 536DX series to knowndrives table.
  [BA] many warning messages now give the file name AND VERSION
  [BA] smartd: when the user provides multiple address recipients
       to the '-m' Directive in a comma-delineated list, the commas
       are stripped out before passing the list of addresses to the
       mailer program. (Thanks to Calin A. Culianu for pointing this out
       and providing a patch.)
  [BA] smartd: when the '-M exec path' Directive is used, any stdout OR
       stderr output from the executable "path" is assumed to indicate a
       problem, and is echoed to SYSLOG.
  [BA] Added all missing IBM/Hitachi Deskstar 180GXP models to knowndrives
       table.
  [PW] Added some missing IBM/Hitachi Deskstar 120GXP models to knowndrives
       table.
  [PW] Added IBM Travelstar 14GS to knowndrives table.
  [PW] Modified knowndrives table to match entire Hitachi Travelstar
       DK23BA and DK23EA series of drives (thanks to Norikatsu Shigemura
       for submitting the patch).
  [PW] Added some missing Fujitsu MPE series drives to knowndrives table.
  [PW] Added TOSHIBA MK4019GAX, TOSHIBA MK6409MAV, and QUANTUM
       FIREBALLlct15 20 to knowndrives table.
  [EM] Fixup example command output for FreeBSD
  [PW] Added Maxtor DiamondMax 80 family to knowndrives table.
  [EM] Catch up FreeBSD code to switch PROJECTHOME to PACKAGE_HOMEPAGE
       macros.
  [BA] smartd: now watches stdout/stderr when trying to run mail, mailx
       or mail warning script, and reports any output to SYSLOG.  This
       gives a clearer error message if something is wrong.
  [BA] smartd: Solaris init script modified to accomodate grep that
       lacks '-q' quiet option.  Also check for running process to kill
       on stop.
  [PW] Added some missing Seagate Barracuda 7200.7 and 7200.7 Plus drives
       to knowndrives table.
  [PW] Added Maxtor DiamondMax Plus 60 family and Seagate U Series 5 20413
       to knowndrives table.
  [BA] smartd: under Solaris, made default mailer be 'mailx' not
       'mail', since Solaris 'mail' does not accept a '-s' argument.
       A workaround for Solaris users of earlier versions is to
       have '-M exec /bin/mailx' in their smartd.conf config file.
  [DG] some SCSI controllers don't like odd length transfers so make
       sure LOG SENSE transfers are rounded up to an even number when
       and odd length is reported (i.e. there is a double fetch, the
       first to find the length, the second gets the data)
  [BA] smartd man pages: under Solaris, correct section numbers in the
       'See also' section.
  [KS/BA] smartd man page: describe how to set Solaris syslog.conf
       file to catch all messages.  Give correct Solaris SYSLOG default
       path /var/adm/messages in man pages.
  [BA] smartd: incorporated Debian startup script submitted by user.
  [BA] smartctl: modified printing of self-test log entry number.  Seagate
       firmware can leave 'holes' in the self-test log while a test is
       actually running.  We now print entry numbers consistently in this
       case, not assuming that entries are contiguous.
  [PW] Added QUANTUM FIREBALL CX10.2A and Western Digital Caviar AC23200L
       to knowndrives table.
  [PW] Added QUANTUM FIREBALLlct20 20 to knowndrives table.
  [PW] Added Maxtor DiamondMax Plus D740X family to knowndrives table.
  [PW] Added IBM Travelstar 32GH, 30GT, and 20GN family to knowndrives
       table.
  [BA] Slackware init script modified to search for /etc/slackware-version
       rather than /etc/slackware-release.
  [PW] Added Seagate Barracuda ATA II family and TOSHIBA MK4019GAXB to
       knowndrives table.
  [GG] explain howto use autoreconf in autogen.sh
  [KS] Makefile.am/configure.in: changed manual page sections for
       Solaris.
  [BA] smartd: reduced number of scheduled self-test messages if
       test already run in current hour.
  [PW] Added Maxtor DiamondMax Plus 8 family to knowndrives table.
  [BA] linux: check for linux/hdreg.h.  If it's there, use it. If
       not, provide the necessary definitions ourselves.
  [PW] Removed warning for IBM Deskstar 40GV & 75GXP series drives
       with TXAOA5AA firmware
  [PW] Added IBM Travelstar 25GS, 18GT, and 12GN family to knowndrives
       table.
  [PW] Added IBM/Hitachi Travelstar 60GH & 40GN family to knowndrives
       table.
  [BA] smartd: made '-s' Directive more efficient.  Now store
       compiled regex, and re-use.  If device lacks certain self-test
       capabilities, track it and don't try again.
  [BA] smartd: made memory allocation for device lists completely
       dynamic (eliminating compile-time maximum length constants).
  [PW] Removed warning for SAMSUNG SP0802N with TK100-23 firmware
  [PW] Added Seagate Barracuda ATA IV family to knowndrives table.
  [BA] smartd: reduce per-device memory footprint by making
       mail-warning info dynamically allocated.  Also remove
       potential memory leak if use has -m Directive twice and
       keeps reloading the config file (highly unlikely this would
       ever be noticed!)  
  [DG] smartd: added SCSI scheduled self-tests (Background
       short or extended).
  [BA] smartd: can now run scheduled offline immediate and
       self-tests.  See man page and -s Directive for details.
  [GG] don't include manpages in make-dist-tarball.
  [BA] smartctl: on-line examples given with -h are now correct
       for solaris and linux, but wrong for freebsd.  Ed?
  [BA] smartd: man page now explains device scanning for solaris as
       well as linux and freebsd.
  [BA] smartd/smartctl: man pages now report correct CVS tag release
       date, and executables '-V' options reports more build info.

* Sat Nov 29 2003 Bruce Allen <smartmontools-support@lists.sourceforge.net>
  [BA] Improved user messages that appear from 'make install'
  [PW] Removed warning for SAMSUNG SP1213N with firmware TL100-23
  [BA] incorporated SuSE init script from user.
  [DG] if SCSI device is read only, then open it read only.
  [BA] when compiled on non-supported system (NOT linux, freebsd or solaris) then
       the run-time error messages now clearly say 'your system is not supported'
       and give clear directions.
  [BA] ./configure script now works correctly on SuSE linux boxes
  [BA] minor improvements to man pages
  [BA] simplified detection of packet (ATAPI, CD) devices.
  [BA] init script (redhat, mandrake, yellowdog) now uses correct
       strings for translation and is slightly more standard.
  [DG] smartctl: output scsi Seagate vendor pages for disks (not tapes)
* Wed Nov 19 2003 Bruce Allen <smartmontools-support@lists.sourceforge.net>
  [DG] smartd/smartctl: changed scsiClearControlGLTSD() to
       scsiSetControlGLTSD() with an 'enabled' argument so '-S on'
       and '-S off' work for SCSI devices (if changing GLTSD supported).
  [BA] smartd/smartctl: wired in scsiClearControlGLTSD(). Could still
       use a corresponding Set function.  Left stubs for this purpose.
  [DG] scsicmds: added scsiClearControlGLTSD() [still to be wired in]
  [BA] smartctl: make SCSI -T options behave the same way as the
       ATA ones.
  [DG] smartctl: output scsi transport protocol if available
  [DG] scsi: stop device scan in smartd and smartctl if badly formed
       mode response [heuristic to filter out USB devices before we
       (potentially) lock them up].
  [BA] smartd: deviceclose()->CloseDevice(). Got rid of SCSIDEVELOPMENT
       macro-enabled code.  Added -W to list of gcc specific options to
       always enable. Made code clean for -W warnings.
  [PW] Added Maxtor DiamondMax VL 30 family to knowndrives table.
  [DG] scsi: add warning (when '-l error' active) if Control mode page
       GLTSD bit is set (global disable of saving log counters)
  [DG] scsi: remember mode sense cmd length. Output trip temperature
       from IE lpage (IBM extension) when unavailable from temp lpage.
  [BA] smartd: for both SCSI and ATA now warns user if either
       the number of self-test errors OR timestamp of most
       recent self-test error have increased.
  [DG] smartctl: output Seagate scsi Cache and Factory log pages (if
       available) when vendor attributes chosen
  [DG] smartd: add scsiCountFailedSelfTests() function.
  [DG] Do more sanity checking of scsi log page responses.
  [BA] smartd: now warns user if number of self-test errors has
       increased for SCSI devices.
  [BA] smartd: warn user if number of ATA self-test errors increases
       (as before) OR if hour time stamp of most recent self-test
       error changes.
  [DG] More checks for well formed mode page responses. This has the side
       effect of stopping scans on bad SCSI implementations (e.g. some
       USB disks) prior to sending commands (typically log sense) that
       locks them up.
  [PW] Added Western Digital Caviar family and Caviar SE family to
       knowndrives table.
  [BA] smartd: added -l daemon (which is the default value if -l
       is not used).
  [PW] Added Seagate Barracuda ATA V family to knowndrives table.
  [BA] smartd: added additional command line argument -l FACILITY
       or --logfacility FACILITY.  This can be used to redirect
       messages from smartd to a different file than the one used
       by other system daemons.
  [PW] Added Seagate Barracuda 7200.7, Western Digital Protege WD400EB,
       and Western Digital Caviar AC38400 to knowndrives table.
  [BA] smartd: scanning should now also work correctly for
       devfs WITHOUT traditional links /dev/hd[a-t] or /dev/sd[a-z].
  [PW] Added Maxtor 4W040H3, Seagate Barracuda 7200.7 Plus,
       IBM Deskstar 120GXP (40GB), Seagate U Series 20410,
       Fujitsu MHM2100AT, MHL2300AT, MHM2150AT, and IBM-DARA-212000
       to knowndrives table.
  [PW] Added remaining Maxtor DiamondMax Plus 9 models to knowndrives
       table.
  [EM] smartd: If no matches found, then return 0, rather than an error
       indication, as it just means no devices of the given type exist.
       Adjust FreeBSD scan code to mirror Linux version.
  [BA] smartd: made device scan code simpler and more robust. If
       too many devices detected, warn user but scan as many
       as possible.  If error in scanning, warn user but don't
       die right away.
  [EM] smartd: To keep as consistent as possible, migrate FreeBSD
       devicescan code to also use glob(3). Also verified clean 
       compile on a 4.7 FreeBSD system.
  [BA] smartd: Modified device scan code to use glob(3). Previously
       it appeared to have trouble when scanning devices on an XFS
       file system, and used non-public interface to directory
       entries. Problems were also reported when /dev/ was on an
       ext2/3 file system, but there was a JFS partition on the same
       disk.
  [BA] Clearer error messages when device scanning finds no suitable
       devices.
  [EM] FreeBSD:	Fixup code to allow for proper compilation under 
       -STABLE branch.

* Fri Oct 31 2003 Bruce Allen <smartmontools-support@lists.sourceforge.net>
- [BA] smartd: didn't close file descriptors of ATA packet devices
       that are scanned. Fixed.
- [BA] Added reload/report targets to the smartmontools init script.
       reload: reloads config file
       report: send SIGUSR1 to check devices now

* Mon Oct 27 2003 Bruce Allen <smartmontools-support@lists.sourceforge.net>
- [EM] Fix compile issues for FreeBSD < 5-CURRENT.
- [PW] Added Fujitsu MHM2200AT to knowndrives table.
- [BA] To help catch bugs, clear ATA error structures before all
       ioctl calls.  Disable code that attempted to time-out on SCSI
       devices when they hung (doesn't work).
- [BA] Documented STATUS/ERROR flags added by [PW] below.
- [BA] Improved algorithm to recognize ATA packet devices. Should
       no longer generate SYSLOG kernel noise when user tries either
       smartd or smartctl on packet device (CD-ROM or DVD).  Clearer
       warning messages from smartd when scanning ATA packet device.
- [PW] Added TOSHIBA MK4025GAS to knowndrives table.
- [PW] Added a textual interpretation of the status and error registers
       in the SMART error log (ATA).  The interpretation is
       command-dependent and currently only eight commands are supported
       (those which produced errors in the error logs that I happen to
       have seen).
- [BA] added memory allocation tracking to solaris code.
       Fixed solaris signal handling (reset handler to default
       after first call to handler) by using sigset. Added
       HAVE_SIGSET to configure.in
- [CD] solaris port: added SCSI functionality to solaris
       stubs.
- [BA] smartd: attempt to address bug report about smartd
       hanging on USB devices when scanning:
       https://bugzilla.redhat.com/bugzilla/show_bug.cgi?id=107615
       Set a timeout of SCSITIMEOUT (nominally 7 seconds) before
       giving up.
- [EM] smartd: DEVICESCAN will follow links in a devfs filesystem and
       make sure the end point is a disc.  Update documentation, added
       note about FreeBSD scanning
- [BA] smartd: DEVICESCAN also looks for block devices in
       /dev.  Updated documentation.  Now scans for up to
       20 ATA devices /dev/hda-t rather than previous 12
       /dev/hda-l.
- [EM] smartd: mirror the FreeBSD DEVICESCAN logic for Linux,
       so that smartd now scans only devices found in /dev/. Also,
       make utility memory functions take a line number and file so
       that we report errors with the correct location.
- [GG] add a note about Debian bug #208964 to WARNINGS.
- [BA] smartctl: -T verypermissive option broken.  Use
       -T verpermissive until the next release, please.
- [BA] Syntax mods so that code also compiles on Solaris using
       Sun Workshop compiler.  Need -xmemalign 1i -xCC flags
       for cc.

* Wed Oct 15 2003 Bruce Allen <smartmontools-support@lists.sourceforge.net>
  [DK] Changed configure.in so -Wall is only included if gcc
       is used (this is a gcc specific flag) and -fsignedchar
       is not used at all (this is a gcc specific compiler 
       flag).
  [BA] Modifications so that code now compiles under solaris. Now
       all that's needed (:-) is to fill in os_solaris.[hc].  Added
       os_generic.[hc] as guide to future ports.  Fixed -D option
       of smartd (no file name).  Modified -h opt of smartd/smartctl
       to work properly with solaris getopt().
  [EM] Update MAN pages with notes that 3ware drives are NOT supported
	under FreeBSD. Cleanup FreeBSD warning message handling.
  [EM] FreeBSD only: Fix first user found bug....I guess I was making
       the wrong assumption on how to convert ATA devnames to
       channel/unit numbers.
  [EM] Allow for option --enable-sample to append '.sample' to installed
	smartd.conf and rc script files. Also, let rc script shell setting
	be determined by configure
  [EM] Minor autoconf update to include -lcam for FreeBSD
  [EM] Add conditional logic to allow FreeBSD to compile pre-ATAng.
	-- note, not tested
	Add some documentation to INSTALL for FreeBSD.
  [EM] Implement SCSI CAM support for FreeBSD.  NOTE: I am not an expert
	in the use of CAM.  It seems to work for me, but I may be doing
	something horribly wrong, so please exercise caution.
  [EM] Switch over to using 'atexit' rather than 'on_exit' routine. This also
  	meant we needed to save the exit status elsewhere so our 'Goodbye'
	routine could examine it.
  [EM] Move the DEVICESCAN code to os specific files. Also moved some of the
	smartd Memory functions to utility.c to make available to smartctl.
  [EM] Code janitor work on os_freebsd.c.
  [EM] Added os_freebsd.[hc] code.  Additional code janitor
       work.
  [BA] Code janitor working, moving OS dependent code into
       os_linux.[hc].
  [GG] conditionally compile os_{freebsd,linux}.o depending on
       host architecture
  [PW] Print estimated completion time for tests
  [BA] Added -F samsung2 flag to correct firmware byte swap.
       All samsung drives with *-23 firmware revision string.

* Sun Oct 05 2003 Bruce Allen <smartmontools-support@lists.sourceforge.net>
- [GG] Fixed broken Makefile.am (zero length smartd.conf.5
       was being created)
- [FM] Improved Slackware init script added to /etc/smartd.initd

* Fri Oct 03 2003 Bruce Allen <smartmontools-support@lists.sourceforge.net>
- [BA] smartctl: added '-T verypermissive' option which is
       equivalent to giving '-T permissive' many times.
- [BA] Try harder to identify from IDENTIFY DEVICE structure
       if SMART supported/enabled.  smartd now does a more
       thorough job of trying to assess this before sending
       a SMART status command to find out for sure.
- [BA] smartctl: it's now possible to override the program's
       guess of the device type (ATA or SCSI) with -d option.
- [BA] try hard to avoid sending IDENTIFY DEVICE to packet
       devices (CDROMS).  They can't do SMART, and this generates
       annoying syslog messages. At the same time, identify type
       of Packet device.
- [BA] smartctl: Can now use permissive option more
       than once, to control how far to go before giving up.
- [BA] smartd: if user asked to monitor either error or self-test
       logs (-l error or -l selftest) WITHOUT monitoring any of the
       Attribute values, code will SEGV.  For 5.1-18 and earlier,
       a good workaround is to enable Auto offline (-o on).
- [BA] smartctl: If enable auto offline command given, update auto
       offline status before printing capabilities.
- [GG] Make autotools build the default, remove autotools.diff
- [GG] Add auto{conf,make} support, not enabled by default. 
- [BA] Eliminated #include <linux/hdreg.h> from code. This
       should simplify porting to solaris, FreeBSD, etc. The
       only linux-specific code is now isolated to three routines,
       one for SCSI, one for Escalade, one for ATA.

* Fri Aug 22 2003 Bruce Allen <smartmontools-support@lists.sourceforge.net>
- [BA] smartd: fixed serious bug - Attributes not monitored unless
       user told smartd to ignore at least one of them!

* Tue Aug 19 2003 Bruce Allen <smartmontools-support@lists.sourceforge.net>
- [BA] Default runlevels for smartd changed from 3 and 5 to
       2, 3, 4, and 5.
- [BA] Removed as much dynamic memory allocation as possible from
       configuration file parsing. Reloading config file, even in
       presence of syntax errors etc. should not cause memory leaks.
- [PW] It is no longer permissible for the integer part (if any) of
       arguments to --report and --device to be followed by non-digits.
       For example, the "foo" in --report=ioctl,2foo was previously
       ignored, but now causes an error.
- [BA] smartd: added -q/--quit command line option to specify
       under what circumstances smartd should exit.  The old
       -c/--checkonce option is now obsoleted by this more
       general-purpose option.
- [BA] smartd now responds to a HUP signal by re-reading its
       configuration file /etc/smartd.conf.  If there are
       errors in this file, then the configuration file is
       ignored and smartd continues to monitor the devices that
       it was monitoring prior to receiving the HUP signal.
- [BA] Now correctly get SMART status from disks behind 3ware
       controllers, thanks to Adam Radford. Need 3w-xxxx driver
       version 1.02.00.037 or later. Previously the smartmontools
       SMART status always returned "OK" for 3ware controllers.
- [BA] Additional work on dynamic memory allocation/deallocation.
       This should have no effect on smartctl, but clears that way
       for smartd to dynamically add and remove entries.  It should
       also now be easier to modify smartd to re-read its config
       file on HUP (which is easy) without leaking memory (which is
       harder). The philosophy is that memory for data structures in
       smartd is now allocated only on demand, the first time it
       is needed.
- [BA] smartd: finished cleanup.  Now use create/rm functions for
       cfgentries and dynamic memory allocation almost everywhere.
       Philosophy: aggresively try and provoke SEGV to help find
       bad code.
- [BA] Added SAMSUNG SV0412H to knowndrives table.
- [BA] smartd: if DEVICESCAN used then knowndrives table might not set
       the -v attributes correctly -- may have been the same for all
       the drives.  Cleaned up some data structures and memory
       allocation to try and ensure segvs if such problems are
       introduced again.
- [BA] Now allow -S on and -o on for the 3ware device type.  For these
       commands to be passed through, the stock 3ware 3w-xxxx driver
       must be patched (8 lines).  I'll post a patch on the smartmontools
       home page after it's been tested by a few other people and 3ware
       have had a chance to look it over.

* Wed Aug 06 2003 Bruce Allen <smartmontools-support@lists.sourceforge.net>
- [BA] smartd - can now monitor ATA drives behind 3ware controllers.
- [BA] smartd - changed some FATAL out of memory error messages from
       syslog level LOG_INFO to LOG_CRIT.
- [BA] smartctl - added code to look at ATA drives behind 3ware RAID
       controllers using the 3w-xxxx driver.  Note that for technical
       reasons related to the 3w-xxxx driver, the "Enable Autosave",
       "Enable Automatic Offline" commands are not implemented.
       I will add this to smartd shortly.
- [BA] smartd - modified sleep loop, so that smartd no longer comes
       on the run queue every second.  Instead, unless interrupted,
       it sleeps until the next polling time, when it wakes up. Now
       smartd also tries to wake up at exactly the right
       intervals (nominally 30 min) even if the user has been sending
       signals to it.
- [GG] add Fujitsu MHN2300AT to vendoropts_9_seconds.
- [EB] Fujitsu change in knowndrives ... match the whole MPD and
       MPE series for vendoropts_9_seconds.
- [BA] smartd bug, might cause segv if a device can not be opened. Was
       due to missing comma in char* list.  Consequence is that email
       failure messages might have had the wrong Subject: heading for
       errorcount, FAILEDhealthcheck, FAILEDreadsmartdata, FAILEDreadsmarterrorlog,
       FAILEDreadsmartsefltestlog, FAILEDopendevice were all displaced by
       one.  And FAILEDopendevice might have caused a segv if -m was being
       used as a smartd Directive.

* Wed Jul 23 2003 Bruce Allen <smartmontools-support@lists.sourceforge.net>
- [BA] Cleaned up smartmontools.spec so that upgrading, removing
       and other such operations correctly preserve running behavior
       and booting behavior of smartd.
- [BA] Improved formatting of ATA Error Log printout, and added
       listing of names of commands that caused the error. Added
       obsolete ATA-4 SMART feature commands to table, along with
       obsolete SFF-8035i SMART feature command.
- [PW] Added atacmdnames.[hc], which turn command register &
       feature register pairs into ATA command names.
- [BA] Added conveyance self-test.  Some code added for selective
       self-tests, but #ifdefed out.
- [BA] Modified smartd exit status and log levels.  If smartd is
       "cleanly" terminated, for example with SIGTERM, then its
       exit messages are now logged at LOG_INFO not LOG_CRIT
- [BA] Added Attribute IDs  (Fujitsu) 0xCA - 0xCE.  This is decimal
       202-206. Added -v switches for interpretation of Attributes
       192, 198 and 201. 
- [BA] Made smartmontools work with any endian order machine for:
       - SMART selftest log
       - SMART ATA error log
       - SMART Attributes values
       - SMART Attributes thesholds
       - IDENTIFY DEVICE information
       - LOG DIRECTORY
       Smartmontools is now free of endian bias and works correctly
       on both little- and big-endian hardware.  This has been tested by
       three independent PPC users on a variety of ATA and SCSI hardware.
- [DG] Check that certain SCSI command responses are well formed. If
       IEC mode page response is not well formed exit smartctl. This
       is to protect aacraid. smartd should ignore a aacraid device.

* Mon Jun 16 2003 Bruce Allen <smartmontools-support@lists.sourceforge.net>
- [BA] smartctl: added column to -A output to show if Attributes are
       updated only during off-line testing or also during normal
       operation.

* Thu Jun 10 2003 Bruce Allen <smartmontools-support@lists.sourceforge.net>
- [BA] smartd: attempt to enable/disable automatic offline testing even
       if the disk appears not to support it.  Now the same logic
       as smartctl.
- [BA] Added definition of Attribute 201, soft read error rate.
- [BA] Added IBM/Hitachi IC35L120AVV207-1 (GXP-180) and corresponding
       8MB Cache GXP-120 to drive database.
- [BA] smartd: if DEVICESCAN Directive used in smartd.conf, and
       -I, -R or -r Directives used in conjunction with this, got
       segv errors.  Fixed by correcting memory allocation calls.
- [BA] smartd: enable automatic offline testing was broken due
       to cut-and-paste error that disabled it instead of
       enabling it.  Thanks to Maciej W. Rozycki for pointing
       out the problem and solution.
- [BA] Fixed "spelling" of some Attribute names to replace spaces
       in names by underscores. (Fixed field width easier for awk
       style parsing.)
- [BA] Added mods submitted by Guilhem Frezou to support Attribute 193
       being load/unload cycles. Add -v 193,loadunload option, useful
       for Hitachi drive DK23EA-30, and add this drive to knowndrive.c
       Add meaning of attribute 250 : Read error retry rate
- [BA] Added another entry for Samsung drives to knowndrive table.
- [DG] Refine SCSI log sense command to do a double fetch in most cases
       (but not for the TapeAlert log page). Fix TapeAlert and Self Test
       log pgae response truncation.
- [PW] Added 'removable' argument to -d Directive for smartd.  This indicates
       that smartd should continue (rather than exit) if the device does not 
       appear to be present.
- [BA] Modified smartmontools.spec [Man pages location] and
       smartd.initd [Extra space kills chkconfig!] for Redhat 6.x
       compatibility (thanks to Gerald Schnabel).

* Wed May 7 2003 Bruce Allen <smartmontools-support@lists.sourceforge.net>
- [EB] Add another Fujitsu disk to knowndrives.c
- [GG] match for scsi/ and ide/ in case of devfs to exclude false postives
- [BA] If SCSI device listed in /etc/smartd.conf fails to open or do
       SMART stuff correctly, or not enough space
       to list all SCSI devices, fail with error unless
       -DSCSIDEVELOPMENT set during compile-time.
- [BA] Added automatic recognition of /dev/i* (example: /dev/ide/...)
       as an ATA device.
- [DG] Add "Device type: [disk | tape | medium changer | ...]" line to
       smartctl -i output for SCSI devices.
- [PW] Fixed bug in smartd where test email would be sent regularly (for
       example, daily if the user had specified -M daily) instead of just
       once on startup.
- [KM] More TapeAlert work. Added translations for media changer
       alerts. TapeAlert support reported according to the log page
       presence. ModeSense not attempted for non-ready tapes (all
       drives do not support this after all). Get peripheral type from
       Inquiry even if drive info is not printed. Add QUIETON()
       QUIETOFF() to TapeAlert log check.
- [BA] Stupid bug in atacmds.c minor_str[] affected ataVersionInfo().
       Two missing commas meant that minor_str[] had two few elements,
       leading to output like this:
       Device Model:     Maxtor 6Y120L0
       Serial Number:    Y40BF74E
       Firmware Version: YAR41VW0
       Device is:        Not in smartctl database [for details use: -P showall]
       ATA Version is:   7
       ATA Standard is:  9,minutes
                         ^^^^^^^^^
       Missing commas inserted.
- [BA] Fixed smartd bug.  On device registration, if ATA device did
       not support SMART error or self-test logs but user had asked to
       monitor them, an attempt would be made to read them anyway,
       possibly generating "Drive Seek" errors.  We now check that
       the self-test and error logs are supported before trying to
       access them the first time.
- [GG/BA] Fixed bug where if SMART ATA error log not supported,
       command was tried anyway. Changed some error printing to use
       print handlers.
- [GG] Makefile modifications to ease packaging
- [DG] Did work for TapeAlerts (SCSI). Now can detect /dev/nst0 as a
       SCSI device. Also open SCSI devices O_NONBLOCK so they don't
       hang on open awaiting media. The ATA side should worry about
       this also: during a DEVICESCAN a cd/dvd device without media
       will hang. Added some TapeAlert code suggested by Kai Makisara.

* Mon Apr 21 2003 Bruce Allen <smartmontools-support@lists.sourceforge.net>
- [PW] Extended the -F option/Directive to potentially fix other firmware
       bugs in addition to the Samsung byte-order bug.  Long option name is
       now --firmwarebug and the option/Directive accepts an argument
       indicating the type of firmware bug to fix.
- [BA] Fixed a bug that prevented the enable automatic off-line
       test feature from enabling.  It also prevented the enable Attribute
       autosave from working.  See CVS entry for additional details.
- [PW] Modified the -r/--report option (smartctl and smartd) to allow the
       user to specify the debug level as a positive integer.
- [BA] Added --log directory option to smartctl.  If the disk
       supports the general-purpose logging feature set (ATA-6/7)
       then this option enables the Log Directory to be printed.
       This Log Directory shows which device logs are available, and
       their lengths in sectors.
- [PW] Added -P/--presets option to smartctl and -P Directive to smartd.
- [GG] Introduce different exit codes indicating the type of problem
       encountered for smartd.
- [DG] Add non-medium error count to '-l error' and extended self test
       duration to '-l selftest'. Get scsi IEs and temperature changes
       working in smartd. Step over various scsi disk problems rather
       than abort smartd startup.
- [DG] Support -l error for SCSI disks (and tapes). Output error counter
       log pages.
- [BA] Added -F/--fixbyteorder option to smartctl.  This allows us to read
       SMART data from some disks that have byte-reversed two- and four-
       byte quantities in their SMART data structures.
- [BA] Fixed serious bug: the -v options in smartd.conf were all put
       together and used together, not drive-by-drive.
- [PW] Added knowndrives.h and knowndrives.c.  The knowndrives array
       supersedes the drivewarnings array.
- [GG] add {-p,--pidfile} option to smartd to write a PID file on
       startup. Update the manpage accordingly.
- [DG] Fix scsi smartd problem detecting SMART support. More cleaning
       and fix (and rename) scsiTestUnitReady(). More scsi renaming.
- [BA] Fixed smartd so that if a disk that is explictily listed is not
       found, then smartd will exit with nonzero status BEFORE forking.
       If a disk can't be registered, this will also be detected before
       forking, so that init scripts can react correctly.
- [BA] Replaced all linux-specific ioctl() calls in atacmds.c with
       a generic handler smartcommandhandler().  Now the only routine
       that needs to be implemented for a given OS is os_specific_handler().
       Also implemented the --report ataioctl. This provides 
       two levels of reporting.  Using the option once gives a summary
       report of device IOCTL transactions.  Using the option twice give
       additional info (a printout of ALL device raw 512 byte SMART
       data structures).  This is useful for debugging.
- [DG] more scsi cleanup. Output scsi device serial number (VPD page
       0x80) if available as part of '-i'. Implement '-t offline' as
       default self test (only self test older disks support).
- [BA] Changed crit to info in loglevel of smartd complaint to syslog
       if DEVICESCAN enabled and device not found.
- [BA] Added -v 194,10xCelsius option/Directive. Raw Attribute number
       194 is ten times the disk temperature in Celsius.
- [DG] scsicmds.[hc] + scsiprint.c: clean up indentation, remove tabs.
       Introduce new intermediate interface based on "struct scsi_cmnd_io"
       to isolate SCSI generic commands + responses from Linux details;
       should help port to FreeBSD of SCSI part of smartmontools.
       Make SCSI command builders more parametric.

* Thu Mar 13 2003  Bruce Allen <smartmontools-support@lists.sourceforge.net>
- [BA] smartctl: if HDIO_DRIVE_TASK ioctl() is not implemented (no
       kernel support) then try to assess drive health by examining
       Attribute values/thresholds directly.
- [BA] smartd/smartctl: added -v 200,writeerrorcount option/Directive
       for Fujitsu disks.
- [BA] smartd: Now send email if any of the SMART commands fails,
       or if open()ing the device fails.  This is often noted
       as a common disk failure mode.
- [BA] smartd/smartctl: Added -v N,raw8 -v N,raw16 and -v N,raw48
       Directives/Options for printing Raw Attributes in different
       Formats.
- [BA] smartd: Added -r ID and -R ID for reporting/tracking Raw
       values of Attributes.
- [BA] smartd/smartctl: Changed printing of spin-up-time attribute
       raw value to reflect current/average as per IBM standard.
- [BA] smartd/smartctl: Added -v 9,seconds option for disks which
       use Attribute 9 for power-on lifetime in seconds.
- [BA] smartctl: Added a warning message so that users of some IBM
       disks are warned to update their firmware.  Note: we may want
       to add a command-line flag to disable the warning messages.
       I have done this in a general way, using regexp, so that we
       can add warnings about any type of disk that we wish..

* Wed Feb 12 2003 Bruce Allen <smartmontools-support@lists.sourceforge.net>
- [BA] smartd: Created a subdirectory examplescripts/ of source
       directory that contains executable scripts for the -M exec PATH
       Directive of smartd.
- [BA] smartd: DEVICESCAN in /etc/smartd.conf
       can now be followed by all the same Directives as a regular
       device name like /dev/hda takes.  This allows one to use
       (for example):
       DEVICESCAN -m root@example.com
       in the /etc/smartd.conf file.
- [BA] smartd: Added -c (--checkonce) command-line option. This checks
       all devices once, then exits.  The exit status can be
       used to learn if devices were detected, and if smartd is
       functioning correctly. This is primarily for Distribution
       scripters.
- [BA] smartd: Implemented -M exec Directive for
       smartd.conf.  This makes it possible to run an
       arbitrary script or mailing program with the
       -m option.
- [PW] smartd: Modified -M Directive so that it can be given
       multiple times.  Added -M exec Directive.

* Tue Jan 21 2003 Bruce Allen <smartmontools-support@lists.sourceforge.net>
- [BA] Fixed bug in smartctl pointed out by Pierre Gentile.
       -d scsi didn't work because tryata and tryscsi were 
       reversed -- now works on /devfs SCSI devices.
- [BA] Fixed bug in smartctl pointed out by Gregory Goddard
       <ggoddard@ufl.edu>.  Manual says that bit 6 of return
       value turned on if errors found in smart error log.  But
       this wasn't implemented.
- [BA] Modified printing format for 9,minutes to read
       Xh+Ym not X h + Y m, so that fields are fixed width.
- [BA] Added Attribute 240 "head flying hours"

* Sun Jan 12 2003 Bruce Allen <smartmontools-support@lists.sourceforge.net>
- [BA] As requested, local time/date now printed by smartctl -i

* Thu Jan 9 2003 Bruce Allen <smartmontools-support@lists.sourceforge.net>
- [PW] Added 'help' argument to -v for smartctl
- [PW] Added -D, --showdirectives option to smartd

* Sat Jan 4 2003 Bruce Allen <smartmontools-support@lists.sourceforge.net>
- [DG] add '-l selftest' capability for SCSI devices (update smartctl.8)
- [BA] smartd,smartctl: added additional Attribute modification option
  -v 220,temp and -v 9,temp.
- [PW] Renamed smartd option -X to -d
- [PW] Changed smartd.conf Directives -- see man page
- [BA/DG] Fixed uncommented comment in smartd.conf
- [DG] Correct 'Recommended start stop count' for SCSI devices
- [PW] Replaced smartd.conf directive -C with smartd option -i
- [PW] Changed options for smartctl -- see man page.
- [BA] Use strerror() to generate system call error messages.
- [BA] smartd: fflush() all open streams before fork().
- [BA] smartctl, smartd simplified internal handling of checksums
  for simpler porting and less code.

* Sun Dec 8 2002 Bruce Allen <smartmontools-support@lists.sourceforge.net>
- [PW] smartd --debugmode changed to --debug
- [BA] smartd/smartctl added attribute 230 Head Amplitude from
  IBM DPTA-353750.
- [PW] Added list of proposed new options for smartctl to README.
- [PW] smartd: ParseOpts() now uses getopt_long() if HAVE_GETOPT_LONG is
  defined and uses getopt() otherwise.  This is controlled by CPPFLAGS in
  the Makefile.
- [BA] smartd: Fixed a couple of error messages done with perror()
  to redirect them as needed.
- [BA] smartctl: The -O option to enable an Immediate off-line test
  did not print out the correct time that the test would take to
  complete.  This is because the test timer is volatile and not
  fixed.  This has been fixed, and the smartctl.8 man page has been
  updated to explain how to track the Immediate offline test as it
  progresses, and to further emphasize the differences between the
  off-line immediate test and the self-tests.
- [BA] smartd/smartctl: Added new attribute (200) Multi_Zone_Error_Rate
- [BA] smartctl: modified so that arguments could have either a single -
  as in -ea or multiple ones as in -e -a.  Improved warning message for
  device not opened, and fixed error in redirection of error output of
  HD identity command.
- [PW] smartd: added support for long options.  All short options are still
  supported; see manpage for available long options.
- [BA] smartctl.  When raw Attribute value was 2^31 or larger, did
  not print correctly.

* Fri Nov 22 2002 Bruce Allen <smartmontools-support@lists.sourceforge.net>
- Allen: smartd: added smartd.conf Directives -T and -s.  The -T Directive
  enables/disables Automatic Offline Testing.  The -s Directive
  enables/disables Attribute Autosave. Documentation and
  example configuration file updated to agree.
- Allen: smartd: user can make smartd check the disks at any time
  (ie, interrupt sleep) by sending signal SIGUSR1 to smartd.  This
  can be done for example with:
  kill -USR1 <pid>
  where <pid> is the process ID number of smartd.
- Bolso: scsi: don't trust the data we receive from the drive too
  much. It very well might have errors (like zero response length).
  Seen on Megaraid logical drive, and verified in the driver source.
- Allen: smartd: added Directive -m for sending test email and
  for modifying email reminder behavior.  Updated manual, and sample
  configuration file to illustrate & explain this.
- Allen: smartd: increased size of a continued smartd.conf line to
  1023 characters.
- Allen: Simplified Directive parsers and improved warning/error
  messages.

* Sun Nov 17 2002 Bruce Allen <smartmontools-support@lists.sourceforge.net>
- Fixed bug in smartd where testunitready logic inverted
  prevented functioning on scsi devices.
- Added testunitnotready to smartctl for symmetry with smartd.
- Brabec: added Czech descriptions to .spec file
- Brabec: corrected comment in smartd.conf example
- Changed way that entries in the ATA error log are printed,
  to make it clearer which is the most recent error and
  which is the oldest one.
- Changed Temperature_Centigrade to Temperature_Celsius.
  The term "Centigrade" ceased to exist in 1948.  (c.f
  http://www.bartleby.com/64/C004/016.html).

* Wed Nov 13 2002 Bruce Allen <smartmontools-support@lists.sourceforge.net>
- smartd SCSI devices: can now send warning email message on failure
- Added a new smartd configuration file Directive: -M ADDRESS.
  This sends a single warning email to ADDRESS for failures or
  errors detected with the -c, -L, -l, or -f Directives.

* Mon Nov 11 2002 Bruce Allen <smartmontools-support@lists.sourceforge.net>
- Modified perror() statements in atacmds.c so that printout for SMART
  commands errors is properly suppressed or queued depending upon users
  choices for error reporting modes.
- Added Italian descriptions to smartmontools.spec file.
- Started impementing send-mail-on-error for smartd; not yet enabled.
 
* Sun Nov 10 2002 Bruce Allen <smartmontools-support@lists.sourceforge.net>
- Added -P (Permissive) Directive to smartd.conf file to allow SMART monitoring of
  pre-ATA-3 Rev 4 disks that have SMART but do not have a SMART capability bit.

* Thu Nov 7 2002 Bruce Allen <smartmontools-support@lists.sourceforge.net>
- Added a Man section 5 page for smartd.conf
- Changed Makefile so that the -V option does not reflect file state
  before commit!
- modified .spec file so that locale information now contains
  character set definition.   Changed pt_BR to pt since we do not use any
  aspect other than language.  See man setlocale.
- smartctl: added new options -W, -U, and -P to control if and how the
  smartctl exits if an error is detected in either a SMART data
  structure checksum, or a SMART command returns an error.
- modified manual page to break options into slightly more logical
  categories.
- reformatted 'usage' message order to agree with man page ordering

* Mon Nov 4 2002 Bruce Allen  <smartmontools-support@lists.sourceforge.net>
- smartctl: added new options -n and -N to force device to be ATA or SCSI
- smartctl: no longer dies silently if device path does not start/dev/X
- smartctl: now handles arbitrary device paths
- Added additional macros for manual and sbin paths in this SPEC file.
- Modified Makefile to install /etc/smartd.conf, but without overwriting existing config file
- Modified this specfile to do the same, and to not remove any files that it did not install

* Thu Oct 30 2002 Bruce Allen  <smartmontools-support@lists.sourceforge.net>
- Fixed typesetting error in man page smartd.8
- Removed redundant variable (harmless) from smartd.c

* Wed Oct 29 2002 Bruce Allen  <smartmontools-support@lists.sourceforge.net>
- Added a new directive for the configuration file.  If the word
  DEVICESCAN appears before any non-commented material in the
  configuration file, then the confi file will be ignored and the
  devices wil be scanned.
- Note: it has now been confirmed that the code modifications between
  5.0.23 and 5.0.24 have eliminated the GCC 3.2 problems.  Note that
  there is a GCC bug howerver, see #8404 at
  http://gcc.gnu.org/bugzilla/show_bug.cgi?id=8404
- Added new Directive for Configuration file:
  -C <N> This sets the time in between disk checks to be <N>
  seconds apart.  Note that  although  you  can  give
  this Directive multiple times on different lines of
  the configuration file, only the final  value  that
  is  given  has  an  effect,  and applies to all the
  disks.  The default value of <N> is 1800  sec,  and
  the minimum allowed value is ten seconds.
- Problem wasn't the print format. F.L.W. Meunier <0@pervalidus.net>
  sent me a gcc 3.2 build and I ran it under a debugger.  The
  problem seems to be with passing the very large (2x512+4) byte
  data structures as arguments.  I never liked this anyway; it was
  inherited from smartsuite.  So I've changed all the heavyweight
  functions (ATA ones, anyone) to just passing pointers, not hideous
  kB size structures on the stack.  Hopefully this will now build OK
  under gcc 3.2 with any sensible compilation options.
- Because of reported problems with GCC 3.2 compile, I have gone
  thorough the code and explicitly changed all print format
  parameters to correspond EXACTLY to int unless they have to be
  promoted to long longs.  To quote from the glibc bible: [From
  GLIBC Manual: Since the prototype doesn't specify types for
  optional arguments, in a call to a variadic function the default
  argument promotions are performed on the optional argument
  values. This means the objects of type char or short int (whether
  signed or not) are promoted to either int or unsigned int, as
  required.
- smartd, smartctl now warn if they find an attribute whose ID
  number does not match between Data and Threshold structures.
- Fixed nasty bug which led to wrong number of arguments for a
  varargs statement, with attendent stack corruption.  Sheesh!
  Have added script to CVS attic to help find such nasties in the
  future.

* Tue Oct 29 2002 Bruce Allen  <smartmontools-support@lists.sourceforge.net>
- Eliminated some global variables out of header files and other
  minor cleanup of smartd.
- Did some revision of the man page for smartd and made the usage
  messages for Directives consistent.
- smartd: prints warning message when it gets SIGHUP, saying that it is
  NOT re-reading the config file.
- smartctl: updated man page to say self-test commands -O,x,X,s,S,A
  appear to be supported in the code.  [I can't test these,  can anyone
  report?]
- smartctl: smartctl would previously print the LBA of a self-test
  if it completed, and the LBA was not 0 or 0xff...f However
  according to the specs this is not correct.  According to the
  specs, if the self-test completed without error then LBA is
  undefined.  This version fixes that.  LBA value only printed if
  self-test encountered an error.
- smartd has changed significantly. This is the first CVS checkin of
  code that extends the options available for smartd.  The following
  options can be placed into the /etc/smartd.conf file, and control the
  behavior of smartd.
- Configuration file Directives (following device name):
  -A     Device is an ATA device
  -S     Device is a SCSI device
  -c     Monitor SMART Health Status
  -l     Monitor SMART Error Log for changes
  -L     Monitor SMART Self-Test Log for new errors
  -f     Monitor for failure of any 'Usage' Attributes
  -p     Report changes in 'Prefailure' Attributes
  -u     Report changes in 'Usage' Attributes
  -t     Equivalent to -p and -u Directives
  -a     Equivalent to -c -l -L -f -t Directives
  -i ID  Ignore Attribute ID for -f Directive
  -I ID  Ignore Attribute ID for -p, -u or -t Directive
  #      Comment: text after a hash sign is ignored
  \      Line continuation character
- cleaned up functions used for printing CVS IDs.  Now use string
  library, as it should be.
- modified length of device name string in smartd internal structure
  to accomodate max length device name strings
- removed un-implemented (-e = Email notification) option from
  command line arg list.  We'll put it back on when implemeneted.
- smartd now logs serious (fatal) conditions in its operation at
  loglevel LOG_CRIT rather than LOG_INFO before exiting with error.
- smartd used to open a file descriptor for each SMART enabled
- device, and then keep it open the entire time smartd was running.
  This meant that some commands, like IOREADBLKPART did not work,
  since the fd to the device was open.  smartd now opens the device
  when it needs to read values, then closes it.  Also, if one time
  around it can't open the device, it simply prints a warning
  message but does not give up.  Have eliminated the .fd field from
  data structures -- no longer gets used.
- smartd now opens SCSI devices as well using O_RDONLY rather than
  O_RDWR.  If someone can no longer monitor a SCSI device that used
  to be readable, this may well be the reason why.
- smartd never checked if the number of ata or scsi devices detected
  was greater than the max number it could monitor.  Now it does.

* Fri Oct 25 2002 Bruce Allen  <smartmontools-support@lists.sourceforge.net>
- changes to the Makefile and spec file so that if there are ungzipped manual
  pages in place these will be removed so that the new gzipped man pages are
  visible.
- smartd on startup now looks in the configuration file /etc/smartd.conf for
  a list of devices which to include in its monitoring list.  See man page
  (man smartd) for syntax. If not found, try all ata and ide devices.
- smartd: close file descriptors of SCSI device if not SMART capable
  Closes ALL file descriptors after forking to daemon.
- added new temperature attribute (231, temperature)
- smartd: now open ATA disks using O_RDONLY

* Thu Oct 24 2002 Bruce Allen <smartmontools-support@lists.sourceforge.net>
- smartd now prints the name of a failed or changed attribute into logfile,
  not just ID number
- Changed name of -p (print version) option to -V
- Minor change in philosophy: if a SMART command fails or the device
    appears incapable of a SMART command that the user has asked for,
    complain by printing an error message, but go ahead and try
    anyway.  Since unimplemented SMART commands should just return an
    error but not cause disk problems, this should't cause any
    difficulty.
- Added two new flags: q and Q.  q is quiet mode - only print: For
    the -l option, errors recorded in the SMART error log; For the -L
    option, errors recorded in the device self-test log; For the -c
    SMART "disk failing" status or device attributes (pre-failure or
    usage) which failed either now or in the past; For the -v option
    device attributes (pre-failure or usage) which failed either now
    or in the past.  Q is Very Quiet mode: Print no ouput.  The only
    way to learn about what was found is to use the exit status of
    smartctl.
- smartctl now returns sensible values (bitmask).  See smartctl.h
    for the values, and the man page for documentation.
- The SMART status check now uses the correct ATA call.  If failure
    is detected we search through attributes to list the failed ones.
    If the SMART status check shows GOOD, we then look to see if their
    are any usage attributes or prefail attributes have failed at any
    time.  If so we print them.
- Modified function that prints vendor attributes to say if the
    attribute has currently failed or has ever failed.
- -p option now prints out license info and CVS strings for all
    modules in the code, nicely formatted.
- Previous versions of this code (and Smartsuite) only generate
    SMART failure errors if the value of an attribute is below the
    threshold and the prefailure bit is set.  However the ATA Spec
    (ATA4 <=Rev 4) says that it is a SMART failure if the value of an
    attribute is LESS THAN OR EQUAL to the threshold and the
    prefailure bit is set.  This is now fixed in both smartctl and
    smartd.  Note that this is a troubled subject -- the original
    SFF 8035i specification defining SMART was inconsistent about
    this.  One section says that Attribute==Threshold is pass,
    and another section says it is fail.  However the ATA specs are
    consistent and say Attribute==Threshold is a fail.
- smartd did not print the correct value of any failing SMART attribute.  It
    printed the index in the attribute table, not the attribute
    ID. This is fixed.
- when starting self-tests in captive mode ioctl returns EIO because
    the drive has been busied out.  Detect this and don't return an eror
    in this case.  Check this this is correct (or how to fix it?)
 - fixed possible error in how to determine ATA standard support
    for devices with no ATA minor revision number.
- device opened only in read-only not read-write mode.  Don't need R/W 
    access to get smart data. Check this with Andre.
- smartctl now handles all possible choices of "multiple options"
    gracefully.  It goes through the following phases of operation,
    in order: INFORMATION, ENABLE/DISABLE, DISPLAY DATA, RUN/ABORT TESTS.
    Documentation has bee updated to explain the different phases of
    operation.  Control flow through ataPrintMain()
    simplified.
- If reading device identity information fails, try seeing if the info
    can be accessed using a "DEVICE PACKET" command.  This way we can
    at least get device info.
- Modified Makefile to automatically tag CVS archive on issuance of
    a release
- Modified drive detection so minor device ID code showing ATA-3 rev
    0 (no SMART) is known to not be SMART capable.
- Now verify the checksum of the device ID data structure, and of the
    attributes threshold structure.  Before neither of these
    structures had their checksums verified.
- New behavior vis-a-vis checksums.  If they are wrong, we log
    warning messages to stdout, stderr, and syslog, but carry on
    anyway.  All functions now call a checksumwarning routine if the
    checksum doesn't vanish as it should.
- Changed Read Hard Disk Identity function to get fresh info from
    the disk on each call rather than to use the values that were read
    upon boot-up into the BIOS.  This is the biggest change in this
    release.  The ioctl(device, HDIO_GET_IDENTITY, buf ) call should
    be avoided in such code.  Note that if people get garbled strings
    for the model, serial no and firmware versions of their drives,
    then blame goes here (the BIOS does the byte swapping for you,
    apparently!)
- Function ataSmartSupport now looks at correct bits in drive
    identity structure to verify first that these bits are valid,
    before using them.
- Function ataIsSmartEnabled() written which uses the Drive ID state
    information to tell if SMART is enabled or not.  We'll carry this
    along for the moment without using it.
- Function ataDoesSmartWork() guaranteed to work if the device
    supports SMART.
- Replace some numbers by #define MACROS
- Wrote Function TestTime to return test time associated with each
    different type of test.
- Thinking of the future, have added a new function called
    ataSmartStatus2().  Eventually when I understand how to use the
    TASKFILE API and am sure that this works correctly, it will
    replace ataSmartStatus().  This queries the drive directly to
    see if the SMART status is OK, rather than comparing thresholds to
    attribute values ourselves. But I need to get some drives that fail
    their SMART status to check it.

* Thu Oct 17 2002 Bruce Allen <smartmontools-support@lists.sourceforge.net>
-   Removed extraneous space before some error message printing.
-   Fixed some character buffers that were too short for contents.
    Only used for unrecognized drives, so probably damage was minimal.

* Wed Oct 16 2002 Bruce Allen <smartmontools-support@lists.sourceforge.net>
-   Initial release.  Code is derived from smartsuite, and is
    intended to be compatible with the ATA/ATAPI-5 specifications.
-   For IBM disks whose raw temp data includes three temps. print all
    three
-   print timestamps for error log to msec precision
-   added -m option for Hitachi disks that store power on life in
    minutes
-   added -L option for printing self-test error logs
-   in -l option, now print power on lifetime, so that one can see
    when the error took place
-   updated SMART structure definitions to ATA-5 spec
-   added -p option
-   added -f and -F options to enable/disable autosave threshold
    parameters

