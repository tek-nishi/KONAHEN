
#pragma once

//
// 簡易タスク
//

#include <vector>
#include <map>
#include <memory>
#include <functional>


namespace ngs {

class TaskProc {
public:
	virtual ~TaskProc() {}

	virtual void step(const float delta_time) = 0;
	virtual void draw() = 0;
	virtual void msg(const int msg) = 0;
	virtual bool active() const = 0;
};


class Task {
public:
	typedef std::tr1::shared_ptr<TaskProc> ProcPtr;

private:
	std::map<int, std::vector<ProcPtr> > procs_;
	std::map<int, std::vector<ProcPtr> > adds_;

	struct cleanupProc {
		bool operator()(const ProcPtr proc) const {
			return !proc->active();
		}
	};

	void clean()
	{
		for(std::map<int, std::vector<ProcPtr> >::iterator it = procs_.begin(); it != procs_.end(); ++it)
		{
			std::vector<ProcPtr>& proc = it->second;
			std::vector<ProcPtr>::iterator end = std::remove_if(proc.begin(), proc.end(), cleanupProc());
			proc.erase(end, proc.end());
			// TIPS:↑remove_ifはコンテナのサイズを変えないので、こう実装する
		}
	}

public:
	void step(const float delta_time)
	{
		if(!adds_.empty())
		{
			for(std::map<int, std::vector<ProcPtr> >::iterator it = adds_.begin(); it != adds_.end(); ++it)
			{
				std::vector<ProcPtr>& add = it->second;
				std::copy(add.begin(), add.end(), std::back_inserter(procs_[it->first]));
			}
			adds_.clear();
		}
		// 前回のstep()で生成されたタスクはここで追加(いきなりdraw()を実行させない)

		for(std::map<int, std::vector<ProcPtr> >::iterator it = procs_.begin(); it != procs_.end(); ++it)
		{
			std::vector<ProcPtr>& proc = it->second;
			std::for_each(proc.begin(), proc.end(), std::tr1::bind(&TaskProc::step, std::tr1::placeholders::_1, delta_time));
		}
		this->clean();
		// 終了したタスクはここで後始末(draw()を実行させない)
	}

	void draw()
	{
		for(std::map<int, std::vector<ProcPtr> >::iterator it = procs_.begin(); it != procs_.end(); ++it)
		{
			std::vector<ProcPtr>& proc = it->second;
			std::for_each(proc.begin(), proc.end(), std::tr1::bind(&TaskProc::draw, std::tr1::placeholders::_1));
		}
	}

	void sendMsgAll(const int msg)
	{
		for(std::map<int, std::vector<ProcPtr> >::iterator it = procs_.begin(); it != procs_.end(); ++it)
		{
			std::vector<ProcPtr>& proc = it->second;
			std::for_each(proc.begin(), proc.end(), std::tr1::bind(&TaskProc::msg, std::tr1::placeholders::_1, msg));
		}

		for(std::map<int, std::vector<ProcPtr> >::iterator it = adds_.begin(); it != adds_.end(); ++it)
		{
			std::vector<ProcPtr>& proc = it->second;
			std::for_each(proc.begin(), proc.end(), std::tr1::bind(&TaskProc::msg, std::tr1::placeholders::_1, msg));
		}
		// ↑生成されたタスクにもメッセージを飛ばしてやる
	}

	void dispTaskList() const
	{
		DOUT << "TaskLists" << std::endl;
		int total = 0;
		for(std::map<int, std::vector<ProcPtr> >::const_iterator it = procs_.begin(); it != procs_.end(); ++it)
		{
			int num = it->second.size();
			total += num;
			DOUT << it->first << ":" << num << std::endl;
		}
		DOUT << "Total:" << total << std::endl;
	}

	
	template<typename T, typename ARV> ProcPtr add(const int prio, ARV& arv)
	{
		ProcPtr ptr(new T(arv));
		std::vector<ProcPtr>& add = adds_[prio];
		add.push_back(ptr);
		return ptr;
	}
	
	template<typename T, typename ARG_L, typename ARG_R> ProcPtr add(const int prio, ARG_L& argl, ARG_R& argr)
	{
		ProcPtr ptr(new T(argl, argr));
		std::vector<ProcPtr>& add = adds_[prio];
		add.push_back(ptr);
		return ptr;
	}
	
	template<typename T, typename ARG_L, typename ARG_M, typename ARG_R> ProcPtr add(const int prio, ARG_L& argl, ARG_M& argm, ARG_R& argr)
	{
		ProcPtr ptr(new T(argl, argm, argr));
		std::vector<ProcPtr>& add = adds_[prio];
		add.push_back(ptr);
		return ptr;
	}
};

}

