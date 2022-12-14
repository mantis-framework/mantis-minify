# Mantis Minify ππ¦

> Minifier for (currently) css, js, html and json

Features:
1. lightweight
2. fast (minify tailwind faster than β‘)
3. minimalist
4. zero dependencies
5. low (memory) footprint (in-string minification)
6. multi threading (native binary)
7. cross-platform

Native Binary Example Usage:
```
mantis-minify --dir assets/css/min/ --css assets/css/*.css
mantis-minify --output-path assets/bundled.min.js --js *.js
mantis-minify --html index.html
```

JS/WASM Example Usage:
```
node mantis-minify.js --output-path /pwd/bundled.min.css --css /pwd/*.css
node mantis-minify.js --js /pwd/*.js > bundled.min.js
node mantis-minify.js --html /pwd/index.html
```

Note: To use `mantis-minify.js` you will also need `mantis-minify.wasm`.

Also see - `mantis-minify --help`

> Mantis is a high-performance unopinionated framework for building the πΈοΈ (under development)

[Why Mantis?](https://theoatmeal.com/comics/mantis_shrimp)

Copyright (c) 2022-present [Nicholas Ham](https://n-ham.com).

Website: soon
