#include <inttypes.h>
#include <string.h>
#include <math.h>
#include "env.h"
#include "includes.h"
#include "yw_internal.h"
#include "yw.h"
#include "yw_net.h"
#include "input.h"
#include "windp.h"
#include "loaders.h"

#include "yparobo.h"
#include "font.h"
#include "gui/uacommon.h"
#include "system/inivals.h"

extern uint32_t bact_id;


NC_STACK_bitmap * loadDisk_screen(NC_STACK_ypaworld *yw)
{
    SFXEngine::SFXe.StopMusicTrack();

    const char *v3;

    if ( yw->screen_width <= 360 )
        v3 = "disk320.ilbm";
    else if ( yw->screen_width > 600 )
        v3 = "disk640.ilbm";
    else
        v3 = "disk512.ilbm";

    std::string oldRsrc = Common::Env.SetPrefix("rsrc", "data:mc2res");

    NC_STACK_bitmap *disk = Utils::ProxyLoadImage({
        {NC_STACK_rsrc::RSRC_ATT_NAME, std::string(v3)},
        {NC_STACK_bitmap::BMD_ATT_CONVCOLOR, (int32_t)1}});

    Common::Env.SetPrefix("rsrc", oldRsrc);

    return disk;
}

void draw_splashScreen(NC_STACK_ypaworld *yw, NC_STACK_bitmap *splashScreen)
{
    if ( splashScreen )
    {
        GFX::rstr_arg204 a4;

        a4.pbitm = splashScreen->GetBitmap();

        a4.float4  = Common::FRect(-1.0, -1.0, 1.0, 1.0);
        a4.float14 = Common::FRect(-1.0, -1.0, 1.0, 1.0);

        GFX::displ_arg263 v4;
        v4.bitm = yw->pointers__bitm[5];
        v4.pointer_id = 6;

        GFX::Engine.SetCursor(v4.pointer_id, 0);

        GFX::Engine.BeginFrame();
        GFX::Engine.raster_func202(&a4);
        GFX::Engine.EndFrame();

        GFX::Engine.BeginFrame();
        GFX::Engine.raster_func202(&a4);
        GFX::Engine.EndFrame();
    }
}

void drawSplashScreenWithTOD(NC_STACK_ypaworld *yw, NC_STACK_bitmap *splashScreen, const std::string &text)
{
    if ( splashScreen )
    {
        GFX::rstr_arg204 a4;

        a4.pbitm = splashScreen->GetBitmap();

        a4.float4  = Common::FRect(-1.0, -1.0, 1.0, 1.0);
        a4.float14 = Common::FRect(-1.0, -1.0, 1.0, 1.0);

        GFX::displ_arg263 v4;
        v4.bitm = yw->pointers__bitm[5];
        v4.pointer_id = 6;

        GFX::Engine.SetCursor(v4.pointer_id, 0);

        GFX::Engine.BeginFrame();
        GFX::Engine.raster_func202(&a4);
        splashScreen_OutText(yw, text, yw->screen_width / 7, yw->screen_height / 5);
        GFX::Engine.EndFrame();

        GFX::Engine.BeginFrame();
        GFX::Engine.raster_func202(&a4);
        splashScreen_OutText(yw, text, yw->screen_width / 7, yw->screen_height / 5);
        GFX::Engine.EndFrame();
    }
}

void deleteSplashScreen(NC_STACK_ypaworld *yw, NC_STACK_bitmap *splashScreen)
{
    if ( splashScreen )
        delete_class_obj(splashScreen);
}

void NC_STACK_ypaworld::PowerStationErase(size_t id)
{
    PowerStationRef &station = _powerStations.at(id);

    if ( station.pCell )
    {
        station.pCell->w_id = 0;
        station.pCell->w_type = 0;

        if ( blg_map )
            (*blg_map)(station.Cell.x, station.Cell.y) = 0;
        
        station.pCell = NULL;
    }
    
    if ( id == _powerStations.size() - 1 )
        _powerStations.pop_back();
}

void sb_0x44ca90__sub5(NC_STACK_ypaworld *yw)
{
    memset(yw->field_80, 0, sizeof(yw_f80) * 8);
}

void sb_0x44ca90__sub2(NC_STACK_ypaworld *yw, TLevelDescription *mapp)
{
    if (!mapp->Palettes.empty())
    {
        if (!mapp->Palettes[0].empty())
        {
            NC_STACK_bitmap *ilbm = Utils::ProxyLoadImage({
                {NC_STACK_rsrc::RSRC_ATT_NAME, mapp->Palettes[0]},
                {NC_STACK_bitmap::BMD_ATT_HAS_COLORMAP, (int32_t)1}});

            if (ilbm)
            {
                GFX::Engine.SetPalette(*ilbm->getBMD_palette());
                Nucleus::Delete(ilbm);
            }
            else
            {
                ypa_log_out("WARNING: slot #%d [%s] init failed!\n", 0, mapp->Palettes[0].c_str());
            }
        }
    }
}

int NC_STACK_ypaworld::LevelCommonLoader(TLevelDescription *mapp, int levelID, int a5)
{
    int ok = 0;

    *mapp = TLevelDescription();

    ingamePlayerStatus.fill( World::TPlayerStatus() );

    timeStamp = 0;
    field_1a04 = 0;
    field_1a08 = 0;
    field_1a00 = 0;
    field_1a1c = 0;
    field_161c = 0;

    _levelInfo.LevelID = levelID;
    _levelInfo.Mode = a5;
    _levelInfo.State = TLevelInfo::STATE_PLAYING;
    _levelInfo.OwnerMask = 0;
    _levelInfo.UserMask = 0;

    field_1a60 = 0;
    field_1a98 = NULL;
    field_1aac = 0;
    current_bact = NULL;
    UserRobo = NULL;
    UserUnit = NULL;
    _UserRoboKidsList = NULL;
    field_1ab0 = 0;
    field_160c = 0;
    field_1610 = 0;
    field_7882 = 1;
    field_7886 = 1;
    field_788A = 0;
    field_81AF.clear();
    field_241c = 0;
    field_1628 = 0;
    field_162A = 0;
    field_1624 = 0;
    vehicle_sector_ratio_1 = 0;
    _currentBeamLoad = 0;
    field_1a20 = 0;

    _levelInfo.Gates.clear();
    _levelInfo.SuperItems.clear();

    _Gems.clear();
    
    memset(&field_81CB, 0, sizeof(yw_81cb));
    memset(field_1bac, 0, sizeof(int) * 8);

    dbg_num_sqd_max = 0;
    dbg_num_vhcl_max = 0;
    dbg_num_flk_max = 0;
    dbg_num_wpn_max = 0;
    dbg_num_robo_max = 0;

    if ( _gfxMode != GFX::Engine.GetGfxMode() || _gfxWindowed != GFX::Engine.GetGfxMode().windowed )
    {
        GFX::Engine.SetResolution(_gfxMode, _gfxWindowed);

        screen_width = GFX::Engine.GetScreenW();
        screen_height = GFX::Engine.GetScreenH();

        GFX::Engine.setWDD_cursor( (field_73CE & 0x40) != 0 );

        if ( screen_width >= 512 )
        {
            GFX::Engine.LoadFontByDescr( GetLocaleString(15, "MS Sans Serif,12,400,0") );
            Gui::UA::LoadFont( GetLocaleString(15, "MS Sans Serif,12,400,0") );
        }
        else
        {
            GFX::Engine.LoadFontByDescr( GetLocaleString(16, "Arial,8,400,0") );
            Gui::UA::LoadFont( GetLocaleString(16, "Arial,8,400,0") );
        }
    }

    NC_STACK_bitmap *diskScreenImage = loadDisk_screen(this);

    if ( diskScreenImage )
        draw_splashScreen(this, diskScreenImage);


    std::string oldRsrc = Common::Env.SetPrefix("rsrc", "data:fonts");

    int v19 = load_fonts_and_icons();

    Common::Env.SetPrefix("rsrc", oldRsrc);

    if ( !v19 )
        return 0;

    int tod = loadTOD(this, "tod.def");

    int next_tod = tod + 1;
    
    if ( next_tod + 2490 > 2512 )
        next_tod = 0;

    writeTOD(this, "tod.def", next_tod);

    if ( diskScreenImage )
    {
        drawSplashScreenWithTOD(this, diskScreenImage, GetLocaleString(tod + 2490, " "));
        deleteSplashScreen(this, diskScreenImage);
    }

    p_1_grp_cnt = 0;
    for (int i = 0; i < 8; i++)
    {
        p_1_grp[0][i] = 0;
        p_1_grp[1][i] = 0;
        p_1_grp[2][i] = 100000;
        p_1_grp[3][i] = 0;
    }

    _history.Clear();
    _historyLastIsTimeStamp = false;

    audio_volume = SFXEngine::SFXe.getMasterVolume();

    _voiceMessage.Reset();

    Common::Env.SetPrefix("rsrc", "data:");

    if ( sub_4DA41C(mapp, _mapRegions.MapRegions[_levelInfo.LevelID].MapDirectory) && mapp->IsOk() )
    {       
        Common::DeleteAndNull(&_script);
        
        if (mapp->EventLoopID >= 1 && mapp->EventLoopID <= 3)
        {
            _script = new World::LuaEvents(this);
            _script->LoadFile(fmt::sprintf("lesson%d.lua", mapp->EventLoopID));
            _script->CallInit(timeStamp);
        }
        else if (!_luaScriptName.empty())
        {
            _script = new World::LuaEvents(this);
            _script->LoadFile(_luaScriptName);
            _script->CallInit(timeStamp);
        }

        _energyAccumMap.Clear();
        _powerStations.reserve(256);
        _nextPSForUpdate = 0;

        sb_0x44ca90__sub5(this);

        Common::Env.SetPrefix("rsrc", fmt::sprintf("data:set%d", mapp->SetID));

        sb_0x44ca90__sub2(this, mapp);

        if ( yw_LoadSet(mapp->SetID) )
        {
            if ( yw_loadSky(this, mapp->SkyStr) )
                ok = 1;
        }
    }

    FFeedback_Init();

    return ok;
}

bool NC_STACK_ypaworld::LoadTypeMap(const std::string &mapName)
{
    if ( !typ_map )
        return false;

    SetMapSize(typ_map->Size());

    int32_t id = 0;
    for(int y = 0; y < _mapSize.y; y++)
    {
        for (int x = 0; x < _mapSize.x; x++)
        {
            cellArea &cell = _cells(x, y);
            
            cell.clear();
           
            cell.Id = id;
            cell.PosID = Common::Point(x, y);
           
            secType *sectp = &secTypes[ typ_map->At(x, y) ];

            cell.type_id = typ_map->At(x, y);
            cell.comp_type = sectp->field_0;
            cell.energy_power = 0;

            if ( sectp->field_0 == 1)
            {
                cell.buildings_health.At(0, 0) = sectp->buildings.At(0, 0)->build_health;
            }
            else
            {
                for (int bldY = 0; bldY < 3; bldY++)
                    for (int bldX = 0; bldX < 3; bldX++)
                        cell.buildings_health.At(bldX, bldY) = sectp->buildings.At(bldX, bldY)->build_health;
            }
            
            id++;
        }
    }
    return true;
}

bool NC_STACK_ypaworld::LoadOwnerMap(const std::string &mapName)
{
    for ( auto &cnt : sectors_count_by_owner )
        cnt = 0;

    if ( !own_map )
        return 0;

    if ( (int)own_map->Width() != _mapSize.x || (int)own_map->Height() != _mapSize.y )
    {
        ypa_log_out("Mapsize mismatch %s: is [%d,%d], should be [%d,%d].\n", mapName.c_str(), own_map->Width(), own_map->Height(), _mapSize.x, _mapSize.y);
        delete own_map;
        own_map = NULL;
        return 0;
    }


    for (uint32_t yy = 0; yy < own_map->Height(); yy++)
    {
        for (uint32_t xx = 0; xx < own_map->Width(); xx++)
        {
            if ( IsGamePlaySector( {(int)xx, (int)yy} ) )
            {
                _cells(xx, yy).owner = own_map->At(xx, yy);
                sectors_count_by_owner[ own_map->At(xx, yy) ]++;
            }
            else
            {
                _cells(xx, yy).owner = 0;
                sectors_count_by_owner[ 0 ]++;
            }
        }
    }

    return 1;
}

bool NC_STACK_ypaworld::LoadHightMap(const std::string &mapName)
{
    if ( !hgt_map )
        return false;

    if ( (int)hgt_map->Width() != _mapSize.x || (int)hgt_map->Height() != _mapSize.y )
    {
        ypa_log_out("Mapsize mismatch %s: is [%d,%d], should be [%d,%d].\n", mapName.c_str(), hgt_map->Width(), hgt_map->Height(), _mapSize.x, _mapSize.y);
        delete hgt_map;
        hgt_map = NULL;
        return false;
    }

    for (int y = 0; y < _mapSize.y; y++)
    {
        for (int x = 0; x < _mapSize.x; x++)
            _cells(x, y).height = (-100.0) * hgt_map->At(x, y);
    }

    for (int y = 1; y < _mapSize.y; y++)
    {
        for (int x = 1; x < _mapSize.x; x++)
        {
            _cells(x, y).averg_height = (_cells(x    ,     y).height +
                                         _cells(x - 1,     y).height +
                                         _cells(x - 1, y - 1).height +
                                         _cells(x    , y - 1).height ) / 4.0;
        }
    }

    return true;
}

bool NC_STACK_ypaworld::yw_createRobos(const std::vector<MapRobo> &Robos)
{
    if ( _levelInfo.Mode != 1 )
    {
        _levelInfo.OwnerMask = 0;
        _levelInfo.UserMask = 2;
        
        bool first = true;

        for ( const MapRobo &roboInf : Robos)
        {
            ypaworld_arg136 v14;
            v14.stPos = vec3d::X0Z(roboInf.Pos) - vec3d::OY(30000.0);
            v14.vect = vec3d::OY(50000.0);
            v14.flags = 0;

            ypaworld_arg146 v15;
            v15.vehicle_id = roboInf.VhclID;
            v15.pos = roboInf.Pos;

            ypaworld_func136(&v14);

            if ( v14.isect )
                v15.pos.y += v14.isectPos.y;

            NC_STACK_yparobo *robo = dynamic_cast<NC_STACK_yparobo *>( ypaworld_func146(&v15) );

            if ( robo )
            {
                int v20 = 0;

                ypaworld_func134(robo);

                int v12;

                if ( !first )
                {
                    v12 = roboInf.Energy;
                }
                else
                {
                    v12 = roboInf.Energy / 4;

                    if ( v12 < _maxRoboEnergy )
                    {
                        v12 = _maxRoboEnergy;
                        v20 = _maxReloadConst;
                    }

                }

                robo->_owner = roboInf.Owner;
                robo->_energy = v12;
                robo->_energy_max = v12;

                if ( !v20 )
                {
                    if ( roboInf.ReloadConst )
                        v20 = roboInf.ReloadConst;
                    else
                        v20 = robo->_energy_max;
                }

                robo->_reload_const = v20;

                robo->setBACT_bactCollisions(1);
                robo->setROBO_fillMode(15);
                robo->setROBO_battVehicle(v12);
                robo->setROBO_battBeam(v12);

                _levelInfo.OwnerMask |= 1 << roboInf.Owner;

                robo->setROBO_epConquer(roboInf.ConBudget);
                robo->setROBO_epDefense(roboInf.DefBudget);
                robo->setROBO_epRadar(roboInf.RadBudget);
                robo->setROBO_epPower(roboInf.PowBudget);
                robo->setROBO_epSafety(roboInf.SafBudget);
                robo->setROBO_epChangeplace(roboInf.CplBudget);
                robo->setROBO_epRobo(roboInf.RobBudget);
                robo->setROBO_epReconnoitre(roboInf.RecBudget);
                robo->setROBO_viewAngle(roboInf.ViewAngle);
                robo->setROBO_safDelay(roboInf.SafDelay);
                robo->setROBO_powDelay(roboInf.PowDelay);
                robo->setROBO_cplDelay(roboInf.CplDelay);
                robo->setROBO_radDelay(roboInf.RadDelay);
                robo->setROBO_defDelay(roboInf.DefDelay);
                robo->setROBO_conDelay(roboInf.ConDelay);
                robo->setROBO_recDelay(roboInf.RecDelay);
                robo->setROBO_robDelay(roboInf.RobDelay);

                if ( first )
                {
                    robo->setBACT_viewer(1);
                    robo->setBACT_inputting(1);
                }
            }
            
            first = false;
        }
    }
    return true;
}

bool NC_STACK_ypaworld::LoadBlgMap(const std::string &mapName)
{
    if ( !blg_map )
        return false;

    if ( (int)blg_map->Width() != _mapSize.x || (int)blg_map->Height() != _mapSize.y )
    {
        ypa_log_out("Mapsize mismatch %s: is [%d,%d], should be [%d,%d].\n", mapName.c_str(), blg_map->Width(), blg_map->Height(), _mapSize.x, _mapSize.y);
        delete blg_map;
        blg_map = NULL;
        return false;
    }

    for ( int y = 0; y < _mapSize.y; y++)
    {
        for ( int x = 0; x < _mapSize.x; x++)
        {
            int blg = blg_map->At(x, y);
            cellArea &cell = _cells(x, y);
            
            if (blg && cell.owner)
            {
                ypaworld_arg148 arg148;

                arg148.ownerID = cell.owner;
                arg148.ownerID2 = cell.owner;
                arg148.blg_ID = blg;
                arg148.field_C = 1;
                arg148.field_18 = 0;
                arg148.x = x;
                arg148.y = y;

                ypaworld_func148(&arg148);
            }
        }
    }

    return true;
}

void NC_STACK_ypaworld::yw_InitSquads(const std::vector<MapSquad> &squads)
{
    if ( _levelInfo.Mode != 1 )
    {
        size_t i = 0;
        for ( const MapSquad &squad : squads )
        {
            NC_STACK_yparobo *robo = NULL;

            for( NC_STACK_ypabact *unit : _unitsList )
            {
                if ( unit->_bact_type == BACT_TYPES_ROBO && unit->_owner == squad.Owner)
                {
                    robo = dynamic_cast<NC_STACK_yparobo *>(unit);
                    break;
                }
            }

            if ( !robo )
            {
                ypa_log_out("WARNING: yw_InitSquads(): no host robo for squad[%d], owner %d!\n", i, squad.Owner);
            }
            else
            {
                vec3d squadPos;

                ypaworld_arg136 arg136;
                arg136.stPos.x = squad.X;
                arg136.stPos.y = -50000.0;
                arg136.stPos.z = squad.Z;
                arg136.vect = vec3d::OY(100000.0);
                arg136.flags = 0;
                ypaworld_func136(&arg136);

                if ( arg136.isect )
                    squadPos = arg136.isectPos - vec3d::OY(50.0);
                else
                {
                    yw_130arg sect_info;
                    sect_info.pos_x = squad.X;
                    sect_info.pos_z = squad.Z;

                    if ( !GetSectorInfo(&sect_info) )
                    {
                        ypa_log_out("yw_InitSquads(): no valid position for squad[%d]!\n", i);
                        return;
                    }

                    squadPos.x = squad.X;
                    squadPos.y = sect_info.pcell->height;
                    squadPos.z = squad.Z;
                }
                // Create squad by robo method
                robo->MakeSquad( std::vector<int>(squad.Count, squad.VhclID), squadPos, squad.Useable); // yparobo_func133
            }
            
            i++;
        }
    }
}

