/*
 * Copyright 1993-2012 NVIDIA Corporation.  All rights reserved.
 *
 * Please refer to the NVIDIA end user license agreement (EULA) associated
 * with this source code for terms and conditions that govern your use of
 * this software. Any use, reproduction, disclosure, or distribution of
 * this software and related documentation outside the terms of the EULA
 * is strictly prohibited.
 *
 */

/*
    Particle system example with collisions using uniform grid

    CUDA 2.1 SDK release 12/2008
    - removed atomic grid method, some optimization, added demo mode.

    CUDA 2.2 release 3/2009
    - replaced sort function with latest radix sort, now disables v-sync.
    - added support for automated testing and comparison to a reference value.
*/

/** \file particles.cpp
 * \brief Główny plik projektu.
 * Zawiera main i funkcje sterujące całym programem.
 */


/** \mainpage Cząstki
 * \author Tomasz Jakubczyk
 * \section swtep Wstęp
 * Program symuluje kroplę cieczy z nanocząstkami w środku (zwiesina) w pułapce.
 * W zawiesini może znajdować się kilka różnych rodzajów nanocząstek.
 * Symulowane są odziaływania między nanocząstkami między sobą oraz z
 * napięciem powierzchniowym kropli, która paruje zwmniejszając swój promień.
 * Możliwe jest też opuszczenie kryształu, który pozostał po wyparowaniu cieczy
 * na płaską powierzchnię na dnie pułapki.
 * Obecnie zakłada się, że nanocząstki są sztywnymi kulami i wylicznay jest
 * potencjał Lennarda-Jonesa.
 * \subsection parametry Uwzględniane Parametry Symulacji
 * - Krok czasu obliczeń.
 * - Lepkość cieczy.
 * - Efektywna grawitacja.
 * - Napięcie powierzchniowe.
 * - Współczynnik proporcjonalny potencjału Lennarda-Jonesa.
 * - Współczynnik ruchów Browna.
 * - Odpychanie Coulombowskie.
 * - Ilości różnych rodzajów cząstek wraz z ich promieniami i masami (lub gęstościami).
 * - Szybkość parowania kropli.
 * \section Opis_Ogólny Opis Ogólny
 * - Najpierw wyliczana jest nowa pozycja na podstawie kroku czasu i prędkości.
 * Prędkość to poprzednia prędkość, grawitacja i lepkość.
 * - Potem cząstki dzielone są na komurki według ich położenia.
 * - Dla każdej cząstki wyliczana jest siła jaka na nią zadziała.
 * Składa się ona ze zderzeń ze wszystkimi cząstkami w zasięgu,
 * muszą one być w tej samej komurce, albo w sąsiedniej.
 * - Prędkość jest wyliczana z poprzedniej prękości i siły.
 * \section linki_do_fizyki Linki do fizyki
 http://www.cchem.berkeley.edu/chem195/_l_j___force_short_8m.html
 http://phys.ubbcluj.ro/~tbeu/MD/C2_for.pdf
 http://www2.physics.umd.edu/~alaporta/Lennard-Jones.html
 http://www.fis.agh.edu.pl/~Burda//NEWS/Informacje%20dla%20wszystkich/CwiczenieAFM.pdf
 http://en.wikipedia.org/wiki/Lennard-Jones_potential#Alternative_expressions
 https://pl.wikibooks.org/wiki/Mechanika_teoretyczna/Mechanika_teoretyczna#Doskonale_spr.C4.99.C5.BCyste_zderzenie_dw.C3.B3ch_cz.C4.85stek
 * \section Repozytorium Repozytorium
 https://github.com/sigrond/particles
 * \subsection dokumentacja_repozytorium Dokumentacja Repozytorium
 http://sigrond.github.io/particles
 * \section READMEs READMEs
 * \subsection parametry_uruchamiania Parametry Uruchamiania
 * \verbinclude readme2.txt
 * \subsection oryginalny_readme Oryginalny Readme
 * \verbinclude readme1.txt
 * \subsection koncepcja_symulowania_roznych_czastek Koncepcja symulowania różnych cząstek
 * \verbinclude readme3.txt
 * \subsection rezerwa Opis formatu pliku konfiguracji typów cząstek
 * Niestety trzeba go napisać.
 * Domyślny plik "particleType.cfg".
 * \verbinclude particleType.cfg
 * Więcej można znaleźć w "particleTypesLoader.cpp" oraz "particleType.h"
 * \sa particleTypesLoader::loadTypes
 * \n particleTypesLoader.cpp
 * \n particleType::normalizedEpsilon
 * \n particleType.h
 */

// OpenGL Graphics includes
#include <GL/glew.h>
#if defined (_WIN32)
#include <GL/wglew.h>
#endif
#if defined(__APPLE__) || defined(__MACOSX)
#include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
#endif

// CUDA runtime
#include <cuda_runtime.h>

// CUDA utilities and system includes
#include <helper_functions.h>
#include <helper_cuda.h>    // includes cuda.h and cuda_runtime_api.h
#include <helper_cuda_gl.h> // includes cuda_gl_interop.h// includes cuda_gl_interop.h

// Includes
#include <stdlib.h>
#include <cstdlib>
#include <cstdio>
#include <algorithm>
#include <float.h>
#include <vector>
#include <string>
#include <iostream>

#include "particleSystem.h"
#include "render_particles.h"
#include "paramgl.h"

#include "particleType.h"
#include "particleTypesLoader.h"
#include "GLgraph.h"

