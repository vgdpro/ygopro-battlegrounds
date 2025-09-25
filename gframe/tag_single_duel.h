#ifndef TAG_SINGLE_DUEL_H
#define TAG_SINGLE_DUEL_H

#include <set>
#include "network.h"
#include "deck_manager.h"
#include "replay.h"

namespace ygo {

class TagSingleDuel{
public:
	TagSingleDuel(bool is_match);
	~TagSingleDuel();
	void JoinGame(DuelPlayer* dp, unsigned char* pdata, bool is_creater);
	void TPResult(DuelPlayer* dp, unsigned char tp);
	void Process();
	int Analyze(unsigned char* msgbuffer, unsigned int len);
	void GetResponse(DuelPlayer* dp, unsigned char* pdata, unsigned int len);
	void TimeConfirm(DuelPlayer* dp);
	void TagSingleDuelStopProc(int duelid);
	void TagSingleDuelTimeout(unsigned char last_response);
	void EndDuel();
    #ifdef YGOPRO_SERVER_MODE
        void RequestField(DuelPlayer* dp);
    #endif
	
	void DuelEndProc();
	void WaitforResponse(int playerid);
#ifdef YGOPRO_SERVER_MODE
	void RefreshMzone(int player, int flag = 0x881fff, int use_cache = 1, DuelPlayer* dp = 0);
	void RefreshSzone(int player, int flag = 0x681fff, int use_cache = 1, DuelPlayer* dp = 0);
	void RefreshHand(int player, int flag = 0x681fff, int use_cache = 1, DuelPlayer* dp = 0);
	void RefreshGrave(int player, int flag = 0x81fff, int use_cache = 1, DuelPlayer* dp = 0);
	void RefreshExtra(int player, int flag = 0xe81fff, int use_cache = 1, DuelPlayer* dp = 0);
	void RefreshRemoved(int player, int flag = 0x81fff, int use_cache = 1, DuelPlayer* dp = 0);
	void RefreshDeck(int player, int flag = 0xe81fff, int use_cache = 1, DuelPlayer* dp = 0);
	void RefreshRemove(int player, int flag = 0x681fff, int use_cache = 1, DuelPlayer* dp = 0);
#else
	void RefreshMzone(int player, int flag = 0x881fff, int use_cache = 1);
	void RefreshSzone(int player, int flag = 0x681fff, int use_cache = 1);
	void RefreshHand(int player, int flag = 0x681fff, int use_cache = 1);
	void RefreshGrave(int player, int flag = 0x81fff, int use_cache = 1);
	void RefreshExtra(int player, int flag = 0xe81fff, int use_cache = 1);
	void RefreshDeck(int player, int flag = 0xe81fff, int use_cache = 1);
	void RefreshRemove(int player, int flag = 0x681fff, int use_cache = 1);
#endif
	void RefreshSingle(int player, int location, int sequence, int flag = 0xf81fff);
	void SetFatherDuel(DuelMode* sd, int originplayer);
	void UpdateTimmer();

	static uint32_t MessageHandler(intptr_t fduel, uint32_t type);
	static void SingleTimer(evutil_socket_t fd, short events, void* arg);

	short time_elapsed{ 0 };
    event* etimer { nullptr };
	DuelPlayer* host_player{ nullptr };
	HostInfo host_info;
	int duel_stage{};
	intptr_t pduel{};
	wchar_t name[20]{};
	wchar_t pass[20]{};

private:
	int WriteUpdateData(int& player, int location, int& flag, unsigned char*& qbuf, int& use_cache);
	
protected:
	DuelPlayer* players[2]{};
	DuelPlayer* pplayer[2]{};
	DuelMode* father{};
	bool ready[2]{};
	Deck pdeck[2];
	int deck_error[2]{};
	unsigned char hand_result[2]{};
	unsigned char last_response{ 0 };
	std::set<DuelPlayer*> observers;
#ifdef YGOPRO_SERVER_MODE
	DuelPlayer* cache_recorder{};
	DuelPlayer* replay_recorder{};
	unsigned char turn_player{ 0 };
	unsigned short phase{ 0 };
	bool deck_reversed{ false };
#endif
	Replay last_replay;
	bool match_mode{ false };
	int match_kill{ 0 };
	unsigned char duel_count{ 0 };
	unsigned char tp_player{ 0 };
	unsigned char match_result[3]{};
	short time_limit[2]{};
#ifdef YGOPRO_SERVER_MODE
	short time_compensator[2]{};
	short time_backed[2]{};
	unsigned char last_game_msg{ 0 };
#endif
};

}

#endif //SINGLE_DUEL_H
