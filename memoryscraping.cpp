//
// memoryscraping.cpp
//
// A text-based adventure from HungoverAdventures.
//
// Special thanks to @g'deth for the excellent, bug-free standards-
// compliant code and for showing us how to use function pointers 
// to solve all our problems.
//
// Note: We've put all the code in one file so that it runs faster.
// Note 2: If it crashes, use 'tput cnorm' to get your cursor back
//
// Compile me with:
// gcc -o ms -Wall memoryscraping.cpp

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <termios.h>
#include <unistd.h>
#include <sys/mman.h>

const int width = 53;
const int width_ = 18;
const int height = 40;
const int inv_width = 0;
const int CLARITY_REQUIRED_FOR_JALAPENOS = 999; // 90		- Removing jalapeno scene until we can work out how to do the 'ñ' thing

#define DEFAULT "\x1B[0m"
#define WHITE "\x1B[1m"
#define RED "\x1B[0;31m"
#define LGREEN "\x1B[1;32m"
#define GREEN "\x1B[32m"
#define YELLOW "\x1B[1;33m"
#define ORANGE "\x1B[0;33m"
#define BLUE "\x1B[0;34m"
#define GREY "\x1B[34m"
#define MAGENTA "\x1B[35m"
#define CYAN "\x1B[36m"

typedef void* (*room_fn)(void);
typedef void (*ending_fn)(char*);

void flag(char* text);
void cls();
void ui(const char* text);
char get_selection();

// Game state

enum SLEEP_LOCATION
{
	SL_NONE,
	SL_SOFA,
	SL_BED,
};

const int CMD_HISTORY_LENGTH = 256;
const int INV_ITEM_COUNT= 3;
const char* inventory_items[INV_ITEM_COUNT] = { "Phone", "Wallet", "Tai's number" };
struct _state
{
	char inv_begin;
	char phone;
	char wallet;
	char note;
	char evidence_count;

	char hallway_light;
	char fridge_open;
	char medical_drawer_open;
	char medicated;
	char tap_used;
	char jalapenos_eaten;
	char jeans_picked_up;
	char wallet_picked_up;

	char phone_location;
	char sleep_location;

	int alertness;
	int clarity;

	char* cmd_history;
	int cmd_history_off;
	ending_fn endings[4];
} state = {0};

// Room functions

void* toilet();
void* bedroom();
void* kitchen();
void* living_room();
void* hallway();

void* toilet()
{
	cls();
	char text[width * height] = {0};

	if (state.jeans_picked_up == 1)
	{
		strcat(text, "You pick up the trousers and consider putting them on but decide against it. They are wet at the hems of the legs and leave some grass cuttings on the tiles as you lift them from the floor.\n You reach into the pockets and find nothing of interest at first, but then discover a folded envelope in the back pocket. You don't normally use the back pockets, but thought to check for some reason on this occasion.\n You recognise your own handwriting on the otherwise empty envelope:\n !m'Tai - 07700941628'\n The name doesn't seem familiar and you reserve space for a brief pang of shame.\n ");
		state.note = 1;
		++state.evidence_count;
		state.jeans_picked_up = 2;
	}
	else
	{
		strcat(text, "The bathroom is in your typical shower-over-the-bath arrangement, kitted out with a tasteful ivory suite. The sink is full of stained clothes you left to soak some days ago and haven't since found the time to deal with. On the window sill is an electric razor and a marker pen.\n A can of caffeine drink sits opened but mostly full on the side of the bath tub. Judging from the design on the can it probably tasted awful even before it was opened so you know better than to drink it now.\n ");
	}

	if (!state.jeans_picked_up)
		strcat(text, "Adjacent to the toilet lies a pair of *jeans in a crumpled heap with the belt still in place. ");
	
	strcat(text, "The *hallway is behind you.");

	ui(text);

	while (true)
	{
		char c = tolower(get_selection());
		switch (c)
		{
			case 'j':
				if (!state.jeans_picked_up)
				{
					state.jeans_picked_up = 1;
					return (void*)&toilet;
				}
				break;
			case 'h':
				return (void*)&hallway;
		}
	}
}

