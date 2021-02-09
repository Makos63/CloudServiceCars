docker-compose -f docker-compose-start-rest.yaml stop
sleep 60
docker-compose -f docker-compose-dbstart-normal.yaml stop
