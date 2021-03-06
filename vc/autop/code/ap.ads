--# inherit Surfaces, Instruments, Degrees, Scale;
package AP
  --# own State;
  --# initializes State;
is
   procedure Control;
   --# global in out State;
   --#           out Surfaces.Elevators,
   --#               Surfaces.Ailerons,
   --#               Surfaces.Rudder;
   --#        in     Instruments.Altitude,
   --#               Instruments.Bank,
   --#               Instruments.Heading,
   --#               Instruments.Heading_Bug,
   --#               Instruments.Mach,
   --#               Instruments.Pitch,
   --#               Instruments.Rate_Of_Climb,
   --#               Instruments.Slip;
   --# derives State
   --#          from *,
   --#               Instruments.Altitude,
   --#               Instruments.Bank,
   --#               Instruments.Pitch,
   --#               Instruments.Slip &
   --#  Surfaces.Elevators
   --#          from State,
   --#               Instruments.Altitude,
   --#               Instruments.Mach,
   --#               Instruments.Pitch,
   --#               Instruments.Rate_Of_Climb &
   --#  Surfaces.Ailerons
   --#          from State,
   --#               Instruments.Bank,
   --#               Instruments.Heading,
   --#               Instruments.Heading_Bug,
   --#               Instruments.Mach &
   --#  Surfaces.Rudder
   --#          from State,
   --#               Instruments.Mach,
   --#               Instruments.Slip
   --#  ;
end AP;
