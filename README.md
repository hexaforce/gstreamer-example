# gstreamer-example
rm -rf ~/rpmbuild
rpmdev-setuptree

cp -r /gstreamer-example ~/rpmbuild/BUILD/



rpmbuild -ba rpm/gstreamer-example.spec







tar czvf src.tar.gz /gstreamer-example
mv src.tar.gz ~/rpmbuild/SOURCES/


cp /gstreamer-example/rpm.spec ~/rpmbuild/SPECS/


rpmbuild -ba ~/rpmbuild/SPECS/rpm.spec

