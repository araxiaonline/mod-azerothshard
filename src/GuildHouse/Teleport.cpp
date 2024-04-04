/**
 *
 * @File : sc_npc_teleport.cpp
 *
 * @Authors : Wilibald09
 *
 * @Date : 28/08/2008
 *
 * @Version : 1.2
 *
 **/

#include "Teleport.h"
#include "Player.h"

#define TELE    nsNpcTel::CatDest
#define PAGE    nsNpcTel::Page
#define PAGEI   PAGE::Instance

nsNpcTel::VCatDest nsNpcTel::TabCatDest;

uint32 PAGE::operator [] (Player * const player) const
{
    for (VInst_t i(0); i < m_TabInstance.size(); ++i)
    {
        if (m_TabInstance[i].GetPlayer() == player)
            return m_TabInstance[i].GetPageId();
    }
    return 0;
}

PAGE::Instance & PAGE::operator () (Player * const player)
{
    for (VInst_t i(0); i < m_TabInstance.size(); ++i)
    {
        if (m_TabInstance[i].GetPlayer() == player)
            return m_TabInstance[i];
    }
    m_TabInstance.push_back(Instance(player));
    return m_TabInstance.back();
}

PAGE::Instance & PAGEI::operator = (const uint32 &id)
{
    m_PageId = id;
    return *this;
}

PAGE::Instance & PAGEI::operator ++ (void)
{
    ++m_PageId;
    return *this;
}

PAGE::Instance PAGEI::operator ++ (int32)
{
    Instance tmp (*this);
    ++m_PageId;
    return tmp;
}

PAGE::Instance & PAGEI::operator -- (void)
{
    --m_PageId;
    return *this;
}

PAGE::Instance PAGEI::operator -- (int32)
{
    Instance tmp (*this);
    --m_PageId;
    return tmp;
}

TELE::CatDest(const uint32 &id, const std::string &name,
              const Flag &flag, const uint64 &data0, const uint32 &data1)
    : m_id(id), m_name(name), m_flag(flag), m_data0(data0), m_data1(data1)
{
    m_TabDest.clear();
}

std::string TELE::GetName(const bool IsGM /* = false */) const
{
    if (!IsGM || m_flag != FLAG_TEAM)
        return m_name;

    switch (m_data0)
    {
      case TELEPORT_TEAM_HORDE:      return std::string(m_name + " (H)");
      case TELEPORT_TEAM_ALLIANCE:   return std::string(m_name + " (A)");
    }
    return m_name;
}

bool TELE::IsAllowedToTeleport(Player * const player) const
{
    if (player->IsGameMaster())
    {
        if (m_flag == FLAG_GMLEVEL)
            return player->GetSession()->GetSecurity() >= m_data0;
        return true;
    }

    switch (m_flag)
    {
      case FLAG_TEAM:
        switch (m_data0)
        {
          case TELEPORT_TEAM_HORDE:      return player->GetTeamId() == TEAM_HORDE;
          case TELEPORT_TEAM_ALLIANCE:   return player->GetTeamId() == TEAM_ALLIANCE;
          case TELEPORT_TEAM_ALL:        return true;
        }

      case FLAG_GUILD:
        return player->GetGuildId() == m_data0;

      case FLAG_GMLEVEL:
        return player->GetSession()->GetSecurity() >= m_data0;

      case FLAG_ISGM:
        return player->IsGameMaster();

      case FLAG_ACCOUNT:
        return player->GetSession()->GetAccountId() == m_data0;

      case FLAG_LEVEL:
        return player->getLevel() >= m_data0;

      case FLAG_ITEM:
        return player->HasItemCount(m_data0, m_data1, true);

      case FLAG_QUEST:
        if (m_data1 < MAX_QUEST_STATUS)
            return player->GetQuestStatus(m_data0) == m_data1;
        return player->GetQuestRewardStatus(m_data0);

      case FLAG_GENDER:
        return player->getGender() == m_data0;

      case FLAG_RACE:
        return player->getRace() == m_data0;

      case FLAG_CLASS:
        return player->getClass() == m_data0;

      case FLAG_REPUTATION:
        return player->GetReputationRank(m_data0) >= int32(m_data1);

      case FLAG_PLAYER:
        return player->GetGUID().GetCounter() == m_data0;
      default:
        return true;
    }

    LOG_DEBUG("server", "Invalid flag (category: {}). Important problem...", GetCatID());
    return false;
}

