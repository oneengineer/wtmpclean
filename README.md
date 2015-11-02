wtmpclean
=========

a tool for dumping wtmp files and patching wtmp records

Extended Usage:

* -c dumpout detailed log
* wtmpclean -c -f wtmp
* -A add an entry
* -i ip address integer
* --line pts shown in last
* --start login UNIX time in -A mode / original UNIX time in -E mode
* --end logout UNIX time / modified UNIX time in -E mode
* -p pid in -E mode, which can be found with -c
* -H hostname
* -d specify logout entry in -E mode

	wtmpclean -A -f wtmp2 --line pts/49 --start 1446192465 --end 1446197438 -i 16777343 -H localhost username     //Add an new entry
	wtmpclean -E -f wtmp2 --start 1426238869 --end 1426239169 -p 3386 -d     //edit a log out entry with pid 3386 and start time 1426238869, make the logout time to 1426239169

