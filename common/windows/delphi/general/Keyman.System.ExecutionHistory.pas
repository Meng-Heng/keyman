{
  Keyman is copyright (C) SIL Global. MIT License.

  This module provides functionality to track the execution state of the Keyman
  engine. It uses a global atom to record whether Keyman has started during the
  current session and checks if it has previously run.
}
unit Keyman.System.ExecutionHistory;


interface

const
  AtomName = 'KeymanSessionFlag';

function RecordKeymanStarted : Boolean;
function HasKeymanRun : Boolean;

implementation

uses
  System.SysUtils,
  Winapi.Windows,
  KLog;

function RecordKeymanStarted : Boolean;
var
  atom: WORD;
begin
 Result := False;
  try
    atom := GlobalFindAtom(AtomName);
    if atom = 0 then
    begin
      if GetLastError <> ERROR_FILE_NOT_FOUND then
        RaiseLastOSError;
      atom := GlobalAddAtom(AtomName);
      Result := True;
      if atom = 0 then
        RaiseLastOSError;
    end;
  except
    on E: Exception do
      // TODO: #10210 convert to sentry error
      KL.Log(E.ClassName + ': ' + E.Message);
  end;
end;

function HasKeymanRun : Boolean;
var
  atom: WORD;
begin
  Result := False;
  try
    atom := GlobalFindAtom(AtomName);
    if atom <> 0 then
    begin
      if GetLastError <> ERROR_SUCCESS then
        RaiseLastOSError;
      Result := True;
    end
    else
    begin
      Result := False;
    end;

  except
    on E: Exception do
      // TODO: #10210 convert to sentry error
      KL.log(E.ClassName + ': ' + E.Message);
  end;

end;

end.
