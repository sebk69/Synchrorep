/*
 * tc_thread_utils.h
 *
 *  Created on: 20 déc. 2009
 *      This file is a part of synchrorep under GPL V3 licence
 *      ©2009, 2019 - Sébastien Kus
 *
 *  Mutexing types and class for multi-tread using
 *
 */

#ifndef TC_THREAD_UTILS_H_
#define TC_THREAD_UTILS_H_

#include <gtk/gtk.h>

// simple var exchanges with mutex
template <class T>
class mutex_var
{
private:
	T			var;
	GMutex		*mutex;

public:
	mutex_var()
	{
		if(!g_thread_supported())
			g_thread_init(NULL);
		mutex = g_mutex_new();
	};

	~mutex_var()
	{
		g_mutex_free(this->mutex);
	}

	T
	get()
	{
		T			result;
		while(!g_mutex_trylock(this->mutex));
		result = this->var;
		g_mutex_unlock(this->mutex);
		return result;
	}

	void
	set(T new_value)
	{
		while(!g_mutex_trylock(this->mutex));
		this->var = new_value;
		g_mutex_unlock(this->mutex);
	}
};

template <class Class, class Result, class Data>
class dialog_mutex : Class
{
private:
	GMutex				*ask_mutex;
	GCond				*answer_cond;
	GMutex				*data_mutex;
	Data				data;

public:
						dialog_mutex(GtkWidget *parent) : Class(parent)
	{
		this->ask_mutex = g_mutex_new();
		this->answer_cond = g_cond_new();
		this->data_mutex = g_mutex_new();
	}
						~dialog_mutex()
	{
		g_mutex_free(this->ask_file_conflict_mutex);
		g_cond_free(this->answer_cond);
		g_mutex_free(this->data_mutex);
	}

	// process thread ask other tread to draw dialog
	Result				asking(Data data)
	{
		this->data = data;

		// lock to tell other tread to work
		while(!g_mutex_trylock(this->ask_mutex));
		// wait until user answer
		GMutex			*temp_mutex = g_mutex_new();
		g_mutex_trylock(temp_mutex);
		g_cond_wait(this->answer_cond, temp_mutex);
		g_mutex_unlock(temp_mutex);

		// return dialog result
		Result			result;
		while(!g_mutex_trylock(this->data_mutex));
		result = this->choosen;
		g_mutex_unlock(this->data_mutex);

		return result;
	}

	// user interface thread is asked to draw dialog
	void				asked()
	{
		if(!g_mutex_trylock(this->ask_mutex))
		{
			this->Class :: asking(this->data);
			g_cond_signal(this->answer_cond);
		}

		g_mutex_unlock(this->ask_mutex);
	}

	void				no_confirmation(Result default_answer)
	{
		while(!g_mutex_trylock(this->data_mutex));
		this->Class :: no_confirmation(default_answer);
		g_mutex_unlock(this->data_mutex);
	}
};

#endif /* TC_THREAD_UTILS_H_ */
