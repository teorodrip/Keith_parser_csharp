/* ************************************************************************** */
/*                                                                            */
/*                                                                            */
/*   invert_bytes.c                                                           */
/*                                                                            */
/*   By: Mateo <teorodrip@protonmail.com>                                     */
/*                                                                            */
/*   Created: 2019/01/08 15:27:43 by Mateo                                    */
/*   Updated: 2019/01/08 15:28:15 by Mateo                                    */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/server.h"

static void generic_case(void *arr, const size_t size)
{
	char 	*c_arr;
	char 	aux;
	size_t	i;

	c_arr = (char *)arr;
	i = 0;
	while (i < size / 2)
	{
		aux = c_arr[i];
		c_arr[i] = c_arr[size - i -1];
		c_arr[size - i -1] = aux;
		i++;
	}
}

void		invert_bytes(void *arr, const size_t size)
{
	if (size == 1)
		return ;
	else if (size == 2)
		*((uint16_t *)arr) = (((*((uint16_t *)arr) >> 8) & 0x00FF) |
		                      ((*((uint16_t *)arr) << 8) & 0xFF00));
	else if (size == 4)
		*((uint32_t *)arr) = (((*((uint32_t *)arr) >> 24) & 0x000000FF) |
		                      ((*((uint32_t *)arr) >> 8) & 0x0000FF00) |
		                      ((*((uint32_t *)arr) << 8) & 0x00FF0000) |
		                      ((*((uint32_t *)arr) << 24) & 0xFF000000));
	else if (size == 8)
		*((uint64_t *)arr) =
			((((*((uint64_t *)arr) >> 56) & 0x00000000000000FF)) |
		    (((*((uint64_t *)arr) >> 40) & 0x000000000000FF00)) |
            (((*((uint64_t *)arr) >> 24) & 0x0000000000FF0000)) |
            (((*((uint64_t *)arr) >> 8) & 0x00000000FF000000)) |
            (((*((uint64_t *)arr) << 8) & 0x000000FF00000000)) |
            (((*((uint64_t *)arr) << 24) & 0x0000FF0000000000)) |
            (((*((uint64_t *)arr) << 40) & 0x00FF0000000000)) |
            (((*((uint64_t *)arr) << 56) & 0xFF000000000000)));
	else
		generic_case(arr, size);
}