#include "miman_config.h"
#include "miman_model.h"
#include "miman_orbital.h"

extern StateCheckUnit State;
extern Console console;
Shader * ShaderSatellites2D[32768];
Rectangle * Satellites2D[32768];
Shader * ShaderSatRange2D[32768];
Circle * SatRange2D[32768];
Shader * ShaderSatpoint[32768];
Circle * Satpoint[32768];
Shader * ShaderFatellites2D;
Rectangle * Fatellites2D;
Shader * ShaderFatRange2D;
Circle * FatRange2D;
Shader * ShaderFatpoint;
Circle * Fatpoint;

Shader * ShaderFootprint;
Spline * Footprint;


GLint TextureFromFile( const char *path, std::string directory )
{
    //Generate texture ID and load texture data
    std::string filename = std::string( path );
    filename = directory + '/' + filename;
    GLuint textureID;
    glGenTextures( 1, &textureID );

    int width, height, nrChannels;
    unsigned char *image = stbi_load(filename.c_str(), &width, &height, &nrChannels, 0);
    if(State.Debugmode)
    {
        if (!image) {
        printf("texture %s loading error ... \n", filename.c_str());
        }
        else printf("texture %s loaded\n", filename.c_str());
    }
    

    GLenum format;
    if (nrChannels == 1) format = GL_RED;
    else if (nrChannels == 3) format = GL_RGB;
    else if (nrChannels == 4) format = GL_RGBA;

    glBindTexture(GL_TEXTURE_2D, textureID);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // OpenGL requires a texture image's dimension should be divided by 4. This function solve the problem

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(image);

    return textureID;
}

double SetLatitudeScale(SatelliteObject * Sat)
{
    double R = 6371.0f;
    double theta = acos(R / (Sat->geo.altitude + R));
    return theta;
}


void SatelliteModelInitialize(int index)
{
    if(index == -1)
    {
        if(strlen(State.Fatellites->Name()) != 0 && State.NowTracking)
        {
            Fatellites2D = new Rectangle(false);
            ShaderFatellites2D = new Shader("./src/shader/3DTexture.vs", "./src/shader/3DTexture.fs", NULL);
            Fatellites2D->XYZScale(0.02, 0.02, 0.0);
            FatRange2D = new Circle();
            ShaderFatRange2D = new Shader("./src/shader/3DRGB.vs", "./src/shader/3DRGB.fs", NULL);
            Fatpoint = new Circle();
            ShaderFatpoint = new Shader("./src/shader/3DRGB.vs", "./src/shader/3DRGB.fs", NULL);
        }
    }
    else
    {
        if(strlen(State.Satellites[index]->Name()) != 0 && State.NowTracking)
        {
            Satellites2D[index] = new Rectangle(false);
            ShaderSatellites2D[index] = new Shader("./src/shader/3DTexture.vs", "./src/shader/3DTexture.fs", NULL);
            Satellites2D[index]->XYZScale(0.02, 0.02, 0.0);
            SatRange2D[index] = new Circle();
            ShaderSatRange2D[index] = new Shader("./src/shader/3DRGB.vs", "./src/shader/3DRGB.fs", NULL);
            Satpoint[index] = new Circle();
            ShaderSatpoint[index] = new Shader("./src/shader/3DRGB.vs", "./src/shader/3DRGB.fs", NULL);
        }
    }
    
}

void SatelliteModelDelete(int index)
{
    if(State.Satellites[index] != NULL)
    {
        delete Satellites2D[index];
        Satellites2D[index] = NULL;
        delete ShaderSatellites2D[index];
        ShaderSatellites2D[index] = NULL;
        delete SatRange2D[index];
        SatRange2D[index] == NULL;
        delete ShaderSatRange2D[index];
        ShaderSatRange2D[index] == NULL;
        delete Satpoint[index];
        Satpoint[index] = NULL;
        delete ShaderSatpoint[index];
        ShaderSatpoint[index] = NULL;
    }
    if(State.Fatellites != NULL)
    {
        delete Fatellites2D;
        Fatellites2D = NULL;
        delete ShaderFatellites2D;
        ShaderFatellites2D = NULL;
        delete FatRange2D;
        FatRange2D == NULL;
        delete ShaderFatRange2D;
        ShaderFatRange2D == NULL;
        delete Fatpoint;
        Fatpoint = NULL;
        delete ShaderFatpoint;
        ShaderFatpoint = NULL;
    }
}



