docker-compose -f .\docker-compose-start-rest.yaml stop
Start-Sleep -s 30
docker-compose -f .\docker-compose-dbstart-normal.yaml stop