void* bedroom()
{
	cls();
	char text[width * height] = {0};

	if (state.wallet_picked_up == 1)
	{
		strcat(text, "You pick up the !mwallet !!which feels unusually bulky. Opening it up reveals a wad of fifty-pound notes which you feel no inclination to count, but determine to be at least £800. You have no real experience judging volumes of cash like this but feel confident in your estimate anyway. This isn't your money, you somehow sense, but you know the owner won't come looking.\n ");
		state.wallet = 1;
		++state.evidence_count;
	}

	strcat(text, "You are in your bedroom. The *bed is made and looks like a nice place to be. There's a heap of clean clothes on the floor");
	if (state.wallet_picked_up == 0)
		strcat(text, ", upon which your *wallet has been tossed");
	strcat(text, ". You're trialling a new fabric conditioner and are fairly pleased with it.\n The light is switched off but with the *hallway door open you can see pretty well.\n On your bedside table is a pile of one- and two-pound coins, which you maintain in order to have a reliable supply of tips for takeaway drivers. If you spared a thought you'd realise that this isn't a very good place to keep the ever-present stash of change, but it's always been this way and you don't really notice it anymore.\n The curtains are still open but it's pitch-black in the garden, and the red LCD display on your clock is flashing !r12:00\n !!You're not sure why.");

	ui(text);
	while (true)
	{
		char c = tolower(get_selection());
		switch (c)
		{
			case 'b':
				state.sleep_location = SL_BED;
				state.alertness = 0;
				return (void*)&bedroom;
			case 'w':
				if (!state.wallet_picked_up)
				{
					state.wallet_picked_up = 1;
					return (void*)&bedroom;
				}
			case 'h':
				return (void*)&hallway;
				break;
		}
	}
}

void* kitchen()
{
	cls();
	char text[width * height] = {0};

	switch (state.tap_used) 
	{
		case 0:
			break;
		case 1:
			strcat(text, "You run the cold tap and splash some cold water onto your face. It has surprisingly little effect but you feel a bit better if anything.\n ");
			state.alertness += 15;
			break;
		case 2:
			strcat(text, "After the recent success, you decide to splash your face again. Feels good.\n ");
			state.alertness += 13;
			break;
		case 3:
			strcat(text, "Sure, why stop now? More water will surely help the situation.\n ");
			state.alertness += 12;
			break;
		case 4:
			strcat(text, "You feel really clever to have sussed out this simple way to wake yourself up, and decide to keep the tap running.\n ");
			state.alertness += 11;
			break;
		case 5:
			strcat(text, "Your t-shirt is starting to get wet from all this splashing, but that's fine really.\n ");
			state.alertness += 10;
			break;
		case 6:
			strcat(text, "This time you decide to drink some of the water as you splash your face, and wonder why you didn't think of this in the first place.\n ");
			state.alertness += 20;
			break;
		case 7:
			strcat(text, "The water is making a mess on the floor, but this is probably the least of your problems right now.\n ");
			state.alertness += 10;
			break;
		case 8:
			strcat(text, "The inevitable diminishing returns of repeatedly splashing your face with water are becoming increasingly apparent, but you persist nonetheless.\n ");
			state.alertness += 9;
			break;
		default:
			strcat(text, "With nothing better to do, you just keep on splashing that face.\n ");
			state.alertness += 9;	
			break;
	}
	if (state.alertness > 100) state.alertness = 100;
	if (state.alertness == 100 && state.tap_used > 8)
	{
		strcat(text, "Well done you. You can clean up this mess in the morning, but it might be a good idea to change your clothes before you go to sleep.\n ");
	}

	if (state.fridge_open)
	{
		if (state.clarity <= CLARITY_REQUIRED_FOR_JALAPENOS)
		{
			strcat(text, "You consider looking in the fridge, but know that you'll only find a half-empty jar of pickled jalapenos.\n ");
			state.fridge_open = 0;
		}
		else if (state.jalapenos_eaten)
		{
			strcat(text, "You can't face any more pickled jalapenos.\n ");
			state.fridge_open = 0;
		}
		else
		{
			strcat(text, "You open the fridge and crack open the jar of sorry-looking jalapenos. Using your fingers you fish out a handful of the sliced peppers and scramble them into your mouth, dripping pickling brine on the floor in the process. The jalapenos taste the same way they always do, but the flavour bring back a surprisingly vivid memory from last night of sharing a plate of nachos with a girl called Lisa. This is relevant, somehow.\n As you shut the fridge door a small bundle of flyers falls to the floor, taking the fridge magnet with it. You instinctively look down, with no intention of tidying up the mess, but notice the invitation to last night's fancy-dress party.\n ");
			++state.evidence_count;
			state.jalapenos_eaten = 1;
			state.fridge_open = 0;
		}
	}

	if (state.medical_drawer_open == 1)
	{
		strcat(text, "The medical draw is stuffed with half-used blister packs of supermarket *painkillers and unused plasters. It also boasts and unreasonable number of disposable measuring spoons which have never been used.\n ");
	}
	else if (state.medical_drawer_open == 2)
	{
		if (!state.medicated)
		{
			strcat(text, "You pop two paracetamol from the pack and swallow them without water. It tastes horrible but somehow this helps you feel like the medicine is working.\n ");
			state.alertness += 50;
			if (state.alertness > 100) state.alertness = 100;
			state.medicated = 1;
		}
		else
		{
			strcat(text, "The label reads:\n 'Take two tablets once every four hours.'\n Always read the label.\n ");
		}
		state.medical_drawer_open = 0;
	}

	if (state.clarity > CLARITY_REQUIRED_FOR_JALAPENOS && !state.fridge_open && !state.jalapenos_eaten)
	{
		strcat(text, "You feel an unusual craving for something sour and slightly spicy.\n ");
	}

	strcat(text, "The kitchen is tidy but could do with a clean. A closed pizza box sits on the worktop, beside a rarely used block of knives. The bin needs emptying, and contrary to the cliché, the *tap is not dripping.\n ");
	if (!state.fridge_open)
	{
		strcat(text, "The *fridge hums busily, opposite the doorway to the *living_room");
	}
	if (state.medical_drawer_open)
	{
		strcat(text, ".");
	}
	else
	{
		strcat(text, ", and the overflowing *medical_drawer is held slightly open as always.");
	}

	ui(text);
	while (true)
		{
			char c = tolower(get_selection());
			switch (c)
			{
				case 'f':
					state.fridge_open = 1;
					state.tap_used = 0;
					return (void*)&kitchen;
				case 'm':
					if (state.medical_drawer_open == 0)
					{
						state.medical_drawer_open = 1;
						state.tap_used = 0;
						return (void*)&kitchen;
					}
				case 'p':
					if (state.medical_drawer_open == 1)
					{
						state.medical_drawer_open = 2;
						state.tap_used = 0;
						return (void*)&kitchen;
					}
					break;
				case 't':
					++state.tap_used;
					return (void*)&kitchen;
					break;
				case 'l':
					state.tap_used = 0;
					return (void*)&living_room;
			}
	}
}

