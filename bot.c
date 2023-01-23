#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "discord.h"
#include "log.h"

#define GUILD_ID 123456789012345678 //add your GUILD_ID here

void on_interaction(struct discord *client, const struct discord_interaction *event)
{
	if (event->type != DISCORD_INTERACTION_APPLICATION_COMMAND) {
		return; /* interaction is not a slash command */
	}

	if (strcmp(event->data->name, "info") == 0) {
		struct discord_interaction_response params = {
			.type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
			.data = &(struct discord_interaction_callback_data){
				.content =
					"Hi! I am a bot written in C. "
					"I convert time."
			}
		};
		discord_create_interaction_response(client, event->id, event->token, &params, NULL);
		return;
	}
}

void on_ready(struct discord *client, const struct discord_ready *event)
{
	log_info("Bot connected as %s#%s",
			event->user->username, event->user->discriminator
		);

	struct discord_create_guild_application_command params_info = {
		.name = "info",
		.description = "bot info."
	};
	discord_create_guild_application_command(client, event->application->id, GUILD_ID, &params_info, NULL);
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
	discord_set_on_interaction_create(client, &on_interaction);
	discord_run(client);
	discord_cleanup(client);
	ccord_global_cleanup();
}
