/***********************************************************************

                             Paradise Cracked

                       Copyright by MiST land 2000

   ------------------------------------------------------------------    
    Description:
   ------------------------------------------------------------------    
    Author: Pavel A.Duvanov
    Date:   29.10.2000

************************************************************************/
#include "Precomp.h"
#include "Options.h"
#include "XRegistry.h"
#include "PCOptions.h"

void PCOptions::Register(void)
{
	//	------------- Регистрация переменных --------------
	Options::Set("system.video.driver",0);
	Options::Set("system.video.device",0);
	Options::Set("system.video.width",800);
	Options::Set("system.video.height",600);
	Options::Set("system.video.bpp",16);
	Options::Set("system.video.freq",0);
	Options::Set("system.video.windowed",0);
	Options::Set("system.input.sensitivity",1.7f);
	
	//	настройки звука
	Options::Set("system.sound.type",0);
	Options::Set("system.sound.update_freq",1.0f);// частота обновления звуковых источников
	Options::Set("system.sound.themes",1);
	Options::Set("game.sound.voicesvol",50);
	Options::Set("game.sound.ambientvol",50);
	Options::Set("game.sound.effectsvol",50);
	Options::Set("game.sound.musicvol",50);

	//
	Options::Set("system.video.gamma",0.8f);
	Options::Set("system.video.triple",0);
	Options::Set("system.video.effectsquality",1.0f);
	Options::Set("system.video.tl_optimization",0); //	флаг поддержки кода для T&L 
	Options::Set("system.video.lightmaps",1);		//	флаг для включения/выкл. карт освещенности
	Options::Set("game.anispeed",1.5f);				//	скорость анимации
	Options::Set("game.animaquality",1.0f);
	Options::Set("game.cursor_refreshtime",0.150f);	//	время обновления курсора в игре
	Options::Set("game.marks_on_walls",1);			//	отметины на стенах
	Options::Set("game.dynamic_lights",1);			//	динамический свет
	Options::Set("game.environment",1);				//	окружающая среда
	Options::Set("game.camera_speed",1.5f);			//	скорость перемещения камеры

	//	---- свойства текстур ---------------------------------
	Options::Set("system.video.texquality",0);
	Options::Set("system.video.mipmap",-1);
	Options::Set("system.video.usecompressing",1);
	//	-------------------------------------------------------
	Options::Set("system.net.refreshtime",3.0f);
	Options::Set("game.player","Player");
	//	---- тени персонажей ----------------------------------
	Options::Set("system.video.shadowrate",0.1f);
	Options::Set("system.video.shadowquality",3);
	//	---- включение/отключение эффектов --------------------
	Options::Set("game.mark_wave_front",1);	 // видимость фронта волны куда может дойти юнит
	Options::Set("game.mark_exit_zone",1);	 // видимость зон выхода
	Options::Set("game.mark_path",1);		 // видимость пути (рисуется или не рисуется)
    Options::Set("game.mark_lands", 1);      // помечать точки высадки из техники
    Options::Set("game.show_fos", 0);        // постоянно показывать Field Of Sight 
    Options::Set("game.show_banner",0);      // постоянно показывать banner над entity
	//	для хранения посещенного эпизода
	Options::Set("game.advance",256);		 //	уровень посещенности определяется как (256-value)
																				//256 - видны все интро (первые 3 мультика)
																				//255 +мультик 1 эпизода
																				//254 +мультик 2 эпизода
																				//253 +мультик 3 эпизода
																				//252 +мультик 4 эпизода
																				//251 + финальный мультик
	Options::Set("game.mod","");			//	используемый мод, пустая строка - означает оригинальную версию
	//	отключение хелпера
    Options::Set("game.helper",1);			// постоянно показывать хелпер
	//	уровень сложности игры
	Options::Set("game.type",2);
	//	0 - Novice
	//	1 - Easy
	//	2 - Normal
	//	3 - Hard

    //	---- настройка экрана hidden_movement --------------------

    Options::Set("game.hidden_movement.enemy_related.move",  1);  //показывать перемещение враждебных персонажей
    Options::Set("game.hidden_movement.enemy_related.shoot", 1);  //показывать стрельбу враждебных персонажей

    Options::Set("game.hidden_movement.friend_related.move",  1);  //показывать перемещение дружественных персонажей
    Options::Set("game.hidden_movement.friend_related.shoot", 1);  //показывать стрельбу дружественных персонажей

    Options::Set("game.hidden_movement.neutral_related.move",  1);  //показывать перемещение нейтральных персонажей
    Options::Set("game.hidden_movement.neutral_related.shoot", 1);  //показывать стрельбу нейтральных персонажей

    Options::Set("game.hidden_movement.civilian_vehicles.move",  1);  //показывать перемещение мирной техники
    Options::Set("game.hidden_movement.civilian_vehicles.shoot", 1);  //показывать стрельбу мирной техники

    Options::Set("game.hidden_movement.civilian_humans_traders.move",  1);  //показывать перемещение мирных жителей
    Options::Set("game.hidden_movement.civilian_humans_traders.shoot", 1);  //показывать стрельбу мирных жителей

	//*********************************************************
	//	работа с реестром
	//	инициализируем ключ ***********************************
	Options::Registry()->Init(HKEY_LOCAL_MACHINE,"SOFTWARE\\MiST Land\\Paradise Cracked\\");
	//	регистируем переменные ********************************
	char cur_dir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH,cur_dir);
	//	путь до каталога с мультиками
	Options::Registry()->Register("Video Path",std::string(std::string(cur_dir)+"\\Video"));
	Options::Registry()->Register("Packs Path","");
	//	считываем значения из реестра *************************
	Options::Registry()->Read();
}
