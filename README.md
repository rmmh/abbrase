#Abbrase#

Abbrase is an abbreviated passphrase generator.

Creating secure passwords is easy. Remembering them is hard. [Pwgen](http://sourceforge.net/projects/pwgen/) makes them memorable though prounouncability. XKCD [suggests](http://xkcd.com/936/) using a series of random common words, but memorizing series of unrelated words can be difficult, and typing long phrases can be tedious.

Abbrase is an experiment with generating probable phrases using markov chains, and abbreviating each word to the first few letters. This strikes a balance between excessive password length and excessive mnemonic length.

The markov models are generated with data from Google NGrams [dataset](http://storage.googleapis.com/books/ngrams/books/datasetsv2.html), used under the [Creative Commons Attribution 3.0 Unported License](http://creativecommons.org/licenses/by/3.0/). 

##Sample output##

(don't actually use any of these passwords!)

    Generating 32 passwords with 50 bits of entropy
    Password           Phrase
    ---------------    --------------------
    fibpsygotetemol    fibres psychology got eternal molecules
    crohismacwrosiz    cross his machine wrong size
    haswrigooalsarm    has written good also army
    livfoctelduerow    living focus telescope due row
    achtexmeaimiela    achieved text means imitation elaborate
    poiatmengdocpea    point atmospheric engine doctor pearson
    phosinwayladpha    photographs since way laden phagocytes
    henraptopsawgif    henry rapidly top saw gifford
    sednaravameaago    sed narrative available means agony
    roomecgammedgoo    room mechanical game medical good
    iniglouncmomkey    initial gloomy uncomfortable moment key
    taswirhalpetgue    task wire hall peter guessed
    nutdauliemesaba    nutrition daughter lie messenians abandoned
    vanashairnumedi    van ashamed air number edited
    groiniyarcampec    group initial yards came peculiarly
    putmetvilburbor    put methodist village burnt borax
    iniporadaneradh    initial portion adapted nerves adhere
    liqpriproreapeo    liquid principle process reason people
    nodnotmanpetedi    nodded not man peters edition
    negfaslawelsbec    negative fashion law else because
    nosyesroosoiask    nose yes room soissons asked
    somextdiestumea    some extent die study means
    vietumunrenapop    view tumultuous unrest enable popular
    phyeigdolrejutt    physical eight dollars rejected utterly
    somtoptexguibri    some top text guides bring
    beyhabtoldulbal    beyond habit tolerably dull ball
    timsirskyeldske    time sir sky elder sketch
    bigownupoavolak    big own upon avon lake
    phinowdirunalac    philosophy now direction unavoidable lack
    darexppotegggat    dark experience potential eggs gathered
    floalibadwhyour    flow alike bad why our
    darmanfirpopnoi    dark man first popular noise

