# shell script for creating graphs of the components and the contracts

dot -Tps2 pr_0.gv -o pr_0.ps | ps2pdf pr_0.ps
dot -Tps2 pr_1.gv -o pr_1.ps | ps2pdf pr_1.ps
dot -Tps2 guarantee_0.gv -o guarantee_0.ps | ps2pdf guarantee_0.ps
dot -Tps2 guarantee_1.gv -o guarantee_1.ps | ps2pdf guarantee_1.ps

open pr_0.pdf
open pr_1.pdf
open guarantee_0.pdf
open guarantee_1.pdf
