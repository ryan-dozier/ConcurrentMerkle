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


PostBuild.SetVerifier.Debug:
/Users/admin/area67/ConcurrentMerkle/_build/Debug/SetVerifier:
	/bin/rm -f /Users/admin/area67/ConcurrentMerkle/_build/Debug/SetVerifier


PostBuild.ConcurrentMerkle.Release:
/Users/admin/area67/ConcurrentMerkle/_build/Release/ConcurrentMerkle:
	/bin/rm -f /Users/admin/area67/ConcurrentMerkle/_build/Release/ConcurrentMerkle


PostBuild.SetVerifier.Release:
/Users/admin/area67/ConcurrentMerkle/_build/Release/SetVerifier:
	/bin/rm -f /Users/admin/area67/ConcurrentMerkle/_build/Release/SetVerifier


PostBuild.ConcurrentMerkle.MinSizeRel:
/Users/admin/area67/ConcurrentMerkle/_build/MinSizeRel/ConcurrentMerkle:
	/bin/rm -f /Users/admin/area67/ConcurrentMerkle/_build/MinSizeRel/ConcurrentMerkle


PostBuild.SetVerifier.MinSizeRel:
/Users/admin/area67/ConcurrentMerkle/_build/MinSizeRel/SetVerifier:
	/bin/rm -f /Users/admin/area67/ConcurrentMerkle/_build/MinSizeRel/SetVerifier


PostBuild.ConcurrentMerkle.RelWithDebInfo:
/Users/admin/area67/ConcurrentMerkle/_build/RelWithDebInfo/ConcurrentMerkle:
	/bin/rm -f /Users/admin/area67/ConcurrentMerkle/_build/RelWithDebInfo/ConcurrentMerkle


PostBuild.SetVerifier.RelWithDebInfo:
/Users/admin/area67/ConcurrentMerkle/_build/RelWithDebInfo/SetVerifier:
	/bin/rm -f /Users/admin/area67/ConcurrentMerkle/_build/RelWithDebInfo/SetVerifier




# For each target create a dummy ruleso the target does not have to exist
