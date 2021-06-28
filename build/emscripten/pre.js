Module['preRun'] = function () {
    FS.createPreloadedFile(
	'/', 'data01.arc',

	/////////////////
	// CHANGE HERE //
	/////////////////
	'/html/data01.arc',

	true, false);
};
