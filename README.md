# cheesyd

Daemon to convert HTML to PDF using the mighty **wkhtmltopdf** C bindings.

### Status

It is not a complete work yet but it actually can prove that the idea is valid already. It does the happy path in a elementar way and a few error ones too. So far so good.

### Why?

I've being working with wkhtmltopdf for almost 2 years now in my daily job and lately I got very interested in its C bindings, spending many of my spare time digging into its source code and writing a few pet codes in Go and C instead of being out drinking beer and stuff. I mean, I mean...

Well, anyway, I thought the progression is to write some half decent code and put it out there. So that's why.

### Dependencies & running env

If you look at the Dockerfile you're gonna see that it depends on debian-wkhtmltopdf. No worries, I did it and you can get [here](https://github.com/leandrosilva/debian-wkhtmltopdf).

### Get and run it now

    git clone https://github.com/leandrosilva/cheesyd.git
    cd cheesyd
    docker build -f Dockerfile -t cheesyd .
    docker run -it -v $PWD:/source_code --name cheesyd_test cheesyd
    cheesyd

Or you can build and run it on your own host machine, by typing from cheesy directory:

    make build
    make run

Now it's gonna connect to **redis** on localhost:6379 and dequeue jobs from **cheesyd:queue:job_request**, whose which should provide request data on **cheesyd:job:{job_id}** hash as:

    payload = {"page":"sample1.html", "windowStatus":"ready"}
    status  = request

When it get the job done, it's gonna put the result on the same job hash as:

    result = <base64 encoded PDF content>
    status = done

There you go.
