#include "SteamCBHandler.h"
#include "steam.h"

//defined in dllmain.cpp
extern void nlog(const char* str, ...);

CSteamCBHandler* gSteamCBHandler = NULL;
CSteamCBHandler* SteamCBHandler() { return gSteamCBHandler; }

void CSteamCBHandler::RequestEncryptedAppTicket(uint8_t *encryptedData, size_t encryptedDataLen)
{
	SteamAPICall_t hSteamAPICall = SteamUser()->RequestEncryptedAppTicket(encryptedData, encryptedDataLen);
	m_SteamCallResultEncryptedAppTicket.Set(hSteamAPICall, this, &CSteamCBHandler::OnRequestEncryptedAppTicket);
}

void CSteamCBHandler::OnP2PSessionRequest(P2PSessionRequest_t* pRequest) {
	nlog("User %llu is requesting a P2P session with us", pRequest->m_steamIDRemote.ConvertToUint64());
	SteamNetworking()->AcceptP2PSessionWithUser(pRequest->m_steamIDRemote);
}

void CSteamCBHandler::OnUserStatsReceived(UserStatsReceived_t* pReceived) {
	nlog("Stats were received for %llu = %i", pReceived->m_steamIDUser, pReceived->m_eResult);
}

void CSteamCBHandler::OnRequestEncryptedAppTicket(EncryptedAppTicketResponse_t* pEncryptedAppTicketResponse, bool bIOFailure)
{
	if (pEncryptedAppTicketResponse->m_eResult == k_EResultOK)
	{
		SteamUser()->GetEncryptedAppTicket(encryptedAppTicket, sizeof(encryptedAppTicket), &encryptedAppTicketSize);
		nlog("Got Steam encrypted appticket of size %i", encryptedAppTicketSize);
	}
	else
	{
		nlog("Failed to get encrypted app ticket (response %i), generating dummy ticket for crossplay", pEncryptedAppTicketResponse->m_eResult);
		
		// Generate a fake/dummy ticket to allow MS Store/Xbox users to proceed
		// This bypasses Steam ownership check while maintaining protocol compatibility
		memset(encryptedAppTicket, 0, sizeof(encryptedAppTicket));
		
		// Create a minimal dummy ticket structure (enough to not break base64 encoding)
		// The actual validation will be bypassed by Content_HasEntitlementOwnershipByRef_hook
		for (int i = 0; i < 0x80; i++)
		{
			encryptedAppTicket[i] = (uint8_t)(rand() & 0xFF);
		}
		
		encryptedAppTicketSize = 0x80;
		nlog("Generated dummy appticket of size %i for crossplay support", encryptedAppTicketSize);
	}
}

void CSteamCBHandler::OnGameRichPresenceJoinRequested(GameRichPresenceJoinRequested_t* pCallback)
{
	nlog("Steam invite (rich presence) accepted");
	pendingInvite = pCallback->m_steamIDFriend;
	hasPendingInvite = true;
}

void CSteamCBHandler::OnGameLobbyJoinRequested(GameLobbyJoinRequested_t* pCallback)
{
	nlog("Steam invite (lobby) accepted");
	pendingInvite = pCallback->m_steamIDFriend;
	hasPendingInvite = true;
}

void CSteamCBHandler::__OnValidateAuthTicket(ValidateAuthTicketResponse_t* pCallback)
{
	nlog("Auth ticket validation for %llu resulted in state %u", pCallback->m_SteamID.ConvertToUint64(), pCallback->m_eAuthSessionResponse);

	SteamUser()->EndAuthSession(pCallback->m_SteamID); // we do not care about VAC and we do not care about *exploit here*

	if (OnValidateAuthTicket)
	{
		OnValidateAuthTicket(pCallback);
	}
}

void CSteamCBHandler::OnLobbyEnter(LobbyEnter_t* pCallback)
{
	auto ownerXuid = SteamMatchmaking()->GetLobbyOwner(pCallback->m_ulSteamIDLobby); // <3
	if (ownerXuid != SteamUser()->GetSteamID())
	{
		SteamMatchmaking()->LeaveLobby(pCallback->m_ulSteamIDLobby);
		SteamMatchmaking()->CreateLobby(k_ELobbyTypePublic, 18);
	}
	else
	{
		char xuid[64]{ 0 };
		sprintf(xuid, "%llu", SteamUser()->GetSteamID().ConvertToUint64());
		SteamMatchmaking()->SetLobbyData(pCallback->m_ulSteamIDLobby, "connect_host", xuid);
		gLobbyID = pCallback->m_ulSteamIDLobby;
	}
}