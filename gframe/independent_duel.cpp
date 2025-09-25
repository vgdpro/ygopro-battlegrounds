#include "config.h"
#include "independent_duel.h"
#include "netserver.h"
#include "game.h"
#include "data_manager.h"
#include "single_duel.h"
#include "../ocgcore/mtrandom.h"

namespace ygo {

#ifdef YGOPRO_SERVER_MODE
extern unsigned short replay_mode;
#endif
IndependentDuel::IndependentDuel(bool is_match) {
	match_mode = is_match;
}
IndependentDuel::~IndependentDuel() {
}
void IndependentDuel::Chat(DuelPlayer* dp, unsigned char* pdata, int len) {

}
void IndependentDuel::JoinGame(DuelPlayer* dp, unsigned char* pdata, bool is_creater) {
	if(!players[0] || !players[1]) {
		if(!players[0]) {
			players[0] = dp;
		} else {
			players[1] = dp;
		}
	} 
}
void IndependentDuel::SetFatherDuel(DuelMode* sd, int originplayer){
	father = sd;
	this->originplayerid = originplayer;
}
void IndependentDuel::LeaveGame(DuelPlayer* dp) {

}
void IndependentDuel::ToDuelist(DuelPlayer* dp) {

}
void IndependentDuel::ToObserver(DuelPlayer* dp) {

}
void IndependentDuel::PlayerReady(DuelPlayer* dp, bool is_ready) {

}
void IndependentDuel::PlayerKick(DuelPlayer* dp, unsigned char pos) {
	if(pos > 1 || dp != host_player || dp == players[pos] || !players[pos])
		return;
	LeaveGame(players[pos]);
}
void IndependentDuel::UpdateDeck(DuelPlayer* dp, unsigned char* pdata, int len) {

}
void IndependentDuel::StartDuel(DuelPlayer* dp) {

}
void IndependentDuel::HandResult(DuelPlayer* dp, unsigned char res) {

}
void IndependentDuel::TPResult(DuelPlayer* dp, unsigned char tp) {
	dp->state = CTOS_RESPONSE;
	std::random_device rd;
	ExtendedReplayHeader rh;
	rh.base.id = REPLAY_ID_YRP2;
	rh.base.version = PRO_VERSION;
	rh.base.flag = REPLAY_UNIFORM;
	rh.base.start_time = (uint32_t)std::time(nullptr);
#ifdef YGOPRO_SERVER_MODE
	if (pre_seed_specified[duel_count])
		memcpy(rh.seed_sequence, pre_seed[duel_count], SEED_COUNT * sizeof(uint32_t));
	else
#endif
	for (auto& x : rh.seed_sequence)
		x = rd();
	mtrandom rnd(rh.seed_sequence, SEED_COUNT);
	// last_replay.BeginRecord();
	// last_replay.WriteHeader(rh);
	// last_replay.WriteData(players[0]->name, 40, false);
	// last_replay.WriteData(players[1]->name, 40, false);
	// if(!host_info.no_shuffle_deck) {
	// 	rnd.shuffle_vector(pdeck[0].main);
	// 	rnd.shuffle_vector(pdeck[1].main);
	// }
	time_limit[0] = father->host_info.time_limit;
	time_limit[1] = father->host_info.time_limit;
	if(father->host_info.time_limit){
		host_info.time_limit = father->host_info.time_limit;
	}
	set_script_reader(DataManager::ScriptReaderEx);
	set_card_reader(DataManager::CardReader);
	set_card_reader_random(DataManager::CardReaderRandom);
	set_message_handler(IndependentDuel::MessageHandler);
	pduel = create_duel_v2(rh.seed_sequence);
	set_player_info(pduel, 0, father->host_info.start_lp, 0, 0);
	set_player_info(pduel, 1, father->host_info.start_lp, 0, 0);
#ifdef YGOPRO_SERVER_MODE
	preload_script(pduel, "./script/special.lua");
#endif
	unsigned int opt = (unsigned int)father->host_info.duel_rule << 16;
	if(father->host_info.no_shuffle_deck)
		opt |= DUEL_PSEUDO_SHUFFLE;
	opt |= DUEL_SIMPLE_AI;
	opt |= DUEL_ONLY_MAIN;
	// last_replay.WriteInt32(host_info.start_lp, false);
	// last_replay.WriteInt32(host_info.start_hand, false);
	// last_replay.WriteInt32(host_info.draw_count, false);
	// last_replay.WriteInt32(opt, false);
	// last_replay.Flush();
	// auto load = [&](const std::vector<code_pointer>& deck_container, uint8_t p, uint8_t location) {
	// 	last_replay.WriteInt32(deck_container.size(), false);
	// 	for (auto cit = deck_container.rbegin(); cit != deck_container.rend(); ++cit) {
	// 		new_card(pduel, (*cit)->first, p, p, location, 0, POS_FACEDOWN_DEFENSE);
	// 		last_replay.WriteInt32((*cit)->first, false);
	// 	}
	// };
	// load(pdeck[0].main, 0, LOCATION_DECK);
	// load(pdeck[0].extra, 0, LOCATION_EXTRA);
	// load(pdeck[1].main, 1, LOCATION_DECK);
	// load(pdeck[1].extra, 1, LOCATION_EXTRA);
	last_replay.Flush();
	unsigned char startbuf[32]{};
	auto pbuf = startbuf;
	BufferIO::WriteInt8(pbuf, MSG_START);
	BufferIO::WriteInt8(pbuf, 0);
	BufferIO::WriteInt8(pbuf, father->host_info.duel_rule);
	BufferIO::WriteInt32(pbuf, father->host_info.start_lp);
	BufferIO::WriteInt32(pbuf, father->host_info.start_lp);
	BufferIO::WriteInt16(pbuf, query_field_count(pduel, 0, LOCATION_DECK));
	BufferIO::WriteInt16(pbuf, query_field_count(pduel, 0, LOCATION_EXTRA));
	BufferIO::WriteInt16(pbuf, query_field_count(pduel, 1, LOCATION_DECK));
	BufferIO::WriteInt16(pbuf, query_field_count(pduel, 1, LOCATION_EXTRA));
	NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, startbuf, 19);
#ifdef YGOPRO_SERVER_MODE
	turn_player = 0;
	phase = 1;
	deck_reversed = false;
#endif
	RefreshExtra(0);
	RefreshExtra(1);
	start_duel(pduel, opt);
	if(host_info.time_limit) {
		time_elapsed = 0;
#ifdef YGOPRO_SERVER_MODE
		time_compensator[0] = host_info.time_limit;
		time_compensator[1] = host_info.time_limit;
		time_backed[0] = host_info.time_limit;
		time_backed[1] = host_info.time_limit;
		last_game_msg = 0;
#endif
		timeval timeout = { 1, 0 };
		event_add(etimer, &timeout);
	}
	Process();
}
void IndependentDuel::UpdateTimmer() {
	if(host_info.time_limit) {
		if(host_info.time_limit && host_info.time_limit - father->host_info.time_limit < 120){
			host_info.time_limit += 60;
			time_limit[0] = host_info.time_limit;
			time_limit[1] = host_info.time_limit;
		}
		time_elapsed = 0;
#ifdef YGOPRO_SERVER_MODE
		time_compensator[0] = host_info.time_limit;
		time_compensator[1] = host_info.time_limit;
		time_backed[0] = host_info.time_limit;
		time_backed[1] = host_info.time_limit;
#endif
		timeval timeout = { 1, 0 };
		event_add(etimer, &timeout);
	}
}
void IndependentDuel::Process() {
	std::vector<unsigned char> engineBuffer;
	engineBuffer.reserve(SIZE_MESSAGE_BUFFER);
	unsigned int engFlag = 0;
	int engLen = 0;
	int stop = 0;
	change_lua_duel(pduel);
	while (!stop) {
		if (engFlag == PROCESSOR_END)
			break;
		unsigned int result = process(pduel);
		engLen = result & PROCESSOR_BUFFER_LEN;
		engFlag = result & PROCESSOR_FLAG;
		if (engLen > 0) {
			if (engLen > (int)engineBuffer.size())
				engineBuffer.resize(engLen);
			get_message(pduel, engineBuffer.data());
			stop = Analyze(engineBuffer.data(), engLen);
		}
	}
	// if(stop == 2)
	// 	DuelEndProc();
}
void IndependentDuel::DuelEndProc() {

}
void IndependentDuel::Surrender(DuelPlayer* dp) {

}
// Analyze ocgcore message
int IndependentDuel::Analyze(unsigned char* msgbuffer, unsigned int len) {
	unsigned char* offset, *pbufw, *pbuf = msgbuffer;
	int player, count, type;
	while (pbuf - msgbuffer < (int)len) {
		offset = pbuf;
		unsigned char engType = BufferIO::ReadUInt8(pbuf);
#ifdef YGOPRO_SERVER_MODE
		last_game_msg = engType;
#endif
		switch (engType) {
		case MSG_RETRY: {
			WaitforResponse(last_response);
			NetServer::SendBufferToPlayer(players[last_response], STOC_GAME_MSG, offset, pbuf - offset);
			return 1;
		}
		case MSG_HINT: {
			type = BufferIO::ReadUInt8(pbuf);
			player = BufferIO::ReadUInt8(pbuf);
			BufferIO::ReadInt32(pbuf);
			switch (type) {
			case 1:
			case 2:
			case 3:
			case 5: {
				if(player ==0){
					NetServer::SendBufferToPlayer(players[player], STOC_GAME_MSG, offset, pbuf - offset);
				}
				break;
			}
			case 4:
			case 6:
			case 7:
			case 8:
			case 9:
			case 11: {
				break;
			}
			case 10: {
				NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, offset, pbuf - offset);
				break;
			}
			}
			break;
		}
		case MSG_WIN: {
			player = BufferIO::ReadUInt8(pbuf);
			type = BufferIO::ReadUInt8(pbuf);
			NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, offset, pbuf - offset);
			if(player > 1) {
				match_result[duel_count++] = 2;
				tp_player = 1 - tp_player;
			} else if(players[player] == pplayer[player]) {
				match_result[duel_count++] = player;
				tp_player = 1 - player;
			} else {
				match_result[duel_count++] = 1 - player;
				tp_player = player;
			}
			EndDuel();
			return 2;
		}
		case MSG_SELECT_BATTLECMD: {
			player = BufferIO::ReadUInt8(pbuf);
			count = BufferIO::ReadUInt8(pbuf);
			pbuf += count * 11;
			count = BufferIO::ReadUInt8(pbuf);
			pbuf += count * 8 + 2;
			RefreshMzone(0);
			RefreshMzone(1);
			RefreshSzone(0);
			RefreshSzone(1);
			RefreshHand(0);
			RefreshHand(1);
			WaitforResponse(player);
			NetServer::SendBufferToPlayer(players[player], STOC_GAME_MSG, offset, pbuf - offset);
			return 1;
		}
		case MSG_SELECT_IDLECMD: {
			player = BufferIO::ReadUInt8(pbuf);
			count = BufferIO::ReadUInt8(pbuf);
			pbuf += count * 7;
			count = BufferIO::ReadUInt8(pbuf);
			pbuf += count * 7;
			count = BufferIO::ReadUInt8(pbuf);
			pbuf += count * 7;
			count = BufferIO::ReadUInt8(pbuf);
			pbuf += count * 7;
			count = BufferIO::ReadUInt8(pbuf);
			pbuf += count * 7;
			count = BufferIO::ReadUInt8(pbuf);
			pbuf += count * 11 + 3;
			RefreshMzone(0);
			RefreshMzone(1);
			RefreshSzone(0);
			RefreshSzone(1);
			RefreshHand(0);
			RefreshHand(1);
			WaitforResponse(player);
			NetServer::SendBufferToPlayer(players[player], STOC_GAME_MSG, offset, pbuf - offset);
			return 1;
		}
		case MSG_SELECT_EFFECTYN: {
			player = BufferIO::ReadUInt8(pbuf);
			pbuf += 12;
			WaitforResponse(player);
			NetServer::SendBufferToPlayer(players[player], STOC_GAME_MSG, offset, pbuf - offset);
			return 1;
		}
		case MSG_SELECT_YESNO: {
			player = BufferIO::ReadUInt8(pbuf);
			pbuf += 4;
			WaitforResponse(player);
			NetServer::SendBufferToPlayer(players[player], STOC_GAME_MSG, offset, pbuf - offset);
			return 1;
		}
		case MSG_SELECT_OPTION: {
			player = BufferIO::ReadUInt8(pbuf);
			count = BufferIO::ReadUInt8(pbuf);
			pbuf += count * 4;
			WaitforResponse(player);
			NetServer::SendBufferToPlayer(players[player], STOC_GAME_MSG, offset, pbuf - offset);
			return 1;
		}
		case MSG_SELECT_CARD:
		case MSG_SELECT_TRIBUTE: {
			player = BufferIO::ReadUInt8(pbuf);
			pbuf += 3;
			count = BufferIO::ReadUInt8(pbuf);
			int c/*, l, s, ss, code*/;
			for (int i = 0; i < count; ++i) {
				pbufw = pbuf;
				/*code = */BufferIO::ReadInt32(pbuf);
				c = BufferIO::ReadUInt8(pbuf);
				/*l = */BufferIO::ReadUInt8(pbuf);
				/*s = */BufferIO::ReadUInt8(pbuf);
				/*ss = */BufferIO::ReadUInt8(pbuf);
				if (c != player) BufferIO::WriteInt32(pbufw, 0);
			}
			WaitforResponse(player);
			NetServer::SendBufferToPlayer(players[player], STOC_GAME_MSG, offset, pbuf - offset);
			return 1;
		}
		case MSG_SELECT_UNSELECT_CARD: {
			player = BufferIO::ReadUInt8(pbuf);
			pbuf += 4;
			count = BufferIO::ReadUInt8(pbuf);
			int c/*, l, s, ss, code*/;
			for (int i = 0; i < count; ++i) {
				pbufw = pbuf;
				/*code = */BufferIO::ReadInt32(pbuf);
				c = BufferIO::ReadUInt8(pbuf);
				/*l = */BufferIO::ReadUInt8(pbuf);
				/*s = */BufferIO::ReadUInt8(pbuf);
				/*ss = */BufferIO::ReadUInt8(pbuf);
				if (c != player) BufferIO::WriteInt32(pbufw, 0);
			}
			count = BufferIO::ReadUInt8(pbuf);
			for (int i = 0; i < count; ++i) {
				pbufw = pbuf;
				/*code = */BufferIO::ReadInt32(pbuf);
				c = BufferIO::ReadUInt8(pbuf);
				/*l = */BufferIO::ReadUInt8(pbuf);
				/*s = */BufferIO::ReadUInt8(pbuf);
				/*ss = */BufferIO::ReadUInt8(pbuf);
				if (c != player) BufferIO::WriteInt32(pbufw, 0);
			}
			WaitforResponse(player);
			NetServer::SendBufferToPlayer(players[player], STOC_GAME_MSG, offset, pbuf - offset);
			return 1;
		}
		case MSG_SELECT_CHAIN: {
			player = BufferIO::ReadUInt8(pbuf);
			count = BufferIO::ReadUInt8(pbuf);
			pbuf += 9 + count * 14;
			WaitforResponse(player);
			NetServer::SendBufferToPlayer(players[player], STOC_GAME_MSG, offset, pbuf - offset);
			return 1;
		}
		case MSG_SELECT_PLACE:
		case MSG_SELECT_DISFIELD: {
			player = BufferIO::ReadUInt8(pbuf);
			pbuf += 5;
			WaitforResponse(player);
			NetServer::SendBufferToPlayer(players[player], STOC_GAME_MSG, offset, pbuf - offset);
			return 1;
		}
		case MSG_SELECT_POSITION: {
			player = BufferIO::ReadUInt8(pbuf);
			pbuf += 5;
			WaitforResponse(player);
			NetServer::SendBufferToPlayer(players[player], STOC_GAME_MSG, offset, pbuf - offset);
			return 1;
		}
		case MSG_SELECT_COUNTER: {
			player = BufferIO::ReadUInt8(pbuf);
			pbuf += 4;
			count = BufferIO::ReadUInt8(pbuf);
			pbuf += count * 9;
			WaitforResponse(player);
			NetServer::SendBufferToPlayer(players[player], STOC_GAME_MSG, offset, pbuf - offset);
			return 1;
		}
		case MSG_SELECT_SUM: {
			pbuf++;
			player = BufferIO::ReadUInt8(pbuf);
			pbuf += 6;
			count = BufferIO::ReadUInt8(pbuf);
			pbuf += count * 11;
			count = BufferIO::ReadUInt8(pbuf);
			pbuf += count * 11;
			WaitforResponse(player);
			NetServer::SendBufferToPlayer(players[player], STOC_GAME_MSG, offset, pbuf - offset);
			return 1;
		}
		case MSG_SORT_CARD: {
			player = BufferIO::ReadUInt8(pbuf);
			count = BufferIO::ReadUInt8(pbuf);
			pbuf += count * 7;
			WaitforResponse(player);
			NetServer::SendBufferToPlayer(players[player], STOC_GAME_MSG, offset, pbuf - offset);
			return 1;
		}
		case MSG_CONFIRM_DECKTOP: {
			player = BufferIO::ReadUInt8(pbuf);
			count = BufferIO::ReadUInt8(pbuf);
			pbuf += count * 7;
			NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, offset, pbuf - offset);
			break;
		}
		case MSG_CONFIRM_EXTRATOP: {
			player = BufferIO::ReadUInt8(pbuf);
			count = BufferIO::ReadUInt8(pbuf);
			pbuf += count * 7;
			NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, offset, pbuf - offset);
			break;
		}
		case MSG_CONFIRM_CARDS: {
			player = BufferIO::ReadUInt8(pbuf);
			pbuf += 1;
			count = BufferIO::ReadUInt8(pbuf);
			pbuf += count * 7;
			if(player ==0){
				NetServer::SendBufferToPlayer(players[player], STOC_GAME_MSG, offset, pbuf - offset);
			}
			break;
		}
		case MSG_SHUFFLE_DECK: {
			player = BufferIO::ReadUInt8(pbuf);
			NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, offset, pbuf - offset);
			break;
		}
		case MSG_SHUFFLE_HAND: {
			player = BufferIO::ReadUInt8(pbuf);
			count = BufferIO::ReadUInt8(pbuf);
			if(player == 0) {
				NetServer::SendBufferToPlayer(players[player], STOC_GAME_MSG, offset, (pbuf - offset) + count * 4);
			}
			for(int i = 0; i < count; ++i)
				BufferIO::WriteInt32(pbuf, 0);
			RefreshHand(player, 0x781fff, 0);
			break;
		}
		case MSG_SHUFFLE_EXTRA: {
			player = BufferIO::ReadUInt8(pbuf);
			count = BufferIO::ReadUInt8(pbuf);
			if(player == 0) {
				NetServer::SendBufferToPlayer(players[player], STOC_GAME_MSG, offset, (pbuf - offset) + count * 4);
			}
			for (int i = 0; i < count; ++i)
				BufferIO::WriteInt32(pbuf, 0);
			RefreshExtra(player);
			break;
		}
		case MSG_REFRESH_DECK: {
			pbuf++;
			NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, offset, pbuf - offset);
			break;
		}
		case MSG_SWAP_GRAVE_DECK: {
			player = BufferIO::ReadUInt8(pbuf);
			NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, offset, pbuf - offset);
			RefreshGrave(player);
			break;
		}
		case MSG_REVERSE_DECK: {
			NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, offset, pbuf - offset);
