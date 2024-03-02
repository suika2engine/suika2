<!DOCTYPE html>
<html lang="ja-JP">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width">

  <title>Suika2 Wiki | <?php echo $title ?></title>
  <meta name="description" content="Suika2 ユーザ Wiki">
  <meta name="theme-color" content="#eb8334">

  <!-- Bootstrap -->
	<link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/bootstrap-icons@1.10.5/font/bootstrap-icons.css">
	<link rel="stylesheet" href="/css/bootstrap.css">
	<link rel="stylesheet" href="/css/adjustments.css">
	<script src="/js/bootstrap.bundle.min.js"></script>

  <?php echo $head_tag ?>

</head>
<body>

<!-- BEGIN-HEADER -->
<nav class="navbar navbar-expand-lg bg-dark nav-block" data-bs-theme="dark">
	<div class="container">
		<a class="navbar-brand d-none d-md-block" href="/"><img height="100" src="/img/icons/suika.png" alt="suika2"></a>
		<button class="navbar-toggler" type="button" data-bs-toggle="collapse" data-bs-target="#navbarSupportedContent" aria-controls="navbarSupportedContent" aria-expanded="false" aria-label="Toggle navigation">
			<span class="navbar-toggler-icon"></span>
		</button>
		<div class="collapse navbar-collapse" id="navbarSupportedContent">
			<ul class="navbar-nav me-auto mb-2 mb-lg-0">
				<li class="nav-item"><a class="nav-link" href="/">ホーム</a></li>
				<li class="nav-item dropdown" aria-current="page">
					<a class="nav-link dropdown-toggle" href="#" id="navbarDarkDropdownMenuLink" role="button" data-bs-toggle="dropdown" aria-expanded="false">
						ダウンロード
					</a>
					<ul class="dropdown-menu dropdown-menu-dark" aria-labelledby="navbarDarkDropdownMenuLink">
						<li><a class="dropdown-item" href="https://suika2.com/dl/">アプリ</a></li>
						<li><a class="dropdown-item" href="https://suika2.com/dl/material/">素材</a></li>
					</ul>
				</li>
				<li class="nav-item dropdown" aria-current="page">
					<a class="nav-link dropdown-toggle" href="#" id="navbarDarkDropdownMenuLink" role="button" data-bs-toggle="dropdown" aria-expanded="false">
						ドキュメント
					</a>
					<ul class="dropdown-menu dropdown-menu-dark" aria-labelledby="navbarDarkDropdownMenuLink">
						<li><a class="dropdown-item" href="https://suika2.com/wiki/?%E3%83%89%E3%82%AD%E3%83%A5%E3%83%A1%E3%83%B3%E3%83%88">ドキュメントトップ</a></li>
						<li><a class="dropdown-item" href="https://suika2.com/wiki/?%E3%83%81%E3%83%A5%E3%83%BC%E3%83%88%E3%83%AA%E3%82%A2%E3%83%AB">初級者向け: 最初のチュートリアル</a></li>
						<li><a class="dropdown-item" href="https://suika2.com/wiki/?%E3%82%BF%E3%82%A4%E3%83%88%E3%83%AB%E7%94%BB%E9%9D%A2%E3%81%AE%E4%BD%9C%E3%82%8A%E6%96%B9">中級者向け: GUIのチュートリアル</a></li>
						<li><a class="dropdown-item" href="https://suika2.com/wiki/?WMS%E3%81%AE%E4%BD%BF%E3%81%84%E6%96%B9">上級者向け: WMSのチュートリアル</a></li>
						<li><a class="dropdown-item" href="https://suika2.com/wiki/?%E3%82%B3%E3%83%9E%E3%83%B3%E3%83%89%E3%83%AA%E3%83%95%E3%82%A1%E3%83%AC%E3%83%B3%E3%82%B9">全開発者向け: コマンド一覧</a></li>
						<li><a class="dropdown-item" href="https://suika2.com/wiki/?%E3%82%B3%E3%83%B3%E3%83%95%E3%82%A3%E3%82%B0%E4%B8%80%E8%A6%A7">全開発者向け: コンフィグ一覧</a></li>
						<li><a class="dropdown-item" href="https://suika2.com/wiki/?%E3%82%88%E3%81%8F%E3%81%82%E3%82%8B%E8%B3%AA%E5%95%8F">よくある質問</a></li>
						<li><a class="dropdown-item" href="https://suika2.com/wiki/?cmd=search">ドキュメントを検索...</a></li>
					</ul>
				</li>
				<li class="nav-item dropdown" aria-current="page">
					<a class="nav-link dropdown-toggle" href="#" id="navbarDarkDropdownMenuLink" role="button" data-bs-toggle="dropdown" aria-expanded="false">
						開発
					</a>
					<ul class="dropdown-menu dropdown-menu-dark" aria-labelledby="navbarDarkDropdownMenuLink">
						<li><a class="dropdown-item" href="https://suika2.com/wiki/?%E3%83%90%E3%82%B0%E5%A0%B1%E5%91%8A%E3%81%AE%E4%B8%80%E8%A6%A7">バグ報告の一覧</a></li>
						<li><a class="dropdown-item" href="https://suika2.com/wiki/?%E6%A9%9F%E8%83%BD%E8%A6%81%E6%9C%9B%E3%81%AE%E4%B8%80%E8%A6%A7">機能要望の一覧</a></li>
					</ul>
				</li>
				<li class="nav-item"><a class="nav-link" href="/works/">利用作品</a></li>
				<li class="nav-item"><a class="nav-link" href="/en/">English</a></li>
				<li class="nav-item dropdown d-block d-md-none">
					<a class="btn btn-outline-success rounded-0 dropdown-toggle" href="#" role="button" data-bs-toggle="dropdown" aria-expanded="false"><i class="bi bi-moon"></i></a>
					<ul class="dropdown-menu dropdown-menu-end" aria-labelledby="bd-theme-text">
						<li><button type="button" class="dropdown-item d-flex align-items-center" data-bs-theme-value="light" aria-pressed="false">Light</button></li>
						<li><button type="button" class="dropdown-item d-flex align-items-center" data-bs-theme-value="dark" aria-pressed="false">Dark</button></li>
						<li><button type="button" class="dropdown-item d-flex align-items-center" data-bs-theme-value="auto" aria-pressed="true">Auto</button></li>
					</ul>
				</li>
			</ul>
			<li class="nav-item dropdown d-none d-md-block">
				<a class="btn btn-outline-success rounded-0 dropdown-toggle" href="#" role="button" data-bs-toggle="dropdown" aria-expanded="false"><i class="bi bi-moon"></i></a>
				<ul class="dropdown-menu dropdown-menu-end" aria-labelledby="bd-theme-text">
					<li><button type="button" class="dropdown-item d-flex align-items-center" data-bs-theme-value="light" aria-pressed="false">Light</button></li>
					<li><button type="button" class="dropdown-item d-flex align-items-center" data-bs-theme-value="dark" aria-pressed="false">Dark</button></li>
					<li><button type="button" class="dropdown-item d-flex align-items-center" data-bs-theme-value="auto" aria-pressed="true">Auto</button></li>
				</ul>
			</li>
		</div>
	</div>
