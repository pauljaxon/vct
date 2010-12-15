-- Navigation tracking of missile
-- Version using an FPGA
--
-- $Log: nav_fpga.adb,v $
-- Revision 1.1.1.1  2004/01/12 19:29:12  adi
-- Added from tarfile
--
--
-- Revision 1.1  2003/09/13 17:11:50  adi
-- Initial revision
--
--

with
  Fpga,
  if_barometer,
  if_compass,
  If_Ins,
  If_airspeed,
  Measuretypes.Angle_Ops,
  Measuretypes.Angle_Ops.Trig,
  Sensor_History,
  Systemtypes, Systemtypes.Maths,
  cartesian;
package body Nav_FPGA
  --# own Location_State is
  --#            in head_xy, in head_yz, in dx,
  --#            in dy, in dz, in air_speed &
  --#     fpga_inputs is
  --#            out time_now, out is_restart,
  --#            out airspeed_speed,   out airspeed_valid,
  --#            out barometer_height, out barometer_valid,
  --#            out compass_xy, out compass_yz, out compass_valid,
  --#            out ins_x, out ins_y, out ins_z, out ins_valid
  --#  ;
-- sensor_state has no refinement as it's just an array
is
   subtype Integer32 is Systemtypes.Integer32;

   type Sensor_Status is (Unknown, Valid, Failed, Restarted);
   for Sensor_Status'Size use 2;
   for Sensor_Status use
      (Unknown => 0, Valid => 1, Failed => 2, Restarted => 3);

   -- How big are various types?
   Dist_Record_Bytes : constant :=
     (Sensor_History.Dist_History'Size + 7)/8;  -- = 185/8 = 23 bytes
   Angle_Record_Bytes : constant :=
     (Sensor_History.Angle_History'Size + 7)/8; -- = 141/8 = 17 bytes
   Speed_Record_Bytes : constant :=
     (Sensor_History.Speed_History'Size + 7)/8; -- = 161/8 = 20 bytes

   -- The estimates are all output by the FPGA

   -- LOCATION_STATE
   Dx, Dy, Dz       : Sensor_History.Dist_History;
   for Dx'Address use Fpga.Base_Out_Address;
   for Dy'Address use Fpga.Base_Out_Address + Dist_Record_Bytes*1;
   for Dz'Address use Fpga.Base_Out_Address + Dist_Record_Bytes*2;

   Head_Xy, Head_Yz : Sensor_History.Angle_History;
   for Head_Xy'Address use
      Fpga.Base_Out_Address + Dist_Record_Bytes*3;
   for Head_Yz'Address use
      Fpga.Base_Out_Address + (Dist_Record_Bytes*3 + Angle_Record_Bytes);

   Air_Speed         : Sensor_History.Speed_History;
   for Air_Speed'Address use
      Fpga.Base_Out_Address + (Dist_Record_Bytes*3 + Angle_Record_Bytes*2);

   -- As are the sensor statuses
   Sensor_State_Base : constant := (Fpga.Base_Out_Address + 4) +
     (Dist_Record_Bytes * 3 + (Angle_Record_Bytes * 2 + Speed_Record_Bytes));
   -- about 123 bytes plus 1 word for safety

   type Sensors is (Airspeed, Barometer, Compass, Ins);
   for Sensors'Size use 2;

   -- Size is 4 x 2 = 8 bits
   type Sensor_State_Array is array(Sensors) of Sensor_Status;
   pragma Pack(Sensor_State_Array);
   for Sensor_State_Array'Size use 8;

   Sensor_State : Sensor_State_Array;
   for Sensor_State'Address use Sensor_State_Base;

   -- Now FPGA inputs
   Airspeed_Speed : Meter_Per_Sec;
   for Airspeed_Speed'Address use
      Base_In_Address;
   Airspeed_Valid : Boolean;
   for Airspeed_Valid'Address use
      Base_In_Address + 4;

   Barometer_Height : Meter;
   for Barometer_Height'Address use
      Base_In_Address + 5;
   Barometer_Valid : Boolean;
   for Barometer_Valid'Address use
      Base_In_Address + 9;

   Compass_Xy, Compass_yz : Angle;
   for Compass_Xy'Address use
      Base_In_Address + 10;
   for Compass_Yz'Address use
      Base_In_Address + 11;
   Compass_Valid : Boolean;
   for Compass_Valid'Address use
      Base_In_Address + 12;

   Ins_X, Ins_Y, Ins_Z : Meter;
   for Ins_X'Address use
      Base_In_Address + 13;
   for Ins_Y'Address use
      Base_In_Address + 17;
   for Ins_Z'Address use
      Base_In_Address + 21;
   Ins_Valid : Boolean;
   for Ins_Valid'Address use
      Base_In_Address + 25;

   Time_Now : Clock.Millisecond;
   for Time_Now'Address use Base_In_Address + 26;

   Is_Restart : Boolean;
   for Is_Restart'Address use Base_In_Address + 30;



   ------------------ Public subroutines  --------------------

   procedure Get_Recent_Meter(Item   : in Sensor_History.Dist_History;
                              Recent : out Meter;
                              Timestamp : out Clock.Millisecond)
     --# derives recent,timestamp from item;
   is
      last_Idx : Sensor_History.History_Count;
   begin
      last_Idx := Sensor_History.Previous_Item(Item.New_Idx);
      Recent := Item.Distance(Last_Idx);
      Timestamp := Item.Times(Last_Idx);
   end Get_Recent_Meter;

   procedure Get_Recent_angle(Item   : in Sensor_History.angle_History;
                              Recent : out angle;
                              Timestamp : out Clock.Millisecond)
     --# derives recent,timestamp from item;
   is
      last_Idx : Sensor_History.History_Count;
   begin
      last_Idx := Sensor_History.Previous_Item(Item.New_Idx);
      Recent := Item.bearing(Last_Idx);
      Timestamp := Item.Times(Last_Idx);
   end Get_Recent_angle;

   procedure Get_Recent_speed(Item   : in Sensor_History.speed_History;
                              Recent : out Meter_Per_sec;
                              Timestamp : out Clock.Millisecond)
     --# derives recent,timestamp from item;
   is
      last_Idx : Sensor_History.History_Count;
   begin
      last_Idx := Sensor_History.Previous_Item(Item.New_Idx);
      Recent := Item.speed(Last_Idx);
      Timestamp := Item.Times(Last_Idx);
   end Get_Recent_speed;

   procedure Estimate_Height(M : out Meter;
                             C : out confidence)
   --# global in dz, sensor_state;
   --# derives m,c from sensor_state, dz;
   is
      T : Clock.Millisecond;
      Baro_State, Ins_state : Sensor_Status;
      Tmp_dz : Sensor_History.Dist_History;
   begin
      Baro_State := Sensor_State(Barometer); -- invalid rep OK
      Tmp_Dz := Dz; -- invalid rep ok
      case Baro_state is
         when Unknown | Failed | Restarted =>
            -- Try a backup
            Ins_State := Sensor_State(Ins); -- invalid rep ok
            if Ins_state = Valid then
               -- Secondary sensor valid
               Get_Recent_Meter(Item      => Tmp_Dz,
                                Recent    => M,
                                Timestamp => T);
               if (T = 0) then
                  -- Invalid reading
                  C := None;
               else
                  C := Low;
               end if;
            else
               M := 0;
               C := None;
            end if;
         when Valid =>
            Get_Recent_Meter(Item      => Tmp_Dz,
                             Recent    => M,
                             Timestamp => T);
            -- Primary sensor valid
            if T = 0 then
               -- invalid reading
               C := None;
            else
               C := High;
            end if;
      end case;
   end Estimate_Height;

   procedure Estimate_Origin_Offset(M : out Meter;
                                    C : out confidence)
     --# global in dx, dy, sensor_state;
     --# derives m,c from dx, dy, sensor_state;
   is
      Edx, Edy : Meter;
      I_tmp : Integer32;
      Tmp_Dx, Tmp_Dy : Sensor_History.Dist_History;
      T1, T2 : Clock.Millisecond;
      Ins_State,Compass_State,Airspeed_state : Sensor_Status;
   begin
      Tmp_Dx := Dx; -- invalid rep OK
      Tmp_Dy := Dy; -- invalid rep OK
      Ins_State := Sensor_State(Ins);  -- invalid rep OK
      case Ins_state is
         when Unknown | Failed | Restarted =>
            Compass_State := Sensor_State(Compass); -- invalid rep OK
            Airspeed_State := Sensor_State(Airspeed); -- invalid rep OK
            if Compass_State = Valid and Airspeed_State = Valid then
               -- Add estimation here eventually
               M := 0;
               C := none;
            else
               -- Can't estimate
               M := 0;
               C := none;
            end if;
         when Valid =>
            Get_Recent_Meter(Item => Tmp_Dx,
                             Recent => edx,
                             Timestamp => T1);
            Get_Recent_Meter(Item => Tmp_Dy,
                             Recent => edy,
                             Timestamp => T2);
            -- Primary sensor valid
            if T1 = 0 or T2 = 0 then
               -- invalid reading
               M := 0;
               C := None;
            else
               I_Tmp := (Integer32(Edx) * Integer32(Edx)) +
                 (Integer32(Edy) * Integer32(Edy));
               I_Tmp := Systemtypes.Maths.Sqrt(I_tmp);
               M := Meter(I_Tmp);
               C := High;
            end if;
      end case;
   end Estimate_Origin_Offset;

   procedure Estimate_Heading(A : out Angle;
                              C : out Confidence)
   --# global in dx, dy, head_xy, sensor_state;
   --# derives a,c from dx, dy, head_xy, sensor_state;
   is
      T1, T2 : Clock.Millisecond;
      Edx,Edy : Meter;
      Tmp_Dx, Tmp_Dy : Sensor_History.Dist_History;
      Tmp_Head_xy : Sensor_History.Angle_History;
      Compass_State, Ins_state : Sensor_Status;
   begin
      Tmp_Dx := Dx; -- invalid rep OK
      Tmp_Dy := Dy; -- invalid rep OK
      Tmp_Head_Xy := Head_Xy;  -- invalid rep OK
      Compass_State := Sensor_State(Compass); -- invalid rep OK
      case Compass_state is
         when Unknown | Failed | Restarted =>
            Ins_State := Sensor_State(Ins); -- invalid rep OK
            if Ins_State = Valid then
               -- Read the dx and dy given by Ins
               Get_Recent_Meter(Item => Tmp_Dx,
                                Recent => Edx,
                                Timestamp => T1);
               Get_Recent_Meter(Item => Tmp_Dy,
                                Recent => Edy,
                                Timestamp => T2);
               if T1 = 0 or T2 = 0 then
                  -- invalid
                  A := 0;
                  C := None;
               else
                  A := Measuretypes.Angle_Ops.Trig.Arctan2(X => eDx,
                                                           Y => eDy);
                  C := Low;
               end if;
            else
               A := 0;
               C := None;
            end if;
         when Valid =>
            Get_Recent_Angle(Item => Tmp_Head_Xy,
                             Recent => A,
                             Timestamp => T1);
            if T1 = 0 then
               C := None;
            else
               C := High;
            end if;
      end case;
   end Estimate_Heading;

   procedure Estimate_Speed(S : out Meter_Per_Sec;
                            C : out Confidence)
     --# global in dx, dy, air_speed, sensor_state;
     --#   in out clock.time;
     --# derives s,c from dx, dy, air_speed, sensor_state,
     --#     clock.time &
     --#  clock.time from *, sensor_state;
   is
      T1 : Clock.Millisecond;
      I_Tmp : Integer32;
      M : Meter;
      C_Tmp : Confidence;
      Time_Valid : Boolean;
      Airspeed_State : Sensor_Status;
      Tmp_Air_Speed : Sensor_History.Speed_History;
   begin
      Tmp_Air_Speed := Air_Speed; -- invalid rep OK
      Airspeed_State := Sensor_State(Airspeed); -- invalid rep OK
      case airspeed_State is
         when Unknown | Failed | Restarted =>
            Estimate_Origin_Offset(M => m,
                                   C => C_Tmp);
            Clock.Read(T => T1,
                       Valid => Time_Valid);
            if Time_Valid and C_tmp /= None then
               I_Tmp := Integer32(M);
               -- Estimate speed by distance over time
               I_Tmp := (I_Tmp * 1_000) / Integer32(T1);
               S := Meter_Per_Sec(I_Tmp);
               C := Low;
            else
               s := 0;
               C := None;
            end if;
         when Valid =>
            Get_Recent_speed(Item => Tmp_Air_Speed,
                             Recent => s,
                             Timestamp => T1);
            if T1 = 0 then
               C := None;
            else
               C := High;
            end if;
      end case;
   end Estimate_Speed;


   procedure Maintain(Restart : in Boolean)
     --# global
     --#  in
     --#    if_barometer.State,
     --#    if_compass.state,
     --#    if_airspeed.state,
     --#    if_ins.state;
     --#  out
     --#    time_now, is_restart,
     --#    airspeed_speed, airspeed_valid,
     --#    compass_xy, compass_yz, compass_valid,
     --#    ins_x, ins_y, ins_z, ins_valid,
     --#    barometer_height, barometer_valid;
     --#  in out
     --#    clock.time;
     --# derives
     --#  barometer_height, barometer_valid
     --#   from if_barometer.state &
     --#  airspeed_speed, airspeed_valid
     --#   from if_airspeed.state &
     --#  compass_xy, compass_yz, compass_valid
     --#   from if_compass.state &
     --#  ins_x, ins_y, ins_z, ins_valid
     --#   from if_ins.state &
     --#  is_restart from restart &
     --#  time_now from clock.time &
     --#  clock.time from
     --#     *;
   is
      P : Cartesian.Position;
      D : Meter;
      S : Meter_Per_Sec;
      R : Measuretypes.Millirad;
      V1,V2 : Boolean;
      T : Clock.Millisecond;
   begin
      -- Get airspeed
      If_Airspeed.Get_Speed(Speed => S,
                            Valid => V1);
      Airspeed_Speed := S;
      Airspeed_Valid := V1;
      -- Get height
      If_Barometer.Get_Height(Height => D,
                              Valid  => V1);
      Barometer_Height := D;
      Barometer_Valid := V1;
      -- Get headings
      If_Compass.Get_Xy(Angle => r,
                        Valid => V1);
      Compass_Xy := Measuretypes.Angle_Ops.Round_Degree(R);
      If_Compass.Get_Yz(Angle => r,
                        Valid => V2);
      Compass_Yz := Measuretypes.Angle_Ops.Round_Degree(R);
      Compass_Valid := V1 and V2;
      -- INS
      If_Ins.Get_Location(Position => P,
                          Valid    => V1);
      Ins_X := P.X;
      Ins_Y := P.Y;
      Ins_Z := P.z;
      Ins_Valid := V1;
      -- Get time
      Clock.Read(T => T,
                 Valid => V1);
      if V1 then
         Time_Now := T;
      else
         Time_Now := 0;
      end if;
      -- Restarting?
      Is_Restart := Restart;
   end Maintain;

   procedure Command is separate;
end Nav_fpga;
