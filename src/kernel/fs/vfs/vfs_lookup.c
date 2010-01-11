/*
 * kernel/fs/vfs/vfs_lookup.c
 *
 * Copyright (c) 2007-2009  jianjun jiang <jerryjianjun@gmail.com>
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
#include <error.h>
#include <xboot/panic.h>
#include <xboot/initcall.h>
#include <fs/fs.h>
#include <fs/vfs/vfs.h>


/*
 * convert a full path name into a pointer to a locked vnode.
 */
x_s32 namei(char * path, struct vnode ** vpp)
{
	char *p;
	char node[MAX_PATH];
	char name[MAX_PATH];
	struct mount * mp;
	struct vnode * dvp, * vp;
	x_s32 error, i;

	/*
	 * convert a full path name to its mount point and
	 * the local node in the file system.
	 */
	if(vfs_findroot(path, &mp, &p))
		return ENOTDIR;

	strlcpy((x_s8 *)node, (const x_s8 *)"/", sizeof(node));
	strlcat((x_s8 *)node, (const x_s8 *)p, sizeof(node));
	vp = vn_lookup(mp, node);
	if(vp)
	{
		/* vnode is already active */
		*vpp = vp;
		return 0;
	}

	/*
	 * find target vnode, started from root directory.
	 * this is done to attach the fs specific data to
	 * the target vnode.
	 */
	if((dvp = mp->m_root) == NULL)
		panic("vfs: no root\r\n");

	vref(dvp);
	vn_lock(dvp);
	node[0] = '\0';

	while(*p != '\0')
	{
		/*
		 * get lower directory or file name.
		 */
		while(*p == '/')
			p++;

		for(i = 0; i < MAX_PATH; i++)
		{
			if(*p == '\0' || *p == '/')
				break;
			name[i] = *p++;
		}
		name[i] = '\0';

		/*
		 * get a vnode for the target.
		 */
		strlcat((x_s8 *)node, (const x_s8 *)"/", sizeof(node));
		strlcat((x_s8 *)node, (const x_s8 *)name, sizeof(node));
		vp = vn_lookup(mp, node);
		if(vp == NULL)
		{
			vp = vget(mp, node);
			if(vp == NULL)
			{
				vput(dvp);
				return ENOMEM;
			}

			/*
			 * find a vnode in this directory.
			 */
			error = dvp->v_op->vop_lookup(dvp, name, vp);
			if(error || (*p == '/' && vp->v_type != VDIR))
			{
				/* not found */
				vput(vp);
				vput(dvp);
				return error;
			}
		}

		vput(dvp);
		dvp = vp;
		while(*p != '\0' && *p != '/')
			p++;
	}

	*vpp = vp;

	return 0;
}

/*
 * search a pathname.
 * this is a very central but not so complicated routine.
 *
 * @path: full path.
 * @vpp:  pointer to locked vnode for directory.
 * @name: pointer to file name in path.
 */
x_s32 lookup(char * path, struct vnode ** vpp, char ** name)
{
	char buf[MAX_PATH];
	char root[] = "/";
	char *file, *dir;
	struct vnode * vp;
	x_s32 error;

	/*
	 * get the path for directory.
	 */
	strlcpy((x_s8 *)buf, (const x_s8 *)path, sizeof(buf));
	file = (char *)strrchr((const x_s8 *)buf, '/');

	if(!buf[0])
		return ENOTDIR;
	if(file == buf)
		dir = root;
	else
	{
		*file = '\0';
		dir = buf;
	}

	/*
	 * get the vnode for directory
	 */
	if((error = namei(dir, &vp)) != 0)
		return error;
	if (vp->v_type != VDIR)
	{
		vput(vp);
		return ENOTDIR;
	}
	*vpp = vp;

	/*
	 * get the file name
	 */
	*name = (char *)strrchr((const x_s8 *)path, '/') + 1;

	return 0;
}
