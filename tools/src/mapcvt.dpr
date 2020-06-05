{$apptype console}

uses  windows, classes, sysutils, inifiles, math;

const sec_sys  = 'system';
var   data     : pAnsiChar;
      voc, rum : ansistring;
      x,y,z    : longint;
      lst      : tStringList;
      i, sz    : longint;
      ini      : tIniFile;
      outfile  : tMemoryStream;
      outhdr   : tMemoryStream;
      bank     : longint;
      outname  : ansistring;
      outpath  : ansistring;

function LoadRoom(aini: tIniFile; const aroom: ansistring; adata: pAnsiChar): boolean;
var lvl     : tStringList;
    i, j, k : longint;
    tmp     : ansistring;
begin
  result:= false;
  fillchar(data^, sz, 0);
  lvl:= tStringList.create;
  try
    lvl.CommaText:= aini.ReadString(sec_sys, aroom, '');
    if (lvl.Count = z) then begin
      for i:= 0 to z - 1 do begin
        for j:= 0 to x - 1 do begin
          tmp:= aini.ReadString(lvl[i], inttostr(j), '');
          for k:= 0 to min(length(tmp), y) - 1 do
            adata[(i * x * y) + (j * y) + k]:= chr(max(0, pos(tmp[k + 1], voc) - 1));
        end;
      end;
      result:= true;
    end else writeln(format('ERROR: room %s level count: %d must be: %d', [aroom, lvl.Count, z]));
  finally freeandnil(lvl); end;
end;

function filterchars(const astr, pattern: ansistring): ansistring;
var i, j : longint;
begin
  setlength(result, length(astr));
  j:= 0;
  for i:= 1 to length(astr) do
    if (pos(astr[i], pattern) = 0) then begin
      inc(j); result[j]:= astr[i];
    end;
  setlength(result, j);
end;
function writestr(res: tMemoryStream; const astr: ansistring): longint;
begin result:= res.Write(astr[1], length(astr)); end;

procedure Room2Source(const aroom: ansistring; adata: pAnsiChar; res: tMemoryStream);
var i, j, k : longint;
    id, cnt : byte;
    tmp     : ansistring;
  function get(dx, dy, dz: longint): byte;
  begin result:= byte(adata[(dz * x * y) + (dx * y) + dy]); end;
begin
  cnt:= 1;
  writestr(res, format('const scene_item_t %s[] = {'#$0d#$0a, [aroom]));
  for i:= 0 to x - 1 do
    for j:= y - 1 downto 0 do
      for k:= 0 to z - 1 do begin
        id:= get(i, j, k);
        if (id <> 0) then begin
          writestr(res, format('{%d, to_x(%d, %d, %d), to_y(%d, %d, %d), to_coords(%d, %d, %d)', [id - 1, i,j,k, i,j,k, i,j,k]));
          tmp:= format(', &%s[%d]},'#$0d#$0a, [aroom, cnt]);
          writestr(res, tmp);
          inc(cnt);
        end;
      end;
  res.Size:= res.Size - length(tmp);
  writestr(res, ', 0}'#$0d#$0a'};'#$0d#$0a);
end;

begin
  data:= nil;
  if (ParamCount = 2) and fileexists(paramstr(1)) then begin
    outname:= extractfilename(paramstr(2));
    outpath:= extractfilepath(paramstr(2));
    outfile:= tMemoryStream.create;
    outhdr:= tMemoryStream.create;
    lst:= tStringList.Create;
    try
      ini:= tIniFile.Create(paramstr(1));
      with ini do try
        bank:= ReadInteger(sec_sys, 'bank', 0);
        voc:= ReadString(sec_sys, 'voc', '');
        lst.CommaText:= ReadString(sec_sys, 'XYZ', '0,0,0');
        if (lst.count = 3) then begin
          x:= strtointdef(lst[0], 0);
          y:= strtointdef(lst[1], 0);
          z:= strtointdef(lst[2], 0);
          sz:= x * Y * z;
          if (sz > 0) then begin
            data:= allocmem(sz);
            lst.CommaText:= ReadString(sec_sys, 'rooms', '');
            if (lst.Count > 0) then begin
              for i:= 0 to lst.count - 1 do
                rum:= lst[i];
                if LoadRoom(ini, rum, data) then begin
                  if (outfile.Size = 0) then begin
                    if (bank > 0) then writestr(outfile, format('#pragma bank %d'#$0d#$0a#$0d#$0a, [bank]));
                    writestr(outfile, format('#include "%s"'#$0d#$0a#$0d#$0a, [changefileext(outname, '.h')]));
                  end;
                  if (outhdr.Size = 0) then begin
                    writestr(outhdr, format('#ifndef %s_h_INCLUDE'#$0d#$0a, [filterchars(outname, '.,:;"''/\[]{}')]));
                    writestr(outhdr, format('#define %s_h_INCLUDE'#$0d#$0a#$0d#$0a, [filterchars(outname, '.,:;"''/\[]{}')]));
                    writestr(outhdr, '#include "scenes.h"'#$0d#$0a#$0d#$0a);
                  end;
                  Room2Source(rum, data, outfile);
                  writestr(outhdr, format('extern const scene_item_t %s[];'#$0d#$0a, [rum]));
                end;
            end else writeln('ERROR: no rooms in map file');
          end else writeln('ERROR: invalid dimentions');
        end else writeln('ERROR: incorrect XYZ parameter');
      finally free; end;
    finally
      freeandnil(lst);
      if (outfile.Size > 0) then outfile.SaveToFile(format('%s%s.c', [outpath, outname]));
      freeandnil(outfile);
      if (outhdr.Size > 0) then begin
        writestr(outhdr, #$0d#$0a'#endif'#$0d#$0a);
        outhdr.SaveToFile(format('%s%s', [outpath, changefileext(outname, '.h')]));
      end;
      freeandnil(outhdr);
      if assigned(data) then freemem(data);
    end;
  end else writeln('usage: mapcvt <input map file> <output source file>');
end.

