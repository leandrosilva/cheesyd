# cheesyd

Daemon to convert HTML to PDF using the mighty wkhtmltopdf C bindings.

### Status

It is a stub project yet. But it's gonna change soon.

### Why?

I've being working with wkhtmltopdf for almost 2 years now in my daily job and lately I got very interested in its C bindings, spending many of my spare time digging into its source code and writing a few pet codes in Go and C instead of being out drinking beer and stuff. I mean, I mean...

Well, anyway, I thought the progression is to write some half decent code and put it out there. So that's why.

### Dependencies & running env

If you look at the Dockerfile you're gonna see that it depends on jessie-wkhtmltopdf. No worries, I did it and you can get [here](https://github.com/leandrosilva/jessie-wkhtmltopdf).

### Get it now

    git clone https://github.com/leandrosilva/cheesyd.git
    cd cheesyd
    docker build -f Dockerfile -t cheesyd .
    docker run -it -v $PWD:/source_code --name cheesyd_test cheesyd
    cheesyd
    ls -la

Or you can build and run it on your own host machine, by typing from cheesy directory:

    make build
    make run
    ls -la

See? Five .pdf files done.

Ok, ok... It's not that impressing... Meh.