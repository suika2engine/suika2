Module['preRun'] = function () {
    FS.createPreloadedFile(
	'/', 'data01.arc',

	// Change here
	'/html/data01.arc',

	true, false);
};