void SatDisplayer(GLint * SatTexture)
{
    if(State.NowTracking)
    {
        for(int index = 0; index < sizeof(State.Satellites) / sizeof(SatelliteObject *); index++)
        {
            if(State.Satellites[index] == NULL)
                break;
            if(State.Satellites[index]->use == false)
                continue;

            ShaderSatRange2D[index]->use();
            // SatRange2D[index]->Setradius(0.5f);
            SatRange2D[index]->Setradius((float)SetLatitudeScale(State.Satellites[index]) / (M_PI / 2));
            SatRange2D[index]->Mercator((float)State.Satellites[index]->geo.latitude, (float)State.Satellites[index]->geo.longitude);
            SatRange2D[index]->Setcolor(1.0f, 1.0f, 1.0f);
            SatRange2D[index]->Update();
            ShaderSatRange2D[index]->setFloat("alpha", 0.25f);
            SatRange2D[index]->drawFan(ShaderSatRange2D[index]);
            SatRange2D[index]->Setcolor(1.0f, 1.0f, 0.0f);
            SatRange2D[index]->Update();
            ShaderSatRange2D[index]->setFloat("alpha", 1.0f);
            SatRange2D[index]->drawLoop(ShaderSatRange2D[index]);
            
            Satellites2D[index]->XYTranslate(State.Satellites[index]->DisplayPoint_Longitude(), State.Satellites[index]->DisplayPoint_Latitude());
            ShaderSatellites2D[index]->use();
            glActiveTexture(GL_TEXTURE0 + 2);
            glBindTexture(GL_TEXTURE_2D, *SatTexture);
            ShaderSatellites2D[index]->setInt("texture1", 2);
            Satellites2D[index]->Draw(ShaderSatellites2D[index]);
        }
        if(State.Fatellites != NULL && State.Fatellites->use == true)
        {
            ShaderFatRange2D->use();
            // FatRange2D]->Setradius(0.5f);
            FatRange2D->Setradius((float)SetLatitudeScale(State.Fatellites) / (M_PI / 2));
            FatRange2D->Mercator((float)State.Fatellites->geo.latitude, (float)State.Fatellites->geo.longitude);
            FatRange2D->Setcolor(1.0f, 1.0f, 1.0f);
            FatRange2D->Update();
            ShaderFatRange2D->setFloat("alpha", 0.25f);
            FatRange2D->drawFan(ShaderFatRange2D);
            FatRange2D->Setcolor(1.0f, 1.0f, 0.0f);
            FatRange2D->Update();
            ShaderFatRange2D->setFloat("alpha", 1.0f);
            FatRange2D->drawLoop(ShaderFatRange2D);
            
            Fatellites2D->XYTranslate(State.Fatellites->DisplayPoint_Longitude(), State.Fatellites->DisplayPoint_Latitude());
            ShaderFatellites2D->use();
            glActiveTexture(GL_TEXTURE0 + 2);
            glBindTexture(GL_TEXTURE_2D, *SatTexture);
            ShaderFatellites2D->setInt("texture1", 2);
            Fatellites2D->Draw(ShaderFatellites2D);
        }

            
    }
}

void SatpointDisplayer()
{
    if(State.NowTracking)
    {
        for(int i = 0; i < sizeof(State.Satellites) / sizeof(SatelliteObject *); i++)
        {
            if(State.Satellites[i] == NULL)
                break;
            if(State.Satellites[i]->use == false)
                continue;
            if(State.Satellites[i]->topo.elevation < 0.0f)
                continue;
            Satpoint[i]->Setradius(0.02f);
            Satpoint[i]->Setcolor(1.000f, 1.000f, 0.000f);
            Satpoint[i]->translate(0.9f * (float)(sin(State.Satellites[i]->topo.azimuth) * (M_PI / 2 - State.Satellites[i]->topo.elevation) / (M_PI / 2)), 0.9f * (float)(cos(State.Satellites[i]->topo.azimuth) * (M_PI / 2 - State.Satellites[i]->topo.elevation) / (M_PI / 2)), 0);
            Satpoint[i]->Update();
            ShaderSatpoint[i]->use();
            ShaderSatpoint[i]->setFloat("alpha", 1.0f);
            Satpoint[i]->drawLoop(ShaderSatpoint[i]);
        }
        if(State.Fatellites != NULL && State.Fatellites->use != false && State.Fatellites->topo.elevation >= 0.0f)
        {
            Fatpoint->Setradius(0.02f);
            Fatpoint->Setcolor(1.000f, 1.000f, 0.000f);
            Fatpoint->translate(0.9f * (float)(sin(State.Fatellites->topo.azimuth) * (M_PI / 2 - State.Fatellites->topo.elevation) / (M_PI / 2)), 0.9f * (float)(cos(State.Fatellites->topo.azimuth) * (M_PI / 2 - State.Fatellites->topo.elevation) / (M_PI / 2)), 0);
            Fatpoint->Update();
            ShaderFatpoint->use();
            ShaderFatpoint->setFloat("alpha", 1.0f);
            Fatpoint->drawLoop(ShaderFatpoint);
        }
    }
    
}

