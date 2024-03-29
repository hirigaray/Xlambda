(use-modules (guix git)
             (guix packages)
             (guix licenses)
             (guix build-system gnu)
             (gnu packages)
             (gnu packages commencement)
             (gnu packages xorg)
             (gnu packages guile)
             (gnu packages pkg-config))

(package
  (name "Xlambda")
  (version "0.0.1")
  (source (git-checkout (url (dirname (current-filename)))))
  (build-system gnu-build-system)
  (native-inputs (list gcc-toolchain pkg-config libxcb))
  (inputs (list guile-3.0-latest))
  (synopsis "An X window manager that is extensible in Guile Scheme")
  (description "Xlambda is a window manager that is extensible in Guile Scheme.")
  (home-page "https://github.com/kori/Xlambda")
  (license gpl3+))
