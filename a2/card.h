typedef enum {
	HERO,
	MINION,
	SPELL,
	WEAPON,
} Type;

const char *type_str[] = {
	"Hero",
	"Minion",
	"Spell",
	"Weapon",
};

typedef enum {
	DEMONHUNTER,
	DRUID,
	HUNTER,
	MAGE,
	NEUTRAL,
	PALADIN,
	PRIEST,
	ROGUE,
	SHAMAN,
	WARLOCK,
	WARRIOR,
} Class;

const char *class_str[] = {
	"Demonhunter",
	"Druid",
	"Hunter",
	"Mage",
	"Neutral",
	"Paladin",
	"Priest",
	"Rogue",
	"Shaman",
	"Warlock",
	"Warrior",
};

typedef enum {
	FREE,
	COMMON,
	EPIC,
	RARE,
	LEGENDARY,
} Rarity;

const char *rarity_str[] = {
	"Free",
	"Common",
	"Epic",
	"Rare",
	"Legendary",
};

typedef struct {
	unsigned id;
	char *name;
	unsigned cost;
	Type type;
	Class class;
	Rarity rarity;
	char *text;
	unsigned attack;
	unsigned health;
} CARD_T;

typedef struct {
	char *card_name;
	off_t offset;
} INDEX_T;