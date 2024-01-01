<!DOCTYPE html>
<html lang="en-US">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width">

  <title>Suika2 Wiki | <?php echo $title ?></title>
  <meta name="description" content="Suika2 User's Wiki">
  <meta name="theme-color" content="#eb8334">

	<link href="https://cdn.jsdelivr.net/npm/beercss@3.4.11/dist/cdn/beer.min.css" rel="stylesheet">
	<script type="module" src="https://cdn.jsdelivr.net/npm/beercss@3.4.11/dist/cdn/beer.min.js" defer></script>
	<style>
		.w-25 {width:25%!important;}
		ul, ol { margin-left: 2em; }
    a { text-decoration: underline; color: blue; }
    h1 { font-size:2.0em!important; }
    h2 { font-size:1.5em!important; }
    h3, h4, h5 { font-size: 1.3em!important; }
		table, tr, td { border: solid 1px; }
		table { margin: 2em; }
	</style>
	<link href="https://suika2.com/css/game.css" rel="stylesheet">

  <?php echo $head_tag ?>

  <!-- INSERT-GTAG-HERE -->
</head>
<body>

<!-- BEGIN-EN-HEADER -->
<footer class="m l">
  <nav style="width: 75%; margin: auto;">
    <button class="transparent small-round"><img class="responsive" style="margin: auto;" src="/img/icons/suika.png"></button>
    <a href="/en/" class="button transparent small-round"><i>home</i>Home</a>
    <a href="/en/dl/" class="button transparent small-round"><i>download</i>Downloads</a>
    <button data-ui="#menu_help" class="button transparent small-round"><i>help</i><span>Help</span><i>arrow_drop_down</i>
      <menu id="menu_help" class="no-wrap">
        <a href="/en/doc/">Index</a>
        <a href="/en/wiki/?Tutorial">Tutorial</a>
        <a href="/en/wiki/?Command+Reference">Command Reference</a>
        <a href="/en/doc/title.html">Title Creation (Under Translation)</a>
        <a href="/en/doc/font.html">Font Change (Under Translation)</a>
        <a href="/en/doc/wms.html">WMS Usage (Under Translation)</a>
        <a href="/en/doc/faq.html">FAQ</a>
        <a href="/en/wiki/">Wiki Top</a>
        <a href="/en/wiki/?cmd=search">Search</a>
      </menu>
    </button>
    <a href="/works/" class="button transparent small-round"><i>gamepad</i>Works</a>
    <a href="/" class="button transparent small-round">日本語</a>
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
    <a href="/en/" class="button transparent small-round"><i>home</i>Home</a>
    <a href="/en/dl/" class="button transparent small-round"><i>download</i>Downloads</a>
    <a href="/en/doc/" class="button transparent small-round"><i>help</i>Help</a>
    <a href="/en/works/" class="button transparent small-round"><i>gamepad</i>Works</a>
    <a href="/" class="button transparent small-round">Japanese</a>
    <div class="max"></div>
    <button id="btn_theme_mobile" class="square round extra secondary-container"><i>dark_mode</i></button>
  </nav>
</dialog>
<!-- END-EN-HEADER -->
<!-- END-EN-HEADER -->

<!-- Title Strip -->
<footer class="medium-padding m l">
	<nav style="width: 75%; margin: auto;">
		<div>
       <h1><?php echo $title ?> - Suika2 Wiki</h1>
	     <p>
         <a href="/en/wiki/">Wiki Top</a>&nbsp;&nbsp;
         <a href="<?php echo $link_edit ?>" title="Edit this page.">Edit</a>&nbsp;&nbsp;
         <a href="<?php echo $script ?>?plugin=newpage" title="Create a new page.">New</a>&nbsp;&nbsp;
         <a href="<?php echo $link_template ?>" title="Dupulicate this page.">Duplicate</a>&nbsp;&nbsp;
         <a href="<?php echo $link_rename ?>" title="Rename this page.">Rename</a>&nbsp;&nbsp;

<?php   if ($is_read and $function_freeze) { ?>
<?php     if ($is_freeze) { ?>
<a href="<?php echo $link_unfreeze ?>" title="Unfreeze this page.">Unfreeze</a>&nbsp;&nbsp;
<?php     } else { ?>
<a href="<?php echo $link_freeze ?>" title="Freeze this page.">Freeze</a>&nbsp;&nbsp;
<?php     } ?>
<?php   } ?>

<?php   if ((bool)ini_get('file_uploads')) { ?>
<a href="<?php echo $link_upload ?>" title="Upload a file to this page. Use with #ref(filename)">Upload</a>&nbsp;&nbsp;
<a href="<?php echo $script ?>?plugin=attach&amp;pcmd=list&amp;refer=<?php echo $r_page ?>" title="List attached files.">Attached</a>&nbsp;&nbsp;
<?php   } ?>
<?php   if ($do_backup) { ?>
<a href="<?php echo $link_backup ?>" title="Show backups of this page.">Backup</a>&nbsp;&nbsp;
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

<!-- BEGIN-EN-FOOTER -->
<footer class="m l">
	<nav style="width: 75%; margin: auto;">
		<button class="transparent small-round">
			<img class="responsive" style="margin: auto;" src="/img/icons/suika.png">
		</button>
		<span>&copy; 2001-2024 Keiichi Tabata.</span>
		<div class="max"></div>
		<button data-ui="#dialogue_about" class="button transparent small-round">About this site</button>
		<a href="https://github.com/suika2engine/suika2" class="button transparent small-round">GitHub</a>
	</nav>
</footer>
<footer class="s">
	<nav>
		<span>&copy; 2001-2024 Keiichi Tabata.</span>
		<div class="max"></div>
		<button data-ui="#dialogue_about" class="button transparent small-round">About this site</button>
	</nav>
</footer>
<dialog id="dialogue_about" class="small-round">
	<h5>suika2.com</h5>
	<p class="large-text large-line">
		suika2.com is a home of a visual novel creation tool, Suika2.
	</p>
	<p class="large-text large-line">Web Designed By: Kyou</p>
	<nav class="left-align no-space">
		<button data-ui="#dialogue_about" class="button secondary transparent small-round">Close</button>
	</nav>
</dialog>
<script src="/js/dm.js" defer></script>
<!-- END-EN-FOOTER -->

</body>
</html>