#define MAX_EPSILON_ERROR 5.00f
#define THRESHOLD         0.30f

#define GRID_SIZE       64
#define NUM_PARTICLES   512

const uint width = 1280, height = 1024;

// view params
int ox, oy;
int buttonState = 0;
float camera_trans[] = {0, 0, -3};
float camera_rot[]   = {0, 0, 0};
float camera_trans_lag[] = {0, 0, -3};
float camera_rot_lag[] = {0, 0, 0};
const float inertia = 0.1f;
/** \var zoom
 * \brief powiększenie
 */
float zoom=1.0f;
ParticleRenderer::DisplayMode displayMode = ParticleRenderer::PARTICLE_SPHERES;

int mode = 0;
bool displayEnabled = true;
bool bPause = false;
bool displaySliders = false;
bool wireframe = false;
bool demoMode = false;
int idleCounter = 0;
int demoCounter = 0;
const int idleDelay = 2000;

enum { M_VIEW = 0, M_MOVE };

uint numParticles = 0;
uint3 gridSize;
int numIterations = 0; // run until exit

// simulation parameters
/** \var timestep
 * \brief krok czasu
 */
float timestep = 0.0f;//0.5f;
/** \var damping
 * \brief lepkość
 */
float damping = 0.99f;//0.08f;//global damping
float gravity = 0.0f;//0.0003f;
int iterations = 1;
int ballr = 10;

/** \var boundaryDamping
 * \brief współczynnik oddziaływania z brzegami
 */
float boundaryDamping= 1.0f;
/** \var particleMass
 * \brief masa cząstki
 */
float particleMass=0.001f;
bool boundaries=true;
/** \var epsi
 * \brief epsilon do siły lenarda jonesa
 */
float epsi=0.01f;
/** \var brown
 * \brief nasilenie ruchów browna
 */
float brown=0.0f;
unsigned long long int brownQuality=10;

int particleTypesNum=1;

float collideSpring = 0.5f;;
float collideDamping = 0.02f;;
float collideShear = 0.1f;
float collideAttraction = 0.08f;

/** \var bigRadius
 * \brief promien duzej kuli w mikronach
 */
float bigRadius=10.0f;//promien duzej kuli
float bigRadius0=bigRadius;//poczatkowy promien duzej kuli
/** \var kurczenie
 * A r=r0-A*sqrt(t)
 * \brief parametr równania na parowanie kropli
 */
float kurczenie=0.1f;//A r=r0-A*sqrt(t)
#define A kurczenie
/** \var licznik
 * \brief ilość kroków od początku symulacji
 */
unsigned long long int licznik=0;
/** \var time_past
 * \brief czas który upłynoł
 */
long double time_past=0.0;

extern bool multiColor=false;

/** \var typyCzastek
 * \brief tablica z typami cząstek
 */
std::vector<particleType> typyCzastek;

/** \brief Zmienna logiczna stwierdzająca czy czas rysować w GL
 * została dodana, żeby móc zachować stały FPS w grafice i
 * dać więcej czasu na obliczenia CUDA.
 */
bool itsTimeToDraw=true;

/** \brief Licznik czasu do utrzymywania stałego FPS.
 */
long double timeFromLastDisplayedFrameIn_ms=0.0f;

/** \brief Wartość ciśnienia wywieranego prze zcżastki na
 * powiercznię kropli. Znajduje się w pamięci po stronie host'a (CPU)
 */
float hostSurfacePreasure=0.0f;

/** \brief rysowacz wykresu
 */
GLgraph* preasureGraph=NULL;

/** \brief tablica ciśnienia
 */
std::vector<float> preasureVector;

ParticleSystem *psystem = 0;

// fps
static int fpsCount = 0;
static int fpsLimit = 1;
StopWatchInterface *timer = NULL;

ParticleRenderer *renderer = 0;

float modelView[16];

ParamListGL *params;

// Auto-Verification Code
const int frameCheckNumber = 4;
unsigned int frameCount = 0;
unsigned int g_TotalErrors = 0;
char        *g_refFile = NULL;

char* save=NULL;

const char *sSDKsample = "CUDA Particles Simulation";

extern "C" void cudaInit(int argc, char **argv);
extern "C" void cudaGLInit(int argc, char **argv);
extern "C" void copyArrayFromDevice(void *host, const void *device, unsigned int vbo, int size);

// initialize particle system
/** \brief initialize particle system
 * ustawienia początkowe i utworzenie obiektu systemu cząstek
 * \param numParticles int ilość cząstek
 * \param gridSize uint3 wymiary gridu
 * \param bUseOpenGL bool czy jest GUI
 * \return void
 */
void initParticleSystem(int numParticles, uint3 gridSize, bool bUseOpenGL)
{
    psystem = new ParticleSystem(numParticles, gridSize, bUseOpenGL);
	psystem->setBigRadius(bigRadius0);
	psystem->setBigRadius0(bigRadius0);
    //psystem->reset(ParticleSystem::CONFIG_GRID);
	psystem->reset(ParticleSystem::CONFIG_RANDOM);
	psystem->setBrownQuality(brownQuality);

    if (bUseOpenGL)
    {
        renderer = new ParticleRenderer;
        renderer->setParticleRadius(psystem->getParticleRadius());
        renderer->setColorBuffer(psystem->getColorBuffer());
    }

    sdkCreateTimer(&timer);
}

