--/*******************************************************************************
--*
--*
-- *  File:
-- *        tuning.vhd
-- *
-- *
-- *  Module:
-- *        precise_timestamp
-- *
-- *  Author:
-- *        Mario Ruiz
-- *
-- *
-- *  Copyright (C) 2015 - Mario Ruiz and HPCN-UAM High Performance Computing and Networking
-- *
-- *  Licence:
-- *        This file is part of the HPCN-NetFPGA 10G development base package.
-- *
-- *        This file is free code: you can redistribute it and/or modify it under
-- *        the terms of the GNU Lesser General Public License version 2.0 as
-- *        published by the Free Software Foundation.
-- *
-- *        This package is distributed in the hope that it will be useful, but
-- *        WITHOUT ANY WARRANTY; without even the implied warranty of
-- *        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
-- *        Lesser General Public License for more details.
-- *
-- *        You should have received a copy of the GNU Lesser General Public
-- *        License along with the NetFPGA source package.  If not, see
-- *        http://www.gnu.org/licenses/.
-- *
-- */
 
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;



entity tuning is
    generic (
                constant width_counter : integer := 32;
                constant counter_increment : integer :=10 -- 10ns
            );
	port (
			aclk				: 	in 		std_logic;
			aresetn			:	in		std_logic;
			pps				:	in 		std_logic;
			correction		: 	in 		std_logic_vector(31 downto 0);
			timedrift			:	out		std_logic_vector(1 downto 0);
			correction_enable	: 	in 		std_logic
		);

end tuning;

architecture Behavioral of tuning is

signal	counter 		:	std_logic_vector(31 downto 0);
signal	aux			:	std_logic_vector(31 downto 0); 	
signal  ant_pps     :   std_logic:='1';

begin

	process (aclk)
	begin
		if rising_edge (aclk) then
			if (aresetn='0' or rising_edge(pps))	then
				counter <= (others =>'0');
				timedrift <="00";
			elsif (pps='1' and ant_pps='0') then	
			     counter <= (others =>'0');
			else
			
                if correction(31)='1' then	-- verify is negative
                    aux<= not(correction)+1	; -- a2 complement
                else
                    aux<= correction;
                end if;

			counter <=counter+ counter_increment;
			
			if (counter>=aux and correction_enable='1') then
				counter <= (others =>'0');
				if correction(31)='1' then
					timedrift <= "10"; -- slow counter
				else
					timedrift <= "01"; -- fast counter
				end if;
			else
				timedrift <="00";	
			end if;
				

			
			
			end if;
			ant_pps <=pps;
		end if;
			
		

	end process;	



end Behavioral;