#ifdef YGOPRO_SERVER_MODE
			deck_reversed = !deck_reversed;
#endif
			break;
		}
		case MSG_DECK_TOP: {
			pbuf += 6;
			NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, offset, pbuf - offset);
			break;
		}
		case MSG_SHUFFLE_SET_CARD: {
			unsigned int loc = BufferIO::ReadUInt8(pbuf);
			count = BufferIO::ReadUInt8(pbuf);
			pbuf += count * 8;
			NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, offset, pbuf - offset);
			if(loc == LOCATION_MZONE) {
				RefreshMzone(0, 0x181fff, 0);
				RefreshMzone(1, 0x181fff, 0);
			}
			else {
				RefreshSzone(0, 0x181fff, 0);
				RefreshSzone(1, 0x181fff, 0);
			}
			break;
		}
		case MSG_NEW_TURN: {
			RefreshMzone(0);
			RefreshMzone(1);
			RefreshSzone(0);
			RefreshSzone(1);
			RefreshHand(0);
			RefreshHand(1);
#ifdef YGOPRO_SERVER_MODE
			turn_player = BufferIO::ReadInt8(pbuf);
#else
			pbuf++;
#endif
			time_limit[0] = host_info.time_limit;
			time_limit[1] = host_info.time_limit;
#ifdef YGOPRO_SERVER_MODE
			time_compensator[0] = host_info.time_limit;
			time_compensator[1] = host_info.time_limit;
			time_backed[0] = host_info.time_limit;
			time_backed[1] = host_info.time_limit;
#endif
			NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, offset, pbuf - offset);
			break;
		}
		case MSG_NEW_PHASE: {
			int phase = BufferIO::ReadInt16(pbuf);
			if(phase == PHASE_BATTLE_START){
				event_del(etimer);
				father->IndependentDuelStopProc(originplayerid);
				return 1;
			}
			NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, offset, pbuf - offset);
			RefreshMzone(0);
			RefreshMzone(1);
			RefreshSzone(0);
			RefreshSzone(1);
			RefreshHand(0);
			RefreshHand(1);
			break;
		}
		case MSG_MOVE: {
			pbufw = pbuf;
			int pc = pbuf[4];
			int pl = pbuf[5];
			/*int ps = pbuf[6];*/
			/*int pp = pbuf[7];*/
			int cc = pbuf[8];
			int cl = pbuf[9];
			int cs = pbuf[10];
			int cp = pbuf[11];
			pbuf += 16;
			NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, offset, pbuf - offset);
			if (!(cl & (LOCATION_GRAVE + LOCATION_OVERLAY)) && ((cl & (LOCATION_DECK + LOCATION_HAND)) || (cp & POS_FACEDOWN)))
				BufferIO::WriteInt32(pbufw, 0);
			if (cl != 0 && (cl & LOCATION_OVERLAY) == 0 && (cl != pl || pc != cc))
				RefreshSingle(cc, cl, cs);
			break;
		}
		case MSG_POS_CHANGE: {
			int cc = pbuf[4];
			int cl = pbuf[5];
			int cs = pbuf[6];
			int pp = pbuf[7];
			int cp = pbuf[8];
			pbuf += 9;
			NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, offset, pbuf - offset);
			if((pp & POS_FACEDOWN) && (cp & POS_FACEUP))
				RefreshSingle(cc, cl, cs);
			break;
		}
		case MSG_SET: {
			BufferIO::WriteInt32(pbuf, 0);
			pbuf += 4;
			NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, offset, pbuf - offset);
			break;
		}
		case MSG_SWAP: {
			int c1 = pbuf[4];
			int l1 = pbuf[5];
			int s1 = pbuf[6];
			int c2 = pbuf[12];
			int l2 = pbuf[13];
			int s2 = pbuf[14];
			pbuf += 16;
			NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, offset, pbuf - offset);
			RefreshSingle(c1, l1, s1);
			RefreshSingle(c2, l2, s2);
			break;
		}
		case MSG_FIELD_DISABLED: {
			pbuf += 4;
			NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, offset, pbuf - offset);
			break;
		}
		case MSG_SUMMONING: {
			pbuf += 8;
			NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, offset, pbuf - offset);
			break;
		}
		case MSG_SUMMONED: {
			NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, offset, pbuf - offset);
			RefreshMzone(0);
			RefreshMzone(1);
			RefreshSzone(0);
			RefreshSzone(1);
			break;
		}
		case MSG_SPSUMMONING: {
			pbuf += 8;
			NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, offset, pbuf - offset);
			break;
		}
		case MSG_SPSUMMONED: {
			NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, offset, pbuf - offset);
			RefreshMzone(0);
			RefreshMzone(1);
			RefreshSzone(0);
			RefreshSzone(1);
			break;
		}
		case MSG_FLIPSUMMONING: {
			RefreshSingle(pbuf[4], pbuf[5], pbuf[6]);
			pbuf += 8;
			NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, offset, pbuf - offset);
			break;
		}
		case MSG_FLIPSUMMONED: {
			NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, offset, pbuf - offset);
			RefreshMzone(0);
			RefreshMzone(1);
			RefreshSzone(0);
			RefreshSzone(1);
			break;
		}
		case MSG_CHAINING: {
			pbuf += 16;
			NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, offset, pbuf - offset);
			break;
		}
		case MSG_CHAINED: {
			pbuf++;
			NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, offset, pbuf - offset);
			RefreshMzone(0);
			RefreshMzone(1);
			RefreshSzone(0);
			RefreshSzone(1);
			RefreshHand(0);
			RefreshHand(1);
			break;
		}
		case MSG_CHAIN_SOLVING: {
			pbuf++;
			NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, offset, pbuf - offset);
			break;
		}
		case MSG_CHAIN_SOLVED: {
			pbuf++;
			NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, offset, pbuf - offset);
			RefreshMzone(0);
			RefreshMzone(1);
			RefreshSzone(0);
			RefreshSzone(1);
			RefreshHand(0);
			RefreshHand(1);
			break;
		}
		case MSG_CHAIN_END: {
			NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, offset, pbuf - offset);
			RefreshMzone(0);
			RefreshMzone(1);
			RefreshSzone(0);
			RefreshSzone(1);
			RefreshHand(0);
			RefreshHand(1);
			break;
		}
		case MSG_CHAIN_NEGATED: {
			pbuf++;
			NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, offset, pbuf - offset);
			break;
		}
		case MSG_CHAIN_DISABLED: {
			pbuf++;
			NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, offset, pbuf - offset);
			break;
		}
		case MSG_CARD_SELECTED: {
			player = BufferIO::ReadUInt8(pbuf);
			count = BufferIO::ReadUInt8(pbuf);
			pbuf += count * 4;
			break;
		}
		case MSG_RANDOM_SELECTED: {
			player = BufferIO::ReadUInt8(pbuf);
			count = BufferIO::ReadUInt8(pbuf);
			pbuf += count * 4;
			NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, offset, pbuf - offset);
			break;
		}
		case MSG_BECOME_TARGET: {
			count = BufferIO::ReadUInt8(pbuf);
			pbuf += count * 4;
			NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, offset, pbuf - offset);
			break;
		}
		case MSG_DRAW: {
			player = BufferIO::ReadUInt8(pbuf);
			count = BufferIO::ReadUInt8(pbuf);
			pbufw = pbuf;
			pbuf += count * 4;
			NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, offset, pbuf - offset);
			for (int i = 0; i < count; ++i) {
				if(!(pbufw[3] & 0x80))
					BufferIO::WriteInt32(pbufw, 0);
				else
					pbufw += 4;
			}
			break;
		}
		case MSG_DAMAGE: {
			pbuf += 5;
			NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, offset, pbuf - offset);
			break;
		}
		case MSG_RECOVER: {
			pbuf += 5;
			NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, offset, pbuf - offset);
			break;
		}
		case MSG_EQUIP: {
			pbuf += 8;
			NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, offset, pbuf - offset);
			

			break;
		}
		case MSG_LPUPDATE: {
			pbuf += 5;
			NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, offset, pbuf - offset);
			

			break;
		}
		case MSG_UNEQUIP: {
			pbuf += 4;
			NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, offset, pbuf - offset);
			

			break;
		}
		case MSG_CARD_TARGET: {
			pbuf += 8;
			NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, offset, pbuf - offset);
			

			break;
		}
		case MSG_CANCEL_TARGET: {
			pbuf += 8;
			NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, offset, pbuf - offset);
			

			break;
		}
		case MSG_PAY_LPCOST: {
			pbuf += 5;
			NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, offset, pbuf - offset);
			

			break;
		}
		case MSG_ADD_COUNTER: {
			pbuf += 7;
			NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, offset, pbuf - offset);
			

			break;
		}
		case MSG_REMOVE_COUNTER: {
			pbuf += 7;
			NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, offset, pbuf - offset);
			

			break;
		}
		case MSG_ATTACK: {
			pbuf += 8;
			NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, offset, pbuf - offset);
			

			break;
		}
		case MSG_BATTLE: {
			pbuf += 26;
			NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, offset, pbuf - offset);
			

			break;
		}
		case MSG_ATTACK_DISABLED: {
			NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, offset, pbuf - offset);
			

			break;
		}
		case MSG_DAMAGE_STEP_START: {
			NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, offset, pbuf - offset);
			

			RefreshMzone(0);
			RefreshMzone(1);
			break;
		}
		case MSG_DAMAGE_STEP_END: {
			NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, offset, pbuf - offset);
			

			RefreshMzone(0);
			RefreshMzone(1);
			break;
		}
		case MSG_MISSED_EFFECT: {
			player = pbuf[0];
			pbuf += 8;
			if(player == 0){
				NetServer::SendBufferToPlayer(players[player], STOC_GAME_MSG, offset, pbuf - offset);
			}
			break;
		}
		case MSG_TOSS_COIN: {
			player = BufferIO::ReadUInt8(pbuf);
			count = BufferIO::ReadUInt8(pbuf);
			pbuf += count;
			NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, offset, pbuf - offset);
			

			break;
		}
		case MSG_TOSS_DICE: {
			player = BufferIO::ReadUInt8(pbuf);
			count = BufferIO::ReadUInt8(pbuf);
			pbuf += count;
			NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, offset, pbuf - offset);
			

			break;
		}
		// case MSG_ROCK_PAPER_SCISSORS: {
		// 	player = BufferIO::ReadUInt8(pbuf);
		// 	WaitforResponse(player);
		// 	NetServer::SendBufferToPlayer(players[player], STOC_GAME_MSG, offset, pbuf - offset);
		// 	return 1;
		// }
		case MSG_HAND_RES: {
			pbuf += 1;
			NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, offset, pbuf - offset);
			
			break;
		}
		case MSG_ANNOUNCE_RACE: {
			player = BufferIO::ReadUInt8(pbuf);
			pbuf += 5;
			WaitforResponse(player);
			NetServer::SendBufferToPlayer(players[player], STOC_GAME_MSG, offset, pbuf - offset);
			return 1;
		}
		case MSG_ANNOUNCE_ATTRIB: {
			player = BufferIO::ReadUInt8(pbuf);
			pbuf += 5;
			WaitforResponse(player);
			NetServer::SendBufferToPlayer(players[player], STOC_GAME_MSG, offset, pbuf - offset);
			return 1;
		}
		case MSG_ANNOUNCE_CARD:
		case MSG_ANNOUNCE_NUMBER: {
			player = BufferIO::ReadUInt8(pbuf);
			count = BufferIO::ReadUInt8(pbuf);
			pbuf += 4 * count;
			WaitforResponse(player);
			NetServer::SendBufferToPlayer(players[player], STOC_GAME_MSG, offset, pbuf - offset);
			return 1;
		}
		case MSG_CARD_HINT: {
			pbuf += 9;
			NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, offset, pbuf - offset);
			

			break;
		}
		case MSG_PLAYER_HINT: {
			pbuf += 6;
			NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, offset, pbuf - offset);
			

			break;
		}
		case MSG_RELOAD_FIELD: {
			pbuf++;
			for(int p = 0; p < 2; ++p) {
				pbuf += 4;
				for(int seq = 0; seq < 7; ++seq) {
					int val = BufferIO::ReadUInt8(pbuf);
					if(val)
						pbuf += 2;
				}
				for(int seq = 0; seq < 8; ++seq) {
					int val = BufferIO::ReadUInt8(pbuf);
					if(val)
						pbuf++;
				}
				pbuf += 6;
			}
			count = BufferIO::ReadUInt8(pbuf);
			pbuf += count * 15;
			NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, offset, pbuf - offset);
			RefreshExtra(0,0xf81fff,0);
			RefreshExtra(1,0xf81fff,0);
			RefreshHand(0,0xf81fff,0);
			RefreshHand(1,0xf81fff,0);
			RefreshGrave(0,0xf81fff,0);
			RefreshGrave(1,0xf81fff,0);
			RefreshMzone(0,0xffdfff,0);
			RefreshMzone(1,0xffdfff,0);
			RefreshSzone(0,0xffdfff,0);
			RefreshSzone(1,0xffdfff,0);
			RefreshDeck(0,0xf81fff,0);
			RefreshDeck(1,0xf81fff,0);
			RefreshRemove(0,0xf81fff,0);
			RefreshRemove(1,0xf81fff,0);
			break;
		}
		case MSG_MATCH_KILL: {
			int code = BufferIO::ReadInt32(pbuf);
			if(match_mode) {
				match_kill = code;
				NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, offset, pbuf - offset);
			}
			break;
		}
		}
	}
	return 0;
}
void IndependentDuel::GetResponse(DuelPlayer* dp, unsigned char* pdata, unsigned int len) {
	unsigned char resb[SIZE_RETURN_VALUE]{};
	if (len > SIZE_RETURN_VALUE)
		len = SIZE_RETURN_VALUE;
	std::memcpy(resb, pdata, len);
	last_replay.Write<uint8_t>(len);
	last_replay.WriteData(resb, len);
	set_responseb(pduel, resb);
	players[0]->state = 0xff;
	if(host_info.time_limit) {
		if(time_limit[0] >= time_elapsed)
			time_limit[0] -= time_elapsed;
		else time_limit[0] = 0;
		time_elapsed = 0;
#ifdef YGOPRO_SERVER_MODE
		if(time_backed[0] > 0 && time_limit[0] < host_info.time_limit && NetServer::IsCanIncreaseTime(last_game_msg, pdata, len)) {
			++time_limit[0];
			++time_compensator[0];
			--time_backed[0];
		}
#endif
	}
	Process();
}
void IndependentDuel::EndDuel() {
	end_duel(pduel);
	event_del(etimer);
	pduel = 0;
}
void IndependentDuel::WaitforResponse(int playerid) {
	last_response = playerid;
	// unsigned char msg = MSG_WAITING;
	// NetServer::SendPacketToPlayer(players[1 - playerid], STOC_GAME_MSG, msg);
	if(host_info.time_limit) {
		STOC_TimeLimit sctl;
		sctl.player = 0;
		sctl.left_time = time_limit[playerid];
		NetServer::SendPacketToPlayer(players[0], STOC_TIME_LIMIT, sctl);
		// NetServer::SendPacketToPlayer(players[1], STOC_TIME_LIMIT, sctl);
		players[playerid]->state = CTOS_TIME_CONFIRM;
	} else
		players[playerid]->state = CTOS_RESPONSE;
}
#ifdef YGOPRO_SERVER_MODE
void IndependentDuel::RequestField(DuelPlayer* dp) {
	if(dp->type > 1)
		return;
	uint8_t player = dp->type;
	NetServer::SendPacketToPlayer(dp, STOC_DUEL_START);

	uint8_t buf[1024];
	uint8_t* temp_buf = buf;
	auto WriteMsg = [&](const std::function<void(uint8_t*&)> &writer) {
		temp_buf = buf;
		writer(temp_buf);
		NetServer::SendBufferToPlayer(dp, STOC_GAME_MSG, buf, temp_buf - buf);
	};

	WriteMsg([&](uint8_t*& pbuf) {
		BufferIO::WriteInt8(pbuf, MSG_START);
		BufferIO::WriteInt8(pbuf, 0);
		BufferIO::WriteInt8(pbuf, father->host_info.duel_rule);
		BufferIO::WriteInt32(pbuf, father->host_info.start_lp);
		BufferIO::WriteInt32(pbuf, father->host_info.start_lp);
		BufferIO::WriteInt16(pbuf, 0);
		BufferIO::WriteInt16(pbuf, 0);
		BufferIO::WriteInt16(pbuf, 0);
		BufferIO::WriteInt16(pbuf, 0);
	});

	uint8_t newturn_count = (turn_player == 1) ? 2 : 1;
	for (uint8_t i = 0; i < newturn_count; ++i) {
		WriteMsg([&](uint8_t*& pbuf) {
			BufferIO::WriteInt8(pbuf, MSG_NEW_TURN);
			BufferIO::WriteInt8(pbuf, i);
		});
	}

	WriteMsg([&](uint8_t*& pbuf) {
		BufferIO::WriteInt8(pbuf, MSG_NEW_PHASE);
		BufferIO::WriteInt16(pbuf, phase);
	});

	WriteMsg([&](uint8_t*& pbuf) {
		auto length = query_field_info(pduel, pbuf);
		pbuf += length;
	});

	RefreshMzone(1 - player, 0xefffff, 0, dp);
	RefreshMzone(player, 0xefffff, 0, dp);
	RefreshSzone(1 - player, 0xefffff, 0, dp);
	RefreshSzone(player, 0xefffff, 0, dp);
	RefreshHand(1 - player, 0xefffff, 0, dp);
	RefreshHand(player, 0xefffff, 0, dp);
	RefreshGrave(1 - player, 0xefffff, 0, dp);
	RefreshGrave(player, 0xefffff, 0, dp);
	RefreshExtra(1 - player, 0xefffff, 0, dp);
	RefreshExtra(player, 0xefffff, 0, dp);
	RefreshRemoved(1 - player, 0xefffff, 0, dp);
	RefreshRemoved(player, 0xefffff, 0, dp);
	RefreshDeck(1 - player, 0xefffff, 0, dp);
	RefreshDeck(player, 0xefffff, 0, dp);

	// send MSG_REVERSE_DECK if deck is reversed
	if(deck_reversed)
		WriteMsg([&](uint8_t*& pbuf) {
			BufferIO::WriteInt8(pbuf, MSG_REVERSE_DECK);
		});

	uint8_t query_buffer[SIZE_QUERY_BUFFER];
	for(uint8_t i = 0; i < 2; ++i) {
		// get decktop card
		auto qlen = query_field_card(pduel, i, LOCATION_DECK, QUERY_CODE | QUERY_POSITION, query_buffer, 0);
		if(!qlen)
			continue; // no cards in deck
		uint8_t *qbuf = query_buffer;
		uint32_t code = 0;
		uint32_t position = 0;
		while(qbuf < query_buffer + qlen) {
			auto clen = BufferIO::ReadInt32(qbuf);
			if(qbuf + clen - 4 == query_buffer + qlen) {
				// last card
				code = *(uint32_t*)(qbuf + 4);
				position = GetPosition(qbuf, 8);
			}
			qbuf += clen - 4;
		}
		if(position & POS_FACEUP)
			code |= 0x80000000; // mark as reversed
		if(deck_reversed || position & POS_FACEUP)
			WriteMsg([&](uint8_t*& pbuf) {
				BufferIO::WriteInt8(pbuf, MSG_DECK_TOP);
				BufferIO::WriteInt8(pbuf, i);
				BufferIO::WriteInt8(pbuf, 0);
				BufferIO::WriteInt32(pbuf, code);
			});
	}

	/*
	if(dp == players[last_response])
		WaitforResponse(last_response);
	*/
	STOC_TimeLimit sctl;
	sctl.player = 1 - last_response;
	sctl.left_time = time_limit[1 - last_response];
	NetServer::SendPacketToPlayer(dp, STOC_TIME_LIMIT, sctl);
	sctl.player = last_response;
	sctl.left_time = time_limit[last_response] - time_elapsed;
	NetServer::SendPacketToPlayer(dp, STOC_TIME_LIMIT, sctl);

	NetServer::SendPacketToPlayer(dp, STOC_FIELD_FINISH);
}
#endif //YGOPRO_SERVER_MODE
void IndependentDuel::TimeConfirm(DuelPlayer* dp) {
	if(host_info.time_limit == 0)
		return;
	if(0 != last_response)
		return;
	players[last_response]->state = CTOS_RESPONSE;
#ifdef YGOPRO_SERVER_MODE
	if(time_elapsed < 10 && time_elapsed <= time_compensator[0]){
		time_compensator[0] -= time_elapsed;
		time_elapsed = 0;
	}
	else {
		time_limit[0] -= time_elapsed;
		time_elapsed = 0;
	}
#else
	if(time_elapsed < 10)
		time_elapsed = 0;
#endif //YGOPRO_SERVER_MODE
}
inline int IndependentDuel::WriteUpdateData(int& player, int location, int& flag, unsigned char*& qbuf, int& use_cache) {
	flag |= (QUERY_CODE | QUERY_POSITION);
	BufferIO::WriteInt8(qbuf, MSG_UPDATE_DATA);
	BufferIO::WriteInt8(qbuf, player);
	BufferIO::WriteInt8(qbuf, location);
	int len = query_field_card(pduel, player, location, flag, qbuf, use_cache);
	return len;
}


