Module['preRun'].push(() => {
    FS.mkdir('/opentyrian')
    FS.mount(IDBFS, {}, '/opentyrian')
    FS.syncfs(true, (err) => {
        if (err) throw err;
        console.log('IDBFS Synced')
    })
});
