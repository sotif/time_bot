#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

#include "discord.h"
#include "log.h"

#define GUILD_ID 123456789012345678 //add your GUILD_ID here
struct timespec start, end;

int string_to_epoch(char* date, char* time, char* timezone, int* result, char errmsg[])
{
	char hours[3] = "";
	int int_hours;
	char minutes[3] = "";
	int int_minutes;
	char year[5] = "";
	int int_year;
	char month[3] = "";
	int int_month;
	char day[3] = "";
	int int_day;

	/* hours */
	strncpy(hours, time, 2);
	int_hours = atoi(hours);
	if ( strspn(hours, "0123456789") != 2 || int_hours < 0 || int_hours > 24) {
		strcpy(errmsg, "hours wrong format");
		return -1;
	}

	/* minutes */
	strncpy(minutes, time+3, 2);
	int_minutes = atoi(minutes);
	if ( strspn(minutes, "0123456789") != 2  || int_minutes < 0 || int_minutes > 59) {
		strcpy(errmsg, "minutes wrong format");
		return -1;
	}

	/* year */
	strncpy(year, date, 4);
	int_year = atoi(year);
	if ( strspn(year, "0123456789") != 4 || int_year < 1970) {
		strcpy(errmsg, "year wrong format");
		return -1;
	}

	/* month */
	strncpy(month, date+5, 2);
	int_month = atoi(month);
	if ( strspn(month, "0123456789") != 2 || int_month < 1 || int_month > 12) {
		strcpy(errmsg, "month wrong format");
		return -1;
	}

	/* day */
	strncpy(day, date+8, 2);
	int_day = atoi(day);
	if ( strspn(day, "0123456789") != 2 || int_day < 1 || int_day > 31 ) {
		strcpy(errmsg, "day wrong format");
		return -1;
	}

	struct tm t;
	time_t epoch_time;
	t.tm_year = int_year - 1900;
	t.tm_mon = int_month-1; /* 0 is january */
	t.tm_mday = int_day;
	t.tm_hour = int_hours;
	t.tm_min = int_minutes;
	t.tm_sec = 0;
	t.tm_isdst = -1;
	epoch_time = mktime(&t);

	*result = epoch_time - 1 * 60 * 60 * (strcmp(timezone, "CET") == 0) - 2 * 60 * 60 * (strcmp(timezone, "CEST") == 0);
	return 1;
}

