GBDK = ../../gbdk
GBDKLIB = $(GBDK)/lib/small/asxxxx
CC = $(GBDK)/bin/lcc
TOOLS = ./tools
MAPCVT = $(TOOLS)/mapcvt
GBR2C = $(TOOLS)/gbr2c

CART_SIZE = 4

ROM_BUILD_DIR = build
OBJDIR = obj

#MUSIC_DRIVER = -Wl-klib -Wl-lhUGEDriver.lib
MUSIC_DRIVER =
SYMBOLS = -Wl-j -Wm-yS -Wl-m -Wl-w

CFLAGS = -Isrc/include -I$(OBJDIR) -Wa-Isrc/include -Wa-I$(GBDKLIB)

LFLAGS_NBANKS += -Wl-yt0x1A -Wl-yo$(CART_SIZE) -Wl-ya4

LFLAGS = $(LFLAGS_NBANKS) $(SYMBOLS) -Wm-yn"ISOENGINE" $(MUSIC_DRIVER)

TARGET = $(ROM_BUILD_DIR)/ISO.gb

MAPS = $(foreach dir,src/maps,$(notdir $(wildcard $(dir)/*.3dmap)))
MAPSRC = $(MAPS:%.3dmap=$(OBJDIR)/%.o)

GFX  = $(foreach dir,src/gfx,$(notdir $(wildcard $(dir)/*.gbr)))
GFXSRC = $(GFX:%.gbr=$(OBJDIR)/%.gbr.o)

ASRC = $(foreach dir,src,$(notdir $(wildcard $(dir)/*.s))) 
CSRC = $(foreach dir,src,$(notdir $(wildcard $(dir)/*.c))) 

OBJS = $(CSRC:%.c=$(OBJDIR)/%.o) $(ASRC:%.s=$(OBJDIR)/%.o)

#all:	directories release $(TARGET)
all:	directories $(TARGET)

.PHONY: clean release debug color profile directories

release:
	$(eval CFLAGS += -Wf'--max-allocs-per-node 50000')
	@echo "RELEASE mode ON"
	
debug:
	$(eval CFLAGS += -Wf--debug $(SYMBOLS) -Wl-y)
	$(eval CFLAGS += -Wf--nolospre -Wf--nogcse)
	$(eval LFLAGS += -Wf--debug -Wl-y)
	@echo "DEBUG mode ON"

color:
	$(eval CFLAGS += -DCGB)
	$(eval LFLAGS += -Wm-yC)
	@echo "COLOR mode ON"

profile:
	$(eval CFLAGS += -Wf--profile)
	@echo "PROFILE mode ON"

.SECONDARY: $(OBJS) 
	
directories: $(ROM_BUILD_DIR) $(OBJDIR) $(REL_OBJDIR)

$(ROM_BUILD_DIR):
	mkdir -p $(ROM_BUILD_DIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(REL_OBJDIR):
	mkdir -p $(REL_OBJDIR)

$(OBJDIR)/%.gbr.c:	src/gfx/%.gbr
	$(GBR2C) $< $(OBJDIR)

$(OBJDIR)/%.c:	src/maps/%.3dmap
	$(MAPCVT) $< $@

$(OBJDIR)/%.o:	src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/%.o:	src/%.s
	$(CC) $(CFLAGS) -c -o $@ $<

$(ROM_BUILD_DIR)/%.gb:	$(GFXSRC) $(MAPSRC) $(OBJS)
	mkdir -p $(ROM_BUILD_DIR)
	$(CC) $(LFLAGS) -o $@ $^

clean:
	@echo "CLEANUP..."
	rm -rf $(OBJDIR)
	rm -rf $(ROM_BUILD_DIR)

rom: directories $(TARGET)
