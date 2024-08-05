(
  (nil
    .
    ((projectile-project-compilation-dir . ".")
      (projectile-project-compilation-cmd . "cmake -S . -B build; cmake --build build;")
      (compile-command . "cmake -S . -B build; cmake --build build; ")
      (eval .
        (add-hook
          'lsp-managed-mode-hook
          (lambda () (add-to-list 'lsp-file-watch-ignored-directories "build"))))))

  (c++-ts-mode
    .
    ((clang-format-style . "file")
      (flycheck-gcc-language-standard . "c++17")
      (flycheck-clang-language-standard . "c++17")
      (flycheck-clang-tidy-build-path . ".")
      (flycheck-gcc-openmp . t)

      (eval .
        (progn
          (let
            (
              (clang-args
                (list
                  "-std=c++11"
                  ;; Using `(projectile-project-root)' will use the full remote path
                  (concat "-I" (expand-file-name "."))))
              (include-path (list (expand-file-name "."))))
            (setq-local
              company-clang-arguments clang-args
              flycheck-clang-args clang-args
              flycheck-gcc-args clang-args
              flycheck-gcc-include-path include-path
              flycheck-clang-include-path include-path
              flycheck-cppcheck-include-path include-path))))

      (eval .
        (add-hook
          'lsp-managed-mode-hook
          (lambda ()
            (add-hook 'before-save-hook #'lsp-format-buffer nil t)
            (let ((compdir (file-name-directory buffer-file-name)))
              (add-to-list 'lsp-clients-clangd-args
                (concat
                  "--compile-commands-dir="
                  (expand-file-name "/build" (projectile-project-root)))
                'append)))))

      (eval .
        (add-hook
          'eglot-managed-mode-hook
          (lambda () (add-hook 'before-save-hook #'eglot-format-buffer nil t))))))

  (c++-mode
    .
    ((clang-format-style . "file")
      (flycheck-gcc-language-standard . "c++17")
      (flycheck-clang-language-standard . "c++17")
      (flycheck-clang-tidy-build-path . ".")
      (flycheck-gcc-openmp . t)

      (eval .
        (progn
          (let
            (
              (clang-args
                (list
                  "-std=c++11"
                  ;; Using `(projectile-project-root)' will use the full remote path
                  (concat "-I" (expand-file-name "."))))
              (include-path (list (expand-file-name "."))))
            (setq-local
              company-clang-arguments clang-args
              flycheck-clang-args clang-args
              flycheck-gcc-args clang-args
              flycheck-gcc-include-path include-path
              flycheck-clang-include-path include-path
              flycheck-cppcheck-include-path include-path))))

      (eval .
        (add-hook
          'lsp-managed-mode-hook
          (lambda ()
            (add-hook 'before-save-hook #'lsp-format-buffer nil t)
            (let ((compdir (file-name-directory buffer-file-name)))
              (add-to-list 'lsp-clients-clangd-args
                (concat
                  "--compile-commands-dir="
                  (expand-file-name "/build" (projectile-project-root)))
                'append)))))

      (eval .
        (add-hook
          'eglot-managed-mode-hook
          (lambda () (add-hook 'before-save-hook #'eglot-format-buffer nil t))))))

  (cmake-ts-mode
    .
    (
      (eval .
        (add-hook
          'lsp-managed-mode-hook
          (lambda () (add-hook 'before-save-hook #'lsp-format-buffer nil t))))

      (eval .
        (add-hook
          'eglot-managed-mode-hook
          (lambda () (add-hook 'before-save-hook #'eglot-format-buffer nil t))))))

  (cmake-mode
    .
    (
      (eval .
        (add-hook
          'lsp-managed-mode-hook
          (lambda () (add-hook 'before-save-hook #'lsp-format-buffer nil t))))

      (eval .
        (add-hook
          'eglot-managed-mode-hook
          (lambda () (add-hook 'before-save-hook #'eglot-format-buffer nil t))))))

  (yaml-ts-mode
    .
    (
      (eval .
        (add-hook
          'lsp-managed-mode-hook
          (lambda () (add-hook 'before-save-hook #'lsp-format-buffer nil t))))

      (eval .
        (add-hook
          'eglot-managed-mode-hook
          (lambda () (add-hook 'before-save-hook #'eglot-format-buffer nil t)))))))

;; Local Variables:
;; eval: (flycheck-mode -1)
;; End:
