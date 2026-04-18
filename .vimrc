" vim 快捷键
nnoremap <C-Up>   :m-2<CR>==
nnoremap <C-Down> :m+<CR>==

" vim from
set nowrap
set tabstop=4
set shiftwidth=4
set number
set fileencodings=utf-8,gbk,gb2312,gb18030

" ctags set
set tags=tags;
set path+=..**
set path+=/home/leige/work/buildroot-rk3588/kernel/include/**
set complete=.,w,b,k

call plug#begin('~/.vim/plugged')

Plug 'z0mbix/vim-shfmt'
Plug 'dense-analysis/ale'
Plug 'ludovicchabant/vim-gutentags'
Plug 'rhysd/vim-clang-format'
" Plug 'neoclide/coc.nvim', {'branch': 'release'}

call plug#end()

" 跳转错误列表的快捷键（可选）
nmap <silent> <C-j> <Plug>(ale_next_wrap)
nmap <silent> <C-k> <Plug>(ale_previous_wrap)

" clang 配置 start
let g:clang_format#command = 'clang-format'
let g:clang_format#detect_style_file = 1
let g:clang_format#auto_format = 0
let g:clang_format#code_style = 'llvm'

" ALE 配置 start
" 设置 C 和 C++ 的 linter
let g:ale_linters = {
\   'c': ['clang', 'gcc', 'cppcheck'],
\   'cpp': ['clang', 'gcc', 'clangtidy', 'cppcheck'],
\}

" 设置自动修复（可选），比如使用 clang-format 格式化
let g:ale_fixers = {
\   'c': ['clang-format'],
\   'cpp': ['clang-format'],
\}

" 开启实时检查
let g:ale_lint_on_text_changed = 'always'
let g:ale_lint_on_insert_leave = 1
let g:ale_lint_on_enter = 1

" 显示错误信息
let g:ale_sign_error = '✗'
let g:ale_sign_warning = '⚠'
let g:ale_echo_msg_error_str = 'E'
let g:ale_echo_msg_warning_str = 'W'
let g:ale_echo_msg_format = '[%linter%] %s [%severity%]'
let g:ale_c_cppcheck_options = '--suppress=danglingLifetime'

" shell 格式化插件
let g:shfmt_optimize = '1'
let g:shfmt_extra_args = '-i 4'   
autocmd BufWritePost *.sh,*.bash execute 'Shfmt'

