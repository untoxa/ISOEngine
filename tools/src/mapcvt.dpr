{$apptype console}

uses  windows, classes, sysutils, inifiles, math;

const sec_sys      = 'system';
var   data         : pAnsiChar;
      voc, rum     : ansistring;
      x,y,z        : longint;
      lst          : tStringList;
      rooms, world : tStringList;
      i, j, sz     : longint;
      ini          : tIniFile;
      outfile      : tMemoryStream;
      outhdr       : tMemoryStream;
      bank         : longint;
      outname      : ansistring;
      outpath      : ansistring;
      modifier     : ansistring;
      includes     : tStringList;

function LoadWorld(aini: tIniFile; const aworldname: ansistring): boolean;
  function GetRoomById(aid: ansichar): ansistring;
  var idx : longint;
  begin
    idx:= rooms.IndexOfObject(pointer(aid));
    if (idx >= 0) then result:= rooms[idx] else setlength(result, 0);
  end;
  procedure AppendProperty(idx: longint; const aprop: ansistring);
  begin
    if (idx >= 0) then world.strings[idx]:= format('%s, %s', [world.strings[idx], aprop]);
  end;
  function SetRoomProperty(x, y: longint; const aprop: ansistring; aforce: boolean): longint;
  var id : longint;
  begin
    result:= -1;
    if (x >= 0) and (y >= 0) then begin
      id:= ((y and $ff) shl 8) or (x and $ff);
      result:= world.IndexOfObject(pointer(id));
      if (result >= 0) then begin
        AppendProperty(result, aprop);
      end else begin
        if aforce then result:= world.AddObject(aprop, pointer(id));
      end;
    end;
  end;
  function GetWorldIndex(x, y: longint): longint;
  var id: longint;
  begin
    result:= -1;
    if (x >= 0) and (y >= 0) then begin
      id:= ((y and $ff) shl 8) or (x and $ff);
      result:= world.IndexOfObject(pointer(id));
    end;
  end;
var x, y, idx, idx2 : longint;
    row, rum        : ansistring;
begin
  result:= false;
  if length(aworldname) > 0 then begin
    for y:= 0 to aini.ReadInteger(aworldname, 'rows', 0) - 1 do begin
      row:= aini.ReadString(aworldname, inttostr(y), '');
      for x:= 0 to length(row) - 1 do begin
        rum:= GetRoomById(row[x + 1]);
        if length(rum) > 0 then begin
          idx:= SetRoomProperty(x, y, format('.room=%s, .room_bank=%d', [rum, bank]), true);

          SetRoomProperty(x - 1, y, format('.N=&%s[%d]', [aworldname, idx]), false);
          idx2:= GetWorldIndex(x - 1, y); if (idx2 >= 0) then SetRoomProperty(x, y, format('.S=&%s[%d]', [aworldname, idx2]), false);

          SetRoomProperty(x, y - 1, format('.E=&%s[%d]', [aworldname, idx]), false);
          idx2:= GetWorldIndex(x, y - 1); if (idx2 >= 0) then SetRoomProperty(x, y, format('.W=&%s[%d]', [aworldname, idx2]), false);
        end;
      end;
    end;
    result:= true;
  end;
end;

function LoadRoom(aini: tIniFile; const aroom: ansistring; adata: pAnsiChar): boolean;
var lvl     : tStringList;
    i, j, k : longint;
    tmp     : ansistring;
begin
  result:= false;
  fillchar(data^, sz, 0);
  lvl:= tStringList.create;
  try
    tmp:= aini.ReadString(aroom, 'id', '');
    if assigned(rooms) and (length(tmp) > 0) then rooms.AddObject(aroom, pointer(byte(tmp[1])));
    lvl.CommaText:= aini.ReadString(aroom, 'layers', '');
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
  writestr(res, format('%sconst scene_item_t %s[] = {'#$0d#$0a, [modifier, aroom]));
  for i:= 0 to x - 1 do
    for j:= y - 1 downto 0 do
      for k:= 0 to z - 1 do begin
        id:= get(i, j, k);
        if (id <> 0) then begin
          writestr(res, format('{.id=%d, .x=to_x(%d, %d, %d), .y=to_y(%d, %d, %d), .coords=to_coords(%d, %d, %d)', [id - 1, i,j,k, i,j,k, i,j,k]));
          tmp:= format(', .next=&%s[%d]},'#$0d#$0a, [aroom, cnt]);
          writestr(res, tmp);
          inc(cnt);
        end;
      end;
  res.Size:= res.Size - length(tmp);
  writestr(res, ', .next=0}'#$0d#$0a'};'#$0d#$0a);
end;

begin
  data:= nil;
  if (ParamCount = 2) and fileexists(paramstr(1)) then begin
    outname:= extractfilename(paramstr(2));
    outpath:= extractfilepath(paramstr(2));
    outfile:= tMemoryStream.create;
    outhdr:= tMemoryStream.create;
    includes:= tStringList.Create;
    world:= tStringList.Create;
    rooms:= tStringList.Create;
    rooms.Sorted:= True; rooms.Duplicates:= dupIgnore;
    lst:= tStringList.Create;
    try
      ini:= tIniFile.Create(paramstr(1));
      with ini do try

        bank:= ReadInteger(sec_sys, 'bank', 0);
        modifier:= ReadString(sec_sys, 'modifier', ''); if (length(modifier) > 0) then modifier:= modifier + ' ';
        includes.CommaText:= ReadString(sec_sys, 'includes', '');
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
              for i:= 0 to lst.count - 1 do begin
                rum:= lst[i];
                if LoadRoom(ini, rum, data) then begin
                  if (outfile.Size = 0) then begin
                    if (bank > 0) then writestr(outfile, format('#pragma bank %d'#$0d#$0a#$0d#$0a, [bank]));
                    if (includes.Count > 0) then begin
                       for j:= 0 to includes.count - 1 do writestr(outfile, format('#include "%s"'#$0d#$0a, [includes[j]]));
                       writestr(outfile, #$0d#$0a);
                    end;
                  end;
                  if (outhdr.Size = 0) then begin
                    writestr(outhdr, format('#ifndef __%s_H_INCLUDE'#$0d#$0a, [filterchars(outname, '.,:;"''/\[]{}')]));
                    writestr(outhdr, format('#define __%s_H_INCLUDE'#$0d#$0a#$0d#$0a, [filterchars(outname, '.,:;"''/\[]{}')]));
                    if (includes.Count > 0) then begin
                       for j:= 0 to includes.count - 1 do writestr(outhdr, format('#include "%s"'#$0d#$0a, [includes[j]]));
                       writestr(outhdr, #$0d#$0a);
                    end;
                  end;
                  Room2Source(rum, data, outfile);
                  writestr(outhdr, format('extern const scene_item_t %s[];'#$0d#$0a, [rum]));
                end;
              end;
            end else writeln('ERROR: no rooms in map file');
          end else writeln('ERROR: invalid dimentions');
        end else writeln('ERROR: incorrect XYZ parameter');

        rum:= ReadString(sec_sys, 'world', '');
        if LoadWorld(ini, rum) then begin
          if world.count > 0 then begin
            // world
            modifier:= ini.ReadString(rum, 'modifier', ''); if (length(modifier) > 0) then modifier:= modifier + ' ';
            writestr(outfile, format(#$0d#$0a'%sconst world_item_t %s[] = {'#$0d#$0a, [modifier, rum]));
            for i:= 0 to world.count - 1 do writestr(outfile, format('{%s}, '#$0d#$0a, [world[i]]));
            writestr(outfile, '};'#$0d#$0a);
            // header
            writestr(outhdr, format('extern const world_item_t %s[];'#$0d#$0a, [rum]));
          end;
        end;

      finally free; end;
    finally
      freeandnil(includes);
      freeandnil(lst);
      freeandnil(rooms);
      freeandnil(world);
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

