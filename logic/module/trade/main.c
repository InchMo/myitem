#include "/rc/rpc/trade.h"
#include <user_key.h>
#include <module.h>

static object logger;

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


void activer_require_trade(object activer_user, int passiver_uid)
{
	logger->Log(activer_user->GetId(), "trade.main.activer_require_trade", "uid:[%d] require trade to uid:[%d]", activer_user->GetId(), passiver_uid);
	object passiver_user=get_user(passiver_uid);	
	if(objectp(passiver_user))
	{
		rpc_client_passiver_require_trade(passiver_uid, activer_user->GetId(), activer_user->GetName());
		return ;
	}
	mapping op="/module/internal_call"->PackCallOp(__FILE__, "remote_activer_require_trade", passiver_uid,  activer_user->GetId(), activer_user->GetName());
	"/module/internal_call"->LogicServerCallByUid(passiver_uid, op, 0);
}

void remote_passiver_cancel_trade(int activer_uid, int result) 
{
	logger->Log(activer_uid, "trade.main.remote_passiver_cancel_trade", "uid:[%d] require trade result:%d", activer_uid,result);
	rpc_client_activer_require_trade(activer_uid, result);
}

void remote_passiver_require_trade(int activer_uid, int result) 
{
	logger->Log(activer_uid, "trade.main.remote_passiver_require_trade", "uid:[%d] require trade result:%d", activer_uid,result);
	rpc_client_activer_require_trade(activer_uid, result);
}


void remote_activer_require_trade(int passiver_uid, int activer_uid, string activer_name)
{
	object passiver_user=get_user(passiver_uid);
	if(!objectp(passiver_user))
	{
		int result=-1;
		mapping op="module/internal_call"->PackCallOp(__FILE__, "remote_passiver_cancel_trade", activer_uid, result);
		"module/internal_call"->LogicServerCallByUid(activer_uid, op, 0);
	}
	rpc_client_passiver_require_trade(passiver_uid, activer_uid, activer_name);
} 


void passiver_require_trade(object passiver_user, int activer_uid, int result)
{
	object activer_user=get_user(activer_uid);
	if(objectp(activer_user))
	{
		logger->Log(activer_user->GetId(), "trade.main.passiver_require_trade", "uid:[%d] require trade result:%d", activer_user->GetId(),result);
		rpc_client_activer_require_trade(activer_uid, result);
		return ;
	}
	mapping op="module/internal_call"->PackCallOp(__FILE__, "remote_passiver_require_trade", activer_uid, result);
	"module/internal_call"->LogicServerCallByUid(activer_uid, op, 0);
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

void sure_trade(object user, int target_id, string tradecar)
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