uint32 TELE::CountOfCategoryAllowedBy(Player * const player)
{
    uint32 count (0);
    for (VCatDest_t i(0); i < TabCatDest.size(); ++i)
    {
        if (TabCatDest[i].IsAllowedToTeleport(player))
            ++count;
    }
    return count;
}

bool nsNpcTel::IsValidData(const uint32 &cat,   const Flag &flag,
                           const uint64 &data0, const uint32 &data1)
{
    switch(flag)
    {
      case FLAG_TEAM:
        if (data1)
            LOG_DEBUG("server", "Invalid data1 (category: {}).", cat);
        if (data0 < MAX_FLAG_TEAM)
            return true;
        LOG_DEBUG("server", "Invalid data0 (Team) (category: {}).", cat);
        return false;

      case FLAG_GUILD:
        if (data1)
            LOG_DEBUG("server", "Invalid data1 (category: {}).", cat);
        if (data0)
            return true;
        LOG_DEBUG("server", "Invalid data0 (GuildID) (category: {}).", cat);
        return false;

      case FLAG_GMLEVEL:
        if (data1)
            LOG_DEBUG("server", "Invalid data1 (category: {}).", cat);
        if (0 < data0 && data0 < 256)
            return true;
        LOG_DEBUG("server", "Invalid data0 (GmLevel) (category: {}).", cat);
        return false;

      case FLAG_ISGM:
        if (data0)
            LOG_DEBUG("server", "Invalid data0 (category: {}).", cat);
        if (data1)
            LOG_DEBUG("server", "Invalid data1 (category: {}).", cat);
        return true;

      case FLAG_ACCOUNT:
       if (data1)
            LOG_DEBUG("server", "Invalid data1 (category: {}).", cat);
        if (data0)
            return true;
        LOG_DEBUG("server", "Invalid data0 (AccountID) (category: {}).", cat);
        return false;

      case FLAG_LEVEL:
        if (data1)
            LOG_DEBUG("server", "Invalid data1 (category: {}).", cat);
        if (0 < data0 && data0 < 256)
            return true;
        LOG_DEBUG("server", "Invalid data0 (Level) (category: {}).", cat);
        return false;

      case FLAG_ITEM:
        if (!data0)
        {
            LOG_DEBUG("server", "Invalid data0 (ItemID) (category: {}).", cat);
            return false;
        }
        if (data1)
            return true;
        LOG_DEBUG("server", "Invalid data1 (Item Count) (category: {}).", cat);
        return false;

      case FLAG_QUEST:
        if (!data0)
        {
            LOG_DEBUG("server", "Invalid data0 (QuestID) (category: {}).", cat);
            return false;
        }
        if (data1 < MAX_QUEST_STATUS + 1)
            return true;
        LOG_DEBUG("server", "Invalid data1 (Quest Status) (category: {]}).", cat);
        return false;

      case FLAG_GENDER:
        if (data1)
            LOG_DEBUG("server", "Invalid data1 (category: {}).", cat);
        if (data0 < GENDER_NONE)
            return true;
        LOG_DEBUG("server", "Invalid data0 (Gender) (category: {}).", cat);
        return false;

      case FLAG_RACE:
        if (data1)
            LOG_DEBUG("server", "Invalid data1 (category: {}).", cat);
        if (0 < data0 && data0 < MAX_RACES)
            return true;
        LOG_DEBUG("server", "Invalid data0 (Race) (category: {}).", cat);
        return false;

      case FLAG_CLASS:
        if (data1)
            LOG_DEBUG("server", "Invalid data1 (category: {]}).", cat);
        if (0 < data0 && data0 < MAX_CLASSES)
            return true;
        LOG_DEBUG("server", "Invalid data0 (Class) (category: {}).", cat);
        return false;

      case FLAG_REPUTATION:
        if (!data0)
        {
            LOG_DEBUG("server", "Invalid data0 (Faction/Reputation) (category: {}).", cat);
            return false;
        }
        if (data1 <= REP_EXALTED)
            return true;
        LOG_DEBUG("server", "Invalid data1 (Faction/Reputation) (category: {}).", cat);
        return false;

      case FLAG_PLAYER:
        if (data1)
            LOG_DEBUG("server", "Invalid data1 (category: {}).", cat);
        if (data0)
            return true;
        LOG_DEBUG("server", "Invalid data0 (PlayerGuid) (category: {}).", cat);
        return false;
      default:
            return false;
    }

    //sLog->outDebug(LOG_FILTER_TSCR, "Invalid flag (category: %u).", cat);
    return false;
}