#ifdef YGOPRO_SERVER_MODE
void IndependentDuel::RefreshMzone(int player, int flag, int use_cache, DuelPlayer* dp)
#else
void IndependentDuel::RefreshMzone(int player, int flag, int use_cache)
#endif //YGOPRO_SERVER_MODE
{
	std::vector<unsigned char> query_buffer;
	query_buffer.resize(SIZE_QUERY_BUFFER);
	auto qbuf = query_buffer.data();
	auto len = WriteUpdateData(player, LOCATION_MZONE, flag, qbuf, use_cache);
#ifdef YGOPRO_SERVER_MODE
if(!dp || dp == players[0])
#endif
	NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, query_buffer.data(), len + 3);
	// int qlen = 0;
	// while(qlen < len) {
	// 	const int clen = BufferIO::ReadInt32(qbuf);
	// 	qlen += clen;
	// 	if (clen <= LEN_HEADER)
	// 		continue;
	// 	auto position = GetPosition(qbuf, 8);
	// 	if (position & POS_FACEDOWN)
	// 		std::memset(qbuf, 0, clen - 4);
	// 	qbuf += clen - 4;
	// }
}


#ifdef YGOPRO_SERVER_MODE
void IndependentDuel::RefreshSzone(int player, int flag, int use_cache, DuelPlayer* dp)
#else
void IndependentDuel::RefreshSzone(int player, int flag, int use_cache)
#endif //YGOPRO_SERVER_MODE
{
	std::vector<unsigned char> query_buffer;
	query_buffer.resize(SIZE_QUERY_BUFFER);
	auto qbuf = query_buffer.data();
	auto len = WriteUpdateData(player, LOCATION_SZONE, flag, qbuf, use_cache);
#ifdef YGOPRO_SERVER_MODE
if(!dp || dp == players[0])
#endif
	NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, query_buffer.data(), len + 3);

	// int qlen = 0;
	// while(qlen < len) {
	// 	const int clen = BufferIO::ReadInt32(qbuf);
	// 	qlen += clen;
	// 	if (clen <= LEN_HEADER)
	// 		continue;
	// 	auto position = GetPosition(qbuf, 8);
	// 	if (position & POS_FACEDOWN)
	// 		std::memset(qbuf, 0, clen - 4);
	// 	qbuf += clen - 4;
	// }
}


