/* -*- coding: utf-8-with-signature; indent-tabs-mode: t; tab-width: 4; c-basic-offset: 4; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2022, TABATA Keiichi. All rights reserved.
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
const wchar_t *get_ui_message(int id)
{
	switch (id) {
	case UIMSG_YES:
		switch (conf_locale) {
		case LOCALE_JA:
			return L"はい";
		case LOCALE_ZH:
			return L"是";
		case LOCALE_TW:
			return L"是";
		case LOCALE_FR:
			return L"Oui";
		case LOCALE_RU:
			return L"да";
		case LOCALE_DE:
			return L"Ja";
		case LOCALE_IT:
			return L"Sì";
		case LOCALE_ES:
			return L"Sí";
		case LOCALE_EL:
			return L"Ναί";
		default:
			return L"Yes";
		}
		break;
	case UIMSG_NO:
		switch (conf_locale) {
		case LOCALE_JA:
			return L"いいえ";
		case LOCALE_ZH:
			return L"不是";
		case LOCALE_TW:
			return L"不是";
		case LOCALE_FR:
			return L"Non";
		case LOCALE_RU:
			return L"Нет";
		case LOCALE_DE:
			return L"Nein";
		case LOCALE_IT:
			return L"No";
		case LOCALE_ES:
			return L"No";
		case LOCALE_EL:
			return L"Οχι";
		default:
			return L"No";
		}
		break;
	case UIMSG_INFO:
		switch (conf_locale) {
		case LOCALE_JA:
			return L"情報";
		case LOCALE_ZH:
			return L"信息";
		case LOCALE_TW:
			return L"信息";
		case LOCALE_FR:
			return L"Informations";
		case LOCALE_RU:
			return L"Информация";
		case LOCALE_DE:
			return L"Information";
		case LOCALE_IT:
			return L"Informazione";
		case LOCALE_ES:
			return L"Información";
		case LOCALE_EL:
			return L"Πληροφορίες";
		default:
			return L"Information";
		}
		break;
	case UIMSG_WARN:
		switch (conf_locale) {
		case LOCALE_JA:
			return L"注意";
		case LOCALE_ZH:
			return L"警告";
		case LOCALE_TW:
			return L"警告";
		case LOCALE_FR:
			return L"Attention";
		case LOCALE_RU:
			return L"Предупреждение";
		case LOCALE_DE:
			return L"Warnung";
		case LOCALE_IT:
			return L"Avvertimento";
		case LOCALE_ES:
			return L"Advertencia";
		case LOCALE_EL:
			return L"Προειδοποίηση";
		default:
			return L"Warning";
		}
		break;
	case UIMSG_ERROR:
		switch (conf_locale) {
		case LOCALE_JA:
			return L"エラー";
		case LOCALE_ZH:
			return L"错误";
		case LOCALE_TW:
			return L"錯誤";
		case LOCALE_FR:
			return L"Erreur";
		case LOCALE_RU:
			return L"Ошибка";
		case LOCALE_DE:
			return L"Fehler";
		case LOCALE_IT:
			return L"Errore";
		case LOCALE_ES:
			return L"Error";
		case LOCALE_EL:
			return L"Λάθος";
		default:
			return L"Error";
		}
		break;
	case UIMSG_CANNOT_OPEN_LOG:
		switch (conf_locale) {
		case LOCALE_JA:
			return L"ログファイルをオープンできません。";
		case LOCALE_ZH:
			return L"无法打开日志文件。";
		case LOCALE_TW:
			return L"無法打開日誌文件。";
		case LOCALE_FR:
			return L"Impossible d'ouvrir le fichier journal.";
		case LOCALE_RU:
			return L"Не удается открыть файл журнала.";
		case LOCALE_DE:
			return L"Protokolldatei kann nicht geöffnet werden.";
		case LOCALE_IT:
			return L"Impossibile aprire il file di registro.";
		case LOCALE_ES:
			return L"No se puede abrir el archivo de registro.";
		case LOCALE_EL:
			return L"Δεν είναι δυνατό το άνοιγμα του αρχείου καταγραφής.";
		default:
			return L"Cannot open log file.";
		}
		break;
	case UIMSG_EXIT:
		switch (conf_locale) {
		case LOCALE_JA:
			return L"ゲームを終了しますか？";
		case LOCALE_ZH:
			return L"游戏结束了吗？";
		case LOCALE_TW:
			return L"遊戲結束了嗎？";
		case LOCALE_FR:
			return L"Le jeu est-il terminé?";
		case LOCALE_RU:
			return L"Вы уверены, что хотите выйти?";
		case LOCALE_DE:
			return L"Sind Sie sicher, dass Sie aufhören wollen?";
		case LOCALE_IT:
			return L"Sei sicuro di voler uscire?";
		case LOCALE_ES:
			return L"¿Seguro que quieres salir?";
		case LOCALE_EL:
			return L"Είσαι σίγουρος ότι θέλεις να παραιτηθείς?";
		default:
			return L"Are you sure you want to quit?";
		}
		break;
	case UIMSG_TITLE:
		switch (conf_locale) {
		case LOCALE_JA:
			return L"タイトルに戻りますか？";
		case LOCALE_ZH:
			return L"回到标题？";
		case LOCALE_TW:
			return L"回到標題？";
		case LOCALE_FR:
			return L"Retour au titre?";
		case LOCALE_RU:
			return L"Вы уверены, что хотите перейти к титулу?";
		case LOCALE_DE:
			return L"Sind Sie sicher, dass Sie zum Titel wechseln möchten?";
		case LOCALE_IT:
			return L"Sei sicuro di voler andare al titolo?";
		case LOCALE_ES:
			return L"¿Seguro que quieres ir al título?";
		case LOCALE_EL:
			return L"Είστε σίγουροι ότι θέλετε να πάτε στον τίτλο;";
		default:
			return L"Are you sure you want to go to title?";
		}
		break;
	case UIMSG_DELETE:
		switch (conf_locale) {
		case LOCALE_JA:
			return L"削除してもよろしいですか？";
		case LOCALE_ZH:
			return L"删除确定要删除吗？";
		case LOCALE_TW:
			return L"刪除確定要刪除嗎？";
		case LOCALE_FR:
			return L"Supprimer Voulez-vous vraiment?";
		case LOCALE_RU:
			return L"Вы уверены, что хотите удалить данные сохранения?";
		case LOCALE_DE:
			return L"Möchten Sie die Speicherdaten wirklich löschen?";
		case LOCALE_IT:
			return L"Sei sicuro di voler eliminare i dati di salvataggio?";
		case LOCALE_ES:
			return L"¿Está seguro de que desea eliminar los datos guardados?";
		case LOCALE_EL:
			return L"Είστε βέβαιοι ότι θέλετε να διαγράψετε τα δεδομένα αποθήκευσης;";
		default:
			return L"Are you sure you want to delete the save data?";
		}
		break;
	case UIMSG_OVERWRITE:
		switch (conf_locale) {
		case LOCALE_JA:
			return L"上書きしてもよろしいですか？";
		case LOCALE_ZH:
			return L"您确定要覆盖吗？";
		case LOCALE_TW:
			return L"您確定要覆蓋嗎？";
		case LOCALE_FR:
			return L"Voulez-vous vraiment écraser?";
		case LOCALE_RU:
			return L"Вы уверены, что хотите перезаписать данные сохранения?";
		case LOCALE_DE:
			return L"Möchten Sie die Speicherdaten wirklich überschreiben?";
		case LOCALE_IT:
			return L"Sei sicuro di voler sovrascrivere i dati di salvataggio?";
		case LOCALE_ES:
			return L"¿Está seguro de que desea sobrescribir los datos guardados?";
		case LOCALE_EL:
			return L"Είστε βέβαιοι ότι θέλετε να αντικαταστήσετε τα δεδομένα αποθήκευσης;";
		default:
			return L"Are you sure you want to overwrite the save data?";
		}
		break;
	case UIMSG_DEFAULT:
		switch (conf_locale) {
		case LOCALE_JA:
			return L"設定をリセットしてもよろしいですか？";
		case LOCALE_ZH:
			return L"您确定要重置设置吗？";
		case LOCALE_TW:
			return L"您確定要重置設置嗎？";
		case LOCALE_FR:
			return L"Voulez-vous vraiment réinitialiser les paramètres?";
		case LOCALE_RU:
			return L"Вы уверены, что хотите сбросить настройки?";
		case LOCALE_DE:
			return L"Möchten Sie die Einstellungen wirklich zurücksetzen?";
		case LOCALE_IT:
			return L"Sei sicuro di voler ripristinare le impostazioni?";
		case LOCALE_ES:
			return L"¿Está seguro de que desea restablecer la configuración?";
		case LOCALE_EL:
			return L"Είστε βέβαιοι ότι θέλετε να επαναφέρετε τις ρυθμίσεις;";
		default:
			return L"Are you sure you want to reset the settings?";
		}
		break;
#ifdef WIN
	case UIMSG_WIN_NO_DIRECT3D:
		switch (conf_locale) {
		case LOCALE_JA:
			return L"Direct3Dはサポートされません。";
		case LOCALE_ZH:
			return L"不支持Direct3D。";
		case LOCALE_TW:
			return L"不支持Direct3D。";
		case LOCALE_FR:
			return L"Direct3D n'est pas pris en charge.";
		case LOCALE_RU:
			return L"Direct3D не поддерживается.";
		case LOCALE_DE:
			return L"Direct3D wird nicht unterstützt.";
		case LOCALE_IT:
			return L"Direct3D non è supportato.";
		case LOCALE_ES:
			return L"Direct3D no es compatible.";
		case LOCALE_EL:
			return L"Το Direct3D δεν υποστηρίζεται.";
		default:
			return L"Direct3D is not supported.";
		}
		break;
	case UIMSG_WIN_SMALL_DISPLAY:
		switch (conf_locale) {
		case LOCALE_JA:
			return L"ディスプレイのサイズが足りません。";
		case LOCALE_ZH:
			return L"显示尺寸不足。";
		case LOCALE_TW:
			return L"顯示尺寸不足。";
		case LOCALE_FR:
			return L"Taille d'affichage insuffisante.";
		case LOCALE_RU:
			return L"Слишком маленький размер дисплея.";
		case LOCALE_DE:
			return L"Anzeigegröße zu klein.";
		case LOCALE_IT:
			return L"Dimensioni del display troppo piccole.";
		case LOCALE_ES:
			return L"Tamaño de la pantalla demasiado pequeño.";
		case LOCALE_EL:
			return L"Το μέγεθος της οθόνης είναι πολύ μικρό.";
		default:
			return L"Display size too small.";
		}
		break;
	case UIMSG_WIN_MENU_FILE:
		switch (conf_locale) {
		case LOCALE_JA:
			return L"ファイル(&F)";
		case LOCALE_ZH:
			return L"文件(&F)";
		case LOCALE_TW:
			return L"文件(&F)";
		case LOCALE_FR:
			return L"Fichier(&F)";
		case LOCALE_RU:
			return L"Файл(&F)";
		case LOCALE_DE:
			return L"Datei(&F)";
		case LOCALE_IT:
			return L"File(&F)";
		case LOCALE_ES:
			return L"Archivar(&F)";
		case LOCALE_EL:
			return L"Αρχείο(&F)";
		default:
			return L"File(&F)";
		}
		break;
	case UIMSG_WIN_MENU_VIEW:
		switch (conf_locale) {
		case LOCALE_JA:
			return L"表示(&V)";
		case LOCALE_ZH:
			return L"展示(&V)";
		case LOCALE_TW:
			return L"展示(&V)";
		case LOCALE_FR:
			return L"Voir(&V)";
		case LOCALE_RU:
			return L"Вид(&V)";
		case LOCALE_DE:
			return L"Aussicht(&V)";
		case LOCALE_IT:
			return L"Vista(&V)";
		case LOCALE_ES:
			return L"Vista(&V)";
		case LOCALE_EL:
			return L"Απεικόνιση(&V)";
		default:
			return L"View(&V)";
		}
		break;
	case UIMSG_WIN_MENU_QUIT:
		switch (conf_locale) {
		case LOCALE_JA:
			return L"終了(&Q)";
		case LOCALE_ZH:
			return L"退出(&Q)";
		case LOCALE_TW:
			return L"退出(&Q)";
		case LOCALE_FR:
			return L"Quitter(&Q)";
		case LOCALE_RU:
			return L"Покидать(&Q)";
		case LOCALE_DE:
			return L"Aufhören(&Q)";
		case LOCALE_IT:
			return L"Smettere(&Q)";
		case LOCALE_ES:
			return L"Abandonar(&Q)";
		case LOCALE_EL:
			return L"Παρατώ(&Q)";
		default:
			return L"Quit(&Q)";
		}
		break;
	case UIMSG_WIN_MENU_FULLSCREEN:
		switch (conf_locale) {
		case LOCALE_JA:
			return L"フルスクリーン(&F)\tAlt+Enter";
		case LOCALE_ZH:
			return L"全屏(&F)\tAlt+Enter";
		case LOCALE_TW:
			return L"全屏(&F)\tAlt+Enter";
		case LOCALE_FR:
			return L"Plein écran(&F)\tAlt+Enter";
		case LOCALE_RU:
			return L"Полноэкранный(&F)\tAlt+Enter";
		case LOCALE_DE:
			return L"Ganzer Bildschirm(&F)\tAlt+Enter";
		case LOCALE_IT:
			return L"Schermo Intero(&F)\tAlt+Enter";
		case LOCALE_ES:
			return L"Pantalla completa(&F)\tAlt+Enter";
		case LOCALE_EL:
			return L"Πλήρης οθόνη(&F)\tAlt+Enter";
		default:
			return L"Full Screen(&F)\tAlt+Enter";
		}
		break;
#endif
	}

	/* Never come here. */
	assert(INVALID_UI_MSG_ID);
	return NULL;
}
