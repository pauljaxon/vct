-- Navigation tracking of missile
--
-- $Log: nav_fpga.ads,v $
-- Revision 1.1.1.1  2004/01/12 19:29:12  adi
-- Added from tarfile
--
--
-- Revision 1.1  2003/09/13 17:11:45  adi
-- Initial revision
--
--

with Measuretypes,Clock,Systemtypes;
use type Clock.Millisecond;
--# inherit if_barometer, if_compass, if_ins, if_airspeed,
--#  measuretypes, systemtypes, cartesian, clock,
--#  measuretypes.angle_ops, measuretypes.angle_ops.trig,
--#  sensor_history, systemtypes.maths, fpga;
package Nav_fpga
  --# own in   Location_State;
  --#     in   sensor_state;
  --#     out  fpga_inputs;
is
   -- Low confidence is single-source, High is multiple-source
   type Confidence is (None,Low,high);

   subtype Meter is Measuretypes.Meter;
   subtype Meter_Per_Sec is Measuretypes.Meter_Per_Sec;
   subtype Angle is Measuretypes.Angle;

   procedure Estimate_Height(M : out Meter;
                             C : out confidence);
   --# global in location_state, sensor_state;
   --# derives m,c from location_state, sensor_state;

   procedure Estimate_Origin_Offset(M : out Meter;
                                    C : out confidence);
   --# global in location_state, sensor_state;
   --# derives m,c from location_state, sensor_state;

   procedure Estimate_Heading(A : out Angle;
                              C : out Confidence);
   --# global in location_state, sensor_state;
   --# derives a,c from location_state, sensor_state;

   procedure Estimate_Speed(S : out Meter_Per_Sec;
                            C : out Confidence);
   --# global in location_state, sensor_state;
   --#        in out clock.time;
   --# derives s,c from location_state, sensor_state, clock.time &
   --#   clock.time from *, sensor_state;

   procedure Maintain(Restart : in Boolean);
   --# global in
   --#    if_barometer.State,
   --#    if_compass.state,
   --#    if_airspeed.state,
   --#    if_ins.state;
   --#  in out clock.time;
   --#  out    fpga_inputs;
   --# derives
   --#  fpga_inputs
   --#  from restart,
   --#       clock.time,
   --#       if_barometer.state,
   --#       if_compass.state,
   --#       if_airspeed.state,
   --#       if_ins.state &
   --#  clock.time
   --#  from *;

   procedure Command;
   --# derives;
end Nav_fpga;
