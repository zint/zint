#/bin/sh

rm -r ./HTML
pandoc -o manual.html manual.pmd
python3 zint_org_uk.py
rm ./chapter6.0.html
mkdir HTML
mv chapter*.html ./HTML
mv appendix*.html ./HTML
cd ./HTML
