<!DOCTYPE html>
<html lang="ja-JP">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width">

  <title>Suika2 Wiki | <?php echo $title ?></title>
  <meta name="description" content="Suika2 ユーザ Wiki">
  <meta name="theme-color" content="#eb8334">

	<link href="https://cdn.jsdelivr.net/npm/beercss@3.4.11/dist/cdn/beer.min.css" rel="stylesheet">
	<script type="module" src="https://cdn.jsdelivr.net/npm/beercss@3.4.11/dist/cdn/beer.min.js" defer></script>
	<link href="https://suika2.com/css/game.css" rel="stylesheet">

	<script async src="https://www.googletagmanager.com/gtag/js?id=G-PLYR5Y3JSJ"></script>
	<script>
		window.dataLayer = window.dataLayer || [];
		function gtag() {
				dataLayer.push(arguments);
		}
		gtag("js", new Date());
		gtag("config", "G-PLYR5Y3JSJ");
	</script>

  <?php echo $head_tag ?>

</head>
<body>

<!-- BEGIN-HEADER -->
<footer class="m l">
  <nav style="width: 75%; margin: auto;">
    <button class="transparent small-round"><img class="responsive" style="margin: auto;" src="/img/icons/suika.png"></button>
    <a href="/" class="button transparent small-round"><i>home</i>ホーム</a>
    <a href="/dl/" class="button transparent small-round"><i>download</i>ダウンロード</a>
    <button data-ui="#menu_help" class="button transparent small-round"><i>help</i><span>ドキュメント</span><i>arrow_drop_down</i>
      <menu id="menu_help" class="no-wrap">
				<a href="https://suika2.com/wiki/?%E3%83%89%E3%82%AD%E3%83%A5%E3%83%A1%E3%83%B3%E3%83%88">一覧</a>
				<a href="https://suika2.com/wiki/?%E3%83%81%E3%83%A5%E3%83%BC%E3%83%88%E3%83%AA%E3%82%A2%E3%83%AB">チュートリアル</a>
        <a href="https://suika2.com/wiki/?%E3%82%B3%E3%83%9E%E3%83%B3%E3%83%89%E3%83%AA%E3%83%95%E3%82%A1%E3%83%AC%E3%83%B3%E3%82%B9">コマンドリファレンス</a>
				<a href="https://suika2.com/wiki/?%E3%82%BF%E3%82%A4%E3%83%88%E3%83%AB%E7%94%BB%E9%9D%A2%E3%81%AE%E4%BD%9C%E3%82%8A%E6%96%B9">タイトル画面の作り方</a>
				<a href="/doc/font.html">フォントの変更方法</a>
				<a href="https://suika2.com/wiki/?%E3%82%B3%E3%83%B3%E3%83%95%E3%82%A3%E3%82%B0%E4%B8%80%E8%A6%A7">コンフィグ一覧</a>
				<a href="https://suika2.com/wiki/?WMS%E3%81%AE%E4%BD%BF%E3%81%84%E6%96%B9">WMSの使い方</a>
				<a href="https://suika2.com/wiki/?%E3%82%88%E3%81%8F%E3%81%82%E3%82%8B%E8%B3%AA%E5%95%8F">よくある質問</a>
				<a href="https://suika2.com/wiki/?%E3%83%90%E3%82%B0%E5%A0%B1%E5%91%8A%E3%81%AE%E4%B8%80%E8%A6%A7">バグ報告の一覧</a>
				<a href="https://suika2.com/wiki/?%E6%A9%9F%E8%83%BD%E8%A6%81%E6%9C%9B%E3%81%AE%E4%B8%80%E8%A6%A7">機能要望の一覧</a>
				<a href="/wiki/">Wikiトップ</a>
				<a href="/wiki/?cmd=search">検索</a>
      </menu>
    </button>
    <a href="/works/" class="button transparent small-round"><i>gamepad</i>利用作品一覧</a>
    <a href="/en/" class="button transparent small-round">English</a>
    <div class="max"></div>
    <button id="btn_theme" class="square round extra secondary-container"><i>dark_mode</i></button>
  </nav>
</footer>
<footer class="s">
  <nav>
    <button class="transparent small-round">
      <img class="responsive" style="margin: auto;" src="/img/icons/suika.png">
    </button>
    <div class="max"></div>
    <button class="square round extra secondary-container" data-ui="#drawer_mobile"><i>menu</i></button>
  </nav>
