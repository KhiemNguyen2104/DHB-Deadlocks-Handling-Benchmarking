PYTHON = python3
SRC_DIR = src
MAIN = $(SRC_DIR)/main.py

.PHONY: run clean help

run:
	$(PYTHON) $(MAIN)

clean:
	find $(SRC_DIR) -type d -name __pycache__ -exec rm -rf {} + 2>/dev/null; \
	rm -f results/*.png results/*.csv; \
	echo "Cleaned."

help:
	@echo "Usage:"
	@echo "  make run    - Run the full benchmark and generate all visualizations"
	@echo "  make clean  - Remove cached files and result outputs"