void NC_STACK_ypaworld::InitBuddies()
{
    if ( !_levelInfo.Buddies.empty() )
    {
        int squad_sn = 0;
        
        std::vector<TMapBuddy> buds = _levelInfo.Buddies;
        while ( 1 )
        {
            std::vector<int> VhclIDS;
            int wrkID = -1;
            for (std::vector<TMapBuddy>::iterator it = buds.begin(); it != buds.end(); )
            {
                if (wrkID == -1 || wrkID == it->CommandID )
                {
                    wrkID = it->CommandID;
                    VhclIDS.push_back(it->Type);
                    it = buds.erase(it);
                }
                else
                    it++;
            }

            if ( wrkID == -1 )
                break;

            vec3d squadPos =    UserRobo->_position +  
                                vec3d(  sin(squad_sn * 1.745) * 500.0,
                                        0.0,
                                        cos(squad_sn * 1.745) * 500.0 );

            ypaworld_arg136 arg136;
            arg136.stPos = squadPos.X0Z() + vec3d(0.5, -50000.0, 0.75);
            arg136.vect = vec3d::OY(100000.0);
            arg136.flags = 0;

            ypaworld_func136(&arg136);

            if ( arg136.isect )
                squadPos.y = arg136.isectPos.y + -100.0;

            NC_STACK_yparobo *robo = dynamic_cast<NC_STACK_yparobo *>(UserRobo);

            robo->MakeSquad(VhclIDS, squadPos, true); //robo 133 method

            squad_sn++;
        }
    }
}

void NC_STACK_ypaworld::yw_InitTechUpgradeBuildings()
{
    field_2b7c = 0;
    last_modify_build = 0;
    last_modify_vhcl = 0;
    last_modify_weapon = 0;
    field_2b78 = -1;

    for (size_t i = 0; i < _Gems.size(); i++)
    {
        MapGem &gem = _Gems[i];
        cellArea &cell = _cells(gem);

        if (gem.BuildingID)
        {
            if ( cell.w_type != 3 || gem.BuildingID != cell.w_id )
            {
                ypaworld_arg148 arg148;
                arg148.ownerID = cell.owner;
                arg148.ownerID2 = cell.owner;
                arg148.blg_ID = gem.BuildingID;
                arg148.x = gem.SecX;
                arg148.y = gem.SecY;
                arg148.field_C = 1;
                arg148.field_18 = 0;

                ypaworld_func148(&arg148);
            }
        }

        cell.w_type = 4;
        cell.w_id = i;
    }
}

void NC_STACK_ypaworld::InitGates()
{
    for (size_t i = 0; i < _levelInfo.Gates.size(); i++)
    {
        MapGate &gate = _levelInfo.Gates[i];

        gate.PCell = &_cells(gate.SecX, gate.SecY);

        ypaworld_arg148 arg148;
        arg148.ownerID = gate.PCell->owner;
        arg148.ownerID2 = gate.PCell->owner;
        arg148.blg_ID = gate.ClosedBldID;
        arg148.field_C = 1;
        arg148.x = gate.SecX;
        arg148.y = gate.SecY;
        arg148.field_18 = 0;

        ypaworld_func148(&arg148);

        gate.PCell->w_type = 5;
        gate.PCell->w_id = i;

        for (MapKeySector &ks : gate.KeySectors)
        {
            if ( IsGamePlaySector( {ks.x, ks.y} ) )
            {
                ks.PCell = &_cells(ks.x, ks.y);
            }
        }
    }
}

void NC_STACK_ypaworld::InitSuperItems()
{
    for ( size_t i = 0; i < _levelInfo.SuperItems.size(); i++ )
    {
        TMapSuperItem &sitem = _levelInfo.SuperItems[i];
        
        sitem.PCell = &_cells(sitem.Sector);

        ypaworld_arg148 arg148;
        arg148.ownerID = sitem.PCell->owner;
        arg148.ownerID2 = sitem.PCell->owner;
        arg148.blg_ID = sitem.InactiveBldID;
        arg148.field_C = 1;
        arg148.x = sitem.Sector.x;
        arg148.y = sitem.Sector.y;
        arg148.field_18 = 0;

        ypaworld_func148(&arg148);

        sitem.PCell->w_type = 8;
        sitem.PCell->w_id = i;

        for ( MapKeySector &ks : sitem.KeySectors )
        {
            if ( IsGamePlaySector( {ks.x, ks.y} ) )
                ks.PCell = &_cells(ks.x, ks.y);

        }

        sitem.ActiveTime = 0;
        sitem.TriggerTime = 0;
        sitem.ActivateOwner = 0;
        sitem.State = TMapSuperItem::STATE_INACTIVE;
    }
}

void NC_STACK_ypaworld::UpdatePowerEnergy()
{
    // Apply power to sectors and clean power matrix for next compute iteration.

    for (int y = 0; y < _mapSize.y; y++)
    {
        for (int x = 0; x < _mapSize.x; x++)
        {
            cellArea &cell = _cells(x, y);
            EnergyAccum &accum = _energyAccumMap(x, y);

            accum.Owner = cell.owner;
            cell.energy_power = accum.Energy; // Apply power to cell
            accum.Energy = 0; // Clean matrix's power
        }
    }

    _nextPSForUpdate = 0; // Next power station for recompute power is first
}


void NC_STACK_ypaworld::CellSetOwner(cellArea *cell, char secX, char secY, uint8_t owner)
{
    if ( cell->owner != owner )
    {
        HistoryEventAdd( World::History::Conq(secX, secY, owner) );

        if ( cell->w_type == 2 )
            HistoryEventAdd( World::History::PowerST(secX, secY, owner) );

        sectors_count_by_owner[cell->owner]--;
        sectors_count_by_owner[owner]++;

        cell->owner = owner;
    }
}

void NC_STACK_ypaworld::CellSetNewOwner(int secX, int secY, cellArea *cell, NC_STACK_ypabact *a5, int newOwner)
{
    int energon[World::FRACTION_MAXCOUNT];

    if ( newOwner < World::OWNER_UNKNOW )
    {
        newOwner = cell->owner;

        for( int &e : energon )
            e = 0;

        for ( NC_STACK_ypabact* &nod : cell->unitsList )
            energon[nod->_owner] += nod->_energy;

        energon[0] = 0;

        for (int i = 0; i < World::FRACTION_MAXCOUNT; i++)
        {
            if ( energon[i] > energon[newOwner] )
                newOwner = i;
        }
    }

    if ( cell->owner != newOwner )
    {
        if ( cell->w_type == 2 )
        {
            if ( UserRobo->_owner == newOwner )
            {
                if ( a5 )
                {
                    yw_arg159 v21;
                    v21.unit = a5;
                    v21.Priority = 78;
                    v21.MsgID = 45;

                    ypaworld_func159(&v21);
                }
            }
            else if ( cell->owner == UserRobo->_owner )
            {
                yw_arg159 v24;
                v24.unit = NULL;
                v24.Priority = 78;
                v24.MsgID = 67;

                ypaworld_func159(&v24);
            }
        }
        else
        {
            for ( const MapGate &gate : _levelInfo.Gates )
            {
                for ( const MapKeySector &ks : gate.KeySectors )
                {
                    if ( cell == ks.PCell )
                    {
                        if ( UserRobo->_owner == newOwner )
                        {
                            yw_arg159 v23;
                            v23.unit = NULL;
                            v23.Priority = 80;
                            v23.MsgID = 82;

                            ypaworld_func159(&v23);
                        }
                        else if ( UserRobo->_owner == cell->owner )
                        {
                            yw_arg159 v22;
                            v22.unit = NULL;
                            v22.Priority = 80;
                            v22.MsgID = 81;

                            ypaworld_func159(&v22);
                        }
                    }
                }
            }
        }
    }

    CellSetOwner(cell, secX, secY, newOwner);
}

void NC_STACK_ypaworld::CellCheckHealth(cellArea *cell, int secX, int secY, int newOwner, NC_STACK_ypabact *a6)
{
    if ( IsGamePlaySector( {secX, secY} ) )
    {
        int helth = 0;

        for (auto h : cell->buildings_health)
            helth += h;

        if ( cell->w_type == 2 )
        {
            if ( helth )
            {
                int effPower = (helth * _powerStations.at( cell->w_id ).Power) / 255 ;

                if ( effPower < 0 )
                    effPower = 0;
                else if ( effPower > 255 )
                    effPower = 255;

                _powerStations.at( cell->w_id ).EffectivePower = effPower;
            }
            else
            {
                PowerStationErase(cell->w_id);
            }
        }

        /* Fix for vanilla game bug that recalc sector owner after loading
         * saved game. */
        if (newOwner != World::OWNER_NOCHANGE)
        {
            if ( cell->comp_type == 1 )
            {
                if ( helth < 224 )
                    CellSetNewOwner(secX, secY, cell, a6, newOwner);
            }
            else if ( helth < 1728 )
            {
                CellSetNewOwner(secX, secY, cell, a6, newOwner);
            }
        }
    }
    else
    {
        CellSetOwner(cell, secX, secY, 0);
    }
}



TSectorCollision NC_STACK_ypaworld::sub_44DBF8(int _dx, int _dz, int _dxx, int _dzz, int flags)
{
    int v8 = flags;
    
    TSectorCollision tmp;
    tmp.sklt = NULL;
    tmp.Flags = 0;
    tmp.CollisionType = 0;
    
    

    if ( _dxx > 0 && _dxx < 4 * _mapSize.x - 1 && _dzz > 0  &&  _dzz < 4 * _mapSize.y - 1)
    {
        tmp.Cell = Common::Point(_dxx / 4, _dzz / 4);
        
        cellArea &cell = _cells(tmp.Cell);

        if ( _dxx % 4 && _dzz % 4 )
        {
            tmp.CollisionType = 1;

            int v14, v16;

            if ( cell.comp_type == 1 )
            {
                v14 = 0;
                v16 = 0;

                if ( (_dxx / 4) == (_dx / 4)  &&  (_dz / 4) == (_dzz / 4) )
                    v8 = flags & ~1;

                tmp.pos = World::SectorIDToCenterPos3(tmp.Cell);
                tmp.pos.y = cell.height;
            }
            else
            {
                v16 = (_dxx % 4) - 1;
                v14 = 2 - ((_dzz % 4) - 1);

                if ( _dxx == _dx && _dzz == _dz )
                    v8 = flags & ~1;

                tmp.pos.z = -(_dzz * 300.0);
                tmp.pos.x = _dxx * 300.0;
                tmp.pos.y = cell.height;
            }

            tmp.Flags = v8;

            int model_id = GetLegoBld(&cell, v16, v14);

            if ( v8 & 1 )
                tmp.sklt = legos[model_id].selected_sklt_intern;
            else
                tmp.sklt = legos[model_id].sklt_obj_intern;
        }
        else
        {
            tmp.pos.y = 0;
            tmp.pos.x = _dxx * 300.0;
            tmp.pos.z = -(_dzz * 300.0);

            if ( _dxx == _dx && _dzz == _dz )
                v8 = flags & ~1;

            tmp.Flags = v8;

            if ( _dxx % 4 == 0 && _dzz % 4 == 0)
            {
                tmp.sklt = FillerCross->GetSkelet();
                tmp.CollisionType = 4;
            }
            else if ( _dxx % 4 == 0 && _dzz % 4 != 0 )
            {
                tmp.sklt = FillerSide->GetSkelet();
                tmp.CollisionType = 2;
            }
            else if ( _dxx % 4 != 0 && _dzz % 4 == 0 )
            {
                tmp.sklt = FillerSide->GetSkelet();
                tmp.CollisionType = 3;
            }
        }

        if ( tmp.CollisionType && !tmp.sklt )
        {
            ypa_log_out("yw_GetSklt: WARNING, not CZT_INVALID, but Sklt NULL!\n");

            const char *v17 = "UNKNOWN";

            switch ( tmp.CollisionType )
            {
            case 4:
                v17 = "czt_cross_slurp";
                break;
            case 2:
                v17 = "czt_vside_slurp";
                break;
            case 3:
                v17 = "czt_hside_slurp";
                break;
            case 1:
                v17 = "czt_lego";
                break;
            default:
                break;
            }
            ypa_log_out("    Type=%s, sec_x=%d, sec_y=%d.\n", v17, tmp.Cell.x, tmp.Cell.y);

            tmp.CollisionType = 0;
        }
    }
    
    return tmp;
}

void sub_44DF60(UAskeleton::Data *arg, int id)
{
    UAskeleton::Polygon &tr = arg->polygons[id];

    int vtx1 = arg->polygons[id].v[0];
    int vtx2 = arg->polygons[id].v[1];
    int vtx3 = arg->polygons[id].v[2];

    vec3d tmp  = arg->POO[vtx2] - arg->POO[vtx1];
    vec3d tmp2 = arg->POO[vtx3] - arg->POO[vtx2];

    vec3d tmp3 = tmp * tmp2;
    tmp3.normalise();

    tr.A = tmp3.x;
    tr.B = tmp3.y;
    tr.C = tmp3.z;

    tr.D = -tmp3.dot( arg->POO[vtx1] );
}

void NC_STACK_ypaworld::sub_44E07C(TSectorCollision &arg)
{
    if ( arg.CollisionType == 2 )
    {
        cellArea &cur = _cells(arg.Cell);
        cellArea &left = _cells(arg.Cell + Common::Point(-1, 0));

        if ( !(arg.Flags & 1) || fabs( (int)(cur.height - left.height)) < 500.0 )
        {

            arg.sklt->POO[0].y = left.height;
            arg.sklt->POO[1].y = cur.height;
            arg.sklt->POO[2].y = cur.height;
            arg.sklt->POO[3].y = left.height;

            sub_44DF60(arg.sklt, 0);
        }
        else
        {
            arg.sklt = colsub_sklt_intrn;

            if ( cur.height > left.height )
                arg.pos.y = cur.height;
            else
                arg.pos.y = left.height;
        }
    }
    else if ( arg.CollisionType == 3 )
    {
        cellArea &cur = _cells(arg.Cell);
        cellArea &up = _cells(arg.Cell + Common::Point(0, -1));

        if ( !(arg.Flags & 1) || fabs( (int)(cur.height - up.height)) < 500.0 )
        {
            arg.sklt->POO[0].y = up.height;
            arg.sklt->POO[1].y = up.height;
            arg.sklt->POO[2].y = cur.height;
            arg.sklt->POO[3].y = cur.height;

            sub_44DF60(arg.sklt, 0);
        }
        else
        {
            arg.sklt = colsub_sklt_intrn;

            if ( cur.height > up.height )
                arg.pos.y = cur.height;
            else
                arg.pos.y = up.height;
        }
    }
    else if ( arg.CollisionType == 4 )
    {

        int kk = 0;

        cellArea &cur = _cells(arg.Cell);
        cellArea &left = _cells(arg.Cell + Common::Point(-1, 0));
        cellArea &up = _cells(arg.Cell + Common::Point(0, -1));
        cellArea &leftup = _cells(arg.Cell + Common::Point(-1, -1));

        if ( arg.Flags & 1 )
        {
            float cs = cur.height;
            float ls = left.height;
            float us = up.height;
            float lus = leftup.height;

            float v15, v16, v17, v18;

            if ( cs >= ls )
                v15 = ls;
            else
                v15 = cs;

            if ( us >= lus )
                v16 = lus;
            else
                v16 = us;

            if ( v15 < v16 )
                v16 = v15;

            if ( cs <= ls )
                v17 = ls;
            else
                v17 = cs;

            if ( us <= lus )
                v18 = lus;
            else
                v18 = us;

            if ( v17 > v18 )
                v18 = v17;

            if ( fabs( (int)(v18 - v16) ) > 300.0 )
            {
                arg.sklt = colsub_sklt_intrn;
                arg.pos.y = v18;
                kk = 1;
            }
        }
        if ( !kk )
        {
            arg.sklt->POO[0].y = leftup.height;
            arg.sklt->POO[1].y = up.height;
            arg.sklt->POO[2].y = cur.height;
            arg.sklt->POO[3].y = left.height;
            arg.sklt->POO[4].y = cur.averg_height;

            sub_44DF60(arg.sklt, 0);
            sub_44DF60(arg.sklt, 1);
            sub_44DF60(arg.sklt, 2);
            sub_44DF60(arg.sklt, 3);
        }
    }
}

int sub_44D36C(const vec3d &v, int id, const UAskeleton::Data *sklt)
{
    int v7 = 0;

    const UAskeleton::Polygon &tr = sklt->polygons[id];
    float nX = fabs(tr.A);
    float nY = fabs(tr.B);
    float nZ = fabs(tr.C);

    float maxAx = (nX <= nY ? nY : nX );
    maxAx = (maxAx <= nZ ? nZ : maxAx);

    if ( maxAx == nX )
    {
        int prev = sklt->polygons[id].num_vertices - 1;

        for (int i = 0; i < sklt->polygons[id].num_vertices; i++)
        {
            const UAskeleton::Vertex &cur = sklt->POO[ sklt->polygons[id].v[i] ];
            const UAskeleton::Vertex &prv = sklt->POO[ sklt->polygons[id].v[prev] ];

            if ( ( (prv.z <= v.z && v.z < cur.z) ||
                    (cur.z <= v.z && v.z < prv.z) ) &&
                    prv.y + (cur.y - prv.y) * (v.z - prv.z) / (cur.z - prv.z) > v.y )
            {
                v7 = v7 == 0;
            }

            prev = i;
        }
    }
    else if ( maxAx == nY )
    {
        int prev = sklt->polygons[id].num_vertices - 1;

        for (int i = 0; i < sklt->polygons[id].num_vertices; i++)
        {
            const UAskeleton::Vertex &cur = sklt->POO[ sklt->polygons[id].v[i] ];
            const UAskeleton::Vertex &prv = sklt->POO[ sklt->polygons[id].v[prev] ];

            if ( ( (prv.z <= v.z && v.z < cur.z) ||
                    (cur.z <= v.z && v.z < prv.z) ) &&
                    prv.x + (cur.x - prv.x) * (v.z - prv.z) / (cur.z - prv.z) > v.x )
            {
                v7 = v7 == 0;
            }

            prev = i;
        }
    }
    else if ( maxAx == nZ )
    {
        int prev = sklt->polygons[id].num_vertices - 1;

        for (int i = 0; i < sklt->polygons[id].num_vertices; i++)
        {
            const UAskeleton::Vertex &cur = sklt->POO[ sklt->polygons[id].v[i] ];
            const UAskeleton::Vertex &prv = sklt->POO[ sklt->polygons[id].v[prev] ];

            if ( ( (prv.y <= v.y && v.y < cur.y) ||
                    (cur.y <= v.y && v.y < prv.y) ) &&
                    prv.x + (cur.x - prv.x) * (v.y - prv.y) / (cur.y - prv.y) > v.x )
            {
                v7 = v7 == 0;
            }

            prev = i;
        }
    }
    return v7;
}

void NC_STACK_ypaworld::sub_44D8B8(ypaworld_arg136 *arg, const TSectorCollision &loc)
{
    for ( size_t i = 0; i < loc.sklt->polygons.size(); i++)
    {
        UAskeleton::Polygon &tr = loc.sklt->polygons[i];
        vec3d norm = tr.Normal();

        float v11 = norm.dot(arg->vect);
        if ( v11 > 0.0 )
        {
            float v19 = -(norm.dot( arg->stPos ) + tr.D) / v11;
            if ( v19 > 0.0 && v19 <= 1.0 && v19 < arg->tVal )
            {
                vec3d px = arg->vect * v19 + arg->stPos;

                if ( sub_44D36C(px, i, loc.sklt) )
                {
                    arg->isect = 1;
                    arg->tVal = v19;
                    arg->isectPos = loc.pos + px;
                    arg->polyID = i;
                    arg->skel = loc.sklt;
                }
            }
        }
    }
}


