/* -*- coding: utf-8; indent-tabs-mode: t; tab-width: 4; c-basic-offset: 4; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2022, Keiichi Tabata. All rights reserved.
 */

/*
 * uimsg.c: UI message management.
 */

#include "suika.h"
#include "uimsg.h"

/* False assertion */
#define INVALID_UI_MSG_ID	(0)

/*
 * Get a UI message
 */
const char *get_ui_message(int id)
{
	switch (id) {
	case UIMSG_YES:
		switch (conf_locale) {
		case LOCALE_EN:
			return U8("YES");
		case LOCALE_JA:
			return U8("はい");
		case LOCALE_ZH:
			return U8("是");
		case LOCALE_TW:
			return U8("是");
		case LOCALE_FR:
			return U8("Oui");
		case LOCALE_RU:
			return U8("Да");
		case LOCALE_DE:
			return U8("Ja");
		case LOCALE_IT:
			return U8("Sì");
		case LOCALE_ES:
			return U8("Sí");
		case LOCALE_EL:
			return U8("Ναί");
		default:
			return U8("Yes");
		}
		break;
	case UIMSG_NO:
		switch (conf_locale) {
		case LOCALE_EN:
			return U8("No");
		case LOCALE_JA:
			return U8("いいえ");
		case LOCALE_ZH:
			return U8("不是");
		case LOCALE_TW:
			return U8("不是");
		case LOCALE_FR:
			return U8("Non");
		case LOCALE_RU:
			return U8("Нет");
		case LOCALE_DE:
			return U8("Nein");
		case LOCALE_IT:
			return U8("No");
		case LOCALE_ES:
			return U8("No");
		case LOCALE_EL:
			return U8("Οχι");
		default:
			return U8("No");
		}
		break;
	case UIMSG_INFO:
		switch (conf_locale) {
		case LOCALE_EN:
			return U8("Information");
		case LOCALE_JA:
			return U8("情報");
		case LOCALE_ZH:
			return U8("信息");
		case LOCALE_TW:
			return U8("信息");
		case LOCALE_FR:
			return U8("Informations");
		case LOCALE_RU:
			return U8("Информация");
		case LOCALE_DE:
			return U8("Information");
		case LOCALE_IT:
			return U8("Informazione");
		case LOCALE_ES:
			return U8("Información");
		case LOCALE_EL:
			return U8("Πληροφορίες");
		default:
			return U8("Information");
		}
		break;
	case UIMSG_WARN:
		switch (conf_locale) {
		case LOCALE_EN:
			return U8("Warning");
		case LOCALE_JA:
			return U8("警告");
		case LOCALE_ZH:
			return U8("警告");
		case LOCALE_TW:
			return U8("警告");
		case LOCALE_FR:
			return U8("Attention");
		case LOCALE_RU:
			return U8("Предупреждение");
		case LOCALE_DE:
			return U8("Warnung");
		case LOCALE_IT:
			return U8("Avvertimento");
		case LOCALE_ES:
			return U8("Advertencia");
		case LOCALE_EL:
			return U8("Προειδοποίηση");
		default:
			return U8("Warning");
		}
		break;
	case UIMSG_ERROR:
		switch (conf_locale) {
		case LOCALE_EN:
			return U8("Error");
		case LOCALE_JA:
			return U8("エラー");
		case LOCALE_ZH:
			return U8("错误");
		case LOCALE_TW:
			return U8("錯誤");
		case LOCALE_FR:
			return U8("Erreur");
		case LOCALE_RU:
			return U8("Ошибка");
		case LOCALE_DE:
			return U8("Fehler");
		case LOCALE_IT:
			return U8("Errore");
		case LOCALE_ES:
			return U8("Error");
		case LOCALE_EL:
			return U8("Λάθος");
		default:
			return U8("Error");
		}
		break;
	case UIMSG_CANNOT_OPEN_LOG:
		switch (conf_locale) {
		case LOCALE_EN:
			return U8("Cannot open log file.");
		case LOCALE_JA:
			return U8("ログファイルをオープンできません。");
		case LOCALE_ZH:
			return U8("无法打开日志文件。");
		case LOCALE_TW:
			return U8("無法打開日誌文件。");
		case LOCALE_FR:
			return U8("Impossible d'ouvrir le fichier journal.");
		case LOCALE_RU:
			return U8("Не удаётся открыть файл журнала.");
		case LOCALE_DE:
			return U8("Protokolldatei kann nicht geöffnet werden.");
		case LOCALE_IT:
			return U8("Impossibile aprire il file di registro.");
		case LOCALE_ES:
			return U8("No se puede abrir el archivo de registro.");
		case LOCALE_EL:
			return U8("Δεν είναι δυνατό το άνοιγμα του αρχείου καταγραφής.");
		default:
			return U8("Cannot open log file.");
		}
		break;
	case UIMSG_EXIT:
		switch (conf_locale) {
		case LOCALE_EN:
			return U8("Are you sure you want to quit?");
		case LOCALE_JA:
			return U8("ゲームを終了しますか？");
		case LOCALE_ZH:
			return U8("游戏结束了吗？");
		case LOCALE_TW:
			return U8("遊戲結束了嗎？");
		case LOCALE_FR:
			return U8("Quitter le jeu?");
		case LOCALE_RU:
			return U8("Вы уверены, что хотите выйти?");
		case LOCALE_DE:
			return U8("Sind Sie sicher, dass Sie aufhören wollen?");
		case LOCALE_IT:
			return U8("Sei sicuro di voler uscire?");
		case LOCALE_ES:
			return U8("¿Seguro que quieres salir?");
		case LOCALE_EL:
			return U8("Είσαι σίγουρος ότι θέλεις να παραιτηθείς?");
		default:
			return U8("Are you sure you want to quit?");
		}
		break;
	case UIMSG_TITLE:
		switch (conf_locale) {
		case LOCALE_EN:
			return U8("Are you sure you want to go to title?");
		case LOCALE_JA:
			return U8("タイトルに戻りますか？");
		case LOCALE_ZH:
			return U8("回到标题？");
		case LOCALE_TW:
			return U8("回到標題？");
		case LOCALE_FR:
			return U8("Retour au titre?");
		case LOCALE_RU:
			return U8("Вы уверены, что хотите вернуться к началу игры?");
		case LOCALE_DE:
			return U8("Sind Sie sicher, dass Sie zum Titel wechseln möchten?");
		case LOCALE_IT:
			return U8("Sei sicuro di voler andare al titolo?");
		case LOCALE_ES:
			return U8("¿Seguro que quieres ir al título?");
		case LOCALE_EL:
			return U8("Είστε σίγουροι ότι θέλετε να πάτε στον τίτλο;");
		default:
			return U8("Are you sure you want to go to title?");
		}
		break;
	case UIMSG_DELETE:
		switch (conf_locale) {
		case LOCALE_EN:
			return U8("Are you sure you want to delete the save data?");
		case LOCALE_JA:
			return U8("削除してもよろしいですか？");
		case LOCALE_ZH:
			return U8("删除确定要删除吗？");
		case LOCALE_TW:
			return U8("刪除確定要刪除嗎？");
		case LOCALE_FR:
			return U8("Supprimer Voulez-vous vraiment?");
		case LOCALE_RU:
			return U8("Вы уверены, что хотите удалить сохранённые данные?");
		case LOCALE_DE:
			return U8("Möchten Sie die Speicherdaten wirklich löschen?");
		case LOCALE_IT:
			return U8("Sei sicuro di voler eliminare i dati di salvataggio?");
		case LOCALE_ES:
			return U8("¿Está seguro de que desea eliminar los datos guardados?");
		case LOCALE_EL:
			return U8("Είστε βέβαιοι ότι θέλετε να διαγράψετε τα δεδομένα αποθήκευσης;");
		default:
			return U8("Are you sure you want to delete the save data?");
		}
		break;
	case UIMSG_OVERWRITE:
		switch (conf_locale) {
		case LOCALE_EN:
			return U8("Are you sure you want to overwrite the save data?");
		case LOCALE_JA:
			return U8("上書きしてもよろしいですか？");
		case LOCALE_ZH:
			return U8("您确定要覆盖吗？");
		case LOCALE_TW:
			return U8("您確定要覆蓋嗎？");
		case LOCALE_FR:
			return U8("Voulez-vous vraiment écraser?");
		case LOCALE_RU:
			return U8("Вы уверены, что хотите перезаписать сохранённые данные?");
		case LOCALE_DE:
			return U8("Möchten Sie die Speicherdaten wirklich überschreiben?");
		case LOCALE_IT:
			return U8("Sei sicuro di voler sovrascrivere i dati di salvataggio?");
		case LOCALE_ES:
			return U8("¿Está seguro de que desea sobrescribir los datos guardados?");
		case LOCALE_EL:
			return U8("Είστε βέβαιοι ότι θέλετε να αντικαταστήσετε τα δεδομένα αποθήκευσης;");
		default:
			return U8("Are you sure you want to overwrite the save data?");
		}
		break;
	case UIMSG_DEFAULT:
		switch (conf_locale) {
		case LOCALE_EN:
			return U8("Are you sure you want to reset the settings?");
		case LOCALE_JA:
			return U8("設定をリセットしてもよろしいですか？");
		case LOCALE_ZH:
			return U8("您确定要重置设置吗？");
		case LOCALE_TW:
			return U8("您確定要重置設置嗎？");
		case LOCALE_FR:
			return U8("Voulez-vous vraiment réinitialiser les paramètres?");
		case LOCALE_RU:
			return U8("Вы уверены, что хотите сбросить настройки?");
		case LOCALE_DE:
			return U8("Möchten Sie die Einstellungen wirklich zurücksetzen?");
		case LOCALE_IT:
			return U8("Sei sicuro di voler ripristinare le impostazioni?");
		case LOCALE_ES:
			return U8("¿Está seguro de que desea restablecer la configuración?");
		case LOCALE_EL:
			return U8("Είστε βέβαιοι ότι θέλετε να επαναφέρετε τις ρυθμίσεις;");
		default:
			return U8("Are you sure you want to reset the settings?");
		}
		break;
	case UIMSG_NO_SOUND_DEVICE:
		switch (conf_locale) {
		case LOCALE_EN:
			return U8("No sound output device.");
		case LOCALE_JA:
			return U8("サウンド出力デバイスがありません。");
		case LOCALE_ZH:
			return U8("No sound output device.");
		case LOCALE_TW:
			return U8("No sound output device.");
		case LOCALE_FR:
			return U8("No sound output device.");
		case LOCALE_RU:
			return U8("No sound output device.");
		case LOCALE_DE:
			return U8("No sound output device.");
		case LOCALE_IT:
			return U8("No sound output device.");
		case LOCALE_ES:
			return U8("No sound output device.");
		case LOCALE_EL:
			return U8("No sound output device.");
		default:
			return U8("No sound output device.");
		}
		break;
	case UIMSG_NO_GAME_FILES:
		switch (conf_locale) {
		case LOCALE_JA:
			return U8("ゲームデータがありません。");
		default:
			return U8("No game data.");
		}
		break;
#ifdef SUIKA_TARGET_WIN32
	case UIMSG_WIN32_NO_DIRECT3D:
		switch (conf_locale) {
		case LOCALE_EN:
			return U8("Direct3D is not supported.");
		case LOCALE_JA:
			return U8("Direct3Dはサポートされません。");
		case LOCALE_ZH:
			return U8("不支持Direct3D。");
		case LOCALE_TW:
			return U8("不支持Direct3D。");
		case LOCALE_FR:
			return U8("Direct3D n'est pas pris en charge.");
		case LOCALE_RU:
			return U8("Direct3D не поддерживается.");
		case LOCALE_DE:
			return U8("Direct3D wird nicht unterstützt.");
		case LOCALE_IT:
			return U8("Direct3D non è supportato.");
		case LOCALE_ES:
			return U8("Direct3D no es compatible.");
		case LOCALE_EL:
			return U8("Το Direct3D δεν υποστηρίζεται.");
		default:
			return U8("Direct3D is not supported.");
		}
		break;
	case UIMSG_WIN32_SMALL_DISPLAY:
		switch (conf_locale) {
		case LOCALE_EN:
			return U8("Display size too small (%d x %d).");
		case LOCALE_JA:
			return U8("ディスプレイのサイズが足りません。(%d x %d)");
		case LOCALE_ZH:
			return U8("显示尺寸不足。(%d x %d)");
		case LOCALE_TW:
			return U8("顯示尺寸不足。(%d x %d)");
		case LOCALE_FR:
			return U8("Taille d'affichage insuffisante. (%d x %d)");
		case LOCALE_RU:
			return U8("Размер экрана слишком маленький. (%d x %d)");
		case LOCALE_DE:
			return U8("Anzeigegrö ÿe zu klein. (%d x %d)");
		case LOCALE_IT:
			return U8("Dimensioni del display troppo piccole. (%d x %d)");
		case LOCALE_ES:
			return U8("Tamaño de la pantalla demasiado pequeño. (%d x %d)");
		case LOCALE_EL:
			return U8("Το μέγεθος της οθόνης είναι πολύ μικρό. (%d x %d)");
		default:
			return U8("Display size too small. (%d x %d)");
		}
		break;
	case UIMSG_WIN32_MENU_FILE:
		switch (conf_locale) {
		case LOCALE_EN:
			return U8("File(&F)");
		case LOCALE_JA:
			return U8("ファイル(&F)");
		case LOCALE_ZH:
			return U8("文件(&F)");
		case LOCALE_TW:
			return U8("文件(&F)");
		case LOCALE_FR:
			return U8("Fichier(&F)");
		case LOCALE_RU:
			return U8("Файл(&F)");
		case LOCALE_DE:
			return U8("Datei(&F)");
		case LOCALE_IT:
			return U8("File(&F)");
		case LOCALE_ES:
			return U8("Archivar(&F)");
		case LOCALE_EL:
			return U8("Αρχείο(&F)");
		default:
			return U8("File(&F)");
		}
		break;
	case UIMSG_WIN32_MENU_VIEW:
		switch (conf_locale) {
		case LOCALE_EN:
			return U8("View(&V)");
		case LOCALE_JA:
			return U8("表示(&V)");
		case LOCALE_ZH:
			return U8("展示(&V)");
		case LOCALE_TW:
			return U8("展示(&V)");
		case LOCALE_FR:
			return U8("Voir(&V)");
		case LOCALE_RU:
			return U8("Вид(&V)");
		case LOCALE_DE:
			return U8("Aussicht(&V)");
		case LOCALE_IT:
			return U8("View(&V)");
		case LOCALE_ES:
			return U8("View(&V)");
		case LOCALE_EL:
			return U8("Απεικόνιση(&V)");
		default:
			return U8("View(&V)");
		}
		break;
	case UIMSG_WIN32_MENU_QUIT:
		switch (conf_locale) {
		case LOCALE_EN:
			return U8("Quit(&Q)");
		case LOCALE_JA:
			return U8("終了(&Q)");
		case LOCALE_ZH:
			return U8("退出(&Q)");
		case LOCALE_TW:
			return U8("退出(&Q)");
		case LOCALE_FR:
			return U8("Quitter(&Q)");
		case LOCALE_RU:
			return U8("Выход(&Q)");
		case LOCALE_DE:
			return U8("Aufhören(&Q)");
		case LOCALE_IT:
			return U8("Smettere(&Q)");
		case LOCALE_ES:
			return U8("Abandonar(&Q)");
		case LOCALE_EL:
			return U8("Παρατώ(&Q)");
		default:
			return U8("Quit(&Q)");
		}
		break;
	case UIMSG_WIN32_MENU_FULLSCREEN:
		switch (conf_locale) {
		case LOCALE_EN:
			return U8("Full Screen(&F)\tAlt+Enter");
		case LOCALE_JA:
			return U8("フルスクリーン(&F)\tAlt+Enter");
		case LOCALE_ZH:
			return U8("全屏(&F)\tAlt+Enter");
		case LOCALE_TW:
			return U8("全屏(&F)\tAlt+Enter");
		case LOCALE_FR:
			return U8("Plein écran(&F)\tAlt+Enter");
		case LOCALE_RU:
			return U8("На весь экран(&F)\tAlt+Enter");
		case LOCALE_DE:
			return U8("Ganzer Bildschirm(&F)\tAlt+Enter");
		case LOCALE_IT:
			return U8("Schermo Intero(&F)\tAlt+Enter");
		case LOCALE_ES:
			return U8("Pantalla completa(&F)\tAlt+Enter");
		case LOCALE_EL:
			return U8("Πλήρης οθόνη(&F)\tAlt+Enter");
		default:
			return U8("Full Screen(&F)\tAlt+Enter");
		}
		break;
#endif
	}

	/* Never come here. */
	assert(INVALID_UI_MSG_ID);
	return NULL;
}
