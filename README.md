<title>Particles</title>
</br>
<h1>"Readme" z przykładu "particles" od Nvidia; (EN) "Readme" from the "particles" example from Nvidia</h1>
<a href="readme1.txt">readme1</a>
</br>
<h1>Opis wywoływania symulacji z parametrami; (EN) Description of calling a simulation with parameters</h1>
<a href="readme2.txt">readme2</a>
</br>
<h1>Koncepcja modyfikacji zwiększającej liczbę różnych cząstek; (EN) Modification concept to increase the number of different particles</h1>
<a href="readme3.txt">readme3</a>
</br>
<h2>License</h2>
Due to NVIDIA EULA for CUDA Samples we are obliged to state that: 
“This software contains source code
provided by NVIDIA Corporation.”


<h1>format save; (EN) file format used by "save"</h1>
<section>
W pliku powstającym w wyniku opcji save zapisuje się następująco: int (4 bajty) ilości cząstek, następnie, co 10tą 
iterację, zapisywane są położenia cząstek: najpierw jest double (8 bajtów) ze znacznikiem czasu w sekundach, potem 
są pozycje cząsek - jest ich tyle ile podano na początku, każda cząstka opisana jest 4-ema float-ami (po 4 bajty), 
pierwsze 3 oznaczają pozycję (x,y,z), 4ty to 1 ze względu na to, że jest potrzebny do operacji macierzowych w 
znormalizowanej postaci. Oznaczenie typu cząstki jest przechowycwane w 4tej "składowej" prędkości, która nie jest 
zapisywana do pliku. Żeby poznać typ cząstki trzeba zajrzeć do pliku particleType.cfg, z którego się korzystało do 
symulacji. Cząstki są zapisane w niezmienionej kolejności, w takiej liczbie i kolejności jak podano w pliku 
konfiguracyjnym. Takie rozwiązanie podyktowane jest ogromnym nakładem czasowym związanym z potencjalną zamianą kążdej 
4tej składowej pozycji cząstki na składową prędkości w której przechowywany jest typ cząstki (ta 4ta "składowa" 
prędkości, w przeciwieństwie do pozycji, nie musi zawierać 1 w celu normalizacji, a nadal jest niewykorzystana; 
musi byc obecna ze względu na kompatybilność długości wektorów pozycji i prędkości).
</section>
(EN)
<section>
The file resulting from the save option stores the following: int (4 bytes) of the number of particles, then, every 
10th iteration, the positions of the particles are stored: first there is a double (8 bytes) with a timestamp in 
seconds, then there are the positions of the particles - there are as many as given at the beginning, each particle 
is described by 4 floats (4 bytes each), the first 3 denote the position (x,y,z), the 4th is 1 because it is needed 
for matrix operations in a normalised form. The particle type designation is stored in the 4th "component" of the 
velocity, which is not saved to the file. To find out the particle type you need to look in the particleType.cfg 
file you used for the simulation. The particles are saved in the same order and number as specified in the 
configuration file. This solution is dictated by the enormous time effort involved in potentially converting every 
4th component of the particle position into the velocity component in which the particle type is stored (this 4th 
"component" of velocity, unlike position, does not need to contain 1 for normalisation purposes and is still unused; 
it must be present for reasons of compatibility between the lengths of the position and velocity vectors).
</section>