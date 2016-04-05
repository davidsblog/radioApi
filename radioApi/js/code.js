function heartbeat(whenDone)
{
    $.ajax({
        url: "counter.api",
        method: "post",
        data: { counter : 0 },
        dataType: "json"
    }).done(function(data)
    {
        whenDone(data);
    });
}

function playsong(song, whenDone)
{
    $.ajax({
        url: "song/"+song,
        type: "get",
    }).done(function(data)
    {
        whenDone(data);
    });
}

function command(command, whenDone)
{
    $.ajax({
        url: "cmd.api",
        method: "post",
        data: { cmd : command }
    }).done(function(data)
    {
        whenDone(data);
    });
}

function getVideos(whenDone)
{
    $.ajax({
        url: "vidlist",
        method: "post",
        contentType: "application/json; charset=utf-8",
        dataType: "json",
    }).done(function(data)
    {
        whenDone(data);
    });
}

function showDebugInfo(response, element)
{
    if (response.debug)
    {
        element.text(JSON.stringify(response));
    }
}
