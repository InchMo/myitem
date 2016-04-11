#include "/rc/rpc/trade.h"
#include <user_key.h>
#include <module.h>
#include <var_prop.h>

#define TRADEORDER_OWNEROBJECT(data)		data["ownerobject"]
#define TRADEORDER_OTHERUID(data)			data["otheruid"]
#define TRADEORDER_OWNERCAR(data)			data["ownercar"]
#define TRADEORDER_OTHERCAR(data)			data["othercar"]
#define TRADEORDER_OWNERLOCK(data)			data["ownerlock"]
#define TRADEORDER_OTHERLOCK(data)			data["otherlock"]
#define TRADEORDER_OWNERSURETRADE(data)		data["ownersuretrade"]
#define TRADEORDER_OTHERSURETRADE(data)		data["othersuretrade"]
#define TRADEORDER_OWNERCANCELTRADE(data)	data["ownercanceltrade"]
#define TRADEORDER_OTHERCANCELTRADE(data)	data["othercanceltrade"]

//MEMORY_VAR(TradeOrder,{}) 
mapping TradeOrder={};

static object logger;

int IsExistTradeOrder(mixed key)
{
	return !undefinedp(TradeOrder[key]);
}

mapping str2map(string str)
{
	mapping map = {};
	string *arr = explode(str,",");
	debug_message("====%O=====",arr[<1]);
	if(arr[<1]=="")
	arr=arr[0..<1];
	string part,key,value; 
	foreach( part in arr)
	{
		sscanf(part,"%s:%s",key,value);	
		map[key]=value;
	}
	return map; 
}


void require_trade(object activer_user, int passiver_uid)
{
	logger->Log(activer_user->GetId(), "trade.main.activer_require_trade", "uid:[%d] require trade to uid:[%d]", activer_user->GetId(), passiver_uid);
	object passiver_user=get_user(passiver_uid);	
	if(objectp(passiver_user))
	{ rpc_client_require_trade(passiver_uid, activer_user->GetId(), activer_user->GetName());
		return ;
	}
	mapping op="/module/internal_call"->PackCallOp(__FILE__, "remote_require_trade", passiver_uid,  activer_user->GetId(), activer_user->GetName());
	"/module/internal_call"->LogicServerCallByUid(passiver_uid, op, 0);
}

void remote_other_cancel_trade(int uid)
{
	rpc_client_cancel_trade(uid);
	map_delete(TradeOrder, uid);
}

void cancel_trade(object user)
{
	int owner_uid=user->GetId();
	mapping owner_map=TradeOrder[owner_uid];
	int other_uid=TRADEORDER_OTHERUID(owner_map);
	object other_user=get_user(other_uid);
	map_delete(TradeOrder,owner_uid);
	if(objectp(other_user))
	{
		rpc_client_cancel_trade(owner_uid);
		map_delete(TradeOrder,other_uid);
		return ;
	}
	mapping op="/module/internal_call"->PackCallOp(__FILE__, "remote_other_cancel_trade",other_uid);
	"/module/internal_call"->LogicServerCallByUid(other_uid, op, 0);
}

void remote_require_trade_result(int activer_uid, int passiver_uid, int result) 
{
	logger->Log(activer_uid, "trade.main.remote_require_trade_result", "uid:[%d] require trade result:%d", activer_uid,result);
	object activer_user=get_user(activer_uid);
	mapping map={};
	if(result==1)
	{
		if(!IsExistTradeOrder(activer_uid))
		{
			TradeOrder[activer_uid]={};
			map=TradeOrder[activer_uid];
			TRADEORDER_OWNEROBJECT(map)=activer_user;
			TRADEORDER_OTHERUID(map)=passiver_uid;
			TRADEORDER_OWNERCAR(map)={};
			TRADEORDER_OTHERCAR(map)={};
		}
	}
	rpc_client_require_trade_result(activer_uid, result);
}


void remote_require_trade(int passiver_uid, int activer_uid, string activer_name)
{
	object passiver_user=get_user(passiver_uid);
	if(!objectp(passiver_user))
	{
		int result=-1;
		mapping op="module/internal_call"->PackCallOp(__FILE__, "remote_cancel_trade", activer_uid, result);
		"module/internal_call"->LogicServerCallByUid(activer_uid, op, 0);
	}
	rpc_client_require_trade(passiver_uid, activer_uid, activer_name);
} 


