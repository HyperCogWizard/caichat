;; guix-package.scm - GNU Guix package definition for AIChat
(define-module (gnu packages aichat)
  #:use-module (guix packages)
  #:use-module (guix download)
  #:use-module (guix git-download)
  #:use-module (guix build-system cargo)
  #:use-module (guix build-system cmake)
  #:use-module ((guix licenses) #:prefix license:)
  #:use-module (gnu packages)
  #:use-module (gnu packages crates-io)
  #:use-module (gnu packages crates-graphics)
  #:use-module (gnu packages crates-web)
  #:use-module (gnu packages curl)
  #:use-module (gnu packages tls)
  #:use-module (gnu packages pkg-config))

(define-public aichat
  (package
    (name "aichat")
    (version "0.29.0")
    (source
     (origin
       (method git-fetch)
       (uri (git-reference
             (url "https://github.com/sigoden/aichat")
             (commit (string-append "v" version))))
       (file-name (git-file-name name version))
       (sha256
        (base32
         "0000000000000000000000000000000000000000000000000000")))) ; Update with actual hash
    (build-system cargo-build-system)
    (arguments
     `(#:cargo-inputs
       (("rust-anyhow" ,rust-anyhow-1)
        ("rust-async-trait" ,rust-async-trait-0.1)
        ("rust-base64" ,rust-base64-0.22)
        ("rust-bytes" ,rust-bytes-1)
        ("rust-chrono" ,rust-chrono-0.4)
        ("rust-clap" ,rust-clap-4)
        ("rust-crossterm" ,rust-crossterm-0.28)
        ("rust-dirs" ,rust-dirs-5)
        ("rust-fancy-regex" ,rust-fancy-regex-0.14)
        ("rust-futures-util" ,rust-futures-util-0.3)
        ("rust-inquire" ,rust-inquire-0.7)
        ("rust-is-terminal" ,rust-is-terminal-0.4)
        ("rust-log" ,rust-log-0.4)
        ("rust-nu-ansi-term" ,rust-nu-ansi-term-0.50)
        ("rust-parking-lot" ,rust-parking-lot-0.12)
        ("rust-reedline" ,rust-reedline-0.39)
        ("rust-reqwest" ,rust-reqwest-0.12)
        ("rust-serde" ,rust-serde-1)
        ("rust-serde-json" ,rust-serde-json-1)
        ("rust-serde-yaml" ,rust-serde-yaml-0.9)
        ("rust-sha2" ,rust-sha2-0.10)
        ("rust-simplelog" ,rust-simplelog-0.12)
        ("rust-syntect" ,rust-syntect-5)
        ("rust-textwrap" ,rust-textwrap-0.16)
        ("rust-tokio" ,rust-tokio-1)
        ("rust-tokio-graceful" ,rust-tokio-graceful-0.2)
        ("rust-tokio-stream" ,rust-tokio-stream-0.1)
        ("rust-unicode-width" ,rust-unicode-width-0.2))
       #:cargo-development-inputs
       (("rust-pretty-assertions" ,rust-pretty-assertions-1)
        ("rust-rand" ,rust-rand-0.9))))
    (native-inputs
     (list pkg-config))
    (inputs
     (list curl openssl-3.0))
    (synopsis "All-in-one LLM CLI tool")
    (description
     "AIChat is a comprehensive command-line interface tool for interacting
with multiple Large Language Model (LLM) providers.  It features shell
assistance, REPL mode, RAG (Retrieval-Augmented Generation), AI tools and
agents, multi-form input support, and integration with over 20 LLM providers
including OpenAI, Claude, Gemini, and many others.")
    (home-page "https://github.com/sigoden/aichat")
    (license (list license:expat license:asl2.0))))

;; Optional: AIChat with OpenCog integration
(define-public aichat-opencog
  (package
    (inherit aichat)
    (name "aichat-opencog")
    (arguments
     `(#:phases
       (modify-phases %standard-phases
         (add-after 'install 'build-opencog-extension
           (lambda* (#:key outputs #:allow-other-keys)
             (let ((out (assoc-ref outputs "out")))
               ;; Build C++ OpenCog integration
               (invoke "cmake" "-B" "build-opencog" 
                      "-DCMAKE_INSTALL_PREFIX=" out)
               (invoke "make" "-C" "build-opencog" "install")))))))
    (native-inputs
     (modify-inputs (package-native-inputs aichat)
       (prepend cmake)))
    (inputs
     (modify-inputs (package-inputs aichat)
       (prepend ;; Add OpenCog dependencies when available
        )))
    (synopsis "AIChat with OpenCog integration")
    (description
     (string-append (package-description aichat)
                    "  This variant includes OpenCog AtomSpace integration
for advanced cognitive architectures and symbolic AI capabilities."))))