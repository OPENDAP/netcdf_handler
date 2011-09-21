
# 
# These macros are used for both the netcdf3 and netcdf4 tests.

AT_INIT([bes.conf besstandalone getdap])
# AT_COPYRIGHT([])

AT_TESTED([besstandalone])

# Usage: _AT_TEST_*(<bescmd source>, <baseline file>)

m4_define([_AT_BESCMD_TEST],   
[AT_SETUP([BESCMD $1])
AT_KEYWORDS([bescmd])
AT_CHECK([besstandalone -c $abs_builddir/bes.conf -i $1 || true], [], [stdout], [stderr])
AT_CHECK([diff -b -B $2 stdout || diff -b -B $2 stderr], [], [ignore],[],[])
AT_CLEANUP])

m4_define([_AT_BESCMD_BINARYDATA_TEST],   
[AT_SETUP([BESCMD $1])
AT_KEYWORDS([bescmd])
AT_CHECK([besstandalone -c $abs_builddir/bes.conf -i $1 | getdap -M - || true], [], [stdout], [stderr])
AT_CHECK([diff -b -B $2 stdout || diff -b -B $2 stderr], [], [ignore],[],[])
AT_CLEANUP])

m4_define([AT_BESCMD_RESPONSE_TEST],
[_AT_BESCMD_TEST([$abs_srcdir/nc/$1], [$abs_srcdir/nc/$1.baseline])
])

m4_define([AT_BESCMD_BINARYDATA_RESPONSE_TEST],
[_AT_BESCMD_BINARYDATA_TEST([$abs_srcdir/nc/$1], [$abs_srcdir/nc/$1.baseline])
])