void on_interaction(struct discord *client, const struct discord_interaction *event)
{
	if (event->type != DISCORD_INTERACTION_APPLICATION_COMMAND) {
		return; /* interaction is not a slash command */
	}

	if (strcmp(event->data->name, "info") == 0) {
		clock_gettime(CLOCK_REALTIME, &end);
		char buf[DISCORD_MAX_MESSAGE_LEN] = "";
		long int uptime = end.tv_sec - start.tv_sec;
		long int days = uptime / 86400L;
		long int hours = (uptime - 86400L * days) / 3600L;
		long int minutes = (uptime - 86400L * days - 3600L * hours)/ 60L;
		long int seconds = (uptime - 86400L * days - 3600L * hours - 60L * minutes);
		snprintf(buf, sizeof(buf),
				"Hi! I am a bot written in C.\n"
				"I convert time.\n"
				"**Uptime**: %02ld day(s), %02ld hours, %02ld minutes, %02ld seconds\n"
				, days, hours, minutes, seconds
			);

		struct discord_interaction_response params = {
			.type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
			.data = &(struct discord_interaction_callback_data){
				.content = buf
			}
		};
		discord_create_interaction_response(client, event->id, event->token, &params, NULL);
		return;
	}

	if (strcmp(event->data->name, "time") == 0) {
		if (!event->data || !event->data->options) {
			log_trace("event->data was missing");
			return;	/* user input was missing */
		}
		char* type;
		char* date;
		char* time;
		char* timezone;

		for(int i = 0; i < event->data->options->size; ++i) {
			char* name = event->data->options->array[i].name;
			char* value = event->data->options->array[i].value;

			if (strcmp(name, "type") == 0) {
				type = value;
			}
			if (strcmp(name, "date") == 0) {
				date = value;
			}
			if (strcmp(name, "time") == 0) {
				time = value;
			}
			if (strcmp(name, "timezone") == 0) {
				timezone = value;
			}
		}

		int epoch;
		char errmsg[100];
		int rc = string_to_epoch(date, time, timezone, &epoch, errmsg);
		log_debug("epoch time is: %d", epoch);
		if(rc == -1) {
			log_error("Error in string_to_epoch: %s.", errmsg);
			return; /* something was wrong */
		}

		char buf[DISCORD_MAX_MESSAGE_LEN] = "";

		if( strcmp(type, "short time") == 0 ) {
			snprintf(buf, sizeof(buf), "<t:%d:t>", epoch);
		} else if ( strcmp(type, "long time") == 0) {
			snprintf(buf, sizeof(buf), "<t:%d:T>", epoch);
		} else if ( strcmp(type, "short date") == 0) {
			snprintf(buf, sizeof(buf), "<t:%d:d>", epoch);
		} else if ( strcmp(type, "long date") == 0) {
			snprintf(buf, sizeof(buf), "<t:%d:D>", epoch);
		} else if ( strcmp(type, "long date with short time") == 0) {
			snprintf(buf, sizeof(buf), "<t:%d:f>", epoch);
		} else if ( strcmp(type, "long date with day of week and short time") == 0) {
			snprintf(buf, sizeof(buf), "<t:%d:F>", epoch);
		} else if ( strcmp(type, "relative") == 0) {
			snprintf(buf, sizeof(buf), "<t:%d:R>", epoch);
		}


		struct discord_interaction_response params = {
			.type = DISCORD_INTERACTION_CHANNEL_MESSAGE_WITH_SOURCE,
			.data = &(struct discord_interaction_callback_data){ .content = buf }
		};

		discord_create_interaction_response(client, event->id, event->token,
			&params, NULL);
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
		.description = "Shows info about the bot."
	};
	discord_create_guild_application_command(client, event->application->id, GUILD_ID, &params_info, NULL);

	struct discord_application_command_option_choice timezone_type_choices[] = {
		{
			.name = "CET (Central European Time)",
			.value = "\"CET\"",
		},
		{
			.name = "CEST (Central European Summer Time)",
			.value = "\"CEST\"",
		},
	};

	struct discord_application_command_option_choice time_type_choices[] = {
		{
			.name = "short time (21:23)",
			.value = "\"short time\"",
		},
		{
			.name = "long time (21:23:00)",
			.value = "\"long time\"",
		},
		{
			.name = "short date (2023-01-23)",
			.value = "\"short date\"",
		},
		{
			.name = "long date (23 January 2023)",
			.value = "\"long date\"",
		},
		{
			.name = "long date with short time (23 January 2023 21:23)",
			.value = "\"long date with short time\"",
		},
		{
			.name = "long date with day of week and short time (Monday, 23 January 2023 21:23)",
			.value = "\"long date with day of week and short time\"",
		},
		{
			.name = "relative (In 3 days)",
			.value = "\"relative\"",
		},
	};

	struct discord_application_command_option options[] = {
		{
			.type = DISCORD_APPLICATION_OPTION_STRING,
			.name = "type",
			.description = "Time type/format",
			.required = true,
			.choices =
				&(struct discord_application_command_option_choices) {
					.size = sizeof(time_type_choices) / sizeof *time_type_choices,
					.array = time_type_choices,
				}
		},
		{
			.type = DISCORD_APPLICATION_OPTION_STRING,
			.name = "date",
			.required = true,
			.description = "date (2020-01-01)",
		},
		{
			.type = DISCORD_APPLICATION_OPTION_STRING,
			.name = "time",
			.required = true,
			.description = "time (15:34)",
		},
		{
			.type = DISCORD_APPLICATION_OPTION_STRING,
			.name = "timezone",
			.description = "timezone",
			.required = true,
			.choices =
				&(struct discord_application_command_option_choices) {
					.size = sizeof(timezone_type_choices) / sizeof *timezone_type_choices,
					.array = timezone_type_choices,
				}
		},
	};

	struct discord_create_guild_application_command params = {
		.name = "time",
		.description = "A command with the slash thing",
		.default_permission = true,
		.options =
			&(struct discord_application_command_options) {
				.size = sizeof(options) / sizeof *options,
				.array = options,
			},

	};
	discord_create_guild_application_command(client, event->application->id, GUILD_ID, &params, NULL);
}

int main(int argc, char** argv)
{
	const char*	config_file;

	if (argc > 1) {
		config_file = argv[1];
	} else {
		config_file = "./config.json";
	}

	clock_gettime(CLOCK_REALTIME, &start);

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
