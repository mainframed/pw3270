
use arg uri

host = .rx3270~new("pw3270:a")

say "PW3270 version is "||host~revision()
say "Connections state is "||rx3270QueryCState()

return 0

::requires "rx3270.cls"