void* living_room()
{
	cls();
	char text[width * height] = {0};

	if (state.phone_location == 1)
	{
		strcat(text, "You misjudge the distance as you reach for your phone and nudge it off the bannister to the hallway below, taking a few coins with it.\n ");
		state.phone_location = 2;
	}

	strcat(text, "The living room is cosy with a particularly inviting *sofa. It doubles up as your office, so it's in much better shape than the rest of the house. Unruly stacks of paper lie heaped on the coffee table beside some empty cans of imported lager and generic-looking caffeine drinks.\n ");
	if (state.phone_location == 0 && state.clarity > 40)
		strcat(text, "Resting on the bannister you notice your *phone and keys, sitting among some small change.\n ");
	strcat(text, "Out the window the sky is a muddy grey colour, and the only movement to be seen are the subtle currents in the mist surrounding the street-lamps, or perhaps that's just in your head.\n Next to you, the *kitchen door is wide open and you can smell the bin from here.\n Stairs lead down to the *hallway.");

	ui(text);
	while (true)
		{
			char c = tolower(get_selection());
			switch (c)
			{
				case 's':
					state.sleep_location = SL_SOFA;
					state.alertness = 0;
					return (void*)&living_room;
				case 'k':
					return (void*)&kitchen;
				case 'h':
					return (void*)&hallway;
				case 'p':
					state.phone_location = 1;
					return (void*)&living_room;
			}
		}

	return NULL;
}

