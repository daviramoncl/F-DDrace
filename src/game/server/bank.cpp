// made by fokkonaut

#include "bank.h"
#include "gamecontext.h"

CBank::CBank(CGameContext *pGameServer) : CHouse(pGameServer, HOUSE_BANK)
{
	for (int i = 0; i < MAX_CLIENTS; i++)
		m_aAssignmentMode[i] = ASSIGNMENT_NONE;
}

const char *CBank::GetWelcomeMessage(int ClientID)
{
	static char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "Welcome to the bank, %s! Press F4 to manage your bank account.", Server()->ClientName(ClientID));
	return aBuf;
}

const char *CBank::GetConfirmMessage(int ClientID)
{
	int Amount = GetAmount(m_aClients[ClientID].m_Page);
	static char aBuf[128];
	if (m_aAssignmentMode[ClientID] == ASSIGNMENT_DEPOSIT)
	{
		str_format(aBuf, sizeof(aBuf), "Are you sure that you want to deposit %d money from your wallet to your account?", Amount);
	}
	else if (m_aAssignmentMode[ClientID] == ASSIGNMENT_WITHDRAW)
	{
		str_format(aBuf, sizeof(aBuf), "Are you sure that you want to withdraw %d money from your account to your wallet?", Amount);
	}
	return aBuf;
}

const char *CBank::GetEndMessage(int ClientID)
{
	if (m_aClients[ClientID].m_State == STATE_CHOSE_ASSIGNMENT)
		return "You can't use the bank without an account. Check '/accountinfo'.";
	return "You cancelled the assignment.";
}

bool CBank::NotLoggedIn(int ClientID)
{
	if (GameServer()->m_apPlayers[ClientID]->GetAccID() < ACC_START)
	{
		EndSession(ClientID, true);
		m_aAssignmentMode[ClientID] = ASSIGNMENT_NONE;
		return true;
	}
	return false;
}

void CBank::OnSuccess(int ClientID)
{
	if (NotLoggedIn(ClientID))
		return;

	CPlayer *pPlayer = GameServer()->m_apPlayers[ClientID];
	CGameContext::AccountInfo *pAccount = &GameServer()->m_Accounts[pPlayer->GetAccID()];
	int Amount = GetAmount(m_aClients[ClientID].m_Page);
	char aMsg[128];

	if (m_aAssignmentMode[ClientID] == ASSIGNMENT_DEPOSIT)
	{
		if (pPlayer->m_WalletMoney < Amount)
		{
			GameServer()->SendChatTarget(ClientID, "You don't have enough money in your wallet to deposit this amount.");
			return;
		}

		pAccount->m_Money += Amount;
		pPlayer->m_WalletMoney -= Amount;

		str_format(aMsg, sizeof(aMsg), "You deposited %d money from your wallet to your bank account.", Amount);
		GameServer()->SendChatTarget(ClientID, aMsg);

	}
	else if (m_aAssignmentMode[ClientID] == ASSIGNMENT_WITHDRAW)
	{
		if (pAccount->m_Money < Amount)
		{
			GameServer()->SendChatTarget(ClientID, "You don't have enough money on your bank account to withdraw this amount.");
			return;
		}

		pAccount->m_Money -= Amount;
		pPlayer += Amount;

		str_format(aMsg, sizeof(aMsg), "You withdrew %d money from your wallet to your bank account.", Amount);
		GameServer()->SendChatTarget(ClientID, aMsg);
	}
}

void CBank::SetAssignment(int ClientID, int Dir)
{
	if (NotLoggedIn(ClientID))
		return;

	switch (Dir)
	{
	case -1: m_aAssignmentMode[ClientID] = ASSIGNMENT_WITHDRAW; break;
	case 1: m_aAssignmentMode[ClientID] = ASSIGNMENT_DEPOSIT; break;
	}

	SetPage(ClientID, AMOUNT_100);
}

void CBank::OnPageChange(int ClientID)
{
	char aMsg[512];
	if (m_aClients[ClientID].m_Page <= PAGE_MAIN)
	{
		m_aAssignmentMode[ClientID] = ASSIGNMENT_NONE;
		str_format(aMsg, sizeof(aMsg), "Welcome to the bank!\n\nPlease select your option:\nF3: Deposit (+)\nF4: Withdraw (-).\n\nOnce you selected an option, shoot to the right to go one step forward, and shoot left to go one step back.");
	}
	else
	{
		const char *pAssignment = m_aAssignmentMode[ClientID] == ASSIGNMENT_DEPOSIT ? "D E P O S I T" : m_aAssignmentMode[ClientID] == ASSIGNMENT_WITHDRAW ? "W I T H D R A W" : "U N K N O W N";
		str_format(aMsg, sizeof(aMsg), "Current balance: %lld\n\n%s\n\n", GameServer()->m_Accounts[GameServer()->m_apPlayers[ClientID]->GetAccID()].m_Money, pAssignment);

		for (int i = AMOUNT_100; i < NUM_PAGES_BANK; i++)
		{
			char aBuf[64];
			str_format(aBuf, sizeof(aBuf), "%s%d%s\n", i == m_aClients[ClientID].m_Page ? ">  " : "", GetAmount(i), i == m_aClients[ClientID].m_Page ? "  <" : "");
			str_append(aMsg, aBuf, sizeof(aMsg));
		}
	}

	SendWindow(ClientID, aMsg);
	m_aClients[ClientID].m_LastMotd = Server()->Tick();
}

int CBank::GetAmount(int Type)
{
	switch (Type)
	{
	case AMOUNT_100: return 100;
	case AMOUNT_1K: return 1000;
	case AMOUNT_5K: return 5000;
	case AMOUNT_10K: return 10000;
	case AMOUNT_50K: return 50000;
	case AMOUNT_100K: return 100000;
	case AMOUNT_500K: return 500000;
	case AMOUNT_1MIL: return 1000000;
	case AMOUNT_5MIL: return 5000000;
	case AMOUNT_10MIL: return 10000000;
	case AMOUNT_50MIL: return 50000000;
	case AMOUNT_100MIL: return 100000000;
	default: return 0;
	}
}
