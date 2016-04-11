#include "/rc/rpc/trade.h"

void rpc_server_require_trade(object activer_user, int passiver_uid)
{
	"module/trade/main"->require_trade(activer_user, passiver_uid);
}

void rpc_server_require_trade_result(object passiver_user, int activer_uid, int result)
{
	"module/trade/main"->require_trade_result(passiver_user, activer_uid, result);
}

void rpc_server_agree_trade(object user, int target, string tradecar)
{
	//debug_message("====rpc_server_sure_trade==========");
	//debug_message("====tradecar:%O==========",tradecar);

	"module/trade/main"->agree_trade(user, target, tradecar);
}

void rpc_server_modified_tradecar(object user, modified_tradecar_t data)
{
	"module/trade/main"->modified_tradecar(user->GetId(), data);
}

void rpc_server_lock_tradecar(object user)
{
	"module/trade/main"->lock_tradecar(user);
}

void rpc_server_sure_trade(object user)
{
	"module/trade/main"->sure_trade(user);
}

void rpc_server_cancel_trade(object user)
{
	"module/trade/main"->cancel_trade(user);
}