void* hallway()
{
	cls();
	if (!state.hallway_light)
	{
		// First visit
		ui("What is this place? And how did you get here?\n These are questions for now.\n Your head is thick, foggy, and your body aches to your bones. You open your eyes, but it's still dark and now your head hurts even more. You peel your hand and face from the cold hard linoleum floor and reach out absently forward to find a tepid radiator.\n Your arm is numb with parasthesia, but with a little effort you manage to pull yourself to some approximation of a standing position. As you steady yourself, your fingers find a *pull-cord hanging from above.\n Nothing makes sense right now, but you have a bad feeling.");
		while (true)
		{
			char c = tolower(get_selection());
			switch (c)
			{
				case 'p':
					state.hallway_light = 1;
					return (void*)&hallway;
			}
		}
	}
	else
	{
		char text[width * height] = {0};
		if (state.hallway_light == 1)
		{
			// Just turned light on
			sprintf(text, "As you tug on the cord, the scene bursts into life with a light so bright that it almost takes you off your feet. It takes a while for the disorientation to subside, along with the misplaced rage you briefly feel toward the light source itself.\n You now find yourself looking at a characterless geometric grey wallpaper that's as familiar as the feeling of stale alcohol grinding its way through your veins. You recognise this room as the hallway of your apartment, and don't know whether to feel relieved or dismayed.\n Something happened last night, but that's about all you recall. Was it something awful? Every fibre within you wants to take refuge in unconsciousness, but you're inexplicably compelled to solve this mystery before the shards of your memory risk being lost forever in a sea of drunken dreams.\n ");
			state.hallway_light = 2;
		}
		if (state.phone_location == 3)
		{
			state.phone_location = 99;
			strcat(text, "You pick up the !mphone !!and instinctively unlock it with your thumbprint. The maps application is already open, showing a walking route from somewhere near Randall's Steakhouse to a street address on Hanselfield Close. The image of an overweight policeman pops into your mind, but nothing rings a bell.\n ");
			state.phone = 1;
			state.clarity += 25;
			if (state.clarity > 100) state.clarity = 100;
			++state.evidence_count;
		}
		// Subsequent visits
		strcat(text, "The hallway is cold and full of clutter that hasn't been touched in years. A bicycle lies haphazardly against the near wall. You used to love that bike, but you were different person back then.\n The door to the *toilet sits ajar to your right. Ahead is the *bedroom, and to your left, a small carpeted staircase leads up to the *living_room.\n ");
		if (state.phone_location == 2)
		{
			strcat(text, "Your *phone is lying faced-down on the floor.\n ");
		}
		ui(text);
		while (true)
		{
			char c = tolower(get_selection());
			switch (c)
			{
				case 't':
					return (void*)&toilet;
				case 'b':
					return (void*)&bedroom;
				case 'l':
					return (void*)&living_room;
				case 'p':
					if (state.phone_location == 2)
						state.phone_location = 3;
					return (void*)&hallway;
			}
		}
	}
	return NULL;
}

void fall_asleep()
{
	cls();
	char text[width * height] = {0};
	switch (state.sleep_location)
	{
		case SL_SOFA:
			strcat(text, "You fall onto the sofa, and quickly give in to the urge to lie down. Your eyes become heavy and the general feeling of anxiety fades into disinterest.\n ");
			break;
		case SL_BED:
			strcat(text, "You made it to the bed, which you determine to be success in a way. Despite the metaphorical monsters occupying your mind, you feel relatively safe here. For tonight, at least.\n ");
			break;
		default:
			strcat(text, "As your eyes grow heavier and your muscles weaker, the demand of maintaining your balance becomes too much, and you collapse to the floor in a semi-voluntary controlled descent. You know that this position isn't comfortable for sleeping, but it will do until morning. \n ");
			break;
	}
	state.endings[(int)state.evidence_count](text);
}

void ending0(char* text)
{
	strcat(text, "You descend rapidly into a deep sleep and soon forget about having awoken at all. You are no closer to working out what happened last night but this doesn't matter anymore.\n Perhaps you'll work it out tomorrow.\n Perhaps you won't.\n ");
	strcat(text, "\n ---\n Thanks for playing!\n Fork us on GitHub:\n github.com/hungoveradventures/memoryscraping");
	ui(text);
}

void ending1(char* text)
{
	strcat(text, "As your mind floats away into the unconscious, you can't shake the uneasy feeling that you've done something terrible.\n The police badge, the broken glass, the girl with the hearing aid. Disjointed memories that may never become whole again.\n You stir restlessly as you enter a shallow sleep.\n It's going to be a rough night, and an even rougher day tomorrow.\n ");
	strcat(text, "\n ---\n Thanks for playing!\n Fork us on GitHub:\n github.com/hungoveradventures/memoryscraping");
	ui(text);
}

