###Potrzebne zmiany w kodzie, żeby wprowadzić dla typów cząstek osobny parametr interakcji z powierznia kropli
* Do particleType potrzeba dodać nowe pole float "surfaceTensionFactor".
 * Domyślna wartość 1?
* W particleTypesLoader dopisać znalezienie i wczytanie nowego parametru.
* W particles_kernel.cuh dodać do SimParams pole z tablicą wartości nowego parametru.
* W particlesSystem.cpp przepisywać wartość nowego parametru z typyCzastek do m_params.
* W particles_kernel_impl.cuh w integrate_functor w operator() w napięciu powierzchniowym w wyliczaniu forceF1 uwzględnić nowy parametr.
* Rozszerzyć przykładowe pliki particleType.cfg o nowy parametr.