void NC_STACK_ypaworld::FFeedback_VehicleChanged()
{
    if ( field_739A )
    {
        if ( field_73CE & (4 | 8) )
            return;
    }

    if ( UserUnit )
    {
        field_7562 = timeStamp;

        if ( input_class )
            input_class->ForceFeedback(NC_STACK_winp::FF_STATE_STOP, NC_STACK_winp::FF_TYPE_ALL);

        int effectType;
        float v13;
        float v14;
        float v15;
        float v16;
        float v17;
        float v18;
        float v19;
        float v22;
        float v24;
        float v25;


        switch ( UserUnit->_bact_type )
        {
        case BACT_TYPES_BACT:
            effectType = NC_STACK_winp::FF_TYPE_HELIENGINE;
            v16 = 300.0;
            v13 = 800.0;
            v15 = 1.0;
            v14 = 2.0;
            v25 = 1.0;
            v18 = 0.0;
            v17 = 0.7;
            v24 = 0.3;
            v22 = 1.0;
            v19 = 0.5;
            break;

        case BACT_TYPES_TANK:
        case BACT_TYPES_CAR:
            effectType = NC_STACK_winp::FF_TYPE_TANKENGINE;
            v16 = 200.0;
            v13 = 500.0;
            v15 = 0.6;
            v14 = 1.0;
            v25 = 1.0;
            v22 = 1.0;
            v17 = 0.1;
            v24 = 0.3;
            v19 = 0.4;
            v18 = 0.0;
            break;

        case BACT_TYPES_FLYER:
            effectType = NC_STACK_winp::FF_TYPE_JETENGINE;
            v16 = 200.0;
            v13 = 500.0;
            v15 = 1.0;
            v14 = 2.0;
            v25 = 1.0;
            v18 = 0.0;
            v17 = 0.1;
            v24 = 0.3;
            v22 = 1.0;
            v19 = 0.75;
            break;

        default:
            effectType = -1;
            break;
        }

        if ( effectType != -1 )
        {
            float v4 = (UserUnit->_mass - v16) / (v13 - v16);
            float v5 = (UserUnit->_maxrot - v15) / (v14 - v15);

            float v21 = (v19 - v24) * v4 + v24;
            float v23 = (v18 - v22) * v4 + v22;

            float v20 = (v17 - v25) * v5 + v25;

            if ( v21 < v24)
                v21 = v24;
            else if (v21 > v19)
                v21 = v19;

            if ( v23 < v18 )
                v23 = v18;
            else if ( v23 > v22)
                v23 = v22;

            if ( v20 < v17)
                v20 = v17;
            else if ( v20 > v25)
                v20 = v25;

            field_7566 = effectType;
            field_756A = v23;
            field_756E = v21;

            if ( input_class )
                input_class->ForceFeedback(NC_STACK_winp::FF_STATE_START, effectType, v21, v23);

            if ( input_class )
                input_class->ForceFeedback(NC_STACK_winp::FF_STATE_START, NC_STACK_winp::FF_TYPE_ROTDAMPER, v20);
        }
    }
}




NC_STACK_ypabact *NC_STACK_ypaworld::yw_createUnit( int model_id)
{
    std::array<const std::string, 11> unit_classes_names
    {
        "dummy.class",      // 0
        "ypabact.class",    // 1
        "ypatank.class",    // 2
        "yparobo.class",    // 3
        "ypamissile.class", // 4
        "ypazepp.class",    // 5
        "ypaflyer.class",   // 6
        "ypaufo.class",     // 7
        "ypacar.class",     // 8
        "ypagun.class",     // 9
        "ypahovercraft.class" //10
    };

    NC_STACK_ypabact *bacto = NULL;

    // Find dead units
    for ( NC_STACK_ypabact * &unit : _deadCacheList )
    {
        if (unit->_bact_type == model_id)
        {
            bacto = unit;
            break;
        }
    }

    if ( !bacto )
    {
        bacto = Nucleus::CTFInit<NC_STACK_ypabact>(unit_classes_names[model_id], 
            {{NC_STACK_ypabact::BACT_ATT_WORLD, this}} );

        if ( !bacto )
            return NULL;
    }

    bacto->Renew(); // Reset bact

    bacto->_gid = bact_id;
    bacto->_owner = 0;

    bacto->_rotation = mat3x3::Ident();

    bact_id++;

    return bacto;
}




void NC_STACK_ypaworld::RenderAdditionalBeeBox(Common::Point sect, TRenderingSector *sct, baseRender_msg *bs77)
{
    sct->dword8 = 0;
    sct->dword4 = 0;

    if ( IsSector( sect ) )
    {
        sct->dword4 = 1;
        sct->p_cell = &_cells( sect );
        sct->smooth_height = sct->p_cell->averg_height;

        vec3d pos = World::SectorIDToCenterPos3( sect );
        pos.y = sct->p_cell->height;

        sct->x = pos.x;
        sct->y = pos.y;
        sct->z = pos.z;

        additionalBeeBox->SetPosition(pos);

        if ( additionalBeeBox->Render(bs77, NULL) )
        {
            sct->dword8 = 1;
        }
    }
}

void NC_STACK_ypaworld::RenderSector(TRenderingSector *sct, baseRender_msg *bs77)
{
    if ( sct->dword8 )
    {
        cellArea *pcell = sct->p_cell;

        int v22 = 0;

        vec3d scel;
        if ( pcell->w_type == 1 )
        {
            yw_f80 *v5 = &field_80[ pcell->w_id ];

            scel = vec3d::OY((float)v5->field_4 / (float)v5->field_8);

            pcell->type_id = BuildProtos[ v5->blg_ID ].SecType;
            pcell->comp_type = secTypes[ pcell->type_id ].field_0;

            v22 = 1;
        }

        int v17, v20;

        if ( pcell->comp_type == 1 )
        {
            v17 = 0;
            v20 = 1;
        }
        else
        {
            v17 = -1;
            v20 = 3;
        }

        for (int zz = 0; zz < v20; zz++)
        {
            for (int xx = 0; xx < v20; xx++)
            {
                vec3d pos (  sct->x + (v17 + xx) * 300.0
                           , sct->y
                           , sct->z + (v17 + zz) * 300.0);

                if ( v22 )
                {
                    NC_STACK_base *bld = legos[ secTypes[ pcell->type_id ].buildings.At(xx, zz)->health_models[0] ].base;

                    bld->SetStatic(false);

                    bld->SetScale(scel, NC_STACK_base::UF_Y); //Scale only Y
                    bld->SetPosition(pos);
                    
                    NC_STACK_base::CheckOpts( &pcell->BldVPOpts.At(xx, zz), bld );
                    
                    bld->Render(bs77, pcell->BldVPOpts.At(xx, zz));

                    bld->SetStatic(true);
                }
                else
                {
                    NC_STACK_base *bld = legos[ GetLegoBld(pcell, xx, zz) ].base;

                    bld->SetPosition(pos);
                    
                    NC_STACK_base::CheckOpts( &pcell->BldVPOpts.At(xx, zz), bld );
                    
                    bld->Render(bs77, pcell->BldVPOpts.At(xx, zz));
                }
            }
        }
    }

    for ( NC_STACK_ypabact* &bact : sct->p_cell->unitsList )
    {
        if ( sct->dword8 || bact->_bact_type == BACT_TYPES_ROBO)
            bact->Render(bs77);
    }
}

void NC_STACK_ypaworld::yw_renderSky(baseRender_msg *rndr_params)
{
    if ( sky_loaded_base )
    {
        float v6 = rndr_params->maxZ;
        uint32_t flags = rndr_params->flags;

        sky_loaded_base->SetPosition( current_bact->_position + vec3d::OY(field_15f4) );

        rndr_params->maxZ = 32000.0;
        rndr_params->flags = GFX::RFLAGS_SKY | GFX::RFLAGS_COMPUTED_COLOR;
        if ( System::IniConf::GfxNewSky.Get<bool>() )
            rndr_params->flags |= GFX::RFLAGS_DISABLE_ZWRITE;

        sky_loaded_base->Render(rndr_params, NULL);

        rndr_params->maxZ = v6;
        rndr_params->flags = flags;
    }
}


bool NC_STACK_ypaworld::IsVisibleMapPos(vec2d pos)
{
    int v7 = ((pos.x + 150) / 300) / 4;
    int v8 = ((-pos.y + 150) / 300) / 4;

    if ( !IsGamePlaySector( {v7, v8} ) || !current_bact )
        return false;

    int v11 = abs(current_bact->_sectX - v7);
    int v12 = abs(current_bact->_sectY - v8);

    if ( v11 + v12 <= (field_1368 - 1) / 2 )
        return true;

    return false;
}

void NC_STACK_ypaworld::RenderSuperWave(vec2d pos, vec2d fromPos, baseRender_msg *arg)
{
    if ( !superbomb_wall_vproto )
        return;
    
    if ( pos.x > 0.0 && pos.y < 0.0 && pos.x < _mapLength.x && -_mapLength.y < pos.y )
    {
        if ( IsVisibleMapPos(pos) )
        {
            int v10 = VhclProtos[superbomb_wall_vproto].vp_normal;

            NC_STACK_base *wall_base = vhcls_models.at(v10);

            if ( wall_base )
            {
                float v28 = 0.0;

                int v23 = (pos.x + 150) / 300;
                int v26 = (-pos.y + 150) / 300;

                if ( (v23 & 3) && (v26 & 3) )
                {
                    v28 = _cells((v23 / 4), (v26 / 4)).height;
                }
                else
                {
                    ypaworld_arg136 v22;
                    v22.vect = vec3d::OY(50000.0);
                    v22.stPos.x = pos.x;
                    v22.stPos.y = -25000.0;
                    v22.stPos.z = pos.y;
                    v22.flags = 0;

                    ypaworld_func136(&v22);

                    if ( v22.isect )
                    {
                        v28 = v22.isectPos.y;
                    }
                }


                wall_base->TForm().Pos = vec3d(pos.x, v28, pos.y);

                vec2d delt = pos - fromPos;
                delt.normalise();

                wall_base->TForm().SclRot =  mat3x3(delt.y,   0, -delt.x,
                                                      0, 1.0,    0,
                                                    delt.x, 0.0,  delt.y);

                wall_base->Render(arg, NULL);
            }
        }
    }
}

void NC_STACK_ypaworld::RenderSuperItems(baseRender_msg *arg)
{
    // Render super items
    for ( const TMapSuperItem &sitem : _levelInfo.SuperItems )
    {
        if ( sitem.State == TMapSuperItem::STATE_TRIGGED )
        {
            vec2d pos = World::SectorIDToCenterPos2( sitem.Sector );

            float v14 = sqrt( POW2(_mapLength.x) + POW2(_mapLength.y) );

            if ( sitem.CurrentRadius > 300 && sitem.CurrentRadius < v14 )
            {
                float v17 = (2 * sitem.CurrentRadius) * C_PI / 300.0;

                if ( v17 > 2.0 )
                {
                    float v9 = 6.283 / v17;

                    for (float j = 0.0; j < 6.283; j = j + v9 )
                    {
                        vec2d wallpos = vec2d(cos(j), sin(j)) * sitem.CurrentRadius + pos;
                        RenderSuperWave(wallpos, pos, arg);
                    }
                }
            }
        }
    }
}

void NC_STACK_ypaworld::PrepareFiller(cellArea *sct, cellArea *sct2, float v9h, float v8h, bool vertical, TCellFillerCh *out, bool force)
{
    int x = secTypes[ sct->type_id ].field_1;
    int y = secTypes[ sct2->type_id ].field_1;
    
    if (!force && (out->Id1 == x && out->Id2 == y && 
        out->Heights[0] == sct->height && out->Heights[1] == sct2->height && 
        out->Heights[2] == v8h && out->Heights[3] == v9h))
        return;

    NC_STACK_base *bs;
    if (vertical)
        bs = FillersVertical(x, y);
    else
        bs = FillersHorizontal(x, y);
    
    UAskeleton::Data *skel = bs->GetSkeleton()->GetSkelet();
    
    vec2d pos = World::SectorIDToCenterPos2( sct2->PosID );

    bs->SetPosition( vec3d::X0Z( pos ), NC_STACK_base::UF_XZ );

    for (int i = 0; i < 4; i++)
        skel->POO[i].y = sct->height;

    for (int i = 4; i < 8; i++)
        skel->POO[i].y = sct2->height;

    skel->POO[8].y = v8h;
    skel->POO[9].y = v9h;
    
    bs->RecalcInternal(true);
    bs->MakeCoordsCache();
    
    out->FreeVBO();
    
    bs->MakeCache(out);
    
    out->MakeVBO();
    
    out->Id1 = x;
    out->Id2 = y;
    out->Heights[0] = sct->height;
    out->Heights[1] = sct2->height;
    out->Heights[2] = v8h;
    out->Heights[3] = v9h;
}

void NC_STACK_ypaworld::PrepareAllFillers()
{
    for (int i = 0; i < _mapSize.x - 1; i++)
    {
        for (int j = 0; j < _mapSize.y - 2; j++)
        {
            cellArea *sct = &_cells(i, j);
            cellArea *sct2 = &_cells(i, j + 1);
            
            float h;
            if (i == _mapSize.x - 1)
                h = sct2->averg_height;
            else
                h = _cells(i + 1, j + 1).averg_height;
            
            PrepareFiller(sct, sct2, sct2->averg_height, h, false, &_cellsHFCache(i, j), true);
        }
    }
    
    for (int i = 0; i < _mapSize.x - 2; i++)
    {
        for (int j = 0; j < _mapSize.y - 1; j++)
        {
            cellArea *sct = &_cells(i, j);
            cellArea *sct2 = &_cells(i + 1, j);
            
            float h;
            if (i == _mapSize.x - 1)
                h = sct2->averg_height;
            else
                h = _cells(i + 1, j + 1).averg_height;
            
            PrepareFiller(sct, sct2, sct2->averg_height, h, true, &_cellsVFCache(i, j), true);
        }
    }
}




TRenderingSector rendering_sectors[YW_RENDER_SECTORS_DEF * 2][ YW_RENDER_SECTORS_DEF * 2];

void NC_STACK_ypaworld::RenderFillers(baseRender_msg *arg)
{
    //Render landscape linking parts
    for (int i = 0; i < field_1368; i++)
    {
        for (int j = 0; j < field_1368 - 1; j++)
        {
            TRenderingSector &sct = rendering_sectors[j][i];
            TRenderingSector &sct2 = rendering_sectors[j + 1][i];
            
            if (sct.dword4 == 1 && sct2.dword4 == 1 && (sct.dword8 == 1 || sct2.dword8 == 1))
            {
                float h;
                if (rendering_sectors[j + 1][i + 1].dword4 == 1)
                    h = rendering_sectors[j + 1][i + 1].p_cell->averg_height;
                else
                    h = sct2.p_cell->averg_height;
                    
                TCellFillerCh &filler = _cellsVFCache( sct2.p_cell->PosID.x - 1, sct2.p_cell->PosID.y );
                PrepareFiller(sct.p_cell, sct2.p_cell, sct2.p_cell->averg_height, h, true, &filler);
                filler.Render(arg);
            }
        }
    }

    for (int i = 0; i < field_1368 - 1; i++)
    {
        for (int j = 0; j < field_1368; j++)
        {
            TRenderingSector &sct = rendering_sectors[j][i];
            TRenderingSector &sct2 = rendering_sectors[j][i + 1];
            
            if (sct.dword4 == 1 && sct2.dword4 == 1 && (sct.dword8 == 1 || sct2.dword8 == 1))
            {
                float h;
                if (rendering_sectors[j + 1][i + 1].dword4 == 1)
                    h = rendering_sectors[j + 1][i + 1].p_cell->averg_height;
                else
                    h = sct2.p_cell->averg_height;
                    
                TCellFillerCh &filler = _cellsHFCache( sct2.p_cell->PosID.x, sct2.p_cell->PosID.y - 1 );
                PrepareFiller(sct.p_cell, sct2.p_cell, sct2.p_cell->averg_height, h, false, &filler);
                filler.Render(arg);
            }
        }
    }
}

void NC_STACK_ypaworld::RenderGame(base_64arg *bs64, int a2)
{
    if ( !current_bact )
        return;

    TF::TForm3D *v5 = TF::Engine.GetViewPoint();

    if ( v5 )
        v5->CalcGlobal();

    baseRender_msg rndrs;

    rndrs.flags = 0;
    rndrs.frameTime = bs64->DTime;
    rndrs.globTime = bs64->TimeStamp;
    rndrs.adeCount = 0;

    rndrs.minZ = 1.0;

    if ( field_1368 == 5 )
        rndrs.maxZ = 1500.0;
    else
        rndrs.maxZ = 3500.0;

    int v6 = field_1368 - 1;

    for (int j = 0; j < v6; j++)
    {
        for (int i = 0; i < v6; i++)
        {
            rendering_sectors[j][i].dword4 = 0;
            rendering_sectors[j][i].dword8 = 0;
        }
    }

    int v29 = v6 / 2;
    for (int i = 0; i <= v29; i++)
    {
        int v28 = v29 - i;

        for (int j = -i; j <= i; j++)
        {
            TRenderingSector *sct = &rendering_sectors[v29 + j][v29 - v28];

            RenderAdditionalBeeBox( Common::Point(j + current_bact->_sectX, -v28 + current_bact->_sectY), 
                                    sct, &rndrs);

            if ( sct->dword4 )
                RenderSector(sct, &rndrs);

        }

        if ( -v28 != v28 )
        {
            for (int j = -i; j <= i; j++)
            {
                TRenderingSector *sct = &rendering_sectors[v29 + j][v29 + v28];

                RenderAdditionalBeeBox( Common::Point(j + current_bact->_sectX, v28 + current_bact->_sectY), 
                                        sct, &rndrs);

                if ( sct->dword4 )
                    RenderSector(sct, &rndrs);
            }
        }
    }

    
    RenderSuperItems(&rndrs);

    RenderFillers(&rndrs);

    bs64->field_C = rndrs.adeCount;

    field_1B6A = rndrs.adeCount;
    field_1b6c = 7777;


    area_arg_65 rrg;
    rrg.timeStamp = bs64->TimeStamp;
    rrg.frameTime = bs64->DTime;
    rrg.minZ = 1.0;
    rrg.maxZ = rndrs.maxZ;
    rrg.ViewTForm = TF::Engine.GetViewPoint();
    rrg.OwnerTForm = NULL;
    rrg.flags = 0;


    ParticleSystem().UpdateRender(&rrg, bs64->DTime);

    GFX::Engine.BeginScene();
    
    if ( field_15f8 )
        yw_renderSky(&rndrs);

    GFX::Engine.Rasterize();

    GFX::Engine.EndScene();

    if ( a2 )
    {
        uint32_t tpm = profiler_begin();
        sb_0x4d7c08__sub0(this);
        _profile2DDraw = profiler_end(tpm);
    }
}


void NC_STACK_ypaworld::sb_0x456384__sub0__sub0()
{
    for( int y = 0; y < _mapSize.y; y++ )
    {
        for( int x = 0; x < _mapSize.x; x++ )
        {
            EnergyAccum &accum = _energyAccumMap(x, y);
            accum.Energy = 0;
            accum.Owner = _cells(x, y).owner;
        }
    }

    _nextPSForUpdate = 0;
}

void NC_STACK_ypaworld::SetupPowerStationRef(const Common::Point &sec, int power)
{
    PowerStationRef *ps = NULL;

    int32_t wIndex = 0;
    for (PowerStationRef &p : _powerStations)
    {
        if ( !p.pCell )
        {
            ps = &p;
            break;
        }
        wIndex++;
    }

    if ( !ps )
    {
        wIndex = _powerStations.size();
        _powerStations.emplace_back();
        ps = &_powerStations.back();
    }

    ps->Cell = sec;
    ps->Power = power;
    ps->EffectivePower = power;
    ps->pCell = GetSector(sec);

    ps->pCell->w_type = 2;
    ps->pCell->w_id = wIndex;

    sb_0x456384__sub0__sub0();
}