void ending2(char* text)
{
	strcat(text, "Your mind pauses for a moment on what you've gathered about the night. There are parts that make sense, like the last-minute change of dinner plans with Ricky. But so many that don't.\n Why was the green cloth so important? Why was Casey wearing those horn-rimmed glasses? And what was that fat guy so angry about?\n The puzzle pieces don't line up, but maybe justice was served here tonight. If so, why is the feeling of guilt buried deep in your gut?\n So many questions to feed into a night of delirious dreams masquerading as answers, as your eyelids fall for the night.\n ");
	strcat(text, "\n ---\n Thanks for playing!\n Fork us on GitHub:\n github.com/hungoveradventures/memoryscraping");
	ui(text);
}

void ending3(char* text)
{
	strcat(text, "The fragmented memories swim before you as your body relaxes into the aching cloud that engulfs it. You try impossibly to reconcile the consuming feeling of uncertain fear with one of deep satisfaction and victory.\n You feel like an actor struggling to step out of character as you scramble for reassurance that the bad you did was actually for good. But you know that this will never fly, with so many questions unanswered.\n Questions for tomorrow, perhaps.\n ");
	strcat(text, "\n ---\n Thanks for playing!\n Fork us on GitHub:\n github.com/hungoveradventures/memoryscraping");
	ui(text);
}

void ending4(char* text)
{
	strcat(text, "As your body resigns itself to a much needed rest, your mind comes alive.\n At first the feeling is an untamed exhilaration until finally the pieces come together to form a picture so vivid. You can't believe your plan actually worked!\n You spring to a sitting position and reach to look at the message written on the sole of your foot:\n !g");
	flag(text);
	strcat(text, "!!Tomorrow was going to be a good day.");
	ui(text);
}

// Utility functions

void print_stat(const char* name, int val, const char* col)
{
	char stat[20] = {0};
	char stat_num[40] = {0};
	sprintf(stat, " %s: ", name);
	sprintf(stat_num, "%s%d%%" DEFAULT, col, val);
	printf("%s", stat);
	for (int i = strlen(stat) + strlen(stat_num) - strlen(col) - strlen(DEFAULT); i < 16; ++i)
		printf(" ");
	printf("%s |", stat_num);
}

unsigned int inv_id = 0;
void print_inventory(int lines_remaining)
{
	char s_item[20] = {0};
	const char* col = GREEN;
	switch (height - 2 - lines_remaining) 
	{
		case 0:
			printf("   Inventory     |");
			break;
		case 1:
			printf("-----------------|");
			break;
		case (height - 4):
			if (state.alertness <= 50) col = YELLOW;
			if (state.alertness <= 30) col = ORANGE;
			if (state.alertness <= 20) col = RED;
			print_stat("Alertness", state.alertness, col);
			break;
		case (height - 3):
			if (state.clarity <= 50) col = YELLOW;	
			if (state.clarity <= 30) col = ORANGE;
			if (state.clarity <= 20) col = RED;
			print_stat("Clarity", state.clarity / 2, col);
			break;
		default:
			while (inv_id < INV_ITEM_COUNT)
			{
				char count = ((char*)&state.inv_begin)[inv_id + 1];
				if (count == 0)
				{
					++inv_id;
					continue;
				}
				else if (count == 1)
				{
					sprintf(s_item, " %s", inventory_items[inv_id]);
				}
				else if (count > 1)
				{
					sprintf(s_item, " %s x%d", inventory_items[inv_id], count);
				}
				printf("%s", s_item);
				for (int i = strlen(s_item); i < 17; ++i)
					printf(" ");
				printf("|");
				++inv_id;
				return;
			}
		case 2:
			printf("                 |");
	}
}

void break_line(int &cur_length, int &lines_remaining)
{
	printf(DEFAULT);
	for (int i = cur_length; i < width - 3; ++i)
		printf(" ");
	printf("|");
	print_inventory(lines_remaining);
	printf("\n");
	if (lines_remaining > 1) printf("| ");
	--lines_remaining;
	cur_length = 0;
}

