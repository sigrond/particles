cmd line
particles -nazwaParametru=liczba; (EN) -ParameterName=value

np.; e.g.:
particles -bigRadius0=0.1

bigRadius0 - początkowy promień kropli; (EN) initial droplet radius
n - liczba cząstek; (EN) number of particles
grid - rozmiar gridu; (EN) grid size
file - nazwa pliku do porównania z wynikiem; (EN) file name to compare with the result
timestep - krok czasu; (EN) time step; if set then constant
benchmark - obliczenia bez GUI, pokazuje wydajność; (EN) calculations without GUI, shows performance
i - liczba kroków; (EN) number of steps
device - wybór GPU; (EN) GPU choice
epsi - epsilon w sile lenarda jonesa; (EN) epsilon in Lenard-Jones force
damping - lepkość; (EN) viscosity conforming to Stokes formula
boundaryDamping - napięcie powierchniowe; (EN) surface tension
particleMass - masa cząstki; (EN) particle mass
gravity - grawitacja; (EN) gravitation
save - zapis do pliku; (EN) saving to file
divider - krok zapisu do pliku; (EN) step for saving to file
load - plik do odczytania; (EN) file to read
loadState - plik z zapisanym stanem symulacji; (EN) file with the simulation state recorded
A - szybkość parowania kropli; (EN) evaporation rate
particleTypesNum - liczba rodzjów cząstek; (EN) number of particle types
bQuality - liczba naturalna - sterowanie losowością ruchów Browna; (EN) natural number - Brownian motion randomness control
brown - mnożnik ruchów Browna; (EN) Brownian motion scaling factor
multiColor - losowe kolory kulek; (EN) particle color selected randomly
rotation - prędkość (przypadkowych) obrotów kropli; (EN) droplet (random) rotation frequency [Hz]
help