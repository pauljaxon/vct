with Degrees,Instruments;
use type Degrees.Degreespersec;
use type Instruments.Pitchangle;

package body AP.Altitude.Pitch.Rate
is
   SAMPLE_RATE : constant Integer := 20;  -- 20 samples/sec 
   
   subtype History_Range is Integer range 1..10;
   type History_List is array(History_Range) of Instruments.Pitchangle;
   
   Pitch_History : History_List 
        := History_List'(others => 0);
   
   function History_Average(H : History_List)
			   return Instruments.Pitchangle
   is
      Sum : Integer := 0;
      Gap : Integer;
      Average : Instruments.Pitchangle;
   begin
      for I in History_Range 
	--# assert Sum <= (I * Integer(Instruments.Pitchangle'Last)) and
	--#     Sum >= (I * Integer(Instruments.Pitchangle'First));
      loop
	 Sum := Sum + Integer(H(I));
      end loop;
      -- Get the average pitch over this range
      Gap := (2 + History_Range'Last) - History_Range'First;
      Average := Instruments.Pitchangle(Sum / Gap);
     return Average;
   end History_Average;
   
   procedure History_Update(H : in out History_List;
		            V : in Instruments.Pitchangle)
     --# derives H from *,V;
   is
   begin
     for I in History_Range loop
	if I < History_Range'Last then
	   H(I) := H(I+1);
	else
	   H(I) := V;
	end if;
     end loop;
   end History_Update;
   
   procedure Calc_Pitchrate(Pitch             : in  Instruments.Pitchangle;
			    Present_Pitchrate : out Degrees.Degreespersec)
   is
      Average : Instruments.Pitchangle;
      Early_Pitch : Instruments.Pitchangle;
   begin      
     -- Get the average pitch over this range
     Average := History_Average(Pitch_History);
     Early_Pitch := Pitch_History(History_Range'First);
     if (Early_Pitch <= Average and Pitch >= Average) then
   	-- Consistent with increasing pitch
	Present_Pitchrate := Degrees.Degreespersec( 
	  ( Integer(Pitch - Early_Pitch) * 
	  Integer((1 + History_Range'Last) - History_Range'First) )
	  / SAMPLE_RATE );
     elsif (Early_Pitch >= Average and Pitch <= Average) then
   	-- Consistent with decreasing pitch
	Present_Pitchrate := Degrees.Degreespersec( 
	  ( Integer(Pitch - Early_Pitch) * 
	  Integer((1 + History_Range'Last) - History_Range'First) )
	  / SAMPLE_RATE );
     else 
        Present_Pitchrate := 0;
     end if;
     -- Update history with a smoothed value
     History_Update(Pitch_History,(Pitch + Average)/2);
   end Calc_Pitchrate;
   
end AP.Altitude.Pitch.Rate;