void cleanup()
{
    sdkDeleteTimer(&timer);
}

// initialize OpenGL
/** \brief initialize OpenGL
 * \param argc int*
 * \param argv char**
 * \return void
 */
void initGL(int *argc, char **argv)
{
    glutInit(argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(width, height);
    glutCreateWindow("CUDA Particles");
	//glutFullScreen();

    glewInit();

    if (!glewIsSupported("GL_VERSION_2_0 GL_VERSION_1_5 GL_ARB_multitexture GL_ARB_vertex_buffer_object"))
    {
        fprintf(stderr, "Required OpenGL extensions missing.");
        exit(EXIT_FAILURE);
    }

#if defined (_WIN32)

    if (wglewIsSupported("WGL_EXT_swap_control"))
    {
        // disable vertical sync
        wglSwapIntervalEXT(0);
    }

#endif

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.25, 0.25, 0.25, 1.0);

    glutReportErrors();
}

long double tSF=0.0f;
long double SF=boundaryDamping;
bool koncowka_parowania=false;
float time_to_end=0.0f;
/** \brief sterowanie parowaniem kropli
 * \return void
 */
void parowanieKropliWCzasie()
{
	//bigRadius=bigRadius0-A*sqrt(licznik*timestep);//r=r0-A*sqrt(t)
	licznik++;
	time_past+=timestep;
	//std::cout<<psystem->getMaxParticleRadius()*pow(psystem->getNumParticles(),0.3f)<<std::endl;
	if(bigRadius>(psystem->getMaxParticleRadius()*pow(psystem->getNumParticles(),0.3f))
		&& bigRadius>0.0f && hostSurfacePreasure<0.01f)
	{
		if(!koncowka_parowania)
		{
			SF=boundaryDamping;
		}
	}
	else if(tSF<6.0f)
    {
		if(!koncowka_parowania)
		{
			koncowka_parowania=true;
			time_to_end=(bigRadius0/A)*(bigRadius0/A)-time_past;
		}
        tSF+=6.0f*timestep/time_to_end;//*10.0f;
        boundaryDamping=SF*(-(tanh(tSF-3.0f)-1.0f)/2.0f);
        if(boundaryDamping<0.0f)
        {
            boundaryDamping=0.0f;
        }
    }
	else
	{
		boundaries=false;
		boundaryDamping=0.0f;
		//timestep=0.0001f;
	}
	if(bigRadius>0.0f)
	{
		bigRadius=bigRadius0-A*sqrt(time_past);
		psystem->setSurfaceVel(A*sqrt(time_past)/time_past);
	}
	else
	{
		bigRadius=0.0f;
		psystem->setSurfaceVel(0.0f);
	}
	psystem->setBigRadius(bigRadius);
}

/** \brief uruchomienie symulacji bez GUI z zapisem wyniku
 * \param iterations int ilość przebiegów do wykonania
 * \param exec_path char* ścieżka do aktualnego katalogu
 * \return void
 */
void runBenchmark(int iterations, char *exec_path)
{
    printf("Run %u particles simulation for %d iterations...\n\n", numParticles, iterations);
    cudaDeviceSynchronize();
    sdkStartTimer(&timer);
    psystem->setDamping(damping);
    psystem->setGravity(-gravity);
	psystem->setBoundaryDamping(-boundaryDamping);
	psystem->setParticleMass(particleMass);
	psystem->setEpsi(epsi);
	psystem->setBrown(brown);

	float *hPos=NULL;
	FILE* f=NULL;
	if(save)
	{
		f=fopen(save,"wb");
		//fprintf(f,"%d",psystem->getNumParticles());//iloœæ cz¹stek
		fwrite((void*)&numParticles,sizeof(int),1,f);
	}

	//printf("timestep: %f\n",timestep);

	checkCudaErrors(cudaMallocHost(&hPos,sizeof(float)*4*numParticles));
    for (int i = 0; i < iterations; ++i)
    {
		parowanieKropliWCzasie();
        psystem->update(timestep);
		if(save && f && i%10==0)
		{

			checkCudaErrors(cudaMemcpy((void*)hPos,psystem->getCudaPosVBO(),sizeof(float)*4*numParticles,cudaMemcpyDeviceToHost));
			fwrite((void*)&time_past,sizeof(long double),1,f);
			fwrite((void*)hPos,sizeof(float),4*numParticles,f);

		}
    }
	//cudaDeviceSynchronize();
	//copyArrayFromDevice(hPos, psystem->getCudaPosVBO(),0, sizeof(float)*4*psystem->getNumParticles());
	fprintf(f,"%Lf",time_past);//czas klatki
	//fwrite((void *)hPos,sizeof(float),4*psystem->getNumParticles(),f);
	//free(hPos);
	checkCudaErrors(cudaFreeHost(hPos));
	hPos=NULL;

	if(save && f)
	{
		fclose(f);
	}

    for (int i = 0; i < iterations; ++i)
    {
        psystem->update(timestep);
    }

    cudaDeviceSynchronize();
    sdkStopTimer(&timer);
    float fAvgSeconds = ((float)1.0e-3 * (float)sdkGetTimerValue(&timer)/(float)iterations);

    printf("particles, Throughput = %.4f KParticles/s, Time = %.5f s, Size = %u particles, NumDevsUsed = %u, Workgroup = %u\n",
           (1.0e-3 * numParticles)/fAvgSeconds, fAvgSeconds, numParticles, 1, 0);

    if (g_refFile)
    {
        printf("\nChecking result...\n\n");
        float *hPos = (float *)malloc(sizeof(float)*4*psystem->getNumParticles());
        copyArrayFromDevice(hPos, psystem->getCudaPosVBO(),
                            0, sizeof(float)*4*psystem->getNumParticles());

        sdkDumpBin((void *)hPos, sizeof(float)*4*psystem->getNumParticles(), "particles.bin");

        if (!sdkCompareBin2BinFloat("particles.bin", g_refFile, sizeof(float)*4*psystem->getNumParticles(),
                                    MAX_EPSILON_ERROR, THRESHOLD, exec_path))
        {
            g_TotalErrors++;
        }
    }
}

