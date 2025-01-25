FROM maven

RUN apt-get update && apt-get install -yq \
  libgstreamer1.0-dev \
  gstreamer1.0-nice \
  gstreamer1.0-rtsp

COPY . .

RUN mvn install:install-file  \
  -Dfile=gst1-java-core-1.5.0-SNAPSHOT.jar \
  -DgroupId=org.freedesktop.gstreamer \
  -DartifactId=gst1-java-core \
  -Dversion=1.5.0-SNAPSHOT \
  -Dpackaging=jar

RUN mvn clean package -DskipTests

RUN mv target/webrtc-sendrecv-java-1.5.0-SNAPSHOT-jar-with-dependencies.jar app
