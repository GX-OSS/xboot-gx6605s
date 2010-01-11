/*
 * kernel/command/cmd_help.c
 *
 *
 * Copyright (c) 2007-2008  jianjun jiang <jjjstudio@gmail.com>
 * website: http://xboot.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <configs.h>
#include <default.h>
#include <types.h>
#include <string.h>
#include <malloc.h>
#include <xboot/log.h>
#include <xboot/list.h>
#include <xboot/printk.h>
#include <xboot/initcall.h>
#include <shell/command.h>


#if	defined(CONFIG_COMMAND_HELP) && (CONFIG_COMMAND_HELP > 0)

extern struct command_list * command_list;

/**
 * the entry of help command.
 */
static x_s32 help(x_s32 argc, const x_s8 **argv)
{
	struct command_list * list;
	struct list_head * pos;
	struct command ** cmd_array;
	struct command * cmd;
	x_s32 i = 0, j, k = 0, cmd_num, swaps;

	if(argc == 1)
	{
		i = 0;
		cmd_num = command_number();
		cmd_array = malloc(sizeof(struct command *) * cmd_num);

		if(!cmd_array)
		{
			printk("malloc command array fail for sort command list\r\n");
			return -1;
		}

		for(pos = (&command_list->entry)->next; pos != (&command_list->entry); pos = pos->next)
		{
			list = list_entry(pos, struct command_list, entry);
			cmd_array[i++] = list->cmd;
			j = strlen((x_s8*)list->cmd->name);
			if(j > k)	k = j;
		}

		/* sort command list (trivial bubble sort) */
		for (i = cmd_num - 1; i > 0; --i)
		{
			swaps = 0;
			for(j=0; j<i; ++j)
			{
				if (strcmp((x_s8*)cmd_array[j]->name, (x_s8*)cmd_array[j + 1]->name) > 0)
				{
					cmd = cmd_array[j];
					cmd_array[j] = cmd_array[j + 1];
					cmd_array[j + 1] = cmd;
					++swaps;
				}
			}
			if(!swaps)
				break;
		}

		/* display online help (desc) */
		for (i=0; i<cmd_num; i++)
		{
			printk(" %s%*s - %s",cmd_array[i]->name, k-strlen((x_s8*)cmd_array[i]->name), "", cmd_array[i]->desc);
		}
		free(cmd_array);
	}
	else
	{
		for (i=1; i<argc; ++i)
		{
			if( (cmd = command_search((char*)argv[i])) != NULL )
			{
				printk("%s - %s" "usage: %s" "help:\r\n" "%s", cmd->name, cmd->desc, cmd->usage, cmd->help);
			}
			else
			{
				printk("unknown command '%s' - try 'help' without arguments for list of all known commands\r\n", argv[i]);
			}
		}
	}

	return 0;
}

static struct command help_cmd = {
	.name		= "help",
	.func		= help,
	.desc		= "display online help\r\n",
	.usage		= "help [command ...]\r\n",
	.help		= "    show help information (for 'command')\r\n"
				  "    'help' display online help for the monitor commands.\r\n"
				  "    to get detailed help information for specific commands you can type 'help' with one or more command names as arguments.\r\n"
};

static __init void help_cmd_init(void)
{
	if(!command_register(&help_cmd))
		LOG_E("register 'help' command fail");
}

static __exit void help_cmd_exit(void)
{
	if(!command_unregister(&help_cmd))
		LOG_E("unregister 'help' command fail");
}

module_init(help_cmd_init, LEVEL_COMMAND);
module_exit(help_cmd_exit, LEVEL_COMMAND);

#endif
