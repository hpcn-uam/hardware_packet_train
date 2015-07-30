--/*******************************************************************************
-- *
-- *
-- *  File:
-- *        counter.vhd
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


entity counter is
    generic (
            constant width_counter : integer := 32;
            constant max_count : std_logic_vector := x"3B9ACA00"; -- 1.000.000.000 ns
            constant high_time_pps : std_logic_vector := x"05F5E100"; -- 100.000.000 ns
            constant counter_increment : integer :=10 -- 10ns
        );
 	port(
		aclk			: 	in 	std_logic;
		aresetn		: 	in 	std_logic;
		timedrift		:	in 	std_logic_vector (1 downto 0);
		pps			:	in	std_logic;
		pps_internal	:	out	std_logic;		
		

		second 		:	out	std_logic_vector (width_counter-1 downto 0);
		nanosecond	:	out	std_logic_vector (width_counter-1 downto 0);

		mode			:	in	std_logic;

		updatesec		:	in 	std_logic_vector (width_counter-1 downto 0)
	
		
		);
end counter;

-- Mode
-- 0 Stop and load second
-- 1 Start Counting


architecture Behavioral of counter is

	signal	nsec_count,sec_count	:	std_logic_vector (width_counter-1 downto 0);
	type state_t is (load, waiting, counting);
	signal state : state_t := load;
	signal next_state : state_t;
	signal pps_ant :   std_logic :='1';

begin
	
	process (aclk)
	begin
		
		if rising_edge(aclk) then
			
			if (aresetn='0') then
				nsec_count <= (others => '0');
				sec_count  <= (others => '0');
				pps_internal <= '0';
				next_state <= load;
			else 
				case (state) is	
					when load =>					-- load second
						if (mode='0') then
							sec_count <=updatesec;
							nsec_count <= (others =>'0');
							next_state <= load;
						else 
							next_state <= waiting;
						end if;
					when waiting =>				-- wait for pps signal
						if (pps_ant='0' and pps='1') then 	-- when rising_edge pps increment second and start normal counting
							sec_count <=sec_count+1;
							next_state <= counting;
						elsif (mode = '0') then
						    next_state <= load;  	
						else
							next_state <= waiting;
						end if;	
						pps_ant <=pps;
					when counting =>				-- normal counting	
						case (timedrift) is		-- check that correction is available
							when "01" => 	 -- advance counting
								nsec_count <= nsec_count + counter_increment+1; -- nsec_count + 11 
							when "10" => 
								nsec_count <= nsec_count + counter_increment-1; -- nsec_count + 9
							when others => -- normal counting
								nsec_count <= nsec_count + counter_increment; -- nsec_count + 10
						end case;
						
						if (nsec_count > max_count-10) then 		--
							sec_count <= sec_count+1;
							nsec_count <= (others =>'0');
							pps_internal <='1';
						elsif (nsec_count > high_time_pps) then
							pps_internal <='0';	
						end if;	
						
						if (mode = '0') then
                            next_state <= load;
                        else     
                            next_state <= counting;                                                     	
						end if;
						
				end case;
			end if;	
					
		end if;
		
    end process;

	state <= next_state;
	
	second <= sec_count;
	nanosecond <= nsec_count;
	

end Behavioral;