#ifdef YGOPRO_SERVER_MODE
void IndependentDuel::RefreshHand(int player, int flag, int use_cache, DuelPlayer* dp)
#else
void IndependentDuel::RefreshHand(int player, int flag, int use_cache)
#endif //YGOPRO_SERVER_MODE
{
	std::vector<unsigned char> query_buffer;
	query_buffer.resize(SIZE_QUERY_BUFFER);
	auto qbuf = query_buffer.data();
	auto len = WriteUpdateData(player, LOCATION_HAND, flag, qbuf, use_cache);
#ifdef YGOPRO_SERVER_MODE
if(!dp || dp == players[0])
#endif
	NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, query_buffer.data(), len + 3);

	// int qlen = 0;
	// while(qlen < len) {
	// 	const int slen = BufferIO::ReadInt32(qbuf);
	// 	qlen += slen;
	// 	if (slen <= LEN_HEADER)
	// 		continue;
	// 	auto position = GetPosition(qbuf, 8);
	// 	if(!(position & POS_FACEUP))
	// 		std::memset(qbuf, 0, slen - 4);
	// 	qbuf += slen - 4;
	// }

}


#ifdef YGOPRO_SERVER_MODE
void IndependentDuel::RefreshGrave(int player, int flag, int use_cache, DuelPlayer* dp)
#else
void IndependentDuel::RefreshGrave(int player, int flag, int use_cache)
#endif //YGOPRO_SERVER_MODE
{
	std::vector<unsigned char> query_buffer;
	query_buffer.resize(SIZE_QUERY_BUFFER);
	auto qbuf = query_buffer.data();
	auto len = WriteUpdateData(player, LOCATION_GRAVE, flag, qbuf, use_cache);
#ifdef YGOPRO_SERVER_MODE
if(!dp || dp == players[0])
#endif
	NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, query_buffer.data(), len + 3);
}


