/*
 * kernel/vision/inrange.c
 *
 * Copyright(c) 2007-2022 Jianjun Jiang <8192542@qq.com>
 * Official site: http://xboot.org
 * Mobile phone: +86-18665388956
 * QQ: 8192542
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include <xboot.h>
#include <vision/inrange.h>

struct vision_t * vision_inrange(struct vision_t * v, float * l, float * h)
{
	if(v && (v->type == VISION_TYPE_HSV))
	{
		struct vision_t * mask = vision_alloc(VISION_TYPE_GRAY, vision_get_width(v), vision_get_height(v));
		if(mask)
		{
			unsigned char * pmask = (unsigned char *)mask->datas;
			float * ph = &((float *)v->datas)[v->npixel * 0];
			float * ps = &((float *)v->datas)[v->npixel * 1];
			float * pv = &((float *)v->datas)[v->npixel * 2];
			for(int i = 0; i < v->npixel; i++, pmask++, ph++, ps++, pv++)
			{
				if((*ph < l[0]) || (*ph > h[0]) || (*ps < l[1]) || (*ps > h[1]) || (*pv < l[2]) || (*pv > h[2]))
					*pmask = 0;
				else
					*pmask = 255;
			}
			return mask;
		}
	}
	return NULL;
}
