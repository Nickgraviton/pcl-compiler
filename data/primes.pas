program primes;
var limit, number, counter : integer;
function prime (n : integer) : boolean;
var i : integer;
begin
  if n < 0 then
    result := prime(-n)
  else if n < 2 then
    result := false
  else if n = 2 then
    result := true
  else if n mod 2 = 0 then
    result := false
  else
  begin
    i := 3;
    while i <= n div 2 do
    begin
      if n mod i = 0 then
      begin
        result := false;
        return
      end;
      i := i+2
    end
  end;
  result := true
end;
begin
  writeString("Please, give the upper limit : ");
  limit := readInteger();
  writeString("Prime numbers between 0 and ");
  writeInteger(limit);
  writeString("\n\n");
  counter := 0;
  if limit >= 2 then
  begin
    counter := counter + 1;
    writeString("2\n")
  end;
  if limit >= 3 then
  begin
    counter := counter + 1;
    writeString("3\n");
  end;
  number := 6;
  while number <= limit do
  begin
    if prime(number-1) then
    begin
      counter := counter + 1;
      writeInteger(number-1);
      writeString("\n")
    end;
    if (number <> limit) and prime(number+1) then
    begin
      counter := counter + 1;
      writeInteger(number+1);
      writeString("\n")
    end;
    number := number + 6
  end;
  writeString("\n");
  writeInteger(counter);
  writeString(" prime number(s) were found.\n")
end.
