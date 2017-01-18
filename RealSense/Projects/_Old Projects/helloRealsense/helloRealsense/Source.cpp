#include <pxcsensemanager.h>
#include <stdio.h>

int main() {
	// create the PXCSenseManager
	PXCSenseManager *psm = 0;
	psm = PXCSenseManager::CreateInstance();
	if (!psm) {
		wprintf_s(L"Unable to create the PXCSenseManager\n");
		return 1;
	}
	else {
		wprintf_s(L"Success\n");
	}
}