void NC_STACK_ypaworld::sb_0x456384(int x, int y, int ownerid2, int blg_id, int a7)
{
    uamessage_bldVhcl bvMsg;
    memset(&bvMsg, 0, sizeof(bvMsg));

    cellArea &cell = _cells(x, y);
    World::TBuildingProto *bld = &BuildProtos[ blg_id ];
    secType *sectp = &secTypes[ bld->SecType ];

    int v43 = 1;

    NC_STACK_yparobo *robo = NULL;

    if ( IsGamePlaySector( {x, y} ) )
    {
        (*blg_map)(x, y) = blg_id;
        (*typ_map)(x, y) = bld->SecType;

        cell.type_id = bld->SecType;
        cell.energy_power = 0;
        cell.w_type = 3;
        cell.comp_type = sectp->field_0;
        cell.w_id = blg_id;

        int v49;

        if ( sectp->field_0 == 1 )
        {
            cell.buildings_health.fill(0);
            
            v49 = 1;
        }
        else
        {
            v49 = 3;
        }

        for (int yy = 0; yy < v49; yy++)
        {
            for (int xx = 0; xx < v49; xx++)
                cell.buildings_health.At(xx, yy) = sectp->buildings.At(xx, yy)->build_health;
        }

        if ( bld->ModelID == 1 )
            SetupPowerStationRef(Common::Point(x, y), bld->Power);

        CellSetOwner(&cell, x, y, ownerid2);

        for( NC_STACK_ypabact * &unit: _unitsList )
        {
            if (unit->_bact_type == BACT_TYPES_ROBO && unit->_owner == ownerid2)
            {
                robo = dynamic_cast<NC_STACK_yparobo *>(unit);
                break;
            }
        }

        if ( isNetGame )
        {
            if ( robo != UserRobo )
                v43 = 0;
        }

        if ( !a7 )
        {
            if ( robo && robo->_status != BACT_STATUS_DEAD && v43 )
            {
                NC_STACK_ypagun *commander = NULL;

                int v39 = robo->getROBO_commCount();

                v39++;

                robo->setROBO_commCount(v39);

                for ( size_t i = 0; i < bld->Guns.size(); i++)
                {
                    World::TBuildingProto::TGun &GunProto = bld->Guns[i];
                    
                    if ( !GunProto.VhclID )
                        break;

                    ypaworld_arg146 v33;
                    v33.vehicle_id = GunProto.VhclID;
                    v33.pos = GunProto.Pos + World::SectorIDToCenterPos3( {x, y} );

                    NC_STACK_ypabact *gun_obj = ypaworld_func146(&v33);
                    NC_STACK_ypagun *gunn = dynamic_cast<NC_STACK_ypagun *>(gun_obj);

                    if ( gun_obj )
                    {
                        gun_obj->_owner = ownerid2;

                        if (gunn)
                            gunn->ypagun_func128(GunProto.Dir, false);

                        setState_msg v34;
                        v34.newStatus = BACT_STATUS_CREATE;
                        v34.unsetFlags = 0;
                        v34.setFlags = 0;

                        gunn->SetStateInternal(&v34);

                        gun_obj->_scale_time = 500;
                        gun_obj->_scale = vec3d(1.0, 1.0, 1.0);

                        gun_obj->_host_station = robo;
                        gun_obj->_commandID = v39;

                        if ( isNetGame && i < 8 )
                        {
                            gun_obj->_gid |= ownerid2 << 24;

                            bvMsg.vhcl[i].id = gun_obj->_gid;
                            bvMsg.vhcl[i].base = GunProto.Dir;
                            bvMsg.vhcl[i].pos = gun_obj->_position;
                            bvMsg.vhcl[i].protoID = GunProto.VhclID;
                        }

                        if ( commander )
                        {
                            commander->AddSubject(gunn);
                        }
                        else
                        {
                            commander = gunn;

                            robo->AddSubject(gunn);
                        }
                    }
                }

                if ( isNetGame )
                {
                    bvMsg.msgID = UAMSG_BUILDINGVHCL;
                    bvMsg.tstamp = timeStamp;
                    bvMsg.owner = ownerid2;

                    yw_arg181 v31;
                    v31.recvFlags = 2;
                    v31.recvID = 0;
                    v31.senderID = GameShell->callSIGN.c_str();
                    v31.senderFlags = 1;
                    v31.data = &bvMsg;
                    v31.dataSize = sizeof(bvMsg);
                    v31.garant = 1;

                    ypaworld_func181(&v31);
                }
            }
        }
    }
}

void ypaworld_func148__sub0(NC_STACK_ypaworld *yw, int x, int y)
{
    for ( NC_STACK_ypabact* &node : yw->_cells(x, y).unitsList )
    {
        int v5 = 0;

        if ( yw->isNetGame )
        {
            if ( node->_owner == yw->UserUnit->_owner )
            {
                if ( node->_status != BACT_STATUS_DEAD && node->_status != BACT_STATUS_BEAM && node->_status != BACT_STATUS_CREATE )
                {
                    if ( node->_bact_type == BACT_TYPES_GUN )
                    {
                        NC_STACK_ypagun *gun = dynamic_cast<NC_STACK_ypagun *>( node );

                        if (!gun->IsRoboGun())
                            v5 = 1;
                    }
                }
            }
        }
        else
        {
            if ( node->_status != BACT_STATUS_DEAD && node->_status != BACT_STATUS_BEAM && node->_status != BACT_STATUS_CREATE )
            {
                if ( node->_bact_type == BACT_TYPES_GUN )
                {
                    NC_STACK_ypagun *gun = dynamic_cast<NC_STACK_ypagun *>( node );

                    if (!gun->IsRoboGun())
                        v5 = 1;
                }
            }
        }

        if ( v5 )
        {
            bact_arg84 v8;
            v8.energy = -22000000;
            v8.unit = NULL;

            node->ModifyEnergy(&v8);
        }
    }
}

int NC_STACK_ypaworld::ypaworld_func148__sub1(int id, int a4, int x, int y, int ownerID2, char blg_ID)
{
    if ( id < 8 && !field_80[id].field_0 && IsGamePlaySector( {x, y} ) )
    {
        field_80[id].field_4 = 0;
        field_80[id].field_0 = 1;
        field_80[id].field_8 = a4;
        field_80[id].x = x;
        field_80[id].y = y;
        field_80[id].ownerID2 = ownerID2;
        field_80[id].blg_ID = blg_ID;

        cellArea &cell = _cells(x, y);
        cell.w_type = 1;
        cell.w_id = id;

        for (NC_STACK_ypabact * &unit : _unitsList)
        {
            if ( unit->_bact_type == BACT_TYPES_ROBO && ownerID2 == unit->_owner )
            {
                SFXEngine::SFXe.startSound(&unit->_soundcarrier, 11);
                break;
            }
        }

        return 1;
    }

    return 0;
}

int ypaworld_func137__sub0__sub0(UAskeleton::Data *skl, int id, const vec3d &v, float r, vec3d &out)
{
    UAskeleton::Polygon &pg = skl->polygons[id];
    vec3d tmp(0.0, 0.0, 0.0);

    for (int i = 0; i < pg.num_vertices; i++)
    {
        int16_t idd = pg.v[i];
        tmp += static_cast<vec3d> (skl->POO[ idd ]);
    }

    vec3d tmp2 = tmp / pg.num_vertices - v;

    float v26 = tmp2.length();

    if ( v26 <= r )
        return 0;

    out = tmp2 * (r / v26) + v;

    return 1;
}

void NC_STACK_ypaworld::ypaworld_func137__sub0(ypaworld_arg137 *arg, const TSectorCollision &a2)
{
    for (size_t i = 0; i < a2.sklt->polygons.size(); i++)
    {
        const UAskeleton::Polygon &tria = a2.sklt->polygons[i];

        vec3d t0 = tria.Normal();

        float v9 = t0.dot( arg->pos2 );

        if ( v9 > 0.0 )
        {
            float v26 = -( t0.dot( arg->pos ) + tria.D) / ( t0.dot( t0 ) * arg->radius);

            if ( v26 > 0.0 && v26 <= 1.0 )
            {
                vec3d tx = arg->pos + t0 * (arg->radius * v26);

                int v27 = 0;

                vec3d v18;

                if ( ypaworld_func137__sub0__sub0(a2.sklt, i, tx, arg->radius, v18) )
                {
                    if ( sub_44D36C(v18, i, a2.sklt) )
                        v27 = 1;
                }
                else
                    v27 = 1;

                if ( v27 )
                {
                    if ( arg->coll_count < arg->coll_max )
                    {
                        int pos = arg->coll_count;

                        arg->collisions[pos].pos1 = a2.pos + tx;
                        arg->collisions[pos].pos2 = tria.Normal();

                        arg->coll_count++;
                    }
                }
            }
        }
    }
}

NC_STACK_ypabact * NC_STACK_ypaworld::FindBactByCmdOwn(int commandID, char owner)
{
    for ( NC_STACK_ypabact * &robo : _unitsList )
    {
        if ( robo->_bact_type == BACT_TYPES_ROBO && robo->_owner == owner)
        {
            if ( robo->_commandID == commandID )
            {
                if ( robo->_status == BACT_STATUS_DEAD )
                    return NULL;
                else
                    return robo;
            }
            else
            {
                for ( NC_STACK_ypabact * &unit : robo->_kidList )
                {
                    if ( unit->_commandID == commandID )
                    {
                        if ( unit->_status == BACT_STATUS_DEAD )
                            return NULL;
                        else
                            return unit;
                    }
                }
            }
        }
    }

    return NULL;
}


void ypaworld_func64__sub20(NC_STACK_ypaworld *yw, int dtime)
{
    for(int i = 0; i < 8; i++)
    {
        if ( yw->field_80[i].field_0 )
        {
            yw->field_80[i].field_4 += dtime;

            if ( yw->field_80[i].field_4 >= yw->field_80[i].field_8 )
            {
                yw->field_80[i].field_0 = 0;

                int v8 = yw->field_80[i].y;
                int v10 = yw->field_80[i].x;

                cellArea & v11 = yw->_cells(v10, v8);

                int a6 = yw->field_80[i].blg_ID;

                v11.w_type = 0;
                v11.w_id = 0;

                yw->sb_0x456384(v10, v8, yw->field_80[i].ownerID2, a6, 0);

                if ( yw->field_80[i].ownerID2 == yw->UserRobo->_owner )
                {
                    if ( yw->BuildProtos[a6].ModelID )
                    {
                        yw_arg159 arg159;

                        arg159.unit = yw->UserRobo;
                        arg159.Priority = 65;

                        if ( yw->BuildProtos[a6].ModelID == 1 )
                            arg159.MsgID = 36;
                        else if ( yw->BuildProtos[a6].ModelID == 2 )
                            arg159.MsgID = 38;
                        else if ( yw->BuildProtos[a6].ModelID == 3 )
                            arg159.MsgID = 37;
                        else
                            arg159.MsgID = 0;

                        yw->ypaworld_func159(&arg159);
                    }
                }
            }
        }
    }
}

void NC_STACK_ypaworld::ypaworld_func64__sub6__sub0()
{
    for(int i = 0; i < 8; i++)
        field_1bac[i] = 0;

    for ( NC_STACK_ypabact * &robo : _unitsList )
    {
        if ( robo->_bact_type == BACT_TYPES_ROBO && robo->_status != BACT_STATUS_DEAD && robo->_status != BACT_STATUS_BEAM )
        {
            for( NC_STACK_ypabact * &comnd : robo->_kidList )
            {
                if ( comnd->_status != BACT_STATUS_DEAD && comnd->_status != BACT_STATUS_BEAM )
                {
                    bool a4 = false;

                    if ( comnd->_bact_type == BACT_TYPES_GUN )
                    {
                        NC_STACK_ypagun *gun = dynamic_cast<NC_STACK_ypagun *>( comnd );
                        a4 = gun->IsRoboGun();
                    }

                    if ( !a4 )
                    {
                        field_1bac[ comnd->_owner ]++;

                        for( NC_STACK_ypabact * &unit : comnd->_kidList )
                        {
                            if ( unit->_status != BACT_STATUS_DEAD && unit->_status != BACT_STATUS_BEAM )
                                field_1bac[ unit->_owner ]++;
                        }
                    }
                }
            }
        }
    }
}


void ypaworld_func64__sub6(NC_STACK_ypaworld *yw)
{
    int v13[8];

    for (int i = 0; i < 8; i++)
        v13[i] = 0;

    for (const PowerStationRef &ps : yw->_powerStations)
    {
        if (ps.pCell)
            v13[ ps.pCell->owner ] += ps.EffectivePower;
    }

    yw->ypaworld_func64__sub6__sub0();

    for (int i = 0; i < 8; i++)
    {
        v13[i] /= 2;

        if ( v13[i] <= 0 )
        {
            yw->field_1bec[i] = 0;
            yw->field_1bcc[i] = 0;
        }
        else
        {
            int v15 = yw->sectors_count_by_owner[i];

            if ( v15 < 0 )
                v15 = 0;

            yw->field_1bcc[i] = (float)v15 / (float)v13[i];
            yw->field_1bec[i] = (float)v15 / (float)v13[i];

            if ( yw->isNetGame )
            {
                if ( yw->unit_limit_type_1 == 1 )
                {
                    int v16 = yw->field_1bac[yw->UserRobo->_owner] - yw->unit_limit_1;

                    if ( v16 > 0 )
                    {
                        int v10 = (float)yw->unit_limit_arg_1 * 0.01 * (float)v16;

                        yw->field_1bcc[i] -= v10;
                        yw->field_1bec[i] -= v10;
                    }
                }
            }

            if ( yw->field_1bcc[i] >= 0.0 )
            {
                if ( yw->field_1bcc[i] > 1.0 )
                    yw->field_1bcc[i] = 1.0;
            }
            else
            {
                yw->field_1bcc[i] = 0;
            }

            if ( yw->field_1bec[i] < 0.0 )
                yw->field_1bec[i] = 0;
        }
    }
}


void ypaworld_func64__sub5__sub0(NC_STACK_ypaworld *yw, int a2)
{
    PowerStationRef &ps = yw->_powerStations.at(a2);

    int pwrTmp = ps.EffectivePower;
    int powsCount = 0;

    while (pwrTmp > 0)
    {
        pwrTmp >>= 1;
        powsCount++;
    }

    int sdx = -powsCount;
    int edx = powsCount + 1;

    int sdy = -powsCount;
    int edy = powsCount + 1;

    if ( ps.Cell.x + sdx < 1 )
        sdx = 1 - ps.Cell.x;

    if ( ps.Cell.y + sdy < 1 )
        sdy = 1 - ps.Cell.y;

    if ( ps.Cell.x + edx >= yw->_mapSize.x )
        edx = yw->_mapSize.x - ps.Cell.x - 1;

    if ( ps.Cell.y + edy >= yw->_mapSize.y )
        edy = yw->_mapSize.y - ps.Cell.y - 1;

    for (int dy = sdy; dy < edy; dy++)
    {
        for (int dx = sdx; dx < edx; dx++)
        {
            int v17 = ps.EffectivePower  >>  yw->sqrt_table[abs(dx)][abs(dy)];

            EnergyAccum &accum = yw->_energyAccumMap(dx + ps.Cell.x, dy + ps.Cell.y);

            if ( accum.Owner == ps.pCell->owner )
            {
                accum.Energy += v17; // Add power to this cell

                if ( accum.Energy > 255 )
                    accum.Energy = 255;
            }

        }
    }
}

void ypaworld_func64__sub5(NC_STACK_ypaworld *yw)
{
    // Recompute power on sectors
    if ( !yw->_powerStations.empty() ) // If we have powerstations
    {
        size_t pID = yw->_nextPSForUpdate; // current power station

        //If current power station is null - go to first not null or to end
        while ( pID < yw->_powerStations.size() )
        {
            if ( yw->_powerStations.at(pID).pCell )
                break;
            pID++;
        }

        if ( pID < yw->_powerStations.size() ) // if we found any power station
        {
            if ( yw->_powerStations.at(pID).EffectivePower ) // if this power station has power
                ypaworld_func64__sub5__sub0(yw, pID); // Add power to power matrix

            yw->_nextPSForUpdate = pID + 1; // go to next station in next update loop
        }
        else // If we reach end of power stations list, apply power to sectors
        {
            yw->UpdatePowerEnergy(); // Apply power to sectors and clean power matrix for next compute iteration.
        }
    }
}


void NC_STACK_ypaworld::sub_4D12D8(int id, int a3)
{
    TMapSuperItem &sitem = _levelInfo.SuperItems[id];

    sitem.State = TMapSuperItem::STATE_ACTIVE;
    sitem.TriggerTime = 0;
    sitem.ActivateOwner = sitem.PCell->owner;

    if ( !a3 )
    {
        sitem.ActiveTime = timeStamp;
        sitem.LastTenSec = 0;
        sitem.LastSec = 0;
        sitem.CountDown = sitem.TimerValue;
    }

    ypaworld_arg148 arg148;
    arg148.ownerID = sitem.PCell->owner;
    arg148.ownerID2 = sitem.PCell->owner;
    arg148.blg_ID = sitem.ActiveBldID;
    arg148.field_C = 1;
    arg148.x = sitem.Sector.x;
    arg148.y = sitem.Sector.y;
    arg148.field_18 = 0;

    ypaworld_func148(&arg148);

    sitem.PCell->w_type = 8;
    sitem.PCell->w_id = id;

    yw_arg159 arg159;
    arg159.unit = 0;
    arg159.Priority = 94;

    if ( sitem.Type == TMapSuperItem::TYPE_BOMB )
    {
        arg159.txt = GetLocaleString(250, "Superbomb activated.");
        arg159.MsgID = 70;
    }
    else if ( sitem.Type == TMapSuperItem::TYPE_WAVE )
    {
        arg159.txt = GetLocaleString(254, "Superwave activated.");
        arg159.MsgID = 74;
    }
    else
    {
        arg159.MsgID = 0;
        arg159.txt = "Cant happen.";
    }

    ypaworld_func159(&arg159);
}

void NC_STACK_ypaworld::sub_4D1594(int id)
{
    TMapSuperItem &sitem = _levelInfo.SuperItems[id];

    sitem.State = TMapSuperItem::STATE_STOPPED;

    ypaworld_arg148 arg148;
    arg148.ownerID = sitem.PCell->owner;
    arg148.ownerID2 = sitem.PCell->owner;
    arg148.blg_ID = sitem.InactiveBldID;
    arg148.field_C = 1;
    arg148.x = sitem.Sector.x;
    arg148.y = sitem.Sector.y;
    arg148.field_18 = 0;

    ypaworld_func148(&arg148);

    sitem.PCell->w_type = 8;
    sitem.PCell->w_id = id;

    yw_arg159 arg159;
    arg159.unit = 0;
    arg159.Priority = 93;

    if ( sitem.Type == TMapSuperItem::TYPE_BOMB )
    {
        arg159.txt = GetLocaleString(252, "Superbomb frozen.");
        arg159.MsgID = 72;
    }
    else if ( sitem.Type == TMapSuperItem::TYPE_WAVE )
    {
        arg159.txt = GetLocaleString(256, "Superwave frozen.");
        arg159.MsgID = 76;
    }
    else
    {
        arg159.MsgID = 0;
        arg159.txt = "Cant happen.";
    }

    ypaworld_func159(&arg159);
}

