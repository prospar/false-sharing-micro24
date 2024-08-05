(
 (nil . (
         (fill-column . 79)
         (indent-tabs-mode . nil)
         (lsp-file-watch-ignored-directories . ("/\\.git$" "/\\.clangd$"
                                                "/\\.cache$"))

         (eval . (add-hook 'lsp-managed-mode-hook
                           (lambda ()
                             (when (derived-mode-p 'markdown-mode)
                               (setq sb/flycheck-local-checkers
                                     '((lsp . ((next-checkers
                                                . (markdown-markdownlint-cli))))))
                               ;; (flycheck-add-next-checker
                               ;; 'markdown-markdownlint-cli 'grammarly)
                               )

                             ;; (when (derived-mode-p 'gfm-mode)
                             ;;   (setq sb/flycheck-local-checkers
                             ;;         '((lsp . ((next-checkers . (markdown-markdownlint-cli))))))
                             ;;   (flycheck-add-next-checker 'markdown-markdownlint-cli 'grammarly))

                             (when (derived-mode-p 'sh-mode)
                               (setq sb/flycheck-local-checkers
                                     '((lsp . ((next-checkers
                                                . (sh-shellcheck)))))))

                             (when (derived-mode-p 'yaml-mode)
                               (setq sb/flycheck-local-checkers
                                     '((lsp . ((next-checkers
                                                . (yaml-yamllint)))))))

                             (when (derived-mode-p 'json-mode)
                               (setq sb/flycheck-local-checkers
                                     '((lsp . ((next-checkers
                                                . (json-jsonlint)))))))

                             (when (derived-mode-p 'python-mode)
                               (setq sb/flycheck-local-checkers
                                     '((lsp . ((next-checkers
                                                . (python-pylint)))))))

                             (when (derived-mode-p 'c++-mode)
                               (setq sb/flycheck-local-checkers
                                     '((lsp . ((next-checkers
                                                . (c/c++-cppcheck)))))))

                             (when (derived-mode-p 'html-mode)
                               (setq sb/flycheck-local-checkers
                                     '((lsp . ((next-checkers
                                                . (html-tidy)))))))

                             (when (derived-mode-p 'xml-mode)
                               (setq sb/flycheck-local-checkers
                                     '((lsp . ((next-checkers
                                                . (xml-xmllint)))))))

                             ;; Chktex can report lot of errors
                             ;; (when (derived-mode-p 'latex-mode)
                             ;;   (setq sb/flycheck-local-checkers
                             ;;         '((lsp . ((next-checkers . (tex-chktex)))))))
                             )))
         ))

 ;; TODO: This does not seem to work
 (dired-mode . (
                (dired-omit-files . "\\`[.]?#\\|\\`[.][.]?\\'\\|\\.git\\'|\\.cache\\'")
                ))

 (c++-mode . (
              (load-file "./util/emacs/m5-c-style.el")
              (eval ignore-errors (require 'm5-c-style))
              (c-default-style . "m5")
              (c-set-style     . "m5")

              (flycheck-gcc-language-standard   . "c++11")
              (flycheck-clang-language-standard . "c++11")
              (flycheck-clang-tidy-build-path   . ".")

              (eval . (progn
                        (let (
                              (clang-args (list
                                           "-std=c++11"
                                           (concat "-I" (expand-file-name "src" (projectile-project-root)))
                                           (concat "-I" (expand-file-name "src/mem/ruby/structures" (projectile-project-root)))
                                           (concat "-I" (expand-file-name "src/mem" (projectile-project-root)))
                                           (concat "-I" (expand-file-name "src/sim" (projectile-project-root)))
                                           (concat "-I" (expand-file-name "build/X86_MESI_Two_Level_Extended" (projectile-project-root)))
                                           (concat "-I" (expand-file-name "/build/X86_MESI_Two_Level_Extended/params" (projectile-project-root)))
                                           ))
                              (include-path (list
                                             (expand-file-name "src" (projectile-project-root))
                                             (expand-file-name "src/mem/ruby/structures" (projectile-project-root))
                                             (expand-file-name "src/mem" (projectile-project-root))
                                             (expand-file-name "src/sim" (projectile-project-root))
                                             (expand-file-name "build/X86_MESI_Two_Level_Extended" (projectile-project-root))
                                             (expand-file-name "build/X86_MESI_Two_Level_Extended/params" (projectile-project-root))
                                             )))
                          (setq-local company-clang-arguments        clang-args
                                      flycheck-clang-args            clang-args
                                      flycheck-clang-include-path    include-path
                                      flycheck-gcc-args              clang-args
                                      flycheck-gcc-include-path      include-path
                                      flycheck-cppcheck-include-path include-path))
                        (add-hook 'before-save-hook #'lsp-format-buffer nil t)))

              (lsp-clients-clangd-args . ("-j=4"
                                          "--all-scopes-completion"
                                          "--background-index"
                                          "--clang-tidy"
                                          "--completion-style=detailed"
                                          "--cross-file-rename"
                                          "--fallback-style=LLVM"
                                          "--header-insertion=never"
                                          "--header-insertion-decorators"
                                          "--log=error"
                                          "--malloc-trim"
                                          "--pch-storage=memory"
                                          "--pretty"
                                          "--compile-commands-dir=."))
              ))

 (python-mode . (
                 (flycheck-pylintrc . "setup.cfg")
                 (eval . (let (
                               (paths
                                (vconcat (list
                                          "./src/python"
                                          "ext/ply"
                                          "src/sim"
                                          ))
                                ))
                           (setq lsp-pyright-extra-paths paths)
                           ))
                 (eval . (setenv "PYTHONPATH"
                                 (concat
                                  "src/python:ext/ply:src/sim:"
                                  (getenv "PYTHONPATH"))))
                 ;; NOTE: Update the path
                 (pyvenv-activate  . "/data/swarnendu/virtualenvs/gem5-python2-venv")
                 (py-isort-options . '("--settings-path=setup.cfg"))
                 ))
 )

;; Local Variables:
;; eval: (flycheck-mode -1)
;; End:
