# Basic commands
SPARK=spark
SPARKSIMP=sparksimp
POGS=pogs
GNAT=gnatmake
TAR=tar zcf
CP=/bin/cp
CPR=$(CP) -r
RM=/bin/rm -rf
ADAPOD=$(TOOLS)/adapod.exe
P2H=pod2html
GREP=/usr/bin/grep
QUIETGREP=$(GREP) --quiet

# Files
PRVTAR=proof_file.tar
REPFILE=spark.rep
SPARKFILES=missile.idx missile.wrn spark.sw gnat.cfg
HARNESS=./test_harness
BUSTEST=./test_bus

# Directories
TOOLS=../Tools
MANDIR=../Manual

# Specific lists of files and directories
# What pages to generate for the manual
MANPAGES=\
  logging.html \
  airspeed.html    barometer.html\
  bc1553.html      bit_machine.html\
  bus.html         bus_decode.html\
  bus_encode.html  clock.html\
  clock_utils.html compass.html\
  destruct.html    destruct_cfg.html\
  fuel.html        fuze.html\
  ibit.html        if_airspeed.html\
  if_barometer.html if_compass.html\
  if_destruct.html  if_fuel.html\
  if_fuze.html     if_ins.html\
  if_ir.html       if_motor.html\
  if_radar.html    if_steer.html\
  if_warhead.html  ins.html\
  ir.html          ir_cfg.html\
  measuretypes.html  missile.html\
  motor.html       nav.html\
  radar.html       radar_cfg.html\
  random.html      rt1553.html\
  sensor_history.html  state_types.html\
  steer.html       steer_cfg.html\
  warhead.html     warhead_cfg.html\
  watchdog.html

# Directories in which simplified files exist
SIMPDIRS=\
  airspeed\
  barometer\
  bc1553\
  bit_machine\
  bus\
  bus_decode\
  bus_encode\
  clock\
  clock_utils\
  compass\
  destruct\
  destruct_cfg\
  drag\
  environment\
  flying_object\
  fuel\
  fuze\
  ibit\
  if_airspeed\
  if_barometer\
  if_compass\
  if_destruct\
  if_fuel\
  if_fuze\
  if_ins\
  if_ir\
  if_motor\
  if_radar\
  if_steer\
  if_warhead\
  ins\
  ir\
  ir_cfg\
  measuretypes_\
  missile\
  motor\
  nav\
  radar\
  radar_cfg\
  random\
  rt1553\
  sensor_history\
  state_types\
  steer\
  steer_cfg_\
  warhead\
  warhead_cfg\
  watchdog

# Directory of web pages
WEBDIR=../../Suslik/Simulator

# The main targets

harness : $(HARNESS)

spark : $(REPFILE)

.SUFFIXES : .out .pod .html

%.out : %.in
	$(HARNESS) < $< > $@

%.pod : %.ads
	$(ADAPOD) < $< > $@

%.html : %.pod
	$(P2H) --infile=$< --outfile=$(MANDIR)/$@

clean :
	rm -f *.o *.ali *.tmp *.[Ll][sS][bBsS] *.pod $(REPFILE) $(HARNESS)

docs : $(MANPAGES)

simp : Code.sum

simpclean :
	find . -name '*.siv' | xargs rm -f
	find . -name '*.slg' | xargs rm -f

vcgclean : simpclean
	$(RM) $(SIMPDIRS)

# More detailed targets

$(PRVTAR) : $(REPFILE)
	tar cf $(PRVTAR) `find . -name '*.prv'`

$(HARNESS) : *.ad[sb]
	$(GNAT) test_harness.adb

$(TESTBUS) : *.ad[sb]
	$(GNAT) test_bus.adb

$(REPFILE) : *.a[dp][sb] *.smf $(SPARKFILES)
	$(SPARK) @missile

Code.sum : spark.rep
	$(SPARKSIMP)
	$(POGS)
