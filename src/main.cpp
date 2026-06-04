//PROJECT       MIMAN_GS
//VERSION       0.0.0
//AUTHORS       MIMAN COMS/GS Team
//REFERENCE     CANYVAL_GS

//Define Envs
#define STB_IMAGE_IMPLEMENTATION
#define READ_BUF_SIZE   1024




//Custom Headers
#include "miman_config.h"
#include "miman_orbital.h"
#include "miman_imgui.h"
#include "miman_radial.h"
#include "miman_coms.h"
#include "miman_csp.h"
#include "miman_model.h"
#include "miman_s_control.h"
#include "miman_autopilot.h"
#include "miman_ftp.h"

#undef CSP_USE_RDP

FILE *log_ptr;

////Temp
StateCheckUnit State;
Command * command;
Setup * setup = new Setup;
dlreqdata * request = (dlreqdata *)malloc(sizeof(dlreqdata));
bool Showstate = true;

////GUI
extern Console console;
extern Observer Yonsei;
extern int Requested;
extern Sband * sgs;
volatile sig_atomic_t g_shutdown_requested = 0;
////Thread
/* pthread index */
// 0 : Radio Control
// 1 : Rotator Control
// 2 : Rotator Read
// 3 : TLE Trackings
// 4 : TX scheduler
// 5 : Calibration
// 6 : AMP Timer
// 7 : Doppler Shift Correction
// 8 : FTP downlink/uplink
// 9 : 
//10 : Signaltest
//11 : TRX Controller
//12 : Radio Controller
//13 : AutoPilot
//14 : 
//15 : TLE load
pthread_t p_thread[16];
pthread_mutex_t conn_lock = PTHREAD_MUTEX_INITIALIZER;

csp_debug_level_t debug_level;

#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

void shutdown_handler(int sig) {
    g_shutdown_requested = 1;
}


void Initialize_ConfigSettings()
{
    memset(setup->Transciever_devname, 0, sizeof(setup->Transciever_devname));
    memset(setup->Switch_devname, 0, sizeof(setup->Switch_devname));
    memset(setup->Rotator_devname, 0, sizeof(setup->Rotator_devname));
    memset(setup->S_Username, 0, sizeof(setup->S_Username));
    memset(setup->S_Address, 0, sizeof(setup->S_Address));
    memset(setup->S_passwd, 0, sizeof(setup->S_passwd));
    sprintf(setup->Transciever_devname, "/dev/GS102");
    sprintf(setup->Switch_devname, "/dev/KTA223");
    sprintf(setup->Rotator_devname, "/dev/GS232B");
    sprintf(setup->S_Username, "s-band");
    sprintf(setup->S_Address, "165.132.28.235");
    sprintf(setup->S_passwd, "qwe123");
    setup->Transceiver_baud = 500000; // These three baud rate is between computer & GS machine - USART
    setup->Rotator_baud = 9600; 
    setup->Switch_baud = 115200; 
    setup->gs100_node = 20;
    setup->kiss_node = 8;
    setup->obc_node = 3;
    setup->ax100_node = 5;
    setup->default_freq = 436500000; // UHF freq?
    setup->default_timeout = 1000;
    setup->pingsize = 4;
    setup->guard_delay = 150;
    setup->queue_delay = 150;     //255
}

char * fontfile = "../bin/font/Inconsolata.ttf";