/** \brief liczenie i wypisnie danych na pasku
 * \return void
 */
void computeFPS()
{
    frameCount++;
    fpsCount++;

    if (fpsCount == fpsLimit)
    {
        char fps[256];
        float ifps = 1.f / (sdkGetAverageTimerValue(&timer) / 1000.f);
        sprintf(fps, "CUDA Particles (%d particles): %3.1f fps, p:%f, R:%f, i:%llu, T:%Lf", numParticles, ifps, hostSurfacePreasure, bigRadius, licznik, time_past);

        glutSetWindowTitle(fps);
        fpsCount = 0;

        fpsLimit = (int)MAX(ifps, 1.f);
        sdkResetTimer(&timer);
    }
}

/** \brief funkcja odpowiadająca za symulację z włączonym GUI
 * \return void
 */
void display()
{
    sdkStartTimer(&timer);

    // update the simulation
    if (!bPause)
    {
        psystem->setIterations(iterations);
        psystem->setDamping(damping);
        psystem->setGravity(-gravity);
        psystem->setCollideSpring(collideSpring);
        psystem->setCollideDamping(collideDamping);
        psystem->setCollideShear(collideShear);
        psystem->setCollideAttraction(collideAttraction);
		psystem->setBoundaryDamping(-boundaryDamping);
		//psystem->setParticleMass(particleMass);
		psystem->setEpsi(epsi);/**< mnożnik epsilonów */
		psystem->setBrown(brown);
		psystem->setCalcSurfacePreasure(itsTimeToDraw);/**< czy chcemy policzyć i ściągnąć wartość ciśnienia na powierchni kropli */

		parowanieKropliWCzasie();
		//licznik++;
		//time_past+=timestep;
		//bigRadius=bigRadius0-A*sqrt(licznik*timestep);//r=r0-A*sqrt(t)
		//bigRadius=bigRadius0-A*sqrt(time_past);
		//psystem->setBigRadius(bigRadius);

        psystem->update(timestep);/**< tu zachodzą właściwe obliczenia CUDA */

        if (renderer && itsTimeToDraw)
        {
            renderer->setVertexBuffer(psystem->getCurrentReadBuffer(), psystem->getNumParticles(), zoom);
        }
    }

    if(itsTimeToDraw)
    {

        // render
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // view transform
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();


        for (int c = 0; c < 3; ++c)
        {
            camera_trans_lag[c] += (camera_trans[c] - camera_trans_lag[c]) * inertia;
            camera_rot_lag[c] += (camera_rot[c] - camera_rot_lag[c]) * inertia;
        }

        glTranslatef(camera_trans_lag[0], camera_trans_lag[1], camera_trans_lag[2]);
        glRotatef(camera_rot_lag[0], 1.0, 0.0, 0.0);
        glRotatef(camera_rot_lag[1], 0.0, 1.0, 0.0);

        glScalef(zoom, zoom, zoom);

		glPushMatrix();
        preasureVector.push_back(hostSurfacePreasure);
        glDisable(GL_DEPTH_TEST);
        glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO); // invert color
        glEnable(GL_BLEND);
        preasureGraph->render(0, glutGet(GLUT_WINDOW_HEIGHT)-50);
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
		glPopMatrix();

        glGetFloatv(GL_MODELVIEW_MATRIX, modelView);

        //glScalef(zoom, zoom, zoom);

        // cube -> sphere
        glColor3f(1.0, 1.0, 1.0);
        //glutWireCube(2.0);
        if(boundaries)
        {
			glutWireSphere(bigRadius, 20, 10);
		}


        //glScalef(zoom, zoom, zoom);


        //float3 p = psystem->getColliderPos();
        //glTranslatef(p.x, p.y, p.z);
        //glScalef(zoom, zoom, zoom);
        glGetFloatv(GL_MODELVIEW_MATRIX, modelView);

        // cube -> sphere
        glColor3f(1.0, 1.0, 1.0);
        //glutWireCube(2.0);
        glutWireSphere(bigRadius, 20, 10);

        // collider
        glPushMatrix();
        float3 p = psystem->getColliderPos();
        glTranslatef(p.x, p.y, p.z);
        //glColor3f(1.0, 0.0, 0.0);
        //glutSolidSphere(psystem->getColliderRadius(), 20, 10);
        glPopMatrix();

        if (renderer && displayEnabled)
        {
            renderer->display(displayMode);
        }

        /** Ten blok kodu rysuje półprzezroczysty kwadrat pod kroplą
         * na wysokości ograniczenia dolnego.
         */
        glEnable (GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glPushMatrix();
            //glColor3f(0.0, 0.1, 0.3);
            glColor4f(0.2, 0.8, 1.0,0.5);
            glBegin(GL_POLYGON);
                glVertex3f( -20.0f*bigRadius0, -2.0f*bigRadius0, -20.0f*bigRadius0);
                glVertex3f( -20.0f*bigRadius0,  -2.0f*bigRadius0, 20.0f*bigRadius0);
                glVertex3f(  20.0f*bigRadius0,  -2.0f*bigRadius0, 20.0f*bigRadius0);
                glVertex3f(  20.0f*bigRadius0, -2.0f*bigRadius0, -20.0f*bigRadius0);
            glEnd();
            //glTranslatef(0,-2.0f*bigRadius0,0);
        glPopMatrix();
        glDisable(GL_BLEND);

        if (displaySliders)
        {
            glDisable(GL_DEPTH_TEST);
            glBlendFunc(GL_ONE_MINUS_DST_COLOR, GL_ZERO); // invert color
            glEnable(GL_BLEND);
            params->Render(0, 0);
            glDisable(GL_BLEND);
            glEnable(GL_DEPTH_TEST);
        }

        glutSwapBuffers();
        glutReportErrors();
    }/**< koniec warunku itsTimeToDraw */

    sdkStopTimer(&timer);

    long double timeTakenIn_ms=sdkGetAverageTimerValue(&timer);
    timeFromLastDisplayedFrameIn_ms+=timeTakenIn_ms;
    if(timeFromLastDisplayedFrameIn_ms>=33.3333333333333333f)/**< to powinno utrzymać stały FPS */
    {
        itsTimeToDraw=true;
        timeFromLastDisplayedFrameIn_ms=0.0f;
    }
    else
    {
        itsTimeToDraw=false;
    }

    computeFPS();/**< teraz nie jest to FPS, tylko chwilowa prędkość wykonywania programu */
}

