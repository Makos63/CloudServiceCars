docker-compose -f .\docker-compose-init.yaml up -d
Start-Sleep -s 60
docker-compose -f .\docker-compose-dbstart-normal.yaml up -d
Start-Sleep -s 60
docker-compose -f .\docker-compose-start-rest.yaml up