</nav>
<!-- END-HEADER -->

<!-- Title Strip -->
<div class="block">
	<div class="container">
		<div class="row">
			<div class="col-md">
				<a name="navigator"></a>
				<h1><?php echo $title ?> - Suika2 Wiki</h1>
				<p>
					<a href="/wiki/">Wikiトップ</a>&nbsp;&nbsp;
					<a href="<?php echo $link_edit ?>" title="このページを編集します。">編集</a>&nbsp;&nbsp;
					<a href="<?php echo $script ?>?plugin=newpage" title="新しくページを作成します。">新規作成</a>&nbsp;&nbsp;
					<a href="<?php echo $link_template ?>" title="このページをコピーして新しいページを作成します。">複製</a>&nbsp;&nbsp;
					<a href="<?php echo $link_rename ?>" title="ページ名を変更します。パスワードが必要です。">名称変更</a>&nbsp;&nbsp;
<?php if ($is_read and $function_freeze) { ?>
<?php   if ($is_freeze) { ?>
					<a href="<?php echo $link_unfreeze ?>" title="このページの編集を禁止します。パスワードが必要です。">凍結解除</a>&nbsp;&nbsp;
<?php   } else { ?>
          <a href="<?php echo $link_freeze ?>" title="このページの編集を許可します。パスワードが必要です。">凍結</a>&nbsp;&nbsp;
<?php   } ?>
<?php } ?>

<?php if ((bool)ini_get('file_uploads')) { ?>
          <a href="<?php echo $link_upload ?>" title="このページにファイルをアップロードします。#ref(ファイル名)でページに貼り付けられます。">アップロード</a>&nbsp;&nbsp;
<a href="<?php echo $script ?>?plugin=attach&amp;pcmd=list&amp;refer=<?php echo $r_page ?>" title="添付ファイルの一覧を表示します。">添付ファイル一覧</a>&nbsp;&nbsp;
<?php   } ?>
<?php if ($do_backup) { ?>
          <a href="<?php echo $link_backup ?>" title="このページのバックアップを表示します。">バックアップ</a>&nbsp;&nbsp;
<?php } ?>

				</p>
			</div>
		</div>
	</div>
</div>

<!-- BEGIN-CONTENT -->
<div class="container">
	<div class="row">
		<div class="col-xs-12 col-sm-12 col-md-12">
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
    </div>
	</div>
</div>
<!-- END-CONTENT -->

<!-- BEGIN-FOOTER -->
<footer id="footer" class="container-fluid nav-block bg-dark" data-bs-theme="dark">
	<div class="container">
		<div class="row">
			<div class="col-lg-6 col-md-12 mb-4 mb-md-0">
				<h5>ノベルアプリ開発ツール Suika2</h5>
				<p>Suika2はノベルアプリ(スマートフォンのノベルゲーム)を制作するためのツールです。</p>
			</div>
			<div class="col-lg-6 col-md-12 mb-4 mb-md-0">
				<h5>リンク</h5>
				<ul class="list-unstyled mb-0">
					<li><a href="https://github.com/ktabata/suika2">GitHub</a></li>
				</ul>
			</div>
		</div>
	</div>
	<div class="text-center mt-5 ">
		Copyright &copy; 2001-2024, Keiichi Tabata. All rights reserved.
	</div>
</footer>
<script src="/js/darkmode.js"></script>
<!-- END-FOOTER -->

</body>
</html>
