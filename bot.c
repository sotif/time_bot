#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "discord.h"
#include "log.h"

void on_ready(struct discord *client, const struct discord_ready *event)
{
	log_info("Bot connected as %s#%s",
			event->user->username, event->user->discriminator
		);
}

int main(int argc, char** argv)
{
	const char*	config_file;

	if (argc > 1) {
		config_file = argv[1];
	} else {
		config_file = "./config.json";
	}

	ccord_global_init();
	struct discord *client = discord_config_init(config_file);
	assert(NULL != client && "Couldn't initialize client");
	discord_add_intents(client, DISCORD_GATEWAY_MESSAGE_CONTENT);

	discord_set_on_ready(client, &on_ready);
	discord_run(client);
	discord_cleanup(client);
	ccord_global_cleanup();
}
