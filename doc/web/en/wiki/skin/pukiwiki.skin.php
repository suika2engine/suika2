<!DOCTYPE html>
<html lang="en-US">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width">

  <title>Suika2 Wiki | <?php echo $title ?></title>
  <meta name="description" content="Suika2 User's Wiki">
  <meta name="theme-color" content="#eb8334">

  <!-- Bootstrap -->
  <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/bootstrap-icons@1.10.5/font/bootstrap-icons.css">
  <link rel="stylesheet" href="/css/bootstrap.css">
  <link rel="stylesheet" href="/css/adjustments.css">
  <script src="/js/bootstrap.bundle.min.js"></script>

  <?php echo $head_tag ?>

  <!-- INSERT-GTAG-HERE -->
</head>
<body>

<!-- BEGIN-EN-HEADER -->
<nav class="navbar navbar-expand-lg bg-dark nav-block" data-bs-theme="dark">
	<div class="container">
		<a class="navbar-brand d-none d-md-block" href="/en/"><img height="100" src="/img/icons/suika.png" alt="suika2"></a>
		<button class="navbar-toggler" type="button" data-bs-toggle="collapse" data-bs-target="#navbarSupportedContent" aria-controls="navbarSupportedContent" aria-expanded="false" aria-label="Toggle navigation">
      <span class="navbar-toggler-icon"></span>
		</button>
		<div class="collapse navbar-collapse" id="navbarSupportedContent">
      <ul class="navbar-nav me-auto mb-2 mb-lg-0">
        <li class="nav-item"><a class="nav-link" aria-current="page" href="/en/">Home</a></li>
        <li class="nav-item"><a class="nav-link" aria-current="page" href="/en/dl/">Downloads</a></li>
				<li class="nav-item dropdown" aria-current="page">
					<a class="nav-link dropdown-toggle" href="#" id="navbarDarkDropdownMenuLink" role="button" data-bs-toggle="dropdown" aria-expanded="false">
						Document
					</a>
					<ul class="dropdown-menu dropdown-menu-dark" aria-labelledby="navbarDarkDropdownMenuLink">
						<li><a class="dropdown-item" href="/en/doc/">Index</a></li>
						<li><a class="dropdown-item" href="/en/doc/tutorial.html">Tutorial</a></li>
						<li><a class="dropdown-item" href="/en/doc/reference.html">Command Reference</a></li>
						<li><a class="dropdown-item" href="/en/doc/title.html">Title Creation</a></li>
						<li><a class="dropdown-item" href="/en/doc/font.html">Font Change</a></li>
						<li><a class="dropdown-item" href="/en/doc/wms.html">WMS Usage</a></li>
						<li><a class="dropdown-item" href="/en/doc/faq.html">FAQ</a></li>
					</ul>
				</li>
        <li class="nav-item"><a class="nav-link" href="/en/wiki/">Wiki</a></li>
        <li class="nav-item"><a class="nav-link" href="/en/works/">Works</a></li>
        <li class="nav-item"><a class="nav-link" href="/">Japanese</a></li>
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
<!-- END-EN-HEADER -->

<!-- Title Strip -->
<div class="block">
  <div class="container">
   <div class="row">
     <div class="col-md">
			 <a name="navigator"></a>
       <h1><?php echo $title ?> - Suika2 Wiki</h1>
	     <p>
         <a href="/wiki/">Wiki Top</a>&nbsp;&nbsp;
         <a href="<?php echo $link_edit ?>" title="Edit this page.">Edit</a>&nbsp;&nbsp;
         <a href="<?php echo $script ?>?plugin=newpage" title="Create a new page.">New</a>&nbsp;&nbsp;
         <a href="<?php echo $link_template ?>" title="Dupulicate this page.">Duplicate</a>&nbsp;&nbsp;
         <a href="<?php echo $link_rename ?>" title="Rename this page.">Rename</a>&nbsp;&nbsp;

<?php   if ($is_read and $function_freeze) { ?>
<?php     if ($is_freeze) { ?>
<a href="<?php echo $link_unfreeze ?>" title="Unfreeze this page.">Unfreeze</a>
<?php     } else { ?>
<a href="<?php echo $link_freeze ?>" title="Freeze this page.">Freeze</a>
<?php     } ?>
<?php   } ?>

<?php   if ((bool)ini_get('file_uploads')) { ?>
<a href="<?php echo $link_upload ?>" title="Upload a file to this page. Use with #ref(filename)">Upload</a>&nbsp;&nbsp;
<a href="<?php echo $script ?>?plugin=attach&amp;pcmd=list&amp;refer=<?php echo $r_page ?>" title="List attached files.">Attached</a>&nbsp;&nbsp;
<?php   } ?>
<?php   if ($do_backup) { ?>
<a href="<?php echo $link_backup ?>" title="Show backups of this page.">Backup</a>&nbsp;&nbsp;
<?php   } ?>

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

<!-- BEGIN-EN-FOOTER -->
<footer id="footer" class="container-fluid nav-block bg-dark" data-bs-theme="dark">
  <div class="container">
		<div class="row">
			<div class="col-lg-6 col-md-12 mb-4 mb-md-0">
				<h5>Suika2 Visual Novel Game Engine</h5>
				<p>An open-source, cross-platform visual novel engine for beginners and seasoned writers, amateurs and professionals.</p>
			</div>
			<div class="col-lg-6 col-md-12 mb-4 mb-md-0">
				<h5>Links</h5>
				<ul class="list-unstyled mb-0">
					<li><a href="https://github.com/suika2engine">GitHub</a></li>
					<li><a href="https://luxion.jp/">Luxion.jp</a></li>
				</ul>
			</div>
		</div>
	</div>
	<div class="text-center mt-5 ">Copyright &copy; 2023 <a href="https://luxion.jp">Keiichi Tabata</a> and <a href="https://github.com/suika2engine">The Suika2 Development Team</a>. All rights reserved.</div>
</footer>
<script src="/css/darkmode.js"></script>
<!-- END-EN-FOOTER -->

</body>
</html>
