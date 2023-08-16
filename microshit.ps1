function xmr { param([string]$target) xmake build $target && xmake run $target}
function xmt {  xmake build test && xmake run test}

function mt { param([string]$target) make manual_test T=$target }
function mtwd{ param([string]$target) make manual_test_with_deps T=$target }