inline float frand()
{
    return rand() / (float) RAND_MAX;
}

/** \brief wstawianie kuli cząstek na życzenie
 * \return void
 */
void addSphere()
{
    // inject a sphere of particles
    float pr = psystem->getParticleRadius();
    float tr = pr+(pr*2.0f)*ballr;
    float pos[4], vel[4];
    pos[0] = -1.0f + tr + frand()*(2.0f - tr*2.0f);
    pos[1] = 1.0f - tr;
    pos[2] = -1.0f + tr + frand()*(2.0f - tr*2.0f);
    pos[3] = 0.0f;
    //vel[0] = vel[1] = vel[2] = vel[3] = 0.0f;
	vel[0]=frand()/100.0f;
	vel[1]=frand()/100.0f;
	vel[2]=frand()/100.0f;
	vel[3]=frand()/100.0f;
    psystem->addSphere(0, pos, vel, ballr, pr*2.0f);
}

void reshape(int w, int h)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (float) w / (float) h, 0.1, 100.0);

    glMatrixMode(GL_MODELVIEW);
    glViewport(0, 0, w, h);

    if (renderer)
    {
        renderer->setWindowSize(w, h);
        renderer->setFOV(60.0);
    }
}

/** \brief obsługa zdarzeń myszy
 * \param button int
 * \param state int
 * \param x int
 * \param y int
 * \return void
 */
void mouse(int button, int state, int x, int y)
{
    int mods;
	if(button==3)
	{
		if(state==GLUT_DOWN)
		{
			zoom*=1.1f;
		}
	}
	else if(button==4)
	{
		if(state==GLUT_DOWN)
		{
			zoom*=0.9f;
		}
	}
    else if (state == GLUT_DOWN)
    {
        buttonState |= 1<<button;
    }
    else if (state == GLUT_UP)
    {
        buttonState = 0;
    }

    mods = glutGetModifiers();

    if (mods & GLUT_ACTIVE_SHIFT)
    {
        buttonState = 2;
    }
    else if (mods & GLUT_ACTIVE_CTRL)
    {
        buttonState = 3;
    }

    ox = x;
    oy = y;

    demoMode = false;
    idleCounter = 0;

    if (displaySliders)
    {
        if (params->Mouse(x, y, button, state))
        {
            glutPostRedisplay();
            return;
        }
    }

    glutPostRedisplay();
}

// transfrom vector by matrix
/** \brief transfrom vector by matrix
 * \param v float*
 * \param r float*
 * \param m GLfloat*
 * \return void
 */
void xform(float *v, float *r, GLfloat *m)
{
    r[0] = v[0]*m[0] + v[1]*m[4] + v[2]*m[8] + m[12];
    r[1] = v[0]*m[1] + v[1]*m[5] + v[2]*m[9] + m[13];
    r[2] = v[0]*m[2] + v[1]*m[6] + v[2]*m[10] + m[14];
}

// transform vector by transpose of matrix
/** \brief transform vector by transpose of matrix
 * \param v float*
 * \param r float*
 * \param m GLfloat*
 * \return void
 */
void ixform(float *v, float *r, GLfloat *m)
{
    r[0] = v[0]*m[0] + v[1]*m[1] + v[2]*m[2];
    r[1] = v[0]*m[4] + v[1]*m[5] + v[2]*m[6];
    r[2] = v[0]*m[8] + v[1]*m[9] + v[2]*m[10];
}

void ixformPoint(float *v, float *r, GLfloat *m)
{
    float x[4];
    x[0] = v[0] - m[12];
    x[1] = v[1] - m[13];
    x[2] = v[2] - m[14];
    x[3] = 1.0f;
    ixform(x, r, m);
}