#ifdef YGOPRO_SERVER_MODE
void IndependentDuel::RefreshExtra(int player, int flag, int use_cache, DuelPlayer* dp)
#else
void IndependentDuel::RefreshExtra(int player, int flag, int use_cache)
#endif //YGOPRO_SERVER_MODE
{
	std::vector<unsigned char> query_buffer;
	query_buffer.resize(SIZE_QUERY_BUFFER);
	auto qbuf = query_buffer.data();
	auto len = WriteUpdateData(player, LOCATION_EXTRA, flag, qbuf, use_cache);
#ifdef YGOPRO_SERVER_MODE
if(!dp || dp == players[0])
#endif
	NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, query_buffer.data(), len + 3);
}

#ifdef YGOPRO_SERVER_MODE
void IndependentDuel::RefreshRemoved(int player, int flag, int use_cache, DuelPlayer* dp) {
	std::vector<unsigned char> query_buffer;
	query_buffer.resize(SIZE_QUERY_BUFFER);
	auto qbuf = query_buffer.data();
	auto len = WriteUpdateData(player, LOCATION_REMOVED, flag, qbuf, use_cache);
	if(!dp || dp == players[0])
		NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, query_buffer.data(), len + 3);
	// int qlen = 0;
	// while(qlen < len) {
	// 	int clen = BufferIO::ReadInt32(qbuf);
	// 	qlen += clen;
	// 	if (clen <= LEN_HEADER)
	// 		continue;
	// 	auto position = GetPosition(qbuf, 8);
	// 	if (position & POS_FACEDOWN)
	// 		memset(qbuf, 0, clen - 4);
	// 	qbuf += clen - 4;
	// }
}
#endif