void require_trade_result(object passiver_user, int activer_uid, int result)
{
	mapping map={};
	object activer_user=get_user(activer_uid);
	int passiver_uid=passiver_user->GetId();
	if(result==1)
	{
		if(!IsExistTradeOrder(passiver_uid))
		{
			TradeOrder[passiver_uid]={};
			map=TradeOrder[passiver_uid];
			TRADEORDER_OWNEROBJECT(map)=passiver_user;
			TRADEORDER_OTHERUID(map)=activer_uid;
			TRADEORDER_OWNERCAR(map)={};
			TRADEORDER_OTHERCAR(map)={};
		}
	}
	if(objectp(activer_user)&&(result==1))
	{
			if(!IsExistTradeOrder(activer_uid))
			{
				TradeOrder[activer_uid]={};
				map=TradeOrder[activer_uid];
				TRADEORDER_OWNEROBJECT(map)=activer_user;
				TRADEORDER_OTHERUID(map)=passiver_uid;
				TRADEORDER_OWNERCAR(map)={};
				TRADEORDER_OTHERCAR(map)={};
			}
		logger->Log(activer_user->GetId(), "trade.main.passiver_require_trade", "uid:[%d] require trade result:%d", activer_user->GetId(),result);
		rpc_client_require_trade_result(activer_uid, result);
		return ;
	}
	mapping op="module/internal_call"->PackCallOp(__FILE__, "remote_require_trade_result", activer_uid, passiver_uid, result);
	"module/internal_call"->LogicServerCallByUid(activer_uid, op, 0);
}

void remote_modified_tradecar(int uid, mapping m)
{
	modified_tradecar_t var = new modified_tradecar_t;
	mapping map=TradeOrder[uid];
	if(m["op"]==-1)
	{
		if(map["othercar"][m["type"]]>=m["count"])
		{
			map["othercar"][m["type"]]=map["othercar"][m["type"]]-m["count"];
		}
		else
		{}
	}
	if(m["op"]==1)
	{
		map["othercar"][m["type"]]=map["othercar"][m["type"]]+m["count"];
	}
	var->op=m["op"];
	var->type=m["type"];
	var->count=m["count"];
	rpc_client_other_modified_tradecar( uid, var);
	debug_message("=====uid:%O's=====",uid);
	debug_message("=====ownercar:%O's=====",map["ownercar"]);
	debug_message("=====othercar:%O's=====",map["othercar"]);
}

void modified_tradecar(int uid, modified_tradecar_t data)
{
	mapping owner_map=TradeOrder[uid];
	int other_uid=TRADEORDER_OTHERUID(owner_map);
	object other_user=get_user(other_uid);
	mapping m={};
	m["op"]=data->op;
	m["type"]=data->type;
	m["count"]=data->count;

	if(m["op"]==-1)
	{
		if(owner_map["ownercar"][m["tpye"]]>=m["count"])
		{
			owner_map["ownercar"][m["type"]]=owner_map["ownercar"][m["type"]]-m["count"];
		}
		else
		{}
	}
	if(m["op"]==1)
	{
		owner_map["ownercar"][m["type"]]=owner_map["ownercar"][m["type"]]+m["count"];
	}
	if(objectp(other_user))
	{
		modified_tradecar_t var = new modified_tradecar_t;
		mapping other_map=TradeOrder[other_uid];
		if(m["op"]==-1)
		{
			if(other_map["othercar"][m["tpye"]]>=m["count"])
			{
				other_map["othercar"][m["type"]]=other_map["othercar"][m["type"]]-m["count"];
			}
			else
			{}
		}
		if(m["op"]==1)
		{
			other_map["othercar"][m["type"]]=other_map["othercar"][m["type"]]+m["count"];
		}
		var->op=m["op"];
		var->type=m["type"];
		var->count=m["count"];
		rpc_client_other_modified_tradecar( other_uid, var);
		return ;
	}
	mapping op="module/internal_call"->PackCallOp(__FILE__, "remote_modified_tradecar", owner_map["otheruid"], m);
	"module/internal_call"->LogicServerCallByUid( owner_map["otheruid"], op, 0);
	debug_message("=====uid:%O's=====",uid);
	debug_message("=====ownercar:%O's=====",owner_map["ownercar"]);
	debug_message("=====othercar:%O's=====",owner_map["othercar"]);
}