void motion(int x, int y)
{
    float dx, dy;
    dx = (float)(x - ox);
    dy = (float)(y - oy);

    if (displaySliders)
    {
        if (params->Motion(x, y))
        {
            ox = x;
            oy = y;
            glutPostRedisplay();
            return;
        }
    }

    switch (mode)
    {
        case M_VIEW:
            if (buttonState == 3)
            {
                // left+middle = zoom
                camera_trans[2] += (dy / 100.0f) * 0.5f * fabs(camera_trans[2]);
            }
            else if (buttonState & 2)
            {
                // middle = translate
                camera_trans[0] += dx / 100.0f;
                camera_trans[1] -= dy / 100.0f;
            }
            else if (buttonState & 1)
            {
                // left = rotate
                camera_rot[0] += dy / 5.0f;
                camera_rot[1] += dx / 5.0f;
            }

            break;

        case M_MOVE:
            {
                float translateSpeed = 0.003f;
                float3 p = psystem->getColliderPos();

                if (buttonState==1)
                {
                    float v[3], r[3];
                    v[0] = dx*translateSpeed;
                    v[1] = -dy*translateSpeed;
                    v[2] = 0.0f;
                    ixform(v, r, modelView);
                    p.x += r[0];
                    p.y += r[1];
                    p.z += r[2];
                }
                else if (buttonState==2)
                {
                    float v[3], r[3];
                    v[0] = 0.0f;
                    v[1] = 0.0f;
                    v[2] = dy*translateSpeed;
                    ixform(v, r, modelView);
                    p.x += r[0];
                    p.y += r[1];
                    p.z += r[2];
                }

                psystem->setColliderPos(p);
            }
            break;
    }

    ox = x;
    oy = y;

    demoMode = false;
    idleCounter = 0;

    glutPostRedisplay();
}

// commented out to remove unused parameter warnings in Linux
/** \brief obsługa zdarzeń klawiszy i opcji z menu
 * \param key unsigned char
 * \param x int
 * \param y int
 * \return void
 */
void key(unsigned char key, int /*x*/, int /*y*/)
{
    switch (key)
    {
		case '0':
			time_past=0.0;
			bigRadius=bigRadius0;
			tSF=-0.0f;
			SF=0.0f;
			koncowka_parowania=false;
			boundaries=true;
			preasureVector.clear();
			psystem->setBigRadius(bigRadius);
			boundaryDamping=1.0f;
			psystem->reset(ParticleSystem::CONFIG_RANDOM);
			break;

		case 'b':
			boundaries=!boundaries;
			psystem->setBoundaries(boundaries);
			break;

        case ' ':
            bPause = !bPause;
            break;

        case 13:
            psystem->update(timestep);

            if (renderer)
            {
                renderer->setVertexBuffer(psystem->getCurrentReadBuffer(), psystem->getNumParticles(), zoom);
            }

            break;

        case '\033':
        case 'q':
            exit(EXIT_SUCCESS);
            break;

        case 'v':
            mode = M_VIEW;
            break;

        case 'm':
            mode = M_MOVE;
            break;

        case 'p':
            displayMode = (ParticleRenderer::DisplayMode)
                          ((displayMode + 1) % ParticleRenderer::PARTICLE_NUM_MODES);
            break;

        case 'd':
            psystem->dumpGrid();
            break;

        case 'u':
            psystem->dumpParticles(0, numParticles-1);
            break;

        case 'r':
            displayEnabled = !displayEnabled;
            break;

        case '1':
            psystem->reset(ParticleSystem::CONFIG_GRID);
            break;

        case '2':
            psystem->reset(ParticleSystem::CONFIG_RANDOM);
            break;

        case '3':
            addSphere();
            break;

        case '4':
            {
                // shoot ball from camera
                float pr = psystem->getParticleRadius();
                float vel[4], velw[4], pos[4], posw[4];
                vel[0] = 0.0f;
                vel[1] = 0.0f;
                vel[2] = -0.05f;
                vel[3] = 0.0f;
                ixform(vel, velw, modelView);

                pos[0] = 0.0f;
                pos[1] = 0.0f;
                pos[2] = -2.5f;
                pos[3] = 1.0;
                ixformPoint(pos, posw, modelView);
                posw[3] = 0.0f;

                psystem->addSphere(0, posw, velw, ballr, pr*2.0f);
            }
            break;

        case 'w':
            wireframe = !wireframe;
            break;

        case 'h':
            displaySliders = !displaySliders;
            break;
    }

    demoMode = false;
    idleCounter = 0;
    glutPostRedisplay();
}

void special(int k, int x, int y)
{
    if (displaySliders)
    {
        params->Special(k, x, y);
    }

    demoMode = false;
    idleCounter = 0;
}

void idle(void)
{
    if ((idleCounter++ > idleDelay) && (demoMode==false))
    {
        demoMode = true;
        printf("Entering demo mode\n");
    }

    if (demoMode)
    {
        camera_rot[1] += 0.1f;

        if (false && demoCounter++ > 1000)
        {
            ballr = 10 + (rand() % 10);
            addSphere();
            demoCounter = 0;
        }
    }

    glutPostRedisplay();
}

