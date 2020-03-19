#if !defined(SECUROM_3945862039487562038947562938475)
#define SECUROM_3945862039487562038947562938475


class TriggerHolder
{
public:
	virtual void Init() = 0;
	virtual void Shut() = 0;
	static TriggerHolder* GetInst();
};

#endif