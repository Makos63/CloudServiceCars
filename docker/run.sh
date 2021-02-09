docker-compose -f docker-compose-init.yaml up -d
sleep 60
docker-compose -f docker-compose-dbstart-normal.yaml up -d
sleep 60
docker-compose -f docker-compose-start-rest.yaml up