void remote_other_lock_tradecar(int uid)
{
	debug_message("=====remote_other_lock_tradecar()====");
	mapping map=TradeOrder[uid];
	TRADEORDER_OTHERLOCK(map)=1;
	rpc_client_other_lock_tradecar(uid);
}

void lock_tradecar(object user)
{
	debug_message("====lock_tradecar()====");
	mapping ownermap=TradeOrder[user->GetId()];
	int other_uid=TRADEORDER_OTHERUID(ownermap);
	object other_user=get_user(other_uid);

	TRADEORDER_OWNERLOCK(ownermap)=1;

	if(!objectp(other_user))
	{
		mapping op="module/internal_call"->PackCallOp(__FILE__, "remote_other_lock_tradecar",other_uid);
		"module/internal_call"->LogicServerCallByUid(other_uid, op, 0);
		return ;
	}	
	mapping othermap=TradeOrder[other_uid];
	TRADEORDER_OTHERLOCK(othermap)=1;
	rpc_client_other_lock_tradecar(other_uid);
}

void attain_trade(object user)
{
	debug_message("====attain_trade()====");

	mapping map=TradeOrder[user->GetId()];
	mapping ownercar=TRADEORDER_OWNERCAR(map);
	mapping othercar=TRADEORDER_OTHERCAR(map);
	string key;
	int value;
	foreach(key,value in ownercar)
	{
			if (key == I_USER_GOLD)
			{
				ModUtil->SubUserGold(user, value);
				user->TellMe(0, T("金钱%d"), value);
			}
			if(key == I_USER_FOOD)
			{
				ModUtil->SubUserFood(user, value);
				user->TellMe(0, T("粮食%d"), value);
			}
			if (key == I_USER_BD_YUANBAO)
			{
				ModUtil->SubUserBdYuanbao(user, value);
				user->TellMe(0, T("元宝%d"), value);
			}
			if (key == I_USER_UB_YUANBAO)
			{
				ModUtil->SubUserUbYuanbao(user, value);
				user->TellMe(0, T("元宝%d"), value);
			}
	}
	foreach(key,value in othercar)
	{
			if (key == I_USER_GOLD)
			{
				ModUtil->AddUserGold(user, value);
				user->TellMe(0, T("金钱%d"), value);
			}
			if(key == I_USER_FOOD)
			{
				ModUtil->AddUserFood(user, value);
				user->TellMe(0, T("粮食%d"), value);
			}
			if (key == I_USER_BD_YUANBAO)
			{
				ModUtil->AddUserBdYuanbao(user, value);
				user->TellMe(0, T("元宝%d"), value);
			}
			if (key == I_USER_UB_YUANBAO)
			{
				ModUtil->AddUserUbYuanbao(user, value);
				user->TellMe(0, T("元宝%d"), value);
			}
	}
}

void remote_other_sure_trade(int uid)
{
	debug_message("====remote_sure_trade()====");

	mapping map=TradeOrder[uid];
	TRADEORDER_OTHERSURETRADE(map)=1;
	if(TRADEORDER_OWNERSURETRADE(map))
	{
		rpc_client_success_trade(uid);
		attain_trade(get_user(uid));
	}
}

void sure_trade(object user)
{
	debug_message("=====sure_trade()====");

	mapping owner_map=TradeOrder[user->GetId()];
	int other_uid=TRADEORDER_OTHERUID(owner_map)
	object other_user=get_user(other_uid);
	TRADEORDER_OWNERSURETRADE(owner_map)=1;

	if(TRADEORDER_OTHERSURETRADE(owner_map))
	{
		rpc_client_success_trade(user->GetId());
		attain_trade(user);

	}		

	if(objectp(other_user))
	{
		mapping other_map=TradeOrder[other_uid];
		TRADEORDER_OTHERSURETRADE(other_map)=1;
		if(TRADEORDER_OWNERSURETRADE(other_map))
			rpc_client_success_trade(other_uid);
		return ;
	}

	mapping op="module/internal_call"->PackCallOp(__FILE__, "remote_other_sure_trade", other_uid);
	"module/internal_call"->LogicServerCallByUid(other_uid, op, 0);
}

void create()
{
	logger=ModLog->New("trade");	
}
