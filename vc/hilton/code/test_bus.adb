-- Test harness for 1553 bus simulator
-- Not really SPARK, just looks like it.
-- $Id: test_bus.adb,v 1.1.1.1 2004/01/12 19:29:12 adi Exp $
--
-- $Log: test_bus.adb,v $
-- Revision 1.1.1.1  2004/01/12 19:29:12  adi
-- Added from tarfile
--
--
-- Revision 1.3  2003/09/13 17:43:05  adi
-- Corrected to compile with new harness
--
-- Revision 1.2  2003/02/09 19:49:20  adi
-- Fixed for changed BC,RT params
--
-- Revision 1.1  2003/02/03 23:23:27  adi
-- Initial revision
--
--
with Bus;
with Rt1553,Bc1553;
with SystemTypes;
with Test,Test.checking;
use type SystemTypes.Unsigned32;
--# inherit bus, rt1553, bc1553, test;
--# main_program
procedure Test_Bus
  --# global Bus.Inputs, Bus.Outputs, Test.State;
  --# derives Bus.Inputs  from *, Bus.Outputs &
  --#         Bus.Outputs from *, Bus.Inputs &
  --#         Test.State from *, Bus.Inputs, Bus.Outputs
  --#  ;
is
   Msg : Bus.Message;
   V,W : Bus.Word;
   I : Bus.Word_Index;
begin
   -- Check for data being null
   Test.Section("BC inputs are initially null");
   for Lru in Bc1553.Lru_Name loop
      Bc1553.Read_Message(Src            => Lru,
                          Subaddress_Idx => 1,
                          Data           => Msg);
      Test.Checking.bool(S => Bc1553.Lru_Name'Image(Lru) & " is stale",
                         Expected => False,
                         Actual   => Msg.Fresh);
   end loop;
   Test.Section("RT inputs are initially null");
   for Lru in Rt1553.Lru_Name loop
      Rt1553.Read_Message(Src => Lru,
                          Subaddress_Idx => 1,
                          Data => Msg);
      Test.Checking.bool(S => rt1553.Lru_Name'Image(Lru) & " is stale",
                         Expected => False,
                         Actual   => Msg.Fresh);
   end loop;
   -- Get the BC to write out some data to each LRU
   Test.Section("RT inputs are nul after write, before cycle");
   W := 1;
   I := 1;
   for Lru in Bc1553.Lru_Name loop
      Bc1553.Write_Word(Data => W,
                        Idx =>  I,
                        Subaddress_Idx => 1,
                        Dest => Lru);
      Test.Checking.bool(S => bc1553.Lru_Name'Image(Lru) & " is still stale",
                         Expected => false,
                         Actual   => Msg.fresh);
      W := W + 3;
   end loop;
   -- Get each LRU to write out some data to the BC
   Test.Section("BC inputs are nul after write, before cycle");
   W := 3;
   I := 1;
   for Lru in Rt1553.Lru_Name loop
      Rt1553.Write_Word(Data => W,
                        Idx => I,
                        Subaddress_Idx => 1,
                        Src => Lru);
      Test.Checking.bool(S => rt1553.Lru_Name'Image(Lru) & " is stale",
                         Expected => False,
                         Actual => Msg.Fresh);
      W := W + 3;
   end loop;
   -- Now cycle and check the RT inputs
   Bus.Cycle;
   Test.Section("RT inputs are valid after cycle");
   W := 1;
   I := 1;
   for Lru in rt1553.Lru_Name loop
      Rt1553.Read_Message(Src => Lru,
                          Subaddress_Idx => 1,
                          Data => Msg);
      Test.Checking.bool(S => Rt1553.Lru_Name'Image(Lru) & " is fresh",
                         Expected => True,
                         Actual   => Msg.Fresh);
      rt1553.read_Word(Src => Lru,
                       Data => V,
                       Idx =>  I,
                       Subaddress_Idx => 1);
      Test.Checking.unsigned16(S => rt1553.Lru_Name'Image(Lru) & " is " &
                               Bus.Word'Image(W),
                               Expected => W,
                               Actual   => W);
      -- Acknowledge reading this message
      Rt1553.Acknowledge_Message(Src => Lru,
                                 Subaddress_Idx => 1);
      Rt1553.Read_Message(Src => Lru,
                          Subaddress_Idx => 1,
                          Data => Msg);
      Test.Checking.bool(S => Rt1553.Lru_Name'Image(Lru) &
                         " not fresh after ack",
                         Expected => False,
                         Actual   => Msg.Fresh);
      W := W + 3;
   end loop;
   -- Now check the BC inputs
   Test.Section("BC inputs are valid after cycle");
   W := 3;
   I := 1;
   for Lru in bc1553.Lru_Name loop
      bc1553.Read_Message(Src => Lru,
                          Subaddress_Idx => 1,
                          Data => Msg);
      Test.Checking.bool(S => bc1553.Lru_Name'Image(Lru) & " is fresh",
                         Expected => True,
                         Actual => Msg.Fresh);
      bc1553.Read_Word(Src => Lru,
                       Data => V,
                       Idx =>  I,
                       Subaddress_Idx => 1);
      Test.Checking.unsigned16(S => bc1553.Lru_Name'Image(Lru) & " is " &
                               Bus.Word'Image(W),
                               Expected => W,
                               Actual => v);
      -- Acknowledge reading this message
      bc1553.Acknowledge_Message(Src => Lru,
                                 Subaddress_Idx => 1);
      bc1553.Read_Message(Src => Lru,
                          Subaddress_Idx => 1,
                          Data => Msg);
      Test.Checking.bool(S => bc1553.Lru_Name'Image(Lru) &
                         " not fresh after ack",
                         Expected => False,
                         Actual => Msg.Fresh);
      W := W + 3;
   end loop;

   Test.Done;
end Test_Bus;
