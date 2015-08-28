-- Example using mixed-language simulation
--
-- Here we have a VHDL toplevel that instantiates both SV and VHDL
--  sub entities
library ieee;

use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity vhdl_entity is
    port (
        clk                     : in    std_ulogic;
        reset_n                 : in    std_ulogic;
        stream_out_valid        : out   std_ulogic
    );
end;

architecture impl of vhdl_entity is begin

    stream_out_valid <= reset_n;

end architecture;

