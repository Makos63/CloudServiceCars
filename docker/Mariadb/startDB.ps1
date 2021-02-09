docker-compose -f .\docker-compose-init.yaml up -d
Start-Sleep -s 50
docker-compose -f .\docker-compose-run.yaml up