void initParams()
{
    if (g_refFile || save)
    {
        //timestep = 0.0f;
        //damping = 0.0f;
        //gravity = 0.0f;
        ballr = 1;
        collideSpring = 0.0f;
        collideDamping = 0.0f;
        collideShear = 0.0f;
        collideAttraction = 0.0f;

    }
    else
    {

        // create a new parameter list
        params = new ParamListGL("misc");
        params->AddParam(new Param<float>("time step", timestep, 0.0f, 0.002f, 0.00001f, &timestep));
        params->AddParam(new Param<float>("liquid viscosity"  , damping , 0.0f, 1.0f, 0.00001f, &damping));
        params->AddParam(new Param<float>("effective gravity"  , gravity , 0.0f, 1.0f, 0.00001f, &gravity));
        //params->AddParam(new Param<float> ("A", A , 0.0f, 1.0f, 0.0001, &A));
		params->AddParam(new Param<float>("surface tension"  , boundaryDamping , 0.0f, 2.0f, 0.00001f, &boundaryDamping));
		//params->AddParam(new Param<float>("particle mass"  , particleMass , 0.001f, 2.0f, 0.00001f, &particleMass));
		params->AddParam(new Param<float>("epsilon"  , epsi , 0.0f, 1.0f, 0.00001f, &epsi));
		params->AddParam(new Param<float>("brown"  , brown , 0.0f, 0.5f, 0.00001f, &brown));

        //params->AddParam(new Param<float>("collide spring" , collideSpring , 0.0f, 1.0f, 0.001f, &collideSpring));
        //params->AddParam(new Param<float>("collide damping", collideDamping, 0.0f, 0.1f, 0.001f, &collideDamping));
        //params->AddParam(new Param<float>("collide shear"  , collideShear  , 0.0f, 0.1f, 0.001f, &collideShear));
        params->AddParam(new Param<float>("Coulomb repulsion", collideAttraction, 0.0f, 0.01f, 0.00001f, &collideAttraction));
    }
}

void mainMenu(int i)
{
    key((unsigned char) i, 0, 0);
}