void NC_STACK_ypaworld::sub_4D1444(int id)
{
    TMapSuperItem &sitem = _levelInfo.SuperItems[id];
    sitem.State = TMapSuperItem::STATE_TRIGGED;
    sitem.TriggerTime = timeStamp;

    ypaworld_arg148 arg148;
    arg148.ownerID = sitem.PCell->owner;
    arg148.ownerID2 = sitem.PCell->owner;
    arg148.blg_ID = sitem.TriggerBldID;
    arg148.field_C = 1;
    arg148.x = sitem.Sector.x;
    arg148.y = sitem.Sector.y;
    arg148.field_18 = 0;

    ypaworld_func148(&arg148);

    sitem.PCell->w_type = 8;
    sitem.PCell->w_id = id;

    sitem.LastRadius = 0;

    yw_arg159 arg159;
    arg159.Priority = 95;
    arg159.unit = 0;

    if ( sitem.Type == TMapSuperItem::TYPE_BOMB )
    {
        arg159.txt = GetLocaleString(251, "Superbomb triggered.");
        arg159.MsgID = 71;
    }
    else if ( sitem.Type == TMapSuperItem::TYPE_WAVE )
    {
        arg159.txt = GetLocaleString(255, "Superwave triggered.");
        arg159.MsgID = 75;
    }
    else
    {
        arg159.MsgID = 0;
        arg159.txt = "Cant happen.";
    }

    ypaworld_func159(&arg159);
}


NC_STACK_ypabact * NC_STACK_ypaworld::sb_0x47b028__sub0(uint32_t bactid)
{
    for ( NC_STACK_ypabact * &station : _unitsList )
    {
        if ( bactid == station->_gid )
            return station;

        for ( NC_STACK_ypabact * &commander : station->_kidList )
        {
            if ( bactid == commander->_gid )
                return station; // CHECK IT

            for ( NC_STACK_ypabact * &slave : commander->_kidList )
            {
                if ( bactid == slave->_gid )
                    return station;  // CHECK IT
            }
        }
    }

    return NULL;
}

void NC_STACK_ypaworld::RefreshUnitPRT(NC_STACK_ypabact *unit, NC_STACK_ypabact *robo, bool isRobo)
{
    bact_arg80 arg80;
    arg80.pos = unit->_position;

    if ( unit->_bact_type == BACT_TYPES_GUN )
        arg80.field_C = 4;
    else
        arg80.field_C = 0;

    unit->SetPosition(&arg80);

    if ( unit->_bact_type == BACT_TYPES_GUN )
    {
        NC_STACK_ypagun *guno = (NC_STACK_ypagun *)unit;

        guno->ypagun_func128(guno->_gunBasis, false);
    }

    setState_msg arg78;
    arg78.newStatus = unit->_status;
    arg78.setFlags = 0;
    arg78.unsetFlags = 0;
    unit->SetState(&arg78);

    if ( unit->_status_flg & BACT_STFLAG_DEATH2 )
    {
        arg78.newStatus = BACT_STATUS_NOPE;
        arg78.unsetFlags = 0;
        arg78.setFlags = BACT_STFLAG_DEATH2;
        unit->SetState(&arg78);
    }

    if ( unit->_status_flg & BACT_STFLAG_FIRE )
    {
        arg78.newStatus = BACT_STATUS_NOPE;
        arg78.unsetFlags = 0;
        arg78.setFlags = BACT_STFLAG_FIRE;
        unit->SetState(&arg78);
    }

    if ( !isRobo )
    {
        unit->_host_station = dynamic_cast<NC_STACK_yparobo *>(robo);
        unit->_owner = robo->_owner;
    }

    if ( unit->_primTtype == BACT_TGT_TYPE_UNIT )
    {
        unit->_primTtype = BACT_TGT_TYPE_NONE;

        setTarget_msg arg67;
        arg67.tgt.pbact = sb_0x47b028__sub0((int)(size_t)unit->_primT.pbact);
        arg67.tgt_type = BACT_TGT_TYPE_UNIT_IND;
        arg67.priority = 0;
        unit->SetTarget(&arg67);
    }

    if ( unit->_primTtype == BACT_TGT_TYPE_CELL )
    {
        unit->_primTtype = BACT_TGT_TYPE_NONE;

        setTarget_msg arg67_1;
        arg67_1.tgt_type = BACT_TGT_TYPE_CELL_IND;
        arg67_1.tgt_pos = unit->_primTpos;
        arg67_1.priority = 0;

        unit->SetTarget(&arg67_1);
    }
}


int ypaworld_func64__sub4(NC_STACK_ypaworld *yw, base_64arg *arg)
{
    if ( yw->isNetGame )
        return 0;

    if ( !yw->field_160c )
    {
        if ( arg->field_8->HotKeyID == 32 || arg->field_8->KbdLastHit == Input::KC_PAUSE )
        {
            yw->field_160c = 1;
            yw->field_1610 = arg->TimeStamp;
        }
        return 0;
    }

    if ( arg->field_8->KbdLastHit != Input::KC_NONE )
    {
        yw->field_160c = 0;
        arg->TimeStamp = yw->field_1610;
    }
    else
    {
        GFX::Engine.BeginFrame();

        /*yw->_win3d->setRSTR_BGpen(0);

        yw->_win3d->raster_func192(NULL);*/

        vec3d a2a = yw->field_1334 + vec3d::OY(50000.0);

        SFXEngine::SFXe.sub_423EFC(1, a2a, vec3d(0.0), mat3x3::Ident());

        if ( arg->TimeStamp / 500 & 1 )
        {
            const std::string &v6 = yw->GetLocaleString(14, "*** GAME PAUSED, HIT KEY TO CONTINUE ***");

            char v10[256];
            char *pcur = v10;

            FontUA::select_tileset(&pcur, 15);

            FontUA::set_xpos(&pcur, 0);
            FontUA::set_center_ypos(&pcur, -yw->font_default_h / 2);

            pcur = FontUA::FormateCenteredSkipableItem(yw->tiles[15], pcur, v6, yw->screen_width);

            FontUA::set_end(&pcur);

            w3d_a209 arg209;
            arg209.includ = 0;
            arg209.cmdbuf = v10;

            GFX::Engine.raster_func209(&arg209);
        }

        SFXEngine::SFXe.sb_0x424c74();

        GFX::Engine.EndFrame();
    }
    return 1;
}


void ypaworld_func64__sub2(NC_STACK_ypaworld *yw)
{
    yw->field_1b70 = 0;

    if ( yw->UserRobo != yw->UserUnit )
    {
        NC_STACK_yparobo *robo = dynamic_cast<NC_STACK_yparobo *>(yw->UserRobo);

        for (World::TRoboGun &gun : robo->GetGuns())
        {
            if ( yw->UserUnit == gun.gun_obj )
                yw->field_1b70 = 1;
        }
    }
}


void ypaworld_func64__sub9(NC_STACK_ypaworld *yw)
{
    for ( size_t i = 0; i < yw->_levelInfo.Gates.size(); i++ )
    {
        const MapGate &gate = yw->_levelInfo.Gates[i];
        int v21 = 6;

        if ( gate.PCell->owner == yw->UserRobo->_owner )
        {
            for( const MapKeySector &ks : gate.KeySectors )
            {
                if (ks.PCell)
                {
                    if (ks.PCell->owner != yw->UserRobo->_owner)
                    {
                        v21 = 5;
                        break;
                    }
                }
            }
        }
        else
        {
            v21 = 5;
        }

        if ( gate.PCell->w_type != v21 )
        {
            ypaworld_arg148 arg148;
            arg148.ownerID = gate.PCell->owner;
            arg148.ownerID2 = gate.PCell->owner;
            arg148.field_C = 1;
            arg148.x = gate.SecX;
            arg148.y = gate.SecY;
            arg148.field_18 = 0;

            if ( v21 == 6 )
            {
                arg148.blg_ID = gate.OpenBldID;
            }
            else
            {
                arg148.blg_ID = gate.ClosedBldID;

                yw_arg159 arg159;
                arg159.unit = 0;
                arg159.Priority = 65;
                arg159.txt = yw->GetLocaleString(224, "TRANSPORTER GATE CLOSED!");
                arg159.MsgID = 24;

                yw->ypaworld_func159(&arg159);
            }

            yw->ypaworld_func148(&arg148);

            gate.PCell->w_type = v21;
            gate.PCell->w_id = i;
        }

        if ( v21 == 6 )
        {
            int energ = 0;

            for ( NC_STACK_ypabact* &v8 : gate.PCell->unitsList )
            {
                if ( v8->_status != BACT_STATUS_DEAD && v8->_status != BACT_STATUS_BEAM )
                {
                    if ( v8->_bact_type != BACT_TYPES_ROBO && v8->_bact_type != BACT_TYPES_MISSLE && v8->_bact_type != BACT_TYPES_GUN )
                        energ += (v8->_energy_max + 99) / 100;
                }
            }

            if ( energ <= yw->beamenergy )
            {
                if ( yw->timeStamp - yw->field_1a00 > 60000 )
                {
                    yw_arg159 arg159_1;
                    arg159_1.unit = 0;
                    arg159_1.Priority = 49;
                    arg159_1.txt = yw->GetLocaleString(223, "TRANSPORTER GATE OPENED!");
                    arg159_1.MsgID = 23;

                    yw->ypaworld_func159(&arg159_1);
                    yw->field_1a00 = yw->timeStamp;
                }
            }
            else
            {
                if ( yw->timeStamp - yw->field_1a00 > 40000 )
                {
                    yw_arg159 arg159_2;
                    arg159_2.unit = 0;
                    arg159_2.Priority = 10;
                    arg159_2.txt = yw->GetLocaleString(258, "WARNING: BEAM GATE FULL!");
                    arg159_2.MsgID = 46;

                    yw->ypaworld_func159(&arg159_2);
                    yw->field_1a00 = yw->timeStamp;
                }
            }
        }
    }
}


bool NC_STACK_ypaworld::sub_4D11C0(int id, int owner)
{
    const TMapSuperItem &sitem = _levelInfo.SuperItems[id];

    if ( sitem.PCell->owner != owner )
        return false;

    if ( sitem.KeySectors.empty() )
        return true;

    for ( const MapKeySector &ks : sitem.KeySectors )
    {
        if ( ks.PCell->owner != owner )
            return false;
    }
    return true;
}

bool NC_STACK_ypaworld::sub_4D12A0(int owner)
{
    for ( NC_STACK_ypabact * &unit : _unitsList )
    {
        if ( unit->_bact_type == BACT_TYPES_ROBO && owner == unit->_owner )
            return true;
    }

    return false;
}

void NC_STACK_ypaworld::sub_4D16C4(int id)
{
    TMapSuperItem &sitem = _levelInfo.SuperItems[id];

    sitem.State = TMapSuperItem::STATE_INACTIVE;
    sitem.ActiveTime = 0;
    sitem.TriggerTime = 0;
    sitem.ActivateOwner = 0;
    sitem.CountDown = 0;

    ypaworld_arg148 arg148;
    arg148.ownerID = sitem.PCell->owner;
    arg148.ownerID2 = sitem.PCell->owner;
    arg148.blg_ID = sitem.InactiveBldID;
    arg148.field_C = 1;
    arg148.x = sitem.Sector.x;
    arg148.y = sitem.Sector.y;
    arg148.field_18 = 0;

    ypaworld_func148(&arg148);

    sitem.PCell->w_type = 8;
    sitem.PCell->w_id = id;

    yw_arg159 arg159;
    arg159.unit = NULL;
    arg159.Priority = 92;

    if ( sitem.Type == TMapSuperItem::TYPE_BOMB )
    {
        arg159.txt = GetLocaleString(253, "Superbomb deactivated.");
        arg159.MsgID = 73;
    }
    else if ( sitem.Type == TMapSuperItem::TYPE_WAVE )
    {
        arg159.txt = GetLocaleString(257, "Superwave deactivated.");
        arg159.MsgID = 77;
    }
    else
    {
        arg159.MsgID = 0;
        arg159.txt = "Cant happen.";
    }

    ypaworld_func159(&arg159);
}

void NC_STACK_ypaworld::ypaworld_func64__sub19__sub0(int id)
{
    TMapSuperItem &sitem = _levelInfo.SuperItems[id];

    for( NC_STACK_ypabact * &unit : _unitsList )
    {
        if ( unit->_bact_type == BACT_TYPES_ROBO && sitem.ActivateOwner == unit->_owner )
        {
            if ( sub_4D11C0(id, sitem.ActivateOwner) )
            {
                if ( sitem.CountDown > 0 )
                    sitem.CountDown = sitem.CountDown - field_1618;
                else
                    sub_4D1444(id);
            }
            else if ( !sitem.KeySectors.empty() )
            {
                sub_4D1594(id);
            }
            else
            {
                sub_4D12D8(id, 0);
            }
            return;
        }
    }

    sub_4D16C4(id);
}


void ypaworld_func64__sub19__sub3(NC_STACK_ypaworld *yw, int id)
{
    if ( yw->GameShell )
    {
        TMapSuperItem &sitem = yw->_levelInfo.SuperItems[id];

        int v4 = sitem.CountDown / 1024;

        if ( v4 < 10 && v4 != sitem.LastSec )
        {
            SFXEngine::SFXe.startSound(&yw->GameShell->samples1_info, 3);
            sitem.LastSec = v4;
        }

        int v5 = v4 / 10;
        if ( v5 != sitem.LastTenSec )
        {
            SFXEngine::SFXe.startSound(&yw->GameShell->samples1_info, 3);
            sitem.LastTenSec = v5;
        }
    }
}

bool NC_STACK_ypaworld::sub_4D1230(int id, int a3)
{
    const TMapSuperItem &sitem = _levelInfo.SuperItems[id];

    if ( sitem.PCell->owner == a3 )
        return false;

    if ( sitem.KeySectors.empty() )
        return true;

    for ( const MapKeySector &ks : sitem.KeySectors )
    {
        if (ks.PCell->owner == a3)
            return false;
    }

    return true;
}

void NC_STACK_ypaworld::ypaworld_func64__sub19__sub1(int id)
{
    const TMapSuperItem &sitem = _levelInfo.SuperItems[id];

    for ( NC_STACK_ypabact * &unit : _unitsList )
    {
        if ( unit->_bact_type == BACT_TYPES_ROBO && sitem.ActivateOwner == unit->_owner )
        {
            if ( sub_4D11C0(id, sitem.ActivateOwner) )
                sub_4D12D8(id, 1);
            else if ( sub_4D1230(id, sitem.ActivateOwner) )
                sub_4D16C4(id);
            return;
        }
    }

    sub_4D16C4(id);
}

void NC_STACK_ypaworld::ypaworld_func64__sub19__sub2__sub0__sub0(uint8_t activate, float a5, float a6, float a7)
{
    for(cellArea &cell : _cells)
    {
        for( NC_STACK_ypabact* &bct : cell.unitsList )
        {
            int v9 = 1;

            if ( isNetGame )
            {
                if ( bct->_owner != UserUnit->_owner || bct->_owner == activate || bct->_status == BACT_STATUS_DEAD )
                    v9 = 0;
            }
            else if ( bct->_owner == activate || bct->_status == BACT_STATUS_DEAD )
            {
                v9 = 0;
            }

            if ( v9 )
            {
                float v10 = a5 - bct->_position.x;
                float v11 = a6 - bct->_position.z;

                if ( sqrt(POW2(v10) + POW2(v11)) < a7 )
                {
                    bact_arg84 arg84;
                    arg84.energy = -22000000;
                    arg84.unit = NULL;

                    bct->ModifyEnergy(&arg84);
                }
            }
        }

    }
}

void NC_STACK_ypaworld::ypaworld_func64__sub19__sub2__sub0(int id)
{
    TMapSuperItem &sitem = _levelInfo.SuperItems[id];

    sitem.CurrentRadius = (timeStamp - sitem.TriggerTime) * World::CVSectorLength / 2400.0;

    vec2d tmp = World::SectorIDToCenterPos2( sitem.Sector );

    float v19 = sqrt(POW2(_mapLength.x) + POW2(_mapLength.y));

    if ( sitem.CurrentRadius > 300 && sitem.CurrentRadius - sitem.LastRadius > 200 && sitem.CurrentRadius < v19 )
    {
        float v9 = (2 * sitem.CurrentRadius) * C_PI / 150.0;

        sitem.LastRadius = sitem.CurrentRadius;

        if ( v9 > 2.0 )
        {
            for (float v25 = 0.0; v25 < 6.283 ; v25 += 6.283 / v9 )
            {
                float v10 = sitem.CurrentRadius;
                float v26 = cos(v25) * v10 + tmp.x;
                float v21 = sin(v25) * v10 + tmp.y;

                if ( v26 > 600.0 && v21 < -600.0 && v26 < _mapLength.x - 600.0 && v21 > -(_mapLength.y - 600.0) )
                {
                    int v12 = fxnumber;

                    fxnumber = 2;

                    yw_arg129 arg129;
                    arg129.pos.x = v26;
                    arg129.pos.y = sitem.PCell->height;
                    arg129.pos.z = v21;
                    arg129.field_10 = 200000;
                    arg129.OwnerID = sitem.ActivateOwner;
                    arg129.unit = 0;

                    ypaworld_func129(&arg129);

                    fxnumber = v12;
                }
            }
        }
    }

    ypaworld_func64__sub19__sub2__sub0__sub0(sitem.ActivateOwner, tmp.x, tmp.y, sitem.CurrentRadius);
}

void NC_STACK_ypaworld::ypaworld_func64__sub19__sub2(int id)
{
    const TMapSuperItem &sitem = _levelInfo.SuperItems[id];

    if ( !sub_4D1230(id, sitem.ActivateOwner) && sub_4D12A0(sitem.ActivateOwner) )
    {
        if ( sitem.Type == TMapSuperItem::TYPE_BOMB )
            ypaworld_func64__sub19__sub2__sub0(id);
    }
    else
    {
        sub_4D16C4(id);
    }
}

void NC_STACK_ypaworld::ypaworld_func64__sub19()
{
    for (size_t i = 0; i < _levelInfo.SuperItems.size(); i++)
    {
        const TMapSuperItem &sitem = _levelInfo.SuperItems[i];

        if ( sitem.Type != 0 )
        {
            switch ( sitem.State )
            {
            case TMapSuperItem::STATE_INACTIVE:
                if ( sub_4D11C0(i, sitem.PCell->owner) )
                {
                    if ( sub_4D12A0(sitem.PCell->owner) )
                        sub_4D12D8(i, 0);
                }
                break;

            case TMapSuperItem::STATE_ACTIVE:
                ypaworld_func64__sub19__sub0(i);
                ypaworld_func64__sub19__sub3(this, i);
                break;

            case TMapSuperItem::STATE_STOPPED:
                ypaworld_func64__sub19__sub1(i);
                break;

            case TMapSuperItem::STATE_TRIGGED:
                ypaworld_func64__sub19__sub2(i);
                break;

            default:
                break;
            }
        }
    }
}

void NC_STACK_ypaworld::VoiceMessageCalcPositionToUnit()
{
    if ( _voiceMessage.Unit == UserRobo )
    {
        _voiceMessage.Carrier.Position = UserUnit->_position;
    }
    else
    {
        vec3d tmp = _voiceMessage.Unit->_position - UserUnit->_position;

        float v11 = tmp.length();

        if ( v11 > 0.0 )
            tmp *= (100.0 / v11);

        _voiceMessage.Carrier.Position = UserUnit->_position + tmp;
    }
}

