# invoke SourceDir generated makefile for tags-cc1310.pem3
tags-cc1310.pem3: .libraries,tags-cc1310.pem3
.libraries,tags-cc1310.pem3: package/cfg/tags-cc1310_pem3.xdl
	$(MAKE) -f C:\files\atlas\tags-cc1310/src/makefile.libs

clean::
	$(MAKE) -f C:\files\atlas\tags-cc1310/src/makefile.libs clean

