program mean;

var n, k, i, seed : integer;
sum
: real;

begin
  (*writeString("Give n: ");*)
  n := 0(*readInteger()*);
  (*writeString("Give k: ");*)
  k := 0(*readInteger()*);

  i := 0;
  sum := 0.0;
  seed := 65;
  while i < k do
  begin
    seed := (seed * 137 + 221 + i) mod n;
    sum := sum + seed;
    i := i + 1
  end;
  if k > 0 then
  begin
    (*writeString("Mean: ");
    writeReal(sum / k);
    writeString("\n")*)
  end
end.