void nsNpcTel::LoadNpcTele()
{
    QueryResult result = WorldDatabase.Query(R"(
            SELECT 
                `flag`,
                `data0`,
                `data1`,
                `cat_id`,
                C.`name` `namecat`,
                D.`name` `namedest`,
                `pos_X`,
                `pos_Y`,
                `pos_Z`,
                `orientation`,
                `map`,
                `level`,
                `cost`
            FROM 
                custom_npc_tele_category C,
                custom_npc_tele_destination D,
                custom_npc_tele_association A
            WHERE 
                C.`id` = `cat_id`
                AND D.`id` = `dest_id`
            ORDER BY `namecat`, `cat_id`, `namedest`
        )"
    );

    nsNpcTel::TabCatDest.clear();

    if (result)
    {
        //sLog->outString( "TSCR: Loading %s, %s and %s...", Table[0], Table[1], Table[2]);


        uint32 catid = 0;
        uint32 nbDest = 0;
        bool IsValidCat = true;
        bool FirstTime = true;

        do
        {
            Field *fields = result->Fetch();

            if (!IsValidCat && catid == fields[3].Get<uint32>() && !FirstTime)
                continue;

            IsValidCat = true;
            FirstTime = false;

            if (!nsNpcTel::IsValidData(fields[3].Get<uint32>(), (nsNpcTel::Flag)fields[0].Get<uint8>(),
                                       fields[1].Get<uint64>(), fields[2].Get<uint32>()))
            {
                IsValidCat = false;
                catid = fields[3].Get<uint32>();
                continue;
            }

            if (catid != fields[3].Get<uint32>())
            {
                catid = fields[3].Get<uint32>();
                nsNpcTel::CatDest categorie (catid, fields[4].Get<std::string>(), (nsNpcTel::Flag)fields[0].Get<uint8>(),
                                             fields[1].Get<uint64>(), fields[2].Get<uint32>());
                nsNpcTel::TabCatDest.push_back(categorie);
            }

            nsNpcTel::Dest item =
            {
                fields[5].Get<std::string>(), // Name
                fields[6].Get<float>(),       // X
                fields[7].Get<float>(),       // Y
                fields[8].Get<float>(),       // Z
                fields[9].Get<float>(),       // Orientation
                fields[10].Get<uint16>(),     // Map
                fields[11].Get<uint8>(),      // Level
                fields[12].Get<uint32>(),     // Cost
            };

            nsNpcTel::TabCatDest.back().AddDest(item);
            ++nbDest;
        } while (result->NextRow());

        LOG_INFO("server", "TSCR: >> Loaded {} npc_teleport.", nbDest);
    } 
    else LOG_INFO("server", "TSCR: WARNING >> Loaded 0 npc_teleport.");
}

#undef TELE
#undef PAGE
#undef PAGEI