void NC_STACK_ypaworld::VoiceMessageUpdate()
{
    if ( _voiceMessage.Priority >= 0 )
    {
        if ( _voiceMessage.Unit->_status != BACT_STATUS_DEAD )
        {
            VoiceMessageCalcPositionToUnit();

            _voiceMessage.Carrier.Vector = UserUnit->_fly_dir * UserUnit->_fly_dir_length;
        }

        if ( _voiceMessage.Carrier.Sounds[0].IsEnabled() )
        {
            SFXEngine::SFXe.UpdateSoundCarrier(&_voiceMessage.Carrier);
        }
        else
            _voiceMessage.Reset();
    }
}

void ypaworld_func64__sub3(NC_STACK_ypaworld *yw)
{
    if ( yw->UserUnit->_pSector->owner != yw->UserRobo->_owner )
    {
        if ( yw->UserUnit->_pSector->owner )
        {
            if ( yw->field_1b74 == yw->UserRobo->_owner || !yw->field_1b74 )
            {
                if ( yw->timeStamp - yw->field_1a08 > 10000 )
                {
                    yw_arg159 arg159;
                    arg159.unit = yw->UserUnit;
                    arg159.Priority = 24;
                    arg159.txt = yw->GetLocaleString(222, "ENEMY SECTOR ENTERED");
                    arg159.MsgID = 22;

                    yw->ypaworld_func159(&arg159);
                }

                yw->field_1a08 = yw->timeStamp;
            }
        }
    }
}

void sub_44A094(NC_STACK_ypaworld *yw)
{
    yw->p_1_grp_cnt++;

    if ( yw->p_1_grp_cnt >= 5 )
    {
        if ( yw->p_1_grp[0][0] > 200 )
            yw->p_1_grp[0][0] = 0;

        for (int i = 0; i < 8; i++)
        {
            if ( yw->p_1_grp[0][i] )
            {
                if ( yw->p_1_grp[0][i] < yw->p_1_grp[2][i] )
                    yw->p_1_grp[2][i] = yw->p_1_grp[0][i];

                if ( yw->p_1_grp[0][i] > yw->p_1_grp[1][i] )
                    yw->p_1_grp[1][i] = yw->p_1_grp[0][i];

                yw->p_1_grp[3][i] += yw->p_1_grp[0][i];
            }
        }
    }
}

int NC_STACK_ypaworld::yw_RestoreVehicleData()
{
    std::string buf = fmt::sprintf("save:%s/%d.rst", GameShell->UserName, _levelInfo.LevelID);

    ScriptParser::HandlersList parsers {
        new World::Parsers::VhclProtoParser(this),
        new World::Parsers::WeaponProtoParser(this),
        new World::Parsers::BuildProtoParser(this)
    };

    return ScriptParser::ParseFile(buf, parsers, 0);
}

void NC_STACK_ypaworld::EnableLevelPasses()
{
    if ( _levelInfo.State == TLevelInfo::STATE_COMPLETED )
    {
        MapGate &gate = _levelInfo.Gates[ _levelInfo.GateCompleteID ];

        _mapRegions.MapRegions[ _levelInfo.LevelID ].Status = TMapRegionInfo::STATUS_COMPLETED;

        for (int lvl : gate.PassToLevels)
        {
            if ( _mapRegions.MapRegions[ lvl ].Status == TMapRegionInfo::STATUS_DISABLED )
                _mapRegions.MapRegions[ lvl ].Status = TMapRegionInfo::STATUS_ENABLED;
        }
    }
    else if ( _levelInfo.State == TLevelInfo::STATE_ABORTED && !yw_RestoreVehicleData() )
    {
        ypa_log_out("yw_RestoreVehicleData() failed.\n");
    }
}

void NC_STACK_ypaworld::NetReleaseMissiles(NC_STACK_ypabact *bact)
{
    while(!bact->_missiles_list.empty())
    {
        NC_STACK_ypamissile *misl = bact->_missiles_list.front();
        bact->_missiles_list.pop_front();
        
        if ( misl->_primTtype == BACT_TGT_TYPE_UNIT )
        {
            misl->_primT.pbact->_attackersList.remove( misl );
            misl->_primTtype = BACT_TGT_TYPE_NONE;
        }

        misl->CleanAttackersTarget();

        misl->_parent = NULL;

        ypaworld_func144(misl);

        misl->_status_flg |= BACT_STFLAG_DEATH1;
    }
}

void NC_STACK_ypaworld::sub_4F1BE8(NC_STACK_ypabact *bct)
{
    if ( bct->_bact_type == BACT_TYPES_GUN )
    {
        NC_STACK_ypagun *gun = dynamic_cast<NC_STACK_ypagun *>( bct );

        if ( gun->IsRoboGun() )
        {
            for (World::TRoboGun &pgun : bct->_host_station->GetGuns())
            {
                if ( bct == pgun.gun_obj )
                    pgun.gun_obj = NULL;
            }
        }
    }
}

void NC_STACK_ypaworld::NetRemove(NC_STACK_ypabact *bct)
{
    while(!bct->_kidList.empty())
    {
        NC_STACK_ypabact *cmnder = bct->_kidList.front();

        while ( !cmnder->_kidList.empty() )
        {
            NC_STACK_ypabact *slave = cmnder->_kidList.front();

            NetReleaseMissiles(slave);
            slave->CleanAttackersTarget();
            sub_4F1BE8(slave);

            slave->_status_flg |= BACT_STFLAG_DEATH1;
            slave->_status = BACT_STATUS_DEAD;

            ypaworld_func144(slave);
        }

        NetReleaseMissiles(cmnder);
        cmnder->CleanAttackersTarget();
        sub_4F1BE8(cmnder);

        cmnder->_status_flg |= BACT_STFLAG_DEATH1;
        cmnder->_status = BACT_STATUS_DEAD;

        ypaworld_func144(cmnder);
    }

    if ( bct->_bact_type == BACT_TYPES_ROBO )
    {
        NC_STACK_yparobo *robo = dynamic_cast<NC_STACK_yparobo *>(bct);

        for (World::TRoboGun &gun : robo->GetGuns())
            gun.gun_obj = NULL;
    }

    bct->CleanAttackersTarget();
    NetReleaseMissiles(bct);

    bct->_status = BACT_STATUS_DEAD;

    ypaworld_func144(bct);

    bct->_status_flg |= BACT_STFLAG_DEATH1;
}

void NC_STACK_ypaworld::ProtosFreeSounds()
{
    SFXEngine::SFXe.StopPlayingSounds();

    for (World::TVhclProto &vhcl : VhclProtos)
    {
        for (World::TVhclSound &sfx : vhcl.sndFX)
            sfx.ClearSounds();
    }

    for (World::TWeapProto &wep : WeaponProtos)
    {
        for (World::TVhclSound &fx : wep.sndFXes)
            fx.ClearSounds();
    }

    for (World::TBuildingProto &proto : BuildProtos)
        proto.SndFX.ClearSounds();
}


void NC_STACK_ypaworld::FFeedback_Update()
{
    if ( field_739A )
    {
        if ( field_73CE & (4 | 8) )
            return;
    }

    if ( UserUnit )
    {
        if ( timeStamp - field_7562 > 250 )
        {
            field_7562 = timeStamp;

            if ( UserUnit->_status == BACT_STATUS_DEAD )
            {
                if ( input_class )
                    input_class->ForceFeedback(NC_STACK_winp::FF_STATE_UPDATE, field_7566);
            }
            else
            {
                if ( field_7566 != -1 )
                {
                    float a1 = POW2(UserUnit->_force) - POW2(UserUnit->_mass) * 100.0;

                    if (a1 < 0.0)
                        a1 = 0.0;

                    float v17 = fabs(UserUnit->_fly_dir_length) / ( sqrt(a1) / UserUnit->_airconst_static );
                    if ( v17 >= 1.0 )
                        v17 = 1.0;
                    else if (v17 < 0.0)
                        v17 = 0.0;

                    if ( input_class )
                        input_class->ForceFeedback(NC_STACK_winp::FF_STATE_UPDATE, field_7566,
                                                        field_756E, field_756A * v17);
                }
            }
        }

        TSoundSource *top = SFXEngine::SFXe.SndGetTopShake();
        if ( top )
        {
            field_7572 = top;
            if ( top->StartTime == SFXEngine::SFXe.currentTime )
            {
                float p1 = top->ShkMag;
                if ( p1 > 1.0 )
                    p1 = 1.0;

                vec3d tmp = top->PCarrier->Position - UserUnit->_position;

                float p2 = top->PShkFx->time;
                float p3 = UserUnit->_rotation.AxisX().dot( tmp );
                float p4 = -UserUnit->_rotation.AxisZ().dot( tmp );

                if ( p2 > 0.0 )
                {
                    if ( input_class )
                        input_class->ForceFeedback(NC_STACK_winp::FF_STATE_START, NC_STACK_winp::FF_TYPE_SHAKE,
                                                        p1, p2, p3, p4);
                }
            }
        }
        else
        {
            field_7572 = NULL;
        }
    }
}

int recorder_startrec(NC_STACK_ypaworld *yw)
{
    recorder *rcrd = yw->sceneRecorder;

    rcrd->do_record = 0;
    rcrd->field_40 = 0;
    rcrd->seqn++;
    rcrd->level_id = yw->_levelInfo.LevelID;
    rcrd->frame_id = 0;
    rcrd->time = 0;
    rcrd->bacts_count = 0;
    rcrd->field_34 = 0;
    rcrd->ainf_size = 0;

    rcrd->mfileYOYO = IFFile::UAOpenIFFile(fmt::sprintf("env:snaps/m%02d%04d.raw", yw->_levelInfo.LevelID, rcrd->seqn), "wb");

    if ( !rcrd->mfileYOYO.OK() )
    {
        return 0;
    }

    rcrd->mfileYOYO.pushChunk(TAG_SEQN, TAG_FORM, -1);
    rcrd->mfileYOYO.pushChunk(0, TAG_SINF, 4);

    rcrd->mfileYOYO.writeU16L(rcrd->seqn);
    rcrd->mfileYOYO.writeU16L(rcrd->level_id);

    rcrd->mfileYOYO.popChunk();

    rcrd->do_record = 1;
    return 1;
}

void recorder_stoprec(NC_STACK_ypaworld *yw)
{
    recorder *rcrd = yw->sceneRecorder;
    rcrd->do_record = 0;

    rcrd->mfileYOYO.popChunk();

    rcrd->mfileYOYO.close();
}

void sb_0x447720(NC_STACK_ypaworld *yw, InputState *inpt)
{
    if ( inpt->KbdLastHit == Input::KC_NUMMUL && (inpt->ClickInf.flag & ClickBoxInf::FLAG_RM_HOLD || yw->easy_cheat_keys) )
    {
        sub_4476AC(yw);

        yw_arg159 info_msg;
        info_msg.txt = "Screenshot saved.";
        info_msg.unit = NULL;
        info_msg.Priority = 100;
        info_msg.MsgID = 0;

        yw->ypaworld_func159(&info_msg);
    }


    if ( yw->do_screenshooting )
    {
        if ( inpt->KbdLastHit == Input::KC_NUMDIV && (inpt->ClickInf.flag & ClickBoxInf::FLAG_RM_HOLD || yw->easy_cheat_keys) )
        {
            yw->do_screenshooting = 0;

            yw_arg159 info_msg;
            info_msg.txt = "Screenshotting: stopped.";
            info_msg.unit = NULL;
            info_msg.Priority = 100;
            info_msg.MsgID = 0;

            yw->ypaworld_func159(&info_msg);
        }

        GFX::Engine.SaveScreenshot( fmt::sprintf("env:snaps/s%d_%04d", yw->screenshot_seq_id, yw->screenshot_seq_frame_id) );
        
        yw->screenshot_seq_frame_id++;
    }
    else if ( inpt->KbdLastHit == Input::KC_NUMDIV && (inpt->ClickInf.flag & 0x100 || yw->easy_cheat_keys) )
    {
        yw->screenshot_seq_frame_id = 0;
        yw->do_screenshooting = 1;
        yw->screenshot_seq_id++;

        yw_arg159 info_msg;
        info_msg.txt = "Screenshotting: started.";
        info_msg.unit = NULL;
        info_msg.Priority = 100;
        info_msg.MsgID = 0;

        yw->ypaworld_func159(&info_msg);
    }

    if ( yw->sceneRecorder->do_record )
    {
        if ( inpt->KbdLastHit == Input::KC_NUMMINUS && (inpt->ClickInf.flag & ClickBoxInf::FLAG_RM_HOLD || yw->easy_cheat_keys) )
        {
            recorder_stoprec(yw);

            yw_arg159 info_msg;
            info_msg.txt = "Replay recordering: stopped.";
            info_msg.unit = NULL;
            info_msg.Priority = 100;
            info_msg.MsgID = 0;

            yw->ypaworld_func159(&info_msg);
        }

    }
    else
    {
        if ( inpt->KbdLastHit == Input::KC_NUMMINUS && (inpt->ClickInf.flag & ClickBoxInf::FLAG_RM_HOLD || yw->easy_cheat_keys) )
        {
            recorder_startrec(yw);

            yw_arg159 info_msg;
            info_msg.txt = "Replay recordering: started.";
            info_msg.unit = NULL;
            info_msg.Priority = 100;
            info_msg.MsgID = 0;

            yw->ypaworld_func159(&info_msg);
        }
    }
}

void recorder_update_time(NC_STACK_ypaworld *yw, int dtime)
{
    yw->sceneRecorder->time += dtime;
    yw->sceneRecorder->field_40 -= dtime;
}


void NC_STACK_ypaworld::recorder_store_bact( recorder *rcrd, World::MissileList &bct_lst)
{
    for( NC_STACK_ypamissile * &bact : bct_lst )
    {
        if ( bact->_gid >= 0xFFFF || bact == UserRobo )
        {
            if ( rcrd->bacts_count < rcrd->max_bacts )
            {
                rcrd->bacts[ rcrd->bacts_count ] = bact;
                rcrd->bacts_count++;
            }

            recorder_store_bact(rcrd, bact->_missiles_list);
            recorder_store_bact(rcrd, bact->_kidList);
        }
    }
}

void NC_STACK_ypaworld::recorder_store_bact( recorder *rcrd, World::RefBactList &bct_lst)
{
    for( NC_STACK_ypabact * &bact : bct_lst )
    {
        if ( bact->_gid >= 0xFFFF || bact == UserRobo )
        {
            if ( rcrd->bacts_count < rcrd->max_bacts )
            {
                rcrd->bacts[ rcrd->bacts_count ] = bact;
                rcrd->bacts_count++;
            }

            recorder_store_bact(rcrd, bact->_missiles_list);
            recorder_store_bact(rcrd, bact->_kidList);
        }
    }
}

int recorder_sort_bact(const void *a1, const void *a2)
{
    return (*(NC_STACK_ypabact **)a1)->_gid - (*(NC_STACK_ypabact **)a2)->_gid;
}

void NC_STACK_ypaworld::recorder_world_to_frame(recorder *rcrd)
{
    rcrd->bacts_count = 0;
    recorder_store_bact(rcrd, _unitsList);

    qsort(rcrd->bacts, rcrd->bacts_count, sizeof(NC_STACK_ypabact *), recorder_sort_bact);

    for (int i = 0; i < rcrd->bacts_count; i++)
    {
        NC_STACK_ypabact *bact = rcrd->bacts[i];

        trec_bct *oinf = &rcrd->oinf[i];

        oinf->bact_id = bact->_gid;
        oinf->pos = bact->_position;

        vec3d euler = bact->_rotation.GetEuler();

        oinf->rot_x = dround(euler.x * 127.0 / C_2PI);
        oinf->rot_y = dround(euler.y * 127.0 / C_2PI);
        oinf->rot_z = dround(euler.z * 127.0 / C_2PI);

        NC_STACK_base *a4 = bact->GetVP();

        if ( a4 == bact->_vp_normal )
        {
            oinf->vp_id = 1;
        }
        else if ( a4 == bact->_vp_fire )
        {
            oinf->vp_id = 2;
        }
        else if ( a4 == bact->_vp_wait )
        {
            oinf->vp_id = 3;
        }
        else if ( a4 == bact->_vp_dead )
        {
            oinf->vp_id = 4;
        }
        else if ( a4 == bact->_vp_megadeth )
        {
            oinf->vp_id = 5;
        }
        else if ( a4 == bact->_vp_genesis )
        {
            oinf->vp_id = 6;
        }
        else
        {
            oinf->vp_id = 0;
        }

        if (bact->_bact_type == BACT_TYPES_MISSLE)
            oinf->objType = recorder::OBJ_TYPE_MISSILE;
        else
            oinf->objType = recorder::OBJ_TYPE_VEHICLE;

        oinf->vhcl_id = bact->_vehicleID;

        uint16_t *ssnd = &rcrd->sound_status[i * 2];
        ssnd[0] = 0;

        for (int j = 0; j < 16; j++)
        {
            if (bact->_soundcarrier.Sounds[j].IsEnabled() || 
                bact->_soundcarrier.Sounds[j].IsPFxEnabled() || 
                bact->_soundcarrier.Sounds[j].IsShkEnabled())
                ssnd[0] |= 1 << j;
        }

        ssnd[1] = bact->_soundcarrier.Sounds[0].Pitch;
    }
}

void recorder_pack_soundstates(recorder *rcrd)
{
    uint8_t *in = (uint8_t *)rcrd->sound_status;
    int in_pos = 0;

    uint8_t *output = (uint8_t *)rcrd->ainf;
    int out_pos = 0;

    int max_bytes_count = 4 * rcrd->bacts_count;

    while ( in_pos < max_bytes_count )
    {
        if ( in_pos >= max_bytes_count - 1 || in[in_pos] != in[in_pos + 1] )
        {
            int ctrl_byte_pos = out_pos;
            int cnt_bytes = 0;

            while (cnt_bytes < 0x80)
            {
                if ( in_pos >= max_bytes_count )
                    break;
                else if ( in_pos < max_bytes_count - 2 && in[in_pos] == in[in_pos + 1] && in[in_pos] == in[in_pos + 2] )
                    break;

                output[out_pos] = in[in_pos];
                in_pos++;
                out_pos++;

                cnt_bytes++;
            }

            output[ctrl_byte_pos] = cnt_bytes - 1;
        }
        else
        {
            int cnt_bytes = 0;

            uint8_t smplbyte = in[in_pos];
            while ( in_pos < max_bytes_count )
            {
                if ( in[in_pos] != smplbyte )
                    break;

                if ( cnt_bytes >= 0x80 )
                    break;

                in_pos++;
                cnt_bytes++;
            }
            output[out_pos] = 0x101 - cnt_bytes;
            output[out_pos + 1] = smplbyte;
            out_pos += 2;
        }
    }

    rcrd->ainf_size = out_pos;
}

void recorder_unpack_soundstates(recorder *rcrd)
{
    uint8_t *out = (uint8_t *)rcrd->sound_status;
    uint8_t *in = (uint8_t *)rcrd->ainf;
    uint8_t *in_end = ((uint8_t *)rcrd->ainf) + rcrd->ainf_size;

    while ( in < in_end )
    {
        uint8_t bt = *in;
        in++;

        if ( bt > 0x80 )
        {
            for (int i = 0; i < 0x101 - bt; i++)
            {
                *out = *in;
                out++;
            }

            in++;
        }
        else if ( bt < 0x80 )
        {
            bt += 1;

            memcpy(out, in, bt);

            out += bt;
            in += bt;
        }
    }
}