#ifdef YGOPRO_SERVER_MODE
void IndependentDuel::RefreshDeck(int player, int flag, int use_cache, DuelPlayer* dp)
#else
void IndependentDuel::RefreshDeck(int player, int flag, int use_cache) 
#endif //YGOPRO_SERVER_MODE
{
	std::vector<unsigned char> query_buffer;
	query_buffer.resize(SIZE_QUERY_BUFFER);
	auto qbuf = query_buffer.data();
	auto len = WriteUpdateData(player, LOCATION_DECK, flag, qbuf, use_cache);
#ifdef YGOPRO_SERVER_MODE
if(!dp || dp == players[0])
#endif
	NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, query_buffer.data(), len + 3);
}

#ifdef YGOPRO_SERVER_MODE
void IndependentDuel::RefreshRemove(int player, int flag, int use_cache, DuelPlayer* dp)
#else
void IndependentDuel::RefreshRemove(int player, int flag, int use_cache) 
#endif //YGOPRO_SERVER_MODE
{
	std::vector<unsigned char> query_buffer;
	query_buffer.resize(SIZE_QUERY_BUFFER);
	auto qbuf = query_buffer.data();
	auto len = WriteUpdateData(player, LOCATION_REMOVED, flag, qbuf, use_cache);
#ifdef YGOPRO_SERVER_MODE
if(!dp || dp == players[0])
#endif
	NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, query_buffer.data(), len + 3);
}

