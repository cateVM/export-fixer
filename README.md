# Export fixer
Unhooks hooked exports
<br />
Example:
```cpp
#include "Exports.h"
int main() {
	auto ProcHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PIDHERE);
	const auto LdrInitaliseThunk = exports::get::GetExportByName(
		"LdrInitializeThunk",
		"ntdll"
	);
	if (exports::fix::FixExport(
		ProcHandle, 
		LdrInitaliseThunk)
	) {
		return 0;
	}
	return 1;
}
```