void NC_STACK_ypaworld::recorder_write_frame()
{
    recorder *rcrd = sceneRecorder;

    if ( rcrd->field_40 < 0 )
    {
        recorder_world_to_frame(rcrd);
        rcrd->ctrl_bact_id = UserUnit->_gid;
        recorder_pack_soundstates(rcrd);


        int frame_size = 24;
        int oinf_size = 22 * rcrd->bacts_count;
        int v5 = 16 * rcrd->field_34;

        if ( oinf_size )
        {
            frame_size = oinf_size + 32;

            if ( frame_size & 1 )
                frame_size++;
        }

        if ( rcrd->ainf_size )
        {
            frame_size += rcrd->ainf_size + 8;

            if ( frame_size & 1 )
                frame_size++;
        }

        if ( v5 )
        {
            frame_size += v5 + 8;

            if ( frame_size & 1 )
                frame_size++;
        }
        rcrd->mfileYOYO.pushChunk(TAG_FRAM, TAG_FORM, frame_size);
        rcrd->mfileYOYO.pushChunk(0, TAG_FINF, 12);

        rcrd->mfileYOYO.writeS32L(rcrd->frame_id);
        rcrd->mfileYOYO.writeS32L(rcrd->time);
        rcrd->mfileYOYO.writeU32L(rcrd->ctrl_bact_id);

        rcrd->mfileYOYO.popChunk();

        if ( oinf_size )
        {
            rcrd->mfileYOYO.pushChunk(0, TAG_OINF, oinf_size);

            for (int i = 0; i < rcrd->bacts_count; i++)
            {
                trec_bct *oinf = &rcrd->oinf[i];

                rcrd->mfileYOYO.writeU32L(oinf->bact_id);
                TF::Engine.Vec3dWriteIFF(oinf->pos, &rcrd->mfileYOYO, false);
                rcrd->mfileYOYO.writeS8(oinf->rot_x);
                rcrd->mfileYOYO.writeS8(oinf->rot_y);
                rcrd->mfileYOYO.writeS8(oinf->rot_z);
                rcrd->mfileYOYO.writeU8(oinf->vp_id);
                rcrd->mfileYOYO.writeU8(oinf->objType);
                rcrd->mfileYOYO.writeU8(oinf->vhcl_id);
            }

            rcrd->mfileYOYO.popChunk();
        }

        if ( rcrd->ainf_size )
        {
            rcrd->mfileYOYO.pushChunk(0, TAG_AINF, rcrd->ainf_size);
            rcrd->mfileYOYO.write(rcrd->ainf, rcrd->ainf_size);
            rcrd->mfileYOYO.popChunk();
        }

        if ( v5 )
        {
            rcrd->mfileYOYO.pushChunk(0, TAG_MODE, v5);
            rcrd->mfileYOYO.write(rcrd->field_20, v5);
            rcrd->mfileYOYO.popChunk();
        }

        rcrd->mfileYOYO.popChunk();

        rcrd->field_34 = 0;
        rcrd->field_40 += 250;
        rcrd->frame_id += 1;
    }
}


int recorder_open_replay(recorder *rcrd)
{
    rcrd->mfileYOYO = IFFile( uaOpenFile(rcrd->filename, "rb") );

    if ( !rcrd->mfileYOYO.OK() )
    {
        return 0;
    }

    if ( rcrd->mfileYOYO.parse() != IFFile::IFF_ERR_OK )
    {
        rcrd->mfileYOYO.close();
        return 0;
    }

    if ( rcrd->mfileYOYO.GetCurrentChunk().Is(TAG_FORM, TAG_SEQN) )
        return 1;

    return 0;
}


bool NC_STACK_ypaworld::recorder_create_camera()
{
    NC_STACK_ypabact *bacto = Nucleus::CInit<NC_STACK_ypabact>( {{NC_STACK_ypabact::BACT_ATT_WORLD, this}} );

    if ( !bacto )
        return false;

    bacto->Renew();

    bacto->_gid = 0;
    bacto->_owner = 1;

    bacto->_rotation = mat3x3::Ident();

    bacto->_soundcarrier.Clear();

    ypaworld_func134(bacto);

    bacto->setBACT_viewer(1);
    bacto->setBACT_inputting(1);

    UserRobo = bacto;
    _UserRoboKidsList = &bacto->_kidList;

    TF::Engine.SetViewPoint(&bacto->_tForm);

    return true;
}



void recorder_read_framedata(recorder *rcrd)
{
    while ( rcrd->mfileYOYO.parse() != IFFile::IFF_ERR_EOC )
    {
        const IFFile::Context &v3 = rcrd->mfileYOYO.GetCurrentChunk();

        switch ( v3.TAG )
        {
        case TAG_FLSH:
            rcrd->field_78 |= 1;
            rcrd->mfileYOYO.parse();
            break;

        case TAG_FINF:
            rcrd->frame_id = rcrd->mfileYOYO.readS32L();
            rcrd->time = rcrd->mfileYOYO.readS32L();
            rcrd->ctrl_bact_id = rcrd->mfileYOYO.readU32L();
            rcrd->mfileYOYO.parse();
            break;

        case TAG_OINF:
        {
            rcrd->bacts_count = v3.TAG_SIZE / 22;

            for (int i = 0; i < rcrd->bacts_count; i++)
            {
                trec_bct *oinf = &rcrd->oinf[i];

                oinf->bact_id = rcrd->mfileYOYO.readU32L();
                TF::Engine.Vec3dReadIFF(&oinf->pos, &rcrd->mfileYOYO, false);
                oinf->rot_x = rcrd->mfileYOYO.readS8();
                oinf->rot_y = rcrd->mfileYOYO.readS8();
                oinf->rot_z = rcrd->mfileYOYO.readS8();

                oinf->vp_id = rcrd->mfileYOYO.readU8();
                oinf->objType = rcrd->mfileYOYO.readU8();
                oinf->vhcl_id = rcrd->mfileYOYO.readU8();
            }

            rcrd->mfileYOYO.parse();
        }
        break;

        case TAG_AINF:
            rcrd->mfileYOYO.read(rcrd->ainf, v3.TAG_SIZE);
            rcrd->ainf_size = v3.TAG_SIZE;

            recorder_unpack_soundstates(rcrd);

            rcrd->mfileYOYO.parse();
            break;

        case TAG_MODE:
            rcrd->mfileYOYO.read(rcrd->field_20, v3.TAG_SIZE);
            rcrd->field_34 = v3.TAG_SIZE / 16;

            rcrd->mfileYOYO.parse();
            break;

        default:
            rcrd->mfileYOYO.skipChunk();
            break;
        }
    }
}

NC_STACK_ypabact *NC_STACK_ypaworld::recorder_newObject(trec_bct *oinf)
{
    NC_STACK_ypabact *bacto = NULL;

    if ( oinf->objType == recorder::OBJ_TYPE_VEHICLE )
    {
        if ( oinf->vhcl_id )
        {
            ypaworld_arg146 arg146;
            arg146.vehicle_id = oinf->vhcl_id;
            arg146.pos = vec3d(0.0, 0.0, 0.0);

            World::TVhclProto *prot = &VhclProtos[ oinf->vhcl_id ];

            int v6 = prot->model_id;

            prot->model_id = BACT_TYPES_BACT;

            bacto = ypaworld_func146(&arg146);

            VhclProtos[oinf->vhcl_id].model_id = v6;
        }
        else
        {

            bacto = Nucleus::CInit<NC_STACK_ypabact>( {{NC_STACK_ypabact::BACT_ATT_WORLD, this}} );
            if ( bacto )
            {
                bacto->Renew();

                bacto->_gid = 0;
                bacto->_owner = 1;

                bacto->_rotation = mat3x3::Ident();
            }
        }
    }
    else
    {
        ypaworld_arg146 arg147;
        arg147.vehicle_id = oinf->vhcl_id;
        arg147.pos = vec3d(0.0, 0.0, 0.0);

        bacto = ypaworld_func147(&arg147);
    }

    if ( bacto )
    {
        bacto->_kidRef.Detach();

        bacto->_gid = oinf->bact_id;
        bacto->_host_station = (NC_STACK_yparobo *)UserUnit;
        bacto->_parent = UserUnit;
    }

    return bacto;
}

void NC_STACK_ypaworld::recorder_set_bact_pos(NC_STACK_ypabact *bact, const vec3d &pos)
{
    yw_130arg arg130;
    arg130.pos_x = pos.x;
    arg130.pos_z = pos.z;

    if ( GetSectorInfo(&arg130) )
    {
        if ( bact->_pSector )
            bact->_cellRef.Detach();

        bact->_cellRef = arg130.pcell->unitsList.push_back(bact);

        bact->_pSector = arg130.pcell;
        bact->_old_pos = bact->_position;
        bact->_position = pos;
        bact->_sectX = arg130.sec_x;
        bact->_sectY = arg130.sec_z;
    }
}

void NC_STACK_ypaworld::recorder_updateObject(NC_STACK_ypabact *bact, trec_bct *oinf, uint16_t *ssnd, float a5, float a6)
{
    vec3d bct_pos;
    bct_pos = (oinf->pos - bact->_position) * a5 + bact->_position;

    recorder_set_bact_pos(bact, bct_pos);

    bact->_fly_dir = bact->_position - bact->_old_pos;

    float ln = bact->_fly_dir.length();
    if ( ln > 0.0 )
    {
        bact->_fly_dir /= ln;

        if ( a6 <= 0.0 )
            bact->_fly_dir_length = 0;
        else
            bact->_fly_dir_length = (ln / a6) / 6.0;
    }
    else
    {
        bact->_fly_dir = vec3d::OX(1.0);

        bact->_fly_dir_length = 0;
    }

    mat3x3 tmp = mat3x3::Euler( vec3d(oinf->rot_x, oinf->rot_y, oinf->rot_z) / 127.0 * C_2PI );

    vec3d axisX = (tmp.AxisX() - bact->_rotation.AxisX()) * a5 + bact->_rotation.AxisX();

    if ( axisX.normalise() == 0.0 )
        axisX = vec3d::OX(1.0);

    vec3d axisY = (tmp.AxisY() - bact->_rotation.AxisY()) * a5 + bact->_rotation.AxisY();

    if ( axisY.normalise() == 0.0 )
        axisY = vec3d::OY(1.0);

    vec3d axisZ = (tmp.AxisZ() - bact->_rotation.AxisZ()) * a5 + bact->_rotation.AxisZ();

    if ( axisZ.normalise() == 0.0 )
        axisZ = vec3d::OZ(1.0);

    bact->_rotation = mat3x3::Basis(axisX, axisY, axisZ);

    switch ( oinf->vp_id )
    {
    case 1:
        bact->SetVP(bact->_vp_normal);
        break;

    case 2:
        bact->SetVP(bact->_vp_fire);
        break;

    case 3:
        bact->SetVP(bact->_vp_wait);
        break;

    case 4:
        bact->SetVP(bact->_vp_dead);
        break;

    case 5:
        bact->SetVP(bact->_vp_megadeth);
        break;

    case 6:
        bact->SetVP(bact->_vp_genesis);
        break;

    default:
        break;
    }

    bact->_soundcarrier.Sounds[0].Pitch = ssnd[1];

    for(int i = 0; i < 16; i++)
    {
        int v48 = 1 << i;
        if ( v48 & ssnd[0] )
        {
            if ( !(bact->_soundFlags & v48) )
            {
                bact->_soundFlags |= v48;
                SFXEngine::SFXe.startSound(&bact->_soundcarrier, i);
            }
        }
        else
        {
            if ( bact->_soundFlags & v48 )
            {
                bact->_soundFlags &= ~v48;

                if ( bact->_soundcarrier.Sounds[i].IsLoop() )
                    SFXEngine::SFXe.sub_424000(&bact->_soundcarrier, i);
            }
        }
    }
}


void NC_STACK_ypaworld::recorder_updateObjectList(recorder *rcrd, float a5, int period)
{
    float fperiod = period / 1000.0;
    World::RefBactList::iterator it = UserUnit->_kidList.begin();

    int i = 0;

    while ( i < rcrd->bacts_count )
    {
        trec_bct *oinf = &rcrd->oinf[i];
        uint16_t *ssnd = &rcrd->sound_status[2 * i];

        if ( it != UserUnit->_kidList.end() )
        {
            NC_STACK_ypabact *bact = *it;
            
            if ( oinf->bact_id > bact->_gid )
            {
                it++;

                ypaworld_func144(bact);
            }
            else if ( oinf->bact_id < bact->_gid )
            {
                NC_STACK_ypabact *v10 = recorder_newObject(oinf);

                if ( v10 )
                {
                    recorder_updateObject(v10, oinf, ssnd, 1.0, fperiod);
                    
                    v10->_kidRef = UserUnit->_kidList.insert(it, v10);

                    i++;
                }
            }
            else // ==
            {
                recorder_updateObject(bact, oinf, ssnd, a5, fperiod);
                it++;

                i++;
            }
        }
        else
        {
            NC_STACK_ypabact *v13 = recorder_newObject(oinf);

            if ( v13 )
            {
                recorder_updateObject(v13, oinf, ssnd, 1.0, fperiod);

                v13->_kidRef = UserUnit->_kidList.push_back(v13);
                it = v13->_kidRef;
                it++;

                i++;
            }
        }
    }

    while ( it != UserUnit->_kidList.end() )
    {
        NC_STACK_ypabact *bact = *it;
        it++;

        ypaworld_func144(bact);
    }
}

int NC_STACK_ypaworld::recorder_go_to_frame(recorder *rcrd, int wanted_frame_id)
{
    int frame_id = wanted_frame_id;
    int cur_frame_id = 0;

    if ( frame_id >= 0 )
    {
        if ( frame_id >= rcrd->field_74 )
            frame_id = rcrd->field_74 - 1;
    }
    else
    {
        frame_id = 0;
    }

    rcrd->mfileYOYO.close();

    if ( recorder_open_replay(rcrd) )
    {
        while ( rcrd->mfileYOYO.parse() != IFFile::IFF_ERR_EOC )
        {
            if ( rcrd->mfileYOYO.GetCurrentChunk().Is(TAG_FORM, TAG_FRAM) )
            {
                if ( cur_frame_id == frame_id )
                {
                    recorder_read_framedata(rcrd);

                    timeStamp = rcrd->time;

                    recorder_updateObjectList(rcrd, 1.0, 0);
                    return 1;
                }

                cur_frame_id++;
                rcrd->mfileYOYO.skipChunk();
            }
            else
            {
                rcrd->mfileYOYO.skipChunk();
            }            
        }
    }
    return 0;
}


void NC_STACK_ypaworld::ypaworld_func163__sub1(recorder *rcrd, int dTime)
{
    if ( dTime )
    {
        rcrd->field_78 &= 0xFFFFFFFE;

        while ( rcrd->field_74 - 1 != rcrd->frame_id  &&  (dTime + timeStamp) > rcrd->time )
        {
            if ( rcrd->mfileYOYO.parse() != IFFile::IFF_ERR_EOF )
            {
                if ( rcrd->mfileYOYO.GetCurrentChunk().Is(TAG_FORM, TAG_FRAM) )
                    recorder_read_framedata(rcrd);
            }
        }


        if ( rcrd->field_74 - 1 == rcrd->frame_id )
        {
            recorder_go_to_frame(rcrd, 0);
        }
        else
        {
            if ( rcrd->field_78 & 1 )
            {
                timeStamp = rcrd->time;
                recorder_updateObjectList(rcrd, 1.0, dTime);
            }
            else
            {
                float v9 = (float)dTime / (float)(rcrd->time - timeStamp);

                timeStamp += dTime;

                recorder_updateObjectList(rcrd, v9, dTime);
            }
        }
    }
}

void ypaworld_func163__sub2__sub1(NC_STACK_ypaworld *yw, float fperiod, InputState *inpt)
{
    recorder *rcrd = yw->replayer;

    float v20 = rcrd->rotation_matrix.m20;
    float v18 = rcrd->rotation_matrix.m22;

    float v13 = inpt->Sliders[0] * 250.0 * fperiod;
    float v14 = -inpt->Sliders[2] * 250.0 * fperiod;
    float v15 = -inpt->Sliders[1] * 150.0 * fperiod;

    float v17 = sqrt( POW2(v20) + POW2(v18) );
    if ( v17 > 0.0 )
    {
        v20 /= v17;
        v18 /= v17;
    }

    rcrd->field_44.z += v15 * v18;
    rcrd->field_44.x += v15 * v20;

    float v21 = rcrd->rotation_matrix.m00;
    float v19 = rcrd->rotation_matrix.m02;

    float v16 = sqrt( POW2(v21) + POW2(v19) );
    if ( v16 > 0.0 )
    {
        v21 /= v16;
        v19 /= v16;
    }

    rcrd->field_44.y += v14;
    rcrd->field_44.z += v19 * v13;
    rcrd->field_44.x += v21 * v13;
}

void ypaworld_func163__sub2__sub0(NC_STACK_ypaworld *yw, float fperiod, InputState *inpt)
{
    float v3 = inpt->Sliders[10] * 2.5 * fperiod;

    if ( fabs(v3) > 0.001 )
        yw->replayer->rotation_matrix = mat3x3::RotateY(-v3) * yw->replayer->rotation_matrix;

    float v5 = inpt->Sliders[11] * 2.5 * fperiod;

    if ( fabs(v5) > 0.001 )
    {
        yw->replayer->rotation_matrix = mat3x3::RotateX(v5) * yw->replayer->rotation_matrix;
    }
}

void NC_STACK_ypaworld::CameraPrepareRender(recorder *rcrd, NC_STACK_ypabact *bact, InputState *inpt)
{
    extern tehMap robo_map;
    extern squadMan squadron_manager;

    float fperiod = inpt->Period / 1000.0;

    if ( _mouseGrabbed || !(inpt->ClickInf.flag & ClickBoxInf::FLAG_RM_DOWN) )
    {
        if ( _mouseGrabbed )
        {
            if ( inpt->ClickInf.flag & ClickBoxInf::FLAG_RM_DOWN )
                _mouseGrabbed = 0;
        }
    }
    else
    {
        if ( inpt->ClickInf.selected_btn != &robo_map  &&  inpt->ClickInf.selected_btn != &squadron_manager )
            _mouseGrabbed = 1;
    }

    if ( inpt->Buttons.Is(0) )
        rcrd->rotation_matrix = mat3x3::Ident();

    ypaworld_func163__sub2__sub1(this, fperiod, inpt);

    if ( _mouseGrabbed )
        ypaworld_func163__sub2__sub0(this, fperiod, inpt);

    if ( rcrd->field_80 == 16 )
    {
        recorder_set_bact_pos(bact, rcrd->field_44);
        bact->_rotation = rcrd->rotation_matrix;
    }
    else if ( rcrd->field_80 == 18 )
    {
        for ( NC_STACK_ypabact * &unit : *_UserRoboKidsList )
        {
            if ( rcrd->field_84 == unit->_gid )
            {
                vec3d v35 = unit->_position + unit->_rotation.Transpose().Transform(rcrd->field_44);
                recorder_set_bact_pos(bact, v35);

                bact->_rotation = rcrd->rotation_matrix * unit->_rotation;
                break;
            }
        } 
    }
    else if ( rcrd->field_80 == 20 )
    {
        for ( NC_STACK_ypabact * &unit : *_UserRoboKidsList )
        {
            if ( rcrd->ctrl_bact_id == unit->_gid )
            {
                vec3d a3a = unit->_position + unit->_rotation.Transpose().Transform(rcrd->field_44);
                recorder_set_bact_pos(bact, a3a);

                bact->_rotation = rcrd->rotation_matrix * unit->_rotation;
                break;
            }
        }
    }

    bact->_fly_dir = bact->_old_pos - bact->_position;

    float v39 = bact->_fly_dir.length();
    if ( v39 <= 0.0 )
    {
        bact->_fly_dir = vec3d::OX(1.0);
        bact->_fly_dir_length = 0;
    }
    else
    {
        bact->_fly_dir /= v39;

        if ( fperiod <= 0.0 )
            bact->_fly_dir_length = 0;
        else
            bact->_fly_dir_length = (v39 / fperiod) / 6.0;
    }

    bact->_tForm.Pos = bact->_position;
    bact->_tForm.SclRot = bact->_rotation;
}

