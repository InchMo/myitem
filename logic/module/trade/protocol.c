#include "/rc/rpc/trade.h"

void rpc_server_activer_require_trade(object activer_user, int passiver_uid)
{
	"module/trade/main"->activer_require_trade(activer_user, passiver_uid);
}

void rpc_server_passiver_require_trade(object passiver_user, int activer_uid, int result)
{
	"module/trade/main"->passiver_require_trade(passiver_user, activer_uid, result);
}

void rpc_server_sure_trade(object user, int target, string tradecar)
{
	//debug_message("====rpc_server_sure_trade==========");
	//debug_message("====tradecar:%O==========",tradecar);

	"module/trade/main"->sure_trade(user, target, tradecar);
}

void rpc_server_modified_tradecar(object usre, modified_tradecar_t data)
{
	"module/trade/main"->modified_tradecar(usre->GetId(), data);
}