void PathInitializer()
{
    ShaderFootprint = new Shader("./src/shader/3DRGB.vs", "./src/shader/3DRGB.fs", NULL);
    Footprint = new Spline();
}

void PathGenerator(int Selected)
{


        if (Selected < 0 || Selected >= 8192) return;
    if (State.Satellites[Selected] == NULL) return;

    if (Footprint == NULL) return;
    if (Footprint->spline_vertices == NULL) return;


    
    if(Selected == -1)
    {
        State.Fatellites->GeneratePath();
        if(State.NowTracking && Selected > -2)
        {
            
            // int k = 0;
            // for(int i = 0; i < 1024; i++)
            // {
            //     Footprint->spline_vertices[k] = 0.9f * (float)(sin(State.Fatellites->path_az[i]) * (M_PI / 2 - State.Fatellites->path_el[i]) / (M_PI / 2));
            //     Footprint->spline_vertices[k + 1] = 0.9f * (float)(cos(State.Fatellites->path_az[i]) * (M_PI / 2 - State.Fatellites->path_el[i]) / (M_PI / 2));
            //     Footprint->spline_vertices[k + 2] = 0;
            //     k += 3;
                
            // }
            // Footprint->Setcolor(1.000f, 0.447f, 0.463f);
        }
    }
    else
    {
        State.Satellites[Selected]->GeneratePath();
        if(State.NowTracking && Selected > -2)
        {
            
            int k = 0;
            for(int i = 0; i < 1024; i++)
            {
                Footprint->spline_vertices[k] = 0.9f * (float)(sin(State.Satellites[Selected]->path_az[i]) * (M_PI / 2 - State.Satellites[Selected]->path_el[i]) / (M_PI / 2));
                Footprint->spline_vertices[k + 1] = 0.9f * (float)(cos(State.Satellites[Selected]->path_az[i]) * (M_PI / 2 - State.Satellites[Selected]->path_el[i]) / (M_PI / 2));
                Footprint->spline_vertices[k + 2] = 0;

                if(i >= 1)
                {
                    if(abs(State.Satellites[Selected]->path_az[i] - State.Satellites[Selected]->path_az[i - 1]) > 2.0f / RAD_TO_DEG || abs(State.Satellites[Selected]->path_el[i] - State.Satellites[Selected]->path_el[i - 1]) > 2.0f / RAD_TO_DEG)
                    {
                        Footprint->spline_vertices[k] = Footprint->spline_vertices[k - 3];
                        Footprint->spline_vertices[k + 1] = Footprint->spline_vertices[k - 2];
                        console.AddLog("[DEBUG]##PathFinder Error. Calculate again this spline vertices. index : %d, %d, %d", k, k + 1, k + 2);
                        // printf("Az : %lf, El : %lf, Az_passed : %lf, El_passed : %lf\n", State.Satellites[Selected]->path_az[i], State.Satellites[Selected]->path_el[i], State.Satellites[Selected]->path_az[i-1], State.Satellites[Selected]->path_el[i-1]);
                
                    }
                }
                k += 3;
                
            }
            Footprint->Setcolor(1.000f, 0.447f, 0.463f);
        }
    }
    
}

void PathDisplayer(int Selected)
{
    if(Selected > -2)
    {
        ShaderFootprint->use();
        Footprint->Update();
        ShaderFootprint->setFloat("alpha", 1.0f);
        Footprint->draw(ShaderFootprint);
    }
    
}

void PathDelete()
{
    delete ShaderFootprint;
    ShaderFootprint = NULL;
    delete Footprint;
    Footprint = NULL;
}