void initMenus()
{
    glutCreateMenu(mainMenu);
    glutAddMenuEntry("Reset block [1]", '1');
    glutAddMenuEntry("Reset random [2]", '2');
    glutAddMenuEntry("Add sphere [3]", '3');
    glutAddMenuEntry("View mode [v]", 'v');
    glutAddMenuEntry("Move cursor mode [m]", 'm');
    glutAddMenuEntry("Toggle point rendering [p]", 'p');
    glutAddMenuEntry("Toggle animation [ ]", ' ');
    glutAddMenuEntry("Step animation [ret]", 13);
    glutAddMenuEntry("Toggle sliders [h]", 'h');
	glutAddMenuEntry("Reset time [0]", '0');
	glutAddMenuEntry("Boundaries on/off [b]", 'b');
    glutAddMenuEntry("Quit (esc)", '\033');
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

////////////////////////////////////////////////////////////////////////////////
// Program main
////////////////////////////////////////////////////////////////////////////////
int
main(int argc, char **argv)
{
#ifdef _DEBUG
    std::cout<<"Debug Build!\n";
#endif // _DEBUG
	std::cout<<"Uwaga różne typy cząstek!\nKonfiguracja w particleType.cfg\n";
	std::string plikKonfiguracyjny("particleType.cfg");
    particleTypesLoader pTLoader(plikKonfiguracyjny);
    pTLoader.loadTypes(typyCzastek);

#ifdef _DEBUG
	std::clog<<"typyCzastek.size() "<<typyCzastek.size()<<"\n";
    system("pause");
#endif // _DEBUG

    printf("%s Starting...\n\n", sSDKsample);

    numParticles = pTLoader.getParticlesNumber();//NUM_PARTICLES;
    uint gridDim = GRID_SIZE;
    numIterations = 0;

    if (argc > 1)
    {

		if (checkCmdLineFlag(argc, (const char **) argv, "bigRadius0"))
        {
            bigRadius0 = getCmdLineArgumentFloat(argc, (const char **)argv, "bigRadius0");
			bigRadius=bigRadius0;
        }

        if (checkCmdLineFlag(argc, (const char **) argv, "n"))
        {
            numParticles = getCmdLineArgumentInt(argc, (const char **)argv, "n");
        }

		if (checkCmdLineFlag(argc, (const char **) argv, "particleTypesNum"))
        {
            particleTypesNum = getCmdLineArgumentInt(argc, (const char **)argv, "particleTypesNum");
        }

        if (checkCmdLineFlag(argc, (const char **) argv, "grid"))
        {
            gridDim = getCmdLineArgumentInt(argc, (const char **) argv, "grid");
        }

        if (checkCmdLineFlag(argc, (const char **)argv, "file"))
        {
            getCmdLineArgumentString(argc, (const char **)argv, "file", &g_refFile);
            fpsLimit = frameCheckNumber;
            numIterations = 1;
        }

		if (checkCmdLineFlag(argc, (const char **)argv, "save"))
        {
            getCmdLineArgumentString(argc, (const char **)argv, "save", &save);
			fpsLimit = frameCheckNumber;
            numIterations = 1;
			if(!g_refFile)
			{
				g_refFile=save;
			}
        }

		if (checkCmdLineFlag(argc, (const char **) argv, "timestep"))
        {
            timestep = getCmdLineArgumentFloat(argc, (const char **) argv, "timestep");
			//printf("flag timestep set to: %f\n",timestep);
        }
		if (checkCmdLineFlag(argc, (const char **) argv, "epsi"))
        {
            epsi = getCmdLineArgumentFloat(argc, (const char **) argv, "epsi");
        }
		if (checkCmdLineFlag(argc, (const char **) argv, "damping"))
        {
            damping = getCmdLineArgumentFloat(argc, (const char **) argv, "damping");
        }
		if (checkCmdLineFlag(argc, (const char **) argv, "boundaryDamping"))
        {
            boundaryDamping = getCmdLineArgumentFloat(argc, (const char **) argv, "boundaryDamping");
        }
		if (checkCmdLineFlag(argc, (const char **) argv, "particleMass"))
        {
            particleMass = getCmdLineArgumentFloat(argc, (const char **) argv, "particleMass");
        }
		if (checkCmdLineFlag(argc, (const char **) argv, "gravity"))
        {
            gravity = getCmdLineArgumentFloat(argc, (const char **) argv, "gravity");
        }
		if (checkCmdLineFlag(argc, (const char **) argv, "A"))
        {
            A = getCmdLineArgumentFloat(argc, (const char **) argv, "A");
        }
		if (checkCmdLineFlag(argc, (const char **) argv, "brown"))
        {
            brown = getCmdLineArgumentFloat(argc, (const char **) argv, "brown");
			//printf("brown: %f",brown);
        }
		if (checkCmdLineFlag(argc, (const char **) argv, "bQuality"))
		{
			brownQuality = getCmdLineArgumentInt(argc, (const char **) argv, "bQuality");
		}
		if (checkCmdLineFlag(argc, (const char **) argv, "help"))
        {
            printf("cmd line\n");
            printf("particles -nazwaParametru=liczba\n");
            printf("np:\n");
            printf("particles -bigRadius0=0.1 \n");
            printf("bigRadius0 -początkowy promień kropli\n");
            printf("n -liczba cząstek\n");
            printf("grid -rozmiar gridu\n");
            printf("file -nazwa pliku do porównania z wynikiem\n");
            printf("timestep -krok czasu\n");
            printf("benchmark -obliczenia bez GUI, pokazuje wydajnoœæ\n");
            printf("i -liczba kroków\n");
            printf("device -wybór GPU\n");
            printf("epsi -epsilon w sile Lenarda-Jonesa\n");
            printf("damping -lepkość\n");
            printf("boundaryDamping -napięcie powierchniowe\n");
            printf("particleMass -masa cząstki\n");
            printf("gravity -grawitacja\n");
			printf("save -zapis do pliku\n");
			printf("A -stała parowania kropli\n");
			printf("particleTypesNum -ilość rodzjów cząstek\n");
			printf("bQuality -liczba naturalna\n");
			printf("brown -mnożnik róchów Browna\n");
			printf("multiColor -losowe kolory kulek\n");
            printf("help\n");

        }
    }

    gridSize.x = gridSize.y = gridSize.z = gridDim;
    printf("grid: %d x %d x %d = %d cells\n", gridSize.x, gridSize.y, gridSize.z, gridSize.x*gridSize.y*gridSize.z);
	numParticles = pTLoader.getParticlesNumber();
    printf("particles: %d\n", numParticles);

    bool benchmark = checkCmdLineFlag(argc, (const char **) argv, "benchmark") != 0;
	multiColor = checkCmdLineFlag(argc, (const char **) argv, "multiColor") != 0;

    if (checkCmdLineFlag(argc, (const char **) argv, "i"))
    {
        numIterations = getCmdLineArgumentInt(argc, (const char **) argv, "i");
    }
    if (g_refFile || save)
    {
        cudaInit(argc, argv);
    }
    else
    {
        if (checkCmdLineFlag(argc, (const char **)argv, "device"))
        {
            printf("[%s]\n", argv[0]);
            printf("   Does not explicitly support -device=n in OpenGL mode\n");
            printf("   To use -device=n, the sample must be running w/o OpenGL\n\n");
            printf(" > %s -device=n -file=<*.bin>\n", argv[0]);
            printf("exiting...\n");
            exit(EXIT_SUCCESS);
        }
#ifdef _DEBUG
		system("pause");
#endif // _DEBUG
        initGL(&argc, argv);
        cudaGLInit(argc, argv);
#ifdef _DEBUG
		system("pause");
#endif // _DEBUG
    }

	//printf("timestep: %f\n",timestep);
	numParticles = pTLoader.getParticlesNumber();

    initParticleSystem(numParticles, gridSize, g_refFile==NULL);
#ifdef _DEBUG
	std::clog<<"initParticleSystem\n";
	system("pause");
#endif // _DEBUG
    initParams();

#ifdef _DEBUG
	system("pause");
	//std::clog<<"typyCzastek.size() "<<typyCzastek.size()<<"\n";
#endif

    if (!g_refFile)
    {
        initMenus();
    }

    if (benchmark || g_refFile || save)
    {
        if (numIterations <= 0)
        {
            numIterations = 300;
        }

		//printf("timestep: %f\n",timestep);
        runBenchmark(numIterations, argv[0]);
    }
    else
    {
        preasureGraph=new GLgraph();
        preasureGraph->setDataVector(&preasureVector);

        glutDisplayFunc(display);
        glutReshapeFunc(reshape);
        glutMouseFunc(mouse);
        glutMotionFunc(motion);
        glutKeyboardFunc(key);
        glutSpecialFunc(special);
        glutIdleFunc(idle);

        atexit(cleanup);

        glutMainLoop();
    }

    if (psystem)
    {
        delete psystem;
    }

    cudaDeviceReset();
    exit(g_TotalErrors > 0 ? EXIT_FAILURE : EXIT_SUCCESS);
}