char *sub_445654(NC_STACK_ypaworld *yw, char *in, char *buf, const char *fmt, ...)
{
    char *cur = in;

    FontUA::copy_position(&cur);

    va_list va;
    va_start(va, fmt);

    vsprintf(buf, fmt, va);

    va_end(va);

    FontUA::add_txt(&cur, yw->screen_width, 1, buf);

    return cur;
}

void NC_STACK_ypaworld::debug_count_units()
{
    for (int i = 0; i < 8; i++)
    {
        dbg_num_sqd_counter[i] = 0;
        dbg_num_vhcl_counter[i] = 0;
        dbg_num_flk_counter[i] = 0;
        dbg_num_robo_counter[i] = 0;
        dbg_num_wpn_counter[i] = 0;
    }

    dbg_num_sqd = 0;
    dbg_num_vhcl = 0;
    dbg_num_flk = 0;
    dbg_num_wpn = 0;
    dbg_num_robo = 0;

    for ( NC_STACK_ypabact * &robo : _unitsList )
    {
        dbg_num_robo_counter[ robo->_owner ]++;

        if ( robo->_owner )
        {
            for ( NC_STACK_ypabact * &commander : robo->_kidList )
            {
                bool v5 = false;

                if ( commander->_bact_type == BACT_TYPES_GUN )
                {
                    v5 = true;
                    dbg_num_flk_counter[ commander->_owner ]++;
                }
                else
                {
                    dbg_num_sqd_counter[ commander->_owner ]++;
                    dbg_num_vhcl_counter[ commander->_owner ]++;
                }

                dbg_num_wpn_counter[ commander->_owner ] += commander->_missiles_list.size();

                for ( NC_STACK_ypabact * &unit : commander->_kidList )
                {
                    if ( v5 )
                        dbg_num_flk_counter[ unit->_owner ]++;
                    else
                        dbg_num_vhcl_counter[ commander->_owner ]++;


                    dbg_num_wpn_counter[ commander->_owner ] += unit->_missiles_list.size();
                }
            }
        }
    }

    for (int i = 0; i < 8; i++)
    {
        dbg_num_sqd  += dbg_num_sqd_counter[i];
        dbg_num_vhcl += dbg_num_vhcl_counter[i];
        dbg_num_flk  += dbg_num_flk_counter[i];
        dbg_num_wpn  += dbg_num_wpn_counter[i];
        dbg_num_robo += dbg_num_robo_counter[i];
    }

    if ( dbg_num_sqd > dbg_num_sqd_max )
        dbg_num_sqd_max = dbg_num_sqd;

    if ( dbg_num_vhcl > dbg_num_vhcl_max )
        dbg_num_vhcl_max = dbg_num_vhcl;

    if ( dbg_num_flk > dbg_num_flk_max )
        dbg_num_flk_max = dbg_num_flk;

    if ( dbg_num_wpn > dbg_num_wpn_max )
        dbg_num_wpn_max = dbg_num_wpn;

    if ( dbg_num_robo > dbg_num_robo_max )
        dbg_num_robo_max = dbg_num_robo;
}

void NC_STACK_ypaworld::debug_info_draw(InputState *inpt)
{
    if ( _showDebugMode == 0)
    {
        if ( sub_449678(inpt, Input::KC_F9) )
            _showDebugMode++;
    }
    else
    {

        char dbg_txt[4096];
        char buf_sprintf[2048];

        char *cmd = dbg_txt;

        FontUA::select_tileset(&cmd, 15);
        FontUA::set_xpos(&cmd, 8);
        FontUA::set_ypos(&cmd, 16);

        int v104 = 0;

        if ( _showDebugMode == 1 )
        {
            debug_count_units();

            if ( !buildDate.empty() )
            {
                cmd = sub_445654(this, cmd, buf_sprintf, "build id: %s", buildDate.c_str());

                FontUA::next_line(&cmd);
            }

            int this_time = timeStamp / 1024;
            int all_time;

            if ( isNetGame )
                all_time = 0;
            else
                all_time = (timeStamp + playerstatus[1].ElapsedTime) / 1024;

            cmd = sub_445654(
                      this,
                      cmd,
                      buf_sprintf,
                      "time: (this: %02d:%02d:%02d) (all: %02d:%02d:%02d)",
                      this_time / 60 / 60,
                      this_time / 60 % 60,
                      this_time % 60,
                      all_time / 60 / 60,
                      all_time / 60 % 60,
                      all_time % 60 );

            FontUA::next_line(&cmd);

            cmd = sub_445654(this, cmd, buf_sprintf, "prof all: %d", p_1_grp[0][1]);
            FontUA::next_line(&cmd);

            cmd = sub_445654(this, cmd, buf_sprintf, "prof fprint: %d", p_1_grp[0][2]);
            FontUA::next_line(&cmd);

            cmd = sub_445654(this, cmd, buf_sprintf, "prof gui: %d", p_1_grp[0][3]);
            FontUA::next_line(&cmd);

            cmd = sub_445654(this, cmd, buf_sprintf, "prof ai: %d", p_1_grp[0][4]);
            FontUA::next_line(&cmd);

            cmd = sub_445654(this, cmd, buf_sprintf, "prof rend: %d", p_1_grp[0][5]);
            FontUA::next_line(&cmd);
            
            cmd = sub_445654(this, cmd, buf_sprintf, "prof 2d rend: %d", _profile2DDraw);
            FontUA::next_line(&cmd);

            cmd = sub_445654(this, cmd, buf_sprintf, "prof net: %d", p_1_grp[0][6]);
            FontUA::next_line(&cmd);

            cmd = sub_445654(this, cmd, buf_sprintf, "sec type/wtype: %d/%d", UserUnit->_pSector->type_id, UserUnit->_pSector->w_type);
            FontUA::next_line(&cmd);

            cmd = sub_445654(this, cmd, buf_sprintf, "beam energy: %d", beamenergy);
            FontUA::next_line(&cmd);

            cmd = sub_445654(this, cmd, buf_sprintf, "num sqd: %d,%d", dbg_num_sqd, dbg_num_sqd_max);
            FontUA::next_line(&cmd);

            cmd = sub_445654(this, cmd, buf_sprintf, "num vhcl: %d,%d", dbg_num_vhcl, dbg_num_vhcl_max);
            FontUA::next_line(&cmd);

            cmd = sub_445654(this, cmd, buf_sprintf, "num flk: %d,%d", dbg_num_flk, dbg_num_flk_max);
            FontUA::next_line(&cmd);

            cmd = sub_445654(this, cmd, buf_sprintf, "num robo: %d,%d", dbg_num_robo, dbg_num_robo_max);
            FontUA::next_line(&cmd);

            cmd = sub_445654(this, cmd, buf_sprintf, "num wpn: %d,%d", dbg_num_wpn, dbg_num_wpn_max);
            FontUA::next_line(&cmd);

            cmd = sub_445654(this, cmd, buf_sprintf, "reload const: %d", UserRobo->_reload_const);
            FontUA::next_line(&cmd);

            cmd = sub_445654(
                      this,
                      cmd,
                      buf_sprintf,
                      "num all vhcl: %d,%d,%d,%d,%d,%d,%d,%d",
                      field_1bac[0],
                      field_1bac[1],
                      field_1bac[2],
                      field_1bac[3],
                      field_1bac[4],
                      field_1bac[5],
                      field_1bac[6],
                      field_1bac[7]);
            FontUA::next_line(&cmd);

            cmd = sub_445654(
                      this,
                      cmd,
                      buf_sprintf,
                      "rld ratio: %8.2f,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f,%8.2f",
                      field_1bec[0],
                      field_1bec[1],
                      field_1bec[2],
                      field_1bec[3],
                      field_1bec[4],
                      field_1bec[5],
                      field_1bec[6],
                      field_1bec[7]);
            FontUA::next_line(&cmd);

            if ( field_1a20 )
                cmd = sub_445654(this, cmd, buf_sprintf, "invulnerable: %s", "YES");
            else
                cmd = sub_445654(this, cmd, buf_sprintf, "invulnerable: %s", "NO");
            FontUA::next_line(&cmd);
        }
        else if ( _showDebugMode == 2 )
        {
            if ( GameShell && isNetGame )
            {
                FontUA::next_line(&cmd);
                FontUA::next_line(&cmd);

                for (int i = 0; i < 8; i++)
                {
                    if ( GameShell->players[i].status )
                    {
                        const char *v35;
                        const char *v36;

                        switch ( i )
                        {
                        case 1:
                            v35 = "Resistance";
                            break;

                        case 3:
                            v35 = "Mykonier  ";
                            break;

                        case 4:
                            v35 = "Taerkasten";
                            break;

                        case 6:
                            v35 = "Ghorkov   ";
                            break;

                        default:
                            v35 = "Hae?!     ";
                            break;
                        }

                        switch ( GameShell->players[i].status )
                        {
                        case 1:
                            v36 = "OK";
                            break;

                        case 2:
                            v36 = "makes trouble";
                            break;

                        case 3:
                            v36 = "left the game";
                            break;

                        case 4:
                            v36 = "Removed";
                            break;

                        default:
                            v36 = "Hae?!     ";
                            break;
                        }

                        cmd = sub_445654(this, cmd, buf_sprintf, "%s status: %s latency: %d", v35, v36, GameShell->players[i].latency);

                        FontUA::next_line(&cmd);
                    }
                }

                FontUA::next_line(&cmd);

                cmd = sub_445654(this, cmd, buf_sprintf, "net send: %d bytes/sec", GameShell->netsend_speed);
                FontUA::next_line(&cmd);

                cmd = sub_445654(this, cmd, buf_sprintf, "net rcv: %d bytes/sec", GameShell->netrecv_speed);
                FontUA::next_line(&cmd);

                cmd = sub_445654(this, cmd, buf_sprintf, "packet: %d bytes", GameShell->net_packet_size);
                FontUA::next_line(&cmd);

                if ( netInfoOverkill )
                    cmd = sub_445654(this, cmd, buf_sprintf, "WARNING: INFO OVERKILL");

                FontUA::next_line(&cmd);

                if ( windp )
                {
                    int v100[7];
                    windp->GetStats(v100);

                    FontUA::next_line(&cmd);

                    cmd = sub_445654(this, cmd, buf_sprintf, "thread send list now: %d", v100[0]);
                    FontUA::next_line(&cmd);

                    cmd = sub_445654(this, cmd, buf_sprintf, "thread recv list now: %d", v100[1]);
                    FontUA::next_line(&cmd);

                    cmd = sub_445654(this, cmd, buf_sprintf, "thread send list max: %d", v100[3]);
                    FontUA::next_line(&cmd);

                    cmd = sub_445654(this, cmd, buf_sprintf, "thread recv list max: %d", v100[2]);
                    FontUA::next_line(&cmd);

                    cmd = sub_445654(this, cmd, buf_sprintf, "send call now: %d", v100[4]);
                    FontUA::next_line(&cmd);

                    cmd = sub_445654(this, cmd, buf_sprintf, "send call max: %d", v100[5]);
                    FontUA::next_line(&cmd);

                    cmd = sub_445654(this, cmd, buf_sprintf, "send bugs: %d", v100[6]);
                    FontUA::next_line(&cmd);
                }
            }
            else
            {
                cmd = sub_445654(this, cmd, buf_sprintf, "not a network game");
                FontUA::next_line(&cmd);
            }
        }
        else if ( _showDebugMode == 3 )
        {
            for (int i = 0; i < 17; i++)
            {
                cmd = sub_445654(this, cmd, buf_sprintf, "slider[%d] = %f", i, inpt->Sliders[i]);
                FontUA::next_line(&cmd);
            }

            std::string buf;

            for (int i = 0; i < 32; i++)
            {
                if ( inpt->Buttons.Is(i) )
                    buf += 'O';
                else
                    buf += '_';
            }

            cmd = sub_445654(this, cmd, buf_sprintf, buf.c_str());
            FontUA::next_line(&cmd);

            cmd = sub_445654(this, cmd, buf_sprintf, "keycode = %d", inpt->KbdLastDown);
            FontUA::next_line(&cmd);
        }
        else
        {
            int v109 = 0;
            int v110 = 0;

            for ( NC_STACK_ypabact * &bact : _unitsList )
            {
                if (v109)
                    break;

                if ( bact->_bact_type == BACT_TYPES_ROBO && bact->_owner != 1 ) // FIXME owner
                {
                    v110++;

                    if ( _showDebugMode - 3 <= v110 )
                    {
                        NC_STACK_yparobo *robo = dynamic_cast<NC_STACK_yparobo *>(bact);

                        v109 = 1;

                        cmd = sub_445654(this, cmd, buf_sprintf, "robo owner %d with energy %d / %d / %d / %d", robo->_owner, robo->_energy, robo->_roboBuildSpare, robo->_roboVehicleSpare, robo->_energy_max);
                        FontUA::next_line(&cmd);

                        const char *v71;
                        const char *v73;

                        switch (robo->_roboBuildingDuty)
                        {
                        case 0x10:
                            v71 = "radar";
                            break;

                        case 0x20:
                            v71 = "powerstation";
                            break;

                        case 0x100:
                            v71 = "flak";
                            break;

                        case 0x20000:
                            v71 = "location";
                            break;

                        default:
                            v71 = "nothing";
                            break;
                        }

                        switch (robo->_roboVehicleDuty)
                        {
                        case 0:
                            v73 = "nothing";
                            break;

                        case 0x40:
                            v73 = "conquer";
                            break;

                        case 0x80:
                            v73 = "defense";
                            break;

                        case 0x80000:
                            v73 = "recon";
                            break;

                        case 0x200000:
                            v73 = "robo";
                            break;

                        default:
                            v73 = "powerstation";
                            break;
                        }

                        cmd = sub_445654(this, cmd, buf_sprintf, "    do build job   >%s<   and vhcl job   >%s<", v71, v73);
                        FontUA::next_line(&cmd);

                        cmd = sub_445654(this, cmd, buf_sprintf, "    wait power %d, radar %d, flak %d, location %d",
                                         robo->_roboPowerDelay / 1000,
                                         robo->_roboRadarDelay / 1000,
                                         robo->_roboSafetyDelay / 1000,
                                         robo->_roboPositionDelay / 1000);

                        FontUA::next_line(&cmd);

                        cmd = sub_445654(this, cmd, buf_sprintf, "    wait conquer %d, defense %d, recon %d, robo %d",
                                         robo->_roboConqDelay / 1000,
                                         robo->_roboEnemyDelay / 1000,
                                         robo->_roboExploreDelay / 1000,
                                         robo->_roboDangerDelay / 1000);
                        FontUA::next_line(&cmd);

                        cmd = sub_445654(this, cmd, buf_sprintf, "    values  ");
                        FontUA::next_line(&cmd);

                        if ( robo->_roboPowerDelay > 0 )
                            cmd = sub_445654(this, cmd, buf_sprintf, "power -1, ");
                        else
                            cmd = sub_445654(this, cmd, buf_sprintf, "power %d, ", robo->_roboPowerValue);

                        FontUA::next_line(&cmd);

                        if ( robo->_roboRadarDelay > 0 )
                            cmd = sub_445654(this, cmd, buf_sprintf, "radar -1, ");
                        else
                            cmd = sub_445654(this, cmd, buf_sprintf, "radar %d, ", robo->_roboRadarValue);

                        FontUA::next_line(&cmd);

                        if ( robo->_roboSafetyDelay > 0 )
                            cmd = sub_445654(this, cmd, buf_sprintf, "flak -1, ");
                        else
                            cmd = sub_445654(this, cmd, buf_sprintf, "flak %d, ", robo->_roboSafetyValue);

                        FontUA::next_line(&cmd);

                        if ( robo->_roboPositionDelay > 0 )
                            cmd = sub_445654(this, cmd, buf_sprintf, "power -1, ");
                        else
                            cmd = sub_445654(this, cmd, buf_sprintf, "power %d, ", robo->_roboPositionValue);

                        FontUA::next_line(&cmd);

                        if ( robo->_roboEnemyDelay > 0 )
                            cmd = sub_445654(this, cmd, buf_sprintf, "defense -1, ");
                        else
                            cmd = sub_445654(this, cmd, buf_sprintf, "defense %d, ", robo->_roboEnemyValue);

                        FontUA::next_line(&cmd);

                        if ( robo->_roboConqDelay > 0 )
                            cmd = sub_445654(this, cmd, buf_sprintf, "conquer -1, ");
                        else
                            cmd = sub_445654(this, cmd, buf_sprintf, "conquer %d, ", robo->_roboConqValue);

                        FontUA::next_line(&cmd);

                        if ( robo->_roboExploreDelay > 0 )
                            cmd = sub_445654(this, cmd, buf_sprintf, "recon -1, ");
                        else
                            cmd = sub_445654(this, cmd, buf_sprintf, "recon %d, ", robo->_roboExploreValue);

                        FontUA::next_line(&cmd);

                        if ( robo->_roboDangerDelay > 0 )
                            cmd = sub_445654(this, cmd, buf_sprintf, "robo -1, ");
                        else
                            cmd = sub_445654(this, cmd, buf_sprintf, "robo %d, ", robo->_roboDangerValue);

                        FontUA::next_line(&cmd);

                        if ( robo->_roboState & NC_STACK_yparobo::ROBOSTATE_DOCKINUSE )
                            cmd = sub_445654(this, cmd, buf_sprintf, "dock energy %d time %d", robo->_roboDockEnerg, robo->_roboDockTime);
                    }
                }
            }

            if ( !v109 )
                v104 = 1;
        }


        FontUA::next_line(&cmd);

        cmd = sub_445654(this, cmd, buf_sprintf, "fps: %d", field_1B6E);
        FontUA::next_line(&cmd);

        cmd = sub_445654(this, cmd, buf_sprintf, "polys: %d,%d", field_1B6A, field_1b6c);
        FontUA::next_line(&cmd);

        FontUA::set_end(&cmd);

        w3d_a209 arg209;
        arg209.cmdbuf = dbg_txt;
        arg209.includ = 0;

        GFX::Engine.raster_func209(&arg209);


        if ( v104 )
        {
            _showDebugMode = 0;
        }
        else
        {
            if ( sub_449678(inpt, Input::KC_F9) )
                _showDebugMode++;
        }
    }
}





void NC_STACK_ypaworld::HistoryAktCreate(NC_STACK_ypabact *bact)
{
    HistoryEventAdd( World::History::VhclCreate(bact->_owner, bact->_vehicleID, bact->_position.x * 256.0 / bact->_wrldSize.x, bact->_position.z * 256.0 / bact->_wrldSize.y) );
}

void NC_STACK_ypaworld::HistoryAktKill(NC_STACK_ypabact *bact)
{
    if ( bact->_killer )
    {
        uint8_t owners = (bact->_killer->_owner << 3) | bact->_owner;

        if ( bact->_killer->getBACT_viewer() || (bact->_killer->_status_flg & BACT_STFLAG_ISVIEW) )
            owners |= 0x80;

        if ( bact->getBACT_viewer() || (bact->_status_flg & BACT_STFLAG_ISVIEW) )
            owners |= 0x40;

        uint16_t vp = bact->_vehicleID;

        if ( bact->_bact_type == BACT_TYPES_ROBO )
            vp |= 0x8000;

        HistoryEventAdd( World::History::VhclKill(owners, vp, bact->_position.x * 256.0 / bact->_wrldSize.x, bact->_position.z * 256.0 / bact->_wrldSize.y) );
    }
}