void IndependentDuel::RefreshSingle(int player, int location, int sequence, int flag) {
	flag |= (QUERY_CODE | QUERY_POSITION);
	unsigned char query_buffer[0x1000];
	auto qbuf = query_buffer;
	BufferIO::WriteInt8(qbuf, MSG_UPDATE_CARD);
	BufferIO::WriteInt8(qbuf, player);
	BufferIO::WriteInt8(qbuf, location);
	BufferIO::WriteInt8(qbuf, sequence);
	int len = query_card(pduel, player, location, sequence, flag, qbuf, 0);
	NetServer::SendBufferToPlayer(players[0], STOC_GAME_MSG, query_buffer, len + 4);
	// if (len <= LEN_HEADER)
	// 	return;
	// const int clen = BufferIO::ReadInt32(qbuf);
	// auto position = GetPosition(qbuf, 8);
	// if (position & POS_FACEDOWN) {
	// 	BufferIO::WriteInt32(qbuf, QUERY_CODE);
	// 	BufferIO::WriteInt32(qbuf, 0);
	// 	std::memset(qbuf, 0, clen - 12);
	// }
}
uint32_t IndependentDuel::MessageHandler(intptr_t fduel, uint32_t type) {
	if(!enable_log)
		return 0;
	char msgbuf[1024];
	get_log_message(fduel, msgbuf);
	mainGame->AddDebugMsg(msgbuf);
	return 0;
}
void IndependentDuel::IndependentDuelTimeout(unsigned char last_response){

}
void IndependentDuel::SingleTimer(evutil_socket_t fd, short events, void* arg) {
	IndependentDuel* sd = static_cast<IndependentDuel*>(arg);
	sd->time_elapsed++;
	if(sd->time_elapsed >= sd->time_limit[sd->last_response] || sd->time_limit[sd->last_response] <= 0) {
		sd->father->IndependentDuelTimeout(sd->last_response);
		sd->father->IndependentDuelStopProc(sd->originplayerid);
		event_del(sd->etimer);
		return;
	}
	timeval timeout = { 1, 0 };
	event_add(sd->etimer, &timeout);
}
void IndependentDuel::IndependentDuelStopProc(int duelid) {

}

}