</footer>
<dialog class="left no-padding no-round" id="drawer_mobile">
  <nav class="drawer">
    <header>
      <nav>
        <img class="circle" src="/img/icons/suika.png">
        <h6 class="max">Suika2</h6>
        <button class="transparent large small-round" data-ui="#drawer_mobile"><i>close</i></button>
      </nav>
    </header>
    <a href="/" class="button transparent small-round"><i>home</i>Home</a>
    <a href="/dl/" class="button transparent small-round"><i>download</i>Downloads</a>
    <a href="https://suika2.com/wiki/?%E3%83%89%E3%82%AD%E3%83%A5%E3%83%A1%E3%83%B3%E3%83%88" class="button transparent small-round"><i>help</i>Help</a>
    <a href="/works/" class="button transparent small-round"><i>gamepad</i>利用作品紹介</a>
    <a href="/" class="button transparent small-round">English</a>
    <div class="max"></div>
    <button id="btn_theme_mobile" class="square round extra secondary-container"><i>dark_mode</i></button>
  </nav>
</dialog>
<!-- END-HEADER -->

<!-- Title Strip -->
<footer class="medium-padding m l">
	<nav style="width: 75%; margin: auto;">
		<div>
       <h1><?php echo $title ?> - Suika2 Wiki</h1>
	     <p>
         <a href="/wiki/">Wikiトップ</a>&nbsp;&nbsp;
         <a href="<?php echo $link_edit ?>" title="このページを編集します。">編集</a>&nbsp;&nbsp;
         <a href="<?php echo $script ?>?plugin=newpage" title="新しくページを作成します。">新規作成</a>&nbsp;&nbsp;
         <a href="<?php echo $link_template ?>" title="このページをコピーして新しいページを作成します。">複製</a>&nbsp;&nbsp;
         <a href="<?php echo $link_rename ?>" title="ページ名を変更します。パスワードが必要です。">名称変更</a>&nbsp;&nbsp;
<?php   if ($is_read and $function_freeze) { ?>
<?php     if ($is_freeze) { ?>
<a href="<?php echo $link_unfreeze ?>" title="このページの編集を禁止します。パスワードが必要です。">凍結解除</a>&nbsp;&nbsp;
<?php     } else { ?>
<a href="<?php echo $link_freeze ?>" title="このページの編集を許可します。パスワードが必要です。">凍結</a>&nbsp;&nbsp;
<?php     } ?>
<?php   } ?>

<?php   if ((bool)ini_get('file_uploads')) { ?>
<a href="<?php echo $link_upload ?>" title="このページにファイルをアップロードします。#ref(ファイル名)でページに貼り付けられます。">アップロード</a>&nbsp;&nbsp;
<a href="<?php echo $script ?>?plugin=attach&amp;pcmd=list&amp;refer=<?php echo $r_page ?>" title="添付ファイルの一覧を表示します。">添付ファイル一覧</a>&nbsp;&nbsp;
<?php   } ?>
<?php   if ($do_backup) { ?>
<a href="<?php echo $link_backup ?>" title="このページのバックアップを表示します。">バックアップ</a>&nbsp;&nbsp;
<?php   } ?>
		</div>
		<div class="max"></div>
	</nav>
</footer>

<!-- BEGIN-CONTENT -->
<main style="width: 75%; margin: auto;">
	<div class="large-padding"></div>
<!-- ----------- -->

<?php echo $body ?>

<?php if ($notes) { ?>
  <div class="notefoot">
    <p>
      <?php echo $notes ?>
    </p>
  </div>
<?php } ?>

<!-- ----------- -->
<div class="large-padding"></div>
</main>
<!-- END-CONTENT -->

<!-- BEGIN-FOOTER -->
<footer class="m l">
	<nav style="width: 75%; margin: auto;">
		<button class="transparent small-round">
			<img class="responsive" style="margin: auto;" src="/img/icons/suika.png">
		</button>
		<span>&copy; 2001-2024 Keiichi Tabata.</span>
		<div class="max"></div>
		<button data-ui="#dialogue_about" class="button transparent small-round">このサイトについて</button>
		<a href="https://github.com/suika2engine/suika2" class="button transparent small-round">GitHub</a>
	</nav>
</footer>
<footer class="s">
	<nav>
		<span>&copy; 2001-2024 Keiichi Tabata.</span>
		<div class="max"></div>
		<button data-ui="#dialogue_about" class="button transparent small-round">このサイトについて</button>
	</nav>
</footer>
<dialog id="dialogue_about" class="small-round">
	<h5>suika2.com</h5>
	<p class="large-text large-line">
		suika2.comはノベル制作ツールSuika2の公式サイトです。
	</p>
	<p class="large-text large-line">Webデザイン: Kyou</p>
	<nav class="left-align no-space">
		<button data-ui="#dialogue_about" class="button secondary transparent small-round">閉じる</button>
	</nav>
</dialog>
<script src="/js/dm.js" defer></script>
<!-- END-FOOTER -->

</body>
</html>