//Main
int main(int, char**)
{


    Initialize_ConfigSettings();
    Initialize_TLESettings();
    Initialize_CMDLabels();
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    //// Create Main window with graphics context
    GLFWwindow * window = glfwCreateWindow(1920, 1080, "BEE-1000 Ground Station", NULL, NULL);
    if (window == NULL)
        return -1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    
    
    if(glewInit() != GLEW_OK)
        return -1;


    //// ImGUI Initialization
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    // Setup Dear ImGui style
    ImGuiStyle * style = &ImGui::GetStyle();
    ImGuiCustomStyle(style);
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
    // Load Fonts
    io.Fonts->AddFontFromFileTTF(fontfile, 30.0f);
    
    ImVec4 clear_color = ImVec4(0.925f, 0.870f, 0.945f, 1.00f);  
    int WinWidth;
    int WinHeight;
    float ReactiveWidth;
    float ReactiveHeight;

    //MIMAN GS
    State.Fatellites = new SatelliteObject(Yonsei);
    for(int index = 0 ; index < sizeof(State.Satellites) / sizeof(SatelliteObject *); index++)
        State.Satellites[index] = new SatelliteObject(Yonsei);
    for(int index = 0; index < 2; index++)
        State.ChunkState = new chunkstate_t;
    
    // Shader * ShaderPolarmap2D_bg = new Shader("./src/shader/3DRGB.vs", "./src/shader/3DRGB.fs", NULL);
    // Shader * ShaderPolarmap2D_el0 = new Shader("./src/shader/3DRGB.vs", "./src/shader/3DRGB.fs", NULL);
    // Shader * ShaderPolarmap2D_el30 = new Shader("./src/shader/3DRGB.vs", "./src/shader/3DRGB.fs", NULL);
    // Shader * ShaderPolarmap2D_el60 = new Shader("./src/shader/3DRGB.vs", "./src/shader/3DRGB.fs", NULL);
    // Shader * ShaderPolarmap2D_azNS = new Shader("./src/shader/3DRGB.vs", "./src/shader/3DRGB.fs", NULL);
    // Shader * ShaderPolarmap2D_azEW = new Shader("./src/shader/3DRGB.vs", "./src/shader/3DRGB.fs", NULL);

    // Rectangle * Worldmap2D = new Rectangle();
    // Rectangle * Polarmap2D_bg = new Rectangle();
    // Circle * Polarmap2D_el0 = new Circle();
    // Circle * Polarmap2D_el30 = new Circle();
    // Circle * Polarmap2D_el60 = new Circle();
    // StraightLine * Polarmap2D_azNS = new StraightLine();
    // StraightLine * Polarmap2D_azEW = new StraightLine();

    // PathInitializer();
    

    

    // GLint TextureWorldmap2D = TextureFromFile("Earth3686.jpg", "./bin/image");
    // GLint TextureSatellites2D = TextureFromFile("Satellite.png", "./bin/image");

    // Polarmap2D_el0->Setcolor(0.00f, 0.65f, 0.00f);
    // Polarmap2D_el0->scale(0.9f);
    // Polarmap2D_el30->Setcolor(0.00f, 0.65f, 0.00f);
    // Polarmap2D_el30->scale(0.6f);
    // Polarmap2D_el60->Setcolor(0.00f, 0.65f, 0.00f);
    // Polarmap2D_el60->scale(0.3f);

    // Polarmap2D_azNS->Setpoint(0.0f, 0.95f, 0.0f, 0.0f, -0.95f, 0.0f);
    // Polarmap2D_azNS->Setcolor(0.00f, 0.65f, 0.00f);
    // Polarmap2D_azEW->Setpoint(0.95f, 0.0f, 0.0f, -0.95f, 0.0f, 0.0f);
    // Polarmap2D_azEW->Setcolor(0.00f, 0.65f, 0.00f);

    State.GUI = true;
    bool GUIneedshutdown = true;

    State.gslistup = new gsftp_listup_t;
    State.gsmove = new gsftp_move_t;
    State.gscopy = new gsftp_copy_t;
    State.gsremove = new gsftp_remove_t;
    State.gsmkdir = new gsftp_mkdir_t;
    State.gsrmdir = new gsftp_rmdir_t;
    memset(State.gslistup, 0, sizeof(gsftp_listup_t));
    memset(State.gsmove, 0, sizeof(gsftp_move_t));
    memset(State.gscopy, 0, sizeof(gsftp_copy_t));
    memset(State.gsremove, 0, sizeof(gsftp_remove_t));
    memset(State.gsmkdir, 0, sizeof(gsftp_mkdir_t));
    memset(State.gsrmdir, 0, sizeof(gsftp_rmdir_t));

    command = new Command;
    memset(command, 0, sizeof(Command));

    csp_debug_hook_set((csp_debug_hook_func_t)csp_debug_callback);

    char logfilename[128] ={0,};
    time_t tmtime = time(0);
    struct tm * local = localtime(&tmtime);
    sprintf(logfilename, "../data/BEE_TC_Log/log--%04d-%02d-%02d-%02d-%02d-%02d--", local->tm_year+1900, local->tm_mon+1, local->tm_mday,local->tm_hour, local->tm_min, local->tm_sec);
    log_ptr = fopen(logfilename, "wb");
    if(log_ptr == NULL) {
        printf("Invalid log file pointer.\n");
    }
    while(State.AllThread)
    {

        if (g_shutdown_requested) {
        // 시그널로 인한 종료 요청 들어옴
        State.GUI = false;       // GUI 루프 정지
        State.AllThread = false; // while 탈출
    }
        if(State.GUI)
        {   
            GUIneedshutdown = true;
            State.GUI = !glfwWindowShouldClose(window);
            glfwPollEvents();
            // Start the Dear ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGuiIO& io = ImGui::GetIO();
            ImDrawList* bg = ImGui::GetBackgroundDrawList();




            bg->AddRectFilled(
                ImVec2(0, 0),
                io.DisplaySize,
                ImColor(0.141f, 0.364f, 0.376f, 1.00f)  // Colorset_1과 동일
            );


            glfwGetWindowSize(window, &WinWidth, &WinHeight);
            ReactiveWidth = (float)WinWidth / DISPLAY_WINDWIDTH;
            ReactiveHeight = (float)WinHeight / DISPLAY_WINDHEIGHT;
            now_rx_bytes_update();
            
            //Get Current Window Size

            
            ImGui_MainMenu();

            // Show Console Window
            console.ChangeWindowSize(DISPLAY_COL1 * ReactiveWidth, MENUPADDING, DISPLAY_CONSOLE_W * ReactiveWidth, ReactiveHeight * DISPLAY_CONSOLE_H);
            console.Draw("General Console", &Showstate, ReactiveHeight * 0.6);

            //Show Beacon State Window
            ImGui::SetNextWindowPos(ImVec2(DISPLAY_COL3 * ReactiveWidth, MENUPADDING), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(DISPLAY_STATE_W * ReactiveWidth, ReactiveHeight * DISPLAY_STATE_H), ImGuiCond_Always);
            ImGui_BeaconWindow(ReactiveHeight * 0.65);

            //Show Control Window
            ImGui::SetNextWindowPos(ImVec2(DISPLAY_COL1 * ReactiveWidth, DISPLAY_ROW1 * ReactiveHeight), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(DISPLAY_CONTROL_W * ReactiveWidth, DISPLAY_CONTROL_H * ReactiveHeight), ImGuiCond_Always);
            ImGui_ControlWindow(ReactiveHeight * 0.7);

            //Show Rotator Window
            ImGui::SetNextWindowPos(ImVec2(0, ReactiveHeight * DISPLAY_ROW1), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(DISPLAY_ROTAT_W * ReactiveWidth, ReactiveHeight * DISPLAY_ROTAT_H ), ImGuiCond_Always);
            ImGui_RotatorWindow(ReactiveHeight * 0.5);

            // Show Tracking Window
            ImGui::SetNextWindowPos(ImVec2(0 * ReactiveWidth, DISPLAY_ROW3 * ReactiveHeight), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(DISPLAY_TRACK_W * ReactiveWidth, DISPLAY_TRACK_H * ReactiveHeight), ImGuiCond_Always);
            ImGui_TrackWindow(ReactiveHeight * 0.5);


            if (State.Display_Autopilot)
            {
                float x = DISPLAY_COL1 * ReactiveWidth *1.924;
                float y = DISPLAY_ROW2 * ReactiveHeight;
                float w = DISPLAY_MODEL_W * ReactiveWidth/2.38;
                float h = DISPLAY_MODEL_H * ReactiveHeight;

                ImGui::SetNextWindowPos(ImVec2(x, y), ImGuiCond_Always);
                ImGui::SetNextWindowSize(ImVec2(w, h), ImGuiCond_Always);
                ImGui_AutoPilotWindow(ReactiveHeight * 0.5f);
            }

            //Show Frequency Window
            ImGui::SetNextWindowPos(ImVec2(0, MENUPADDING), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(DISPLAY_FREQ_W * ReactiveWidth, ReactiveHeight * DISPLAY_FREQ_H), ImGuiCond_Always);
            ImGui_FrequencyWindow(ReactiveHeight * 0.5);
            
            //Show Command Window
            ImGui::SetNextWindowPos(ImVec2(0 * ReactiveWidth, DISPLAY_ROW2 * ReactiveHeight), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(DISPLAY_COMMAND_W * ReactiveWidth, DISPLAY_COMMAND_H * ReactiveHeight), ImGuiCond_Always);
            ImGui_CommandWindow(ReactiveHeight * 0.5);


            //Display Popup Windows
            if (State.Display_paramt0)
                State.Display_paramt0 = popup_param_table0();
            
            if (State.Display_paramt1)
                State.Display_paramt1 = popup_param_table1();

            if (State.Display_paramt5)
                State.Display_paramt5 = popup_param_table5();

            // if (State.Display_CMD)
            //     State.Display_CMD = popup_cmd();

            if (State.Display_Setup)
                State.Display_Setup = popup_setup(setup);

            if(State.Display_load)
                State.Display_load = popup_load();
            else
            {
                if (State.Display_TLE)
                    State.Display_TLE = popup_tle();
            }

            if (State.Display_FDS)
                State.Display_FDS = popup_fds();

            if (State.Display_Satinfo)
                State.Display_Satinfo = popup_satinfo(Requested);

            if (State.Display_Sband)
                State.Display_Sband = popup_s_band();

            if(State.InitializeRequired)
            {
                //Switch Box
                if(init_switch(setup->Switch_devname, setup->Switch_baud) < 0)
                {
                    console.AddLog("[ERROR]##Connect to Switch Box..............Failed.");
                    State.SwitchConnection = false;
                }
                else
                {
                    console.AddLog("[OK]##Connect to Switch Box.............Success.");
                    State.SwitchConnection = true;
                }


                //Rotator
                if(init_rotator() < 0)
                {
                    console.AddLog("[ERROR]##Connect to Rotator.................Failed.");
                }
                else
                {
                    State.RotatorConnection = true;
                    State.RotatorInitialize = true;
                    console.AddLog("[OK]##Connect to Rotator................Success.");
                }
                if(init_transceiver() < 0)
                {
                    console.AddLog("[ERROR]##Connect to GS100...................Failed.");
                    State.GS100_Connection = false;
                }
                else
                {
                    // pthread_join(p_thread[11], NULL);
                    // pthread_join(p_thread[12], NULL);
                    // pthread_join(p_thread[13], NULL);
                    console.AddLog("[OK]##Connect to GS100..................Success.");
                    State.GS100_Connection = true;
                    
                    pthread_create(&p_thread[12], NULL, RadInitialize, (void *)setup);
                    State.TRx_mode = true;
                    State.downlink_mode = true;
                    pthread_create(&p_thread[11], NULL, TRxController, NULL); 


                    signal(SIGINT, shutdown_handler);
                    
                }
                if(State.SbandUse)
                {
                    if(init_sband(setup) < 0)
                    {
                        console.AddLog("[ERROR]##Connect to S-band GS...............Failed.");
                    }
                    else
                    {
                        console.AddLog("[OK]##Connect to S-band GS...............Success.");
                    }
                }
                State.InitializeRequired = false;
                pthread_create(&p_thread[3], NULL, TrackingSatellites, NULL);
                pthread_create(&p_thread[2], NULL, RotatorReadInfo, NULL);
                
                
            }

            if(State.ModelRefreshRequired)
            {
                SatelliteModelInitialize(GetNowTracking());
                State.ModelRefreshRequired = false;
            }

            // Rendering
            ImGui_ClearColorBuf(window, clear_color);
            
            
            // Exclude about 35 pixels in map's height not to invade tab bar
            // Note that unlike ImGui, origin of openGL screen coordinate locates at bottom left corner
            // (top left for ImGui)
            ImGui::Render();
            glViewport(DISPLAY_COL1 * ReactiveWidth, DISPLAY_WINDHEIGHT * ReactiveHeight - (DISPLAY_ROW2 * ReactiveHeight), DISPLAY_MODEL_W * ReactiveWidth, ReactiveHeight * DISPLAY_MODEL_H );
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // Clear buffers
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_BLEND);  // Enable transparency

            // ShaderWorldmap2D->use();
            // glActiveTexture(GL_TEXTURE0 + 1);
            // glBindTexture(GL_TEXTURE_2D, TextureWorldmap2D);
            // ShaderWorldmap2D->setInt("texture1", 1);
            // Worldmap2D->Draw(ShaderWorldmap2D);
            // // Sattext2D->RenderText("Test", 100.0, 100.0, 1.0, glm::vec3(1.0f, 1.0f, 1.0f));

            // SatDisplayer(&TextureSatellites2D);

            // glViewport(0, DISPLAY_WINDHEIGHT * ReactiveHeight - (DISPLAY_ROW3 * ReactiveHeight), DISPLAY_COL1 * ReactiveWidth, ReactiveHeight * (DISPLAY_ROW3 - DISPLAY_ROW2) );
            // // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // Clear buffers
            // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            // glEnable(GL_BLEND);  // Enable transparency

            // ShaderPolarmap2D_el0->use();
            // Polarmap2D_el0->Update();
            // ShaderPolarmap2D_el60->setFloat("alpha", 1.0f);
            // Polarmap2D_el0->drawLoop(ShaderPolarmap2D_el0);

            // ShaderPolarmap2D_el30->use();
            // Polarmap2D_el30->Update();
            // ShaderPolarmap2D_el30->setFloat("alpha", 1.0f);
            // Polarmap2D_el30->drawLoop(ShaderPolarmap2D_el30);

            // ShaderPolarmap2D_el60->use();
            // Polarmap2D_el60->Update();
            // ShaderPolarmap2D_el0->setFloat("alpha", 1.0f);
            // Polarmap2D_el60->drawLoop(ShaderPolarmap2D_el60);

            // ShaderPolarmap2D_azNS->use();
            // Polarmap2D_azNS->Update();
            // ShaderPolarmap2D_azNS->setFloat("alpha", 1.0f);
            // Polarmap2D_azNS->draw(ShaderPolarmap2D_azNS);

            // ShaderPolarmap2D_azEW->use();
            // Polarmap2D_azEW->Update();
            // ShaderPolarmap2D_azEW->setFloat("alpha", 1.0f);
            // Polarmap2D_azEW->draw(ShaderPolarmap2D_azEW);

            //PathDisplayer(GetNowTracking());
            //SatpointDisplayer();


            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            glfwSwapBuffers(window);
            // sleep(1/MIMAN_FRAMERATE);
            continue;
        }
        
        // Cleanup
        if(GUIneedshutdown)
        {
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
            GUIneedshutdown = false;
        }
        

        fin_rotator();
        fin_transceiver();

        if (State.AllThread) // "Start" at popup
        {
            printf("Ending procedure...\n");
            State.TRx_mode = false;
            State.downlink_mode = false;
            State.uplink_mode = false;
            State.ftp_mode = false;
            State.Engage = false;
            State.Doppler = false;
            State.GS100_Connection =false;
            State.NowTracking = false;
            State.TrackThread = false;
            State.RotatorReading = false;
            
            State.InitializeRequired = true;
            State.Autopilot = false;
            printf("Joined thread : \n");
            for(int i = 0; i < 16; i++)
            {
                pthread_join(p_thread[i], NULL);
                printf("%d\t", i);
            }
            printf("\n");
            
        }

        if(State.Autopilot)
        {
            sleep(10);
        }
        else
        {
            State.AllThread = false;
        }
        
    }

    

    delete setup; 
    setup = NULL;
    // delete ShaderWorldmap2D;
    // ShaderWorldmap2D = NULL;
    // delete Worldmap2D;
    // Worldmap2D = NULL;
    // delete ShaderPolarmap2D_el0;
    // ShaderPolarmap2D_el0 = NULL;
    // delete ShaderPolarmap2D_el30;
    // ShaderPolarmap2D_el30 = NULL;
    // delete ShaderPolarmap2D_el60;
    // ShaderPolarmap2D_el60 = NULL;
    // delete ShaderPolarmap2D_azNS;
    // ShaderPolarmap2D_azNS = NULL;
    // delete ShaderPolarmap2D_azEW;
    // ShaderPolarmap2D_azEW = NULL;
    // delete Polarmap2D_el0;
    // Polarmap2D_el0 = NULL;
    // delete Polarmap2D_el30;
    // Polarmap2D_el30 = NULL;
    // delete Polarmap2D_el60;
    // Polarmap2D_el60 = NULL;
    // delete Polarmap2D_azNS;
    // Polarmap2D_azNS = NULL;
    // delete Polarmap2D_azEW;
    // Polarmap2D_azEW = NULL;
    free(command);
    for(int i = 0; i < sizeof(State.Satellites) / sizeof(SatelliteObject *); i++)
    {
        delete State.Satellites[i];
        State.Satellites[i] = NULL;
    }
    PathDelete();
    if(sgs != NULL)
        sgs->finSband();
    sleep(0.5);
fclose(log_ptr);
system("../amp/ampoff");
printf("Finish BEE-1000 GS.\n");
return 0;

    
}
