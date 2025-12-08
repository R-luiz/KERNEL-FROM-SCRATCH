# Main Makefile for KERNEL-FROM-SCRATCH
# This Makefile builds all sub-projects

# Sub-projects
PROJECTS = kfs-1

# Default target - build all projects
all:
	@for project in $(PROJECTS); do \
		echo "Building $$project..."; \
		$(MAKE) -C $$project || exit 1; \
	done
	@echo "All projects built successfully!"

# Clean all projects
clean:
	@for project in $(PROJECTS); do \
		echo "Cleaning $$project..."; \
		$(MAKE) -C $$project clean; \
	done
	@echo "All projects cleaned!"

# Help target
help:
	@echo "KERNEL-FROM-SCRATCH Build System"
	@echo "================================="
	@echo ""
	@echo "Targets:"
	@echo "  all     - Build all sub-projects (default)"
	@echo "  clean   - Clean all sub-projects"
	@echo "  help    - Show this help message"
	@echo ""
	@echo "Sub-projects:"
	@for project in $(PROJECTS); do \
		echo "  $$project"; \
	done
	@echo ""
	@echo "To build a specific project:"
	@echo "  cd <project-name> && make"

.PHONY: all clean help
