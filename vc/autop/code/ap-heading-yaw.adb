with Instruments,Surfaces,Degrees,Scale;

use type Instruments.Slipangle;

with AP.Heading.Yaw.Rate;

use type Degrees.Degreespersec;
use type Surfaces.Controlangle;

package body AP.Heading.Yaw
  --# own State is AP.Heading.Yaw.Rate.Yaw_History;
is
   procedure Calc_Yawrate(Yaw             : in Instruments.Slipangle;
			  Present_Yawrate : out Degrees.Degreespersec)
     renames Rate.Calc_Yawrate;
   
   subtype Degreespersec is Degrees.Degreespersec;
   
   function Calc_Rudder_Move(Present_Sliprate : Degreespersec;
			     Mach             : Instruments.Machnumber)
     return Surfaces.Controlangle
   is
      Target_Angle : Surfaces.ControlAngle;
   begin
      Target_Angle := Scale.Scale_Movement(
	Mach    => Mach,
	Present => Scale.Scaledata(Present_Sliprate / 2),
	Target  => 0,
        Max     => Surfaces.ControlAngle(20)
      );
      return Target_Angle;
   end Calc_Rudder_Move;
   
   procedure Yaw_AP(Mach     : in Instruments.Machnumber;
		    Slip     : in Instruments.Slipangle)
   --# global in out Rate.Yaw_History;
   --#           out Surfaces.Rudder;
   --# derives Rate.Yaw_History
   --#         from *,
   --#              Slip &
   --#         Surfaces.Rudder
   --#         from Rate.Yaw_History,
   --#              Mach,
   --#              Slip
   --#  ;
   is
      Present_Sliprate : Degreespersec;
      Rudder_Movement  : Surfaces.Controlangle;
   begin
      Calc_Yawrate(Slip,Present_Sliprate);
      Rudder_Movement := Calc_Rudder_Move(Present_Sliprate,Mach);
      Surfaces.Move_Rudder(Rudder_Movement);
   end Yaw_AP;
   
end AP.Heading.Yaw;

   
