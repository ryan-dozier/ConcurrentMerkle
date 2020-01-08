# DO NOT EDIT
# This makefile makes sure all linkable targets are
# up-to-date with anything they link to
default:
	echo "Do not invoke directly"

# Rules to remove targets that are older than anything to which they
# link.  This forces Xcode to relink the targets from scratch.  It
# does not seem to check these dependencies itself.
PostBuild.ConcurrentMerkle.Debug:
/Users/admin/area67/ConcurrentMerkle/_build/Debug/ConcurrentMerkle:
	/bin/rm -f /Users/admin/area67/ConcurrentMerkle/_build/Debug/ConcurrentMerkle


PostBuild.ConcurrentMerkle.Release:
/Users/admin/area67/ConcurrentMerkle/_build/Release/ConcurrentMerkle:
	/bin/rm -f /Users/admin/area67/ConcurrentMerkle/_build/Release/ConcurrentMerkle


PostBuild.ConcurrentMerkle.MinSizeRel:
/Users/admin/area67/ConcurrentMerkle/_build/MinSizeRel/ConcurrentMerkle:
	/bin/rm -f /Users/admin/area67/ConcurrentMerkle/_build/MinSizeRel/ConcurrentMerkle


PostBuild.ConcurrentMerkle.RelWithDebInfo:
/Users/admin/area67/ConcurrentMerkle/_build/RelWithDebInfo/ConcurrentMerkle:
	/bin/rm -f /Users/admin/area67/ConcurrentMerkle/_build/RelWithDebInfo/ConcurrentMerkle




# For each target create a dummy ruleso the target does not have to exist
