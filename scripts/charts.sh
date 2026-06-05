echo "Building charts..."
source .venv/bin/activate
python ./charts/main.py

echo "Updating docs assets..."
mkdir -p ./docs/assets
cp ./output/*.png ./docs/assets

echo "Done!"
