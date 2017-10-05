<title>Particles</title>
</br>
<h1>Readme z particles sample od Nvidia</h1>
<a href="readme1.txt">readme1</a>
</br>
<h1>Opis wywoływania symulacji z parametrami</h1>
<a href="readme2.txt">readme2</a>
</br>
<h1>Koncepcja modyfikacji zwiększającej ilość różnych cząstek</h1>
<a href="readme3.txt">readme3</a>
</br>
<h2>License</h2>
Due to NVIDIA EULA for CUDA Samples we are obliged to state that: 
“This software contains source code
provided by NVIDIA Corporation.”


<h1>format save</h1>
<section>
w pliku podanym do opcji save zapisuje się następująco int (4 bajty) ilości cząstek
następnie co 10 iterację zapisywane są położenia cząstek, najpierw jest double (8 bajtów) ze znacznikiem czasu w sekundach
potem są pozycje cząsek, jest ich tyle ile podano na początku, każda cząstka opisana jest 4-ema float-ami (po 4 bajty),
pierwsze 3 oznaczają pozycję (x,y,z) 4 to 1 ze względu na to, że jest potrzebny do operacji macierzowych w znormalizowanej postaci
oznaczenie typu cząstki jest przechowycwane w 4 pozycji prędkości, która nie jest zapisywana do pliku.
żeby poznać typ cząstki trzeba zajżeć do pliku particleType.cfg, z którego się kożystało do symulacji
cząstki są zapisane w niezmienionej kolejności, w takiej liczbie i kolejności jak podano w pliku konfiguracyjnym.
takie rozwiązanie podyktowane jest ogromnym nakładem czasowym związanym z potencjalną zamianą kążdego 4 argumentu pozycji cząstki
na argument prędkości w którym przechowywany jest typ cząstki (ta pozycja w przeciwieństwie do pozycji nie musi zawierać 1 
w celu normalizacji, a nadal zawiera 4 niewykożystaną pozychję ze względu na kompatybilność długości wektorów pozycji i prędkości)
</section>