void ui(const char* text)
{
	char word[64];
	char hr[width+1] = {0};
	char hr_[width_+1] = {0};
	char gap[width+1] = {0};
	char gap_[width_+1] = {0};

	memset(hr, '-', width);
	memset(hr_, '-', width_);
	memset(gap, ' ', width);
	memset(gap_, ' ', width_);
	hr[0] = '+'; hr[width-1] = '+';
	hr_[width_-1] = '+';
	gap[0] = '|'; gap[width-1] = '|';
	gap_[width_-1] = '|';

	int wrap_width = width - 5;
	const char* p = text;
	const char* end = text + strlen(text);
	int lines_remaining = height - 2;
	inv_id = 0;

	printf("%s%s\n", hr, hr_);
	printf("| ");
	int cur_length = 0;
	while (p != end)
	{
		const char* next = strchr(p, ' ');
		if (next == NULL) next = end;
		bool line_break = (*(next-1) == '\n');
		int word_len = next - p;
		if (cur_length + word_len > wrap_width)
			break_line(cur_length, lines_remaining);
		if (p + word_len != end) ++word_len;
		if (line_break)
			sprintf(word, "%.*s  ", word_len - 2, p);
		else
			sprintf(word, "%.*s", word_len, p);
		if (word_len < 6 || *(int*)(word+2) != 1734437990)	// Don't replace underscores in flag
		{
			for (int i = 0; i < word_len; ++i)
				if (word[i] == '_') word[i] = ' ';
		}
		for (unsigned int i = 0; i < strlen(word); ++i)
		{
			if (word[i] == -62 || word[i] == -61) cur_length -= 1;
		}
		if (word[0] == '*')
		{
			printf(GREEN "%c" CYAN "%s" DEFAULT, toupper(word[1]), &word[2]);
			cur_length -= 1;
		}
		else if (word[0] == '!')
		{
			switch (word[1])
			{
				case 'r':
					printf(RED);
					break;
				case 'g':
					printf(GREEN);
					break;
				case 'b':
					printf(BLUE);
					break;
				case 'm':
					printf(MAGENTA);
					break;
				default:
					printf(DEFAULT);
					break;
			}
			printf("%s", &word[2]);
			cur_length -= 2;
		}
		else
		{
			printf("%s", word);
		}
		p += word_len;
		cur_length += word_len;
		if (line_break)
		{
			break_line(cur_length, lines_remaining);
			break_line(cur_length, lines_remaining);
		}
	}
	break_line(cur_length, lines_remaining);

	while(lines_remaining)
	{
		if (lines_remaining == 2)
		{
			printf(BLUE "<type your selection to continue>" DEFAULT);
			cur_length = 33;
			break_line(cur_length, lines_remaining);
		}
		else
		{
			cur_length = 0;
			break_line(cur_length, lines_remaining);
		}
	}

	printf("%s%s\n", hr, hr_);

	fflush(stdout);
}

void flag(char* text)
{

	FILE *f = fopen("flag.txt", "r");
	fgets(text + strlen(text), width, f);
	strcat(text, "\n ");
	fclose(f);
}

void cls()
{
	printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
	printf("\e[1;1H\e[2J\n");
}

char get_selection()
{
	char c = getchar();
	if (state.cmd_history_off < CMD_HISTORY_LENGTH)
		state.cmd_history[state.cmd_history_off++] = c;
	return c;
}

void init_state()
{
	state.alertness = 52;
	state.clarity = 10;
	state.endings[0] = &ending0;
	state.endings[1] = &ending1;
	state.endings[2] = &ending2;
	state.endings[3] = &ending3;

	// Using mmap 'cause malloc is slow:
	// state.cmd_history = (char*)malloc(CMD_HISTORY_LENGTH);
	state.cmd_history = (char*)mmap(NULL, CMD_HISTORY_LENGTH, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

int main()
{
	// Enable raw input
	termios term;
	tcgetattr(STDIN_FILENO, &term);
	term.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &term);
	printf("\x1b[?25l");	// Hide cursor

	init_state();
	room_fn room = &hallway;
	while (room != NULL)
	{
		room = (room_fn)room();

		state.alertness -= 8;
		if (state.alertness < 0) state.alertness = 0;

		state.clarity += 20;
		if (state.clarity > 100) state.clarity = 100;
		state.clarity -= (100 - state.alertness) / 9;
		if (state.clarity < 0) state.clarity = 0;

		if (state.alertness == 0)
		{
			fall_asleep();
			break;
		}
	}
	
	// Disable raw input
	term.c_lflag |= ICANON | ECHO;
	tcsetattr(STDIN_FILENO, TCSANOW, &term);
	printf("\x1b[?25h");	// Show cursor
	return 0;
}
