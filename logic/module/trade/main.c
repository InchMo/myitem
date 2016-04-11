#include "/rc/rpc/trade.h"
#include <user_key.h>
#include <module.h>
#include <var_prop.h>

#define TRADEORDER_OWNEROBJECT(data) data["ownerobject"]
#define TRADEORDER_OTHERUID(data)	data["otheruid"]
#define TRADEORDER_OWNERCAR(data)	data["ownercar"]
#define TRADEORDER_OTHERCAR(data)	data["othercar"]
#define TRADEORDER_OWNERLOCK(data)	data["ownerlock"]
#define TRADEORDER_OTHERLOCK(data)	data["otherlock"]

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
	{
		rpc_client_require_trade(passiver_uid, activer_user->GetId(), activer_user->GetName());
		return ;
	}
	mapping op="/module/internal_call"->PackCallOp(__FILE__, "remote_require_trade", passiver_uid,  activer_user->GetId(), activer_user->GetName());
	"/module/internal_call"->LogicServerCallByUid(passiver_uid, op, 0);
}

void remote_cancel_trade(int activer_uid, int result) 
{
	logger->Log(activer_uid, "trade.main.remote_cancel_trade", "uid:[%d] require trade result:%d", activer_uid,result);
	rpc_client_require_trade_result(activer_uid, result);
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
	mapping map={};
	map=TradeOrder[uid];
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
	rpc_client_modified_tradecar( uid, var);
	debug_message("=====uid:%O's=====",uid);
	debug_message("=====ownercar:%O's=====",map["ownercar"]);
	debug_message("=====othercar:%O's=====",map["othercar"]);
}

void modified_tradecar(int uid, modified_tradecar_t data)
{
	mapping map={};
	mapping m={};
	m["op"]=data->op;
	m["type"]=data->type;
	m["count"]=data->count;
	map=TradeOrder[uid];

	if(m["op"]==-1)
	{
		if(map["ownercar"][m["tpye"]]>=m["count"])
		{
			map["ownercar"][m["type"]]=map["ownercar"][m["type"]]-m["count"];
		}
		else
		{}
	}
	if(m["op"]==1)
	{
		map["ownercar"][m["type"]]=map["ownercar"][m["type"]]+m["count"];
	}
	mapping op="module/internal_call"->PackCallOp(__FILE__, "remote_modified_tradecar", map["otheruid"], m);
	"module/internal_call"->LogicServerCallByUid( map["otheruid"], op, 0);
	debug_message("=====uid:%O's=====",uid);
	debug_message("=====ownercar:%O's=====",map["ownercar"]);
	debug_message("=====othercar:%O's=====",map["othercar"]);
}

void tradecar_distributing(object user, mapping map)
{
	string key,value;
	int mount;
	foreach(key,value in map)
	{
			mount=atoi(value);
			if (key == I_USER_GOLD)
			{
				ModUtil->AddUserGold(user, mount);
				CmdUser->SendGoldInfo(user);
				user->TellMe(0, T("金钱%d"),mount);
			}
			if(key == I_USER_FOOD)
			{
				ModUtil->AddUserFood(user, mount);
				CmdUser->SendFoodInfo(user);
				user->TellMe(0, T("粮食%d"), mount);
			}
			if (key == I_USER_BD_YUANBAO)
			{
				ModUtil->AddUserBdYuanbao(user, mount);
				CmdUser->SendYuanbaoInfo(user);
				user->TellMe(0, T("元宝%d"), mount);
			}
			if (key == I_USER_UB_YUANBAO)
			{
				ModUtil->AddUserUbYuanbao(user, mount);
				CmdUser->SendYuanbaoInfo(user);
				user->TellMe(0, T("元宝%d"), mount);
			}
	}
}
void remote_attain_trade(int uid, string tradecar)
{
	debug_message("=========remote_attain_trade======");
	//object user = get_user( uid );
	//user->AddGold(tradecar->gold);
	//user->AddFood(tradecar->food);
	//"cmd/user"->SendGoldInfo(user);
	//"cmd/user"->SendFoodInfo(user);
}

void attain_trade(int uid, string tradecar)
{
	object user=get_user(uid);
	mapping mp_tradecar;
	mp_tradecar=str2map(tradecar);
	tradecar_distributing(user,mp_tradecar);
}

void agree_trade(object user, int target_id, string tradecar)
{
	//debug_message("=========tradecar:%O======",typeof(tradecar));
	object target_user=get_user(target_id);
	if(!objectp(target_user))
	{
		mapping op="module/internal_call"->PackCallOp(__FILE__, "remote_attain_trade", target_id, tradecar);
		"module/internal_call"->LogicServerCallByUid(target_id, op, 0);
		debug_message("========!objectp(target_user)=======");
		return ;
	}
	debug_message("========sure_trade=======");
	attain_trade(target_id, tradecar);
}

void create()
{
	logger=ModLog->New("trade");	
}
