#include <fstream>

#include <GL/glew.h>
#include <GL/freeglut.h>

bool rotateTable = false;
float angle = 0.0f;
float rotationSpeed = 2.0f; // Kecepatan rotasi

// Deklarasi fungsi Mouse agar object 3d dapat diputar-putar
float xrot = 0;
float yrot = 0;
float xdiff = 0;
float ydiff = 0;
bool mouseDown = false;
bool lampOn = true; // Initial state of the lamp
bool idles = false;
float changeMode = 0;

static unsigned int texture[1];

// Warna-warna dalam satu variabel
GLfloat backgroundColor[] = {0.0, 0.0, 0.0};   // Warna latar belakang (hitam)
GLfloat tableColor1[] = {250.0, 245.0, 239.0}; // Warna 1
GLfloat tableColor2[] = {0.36, 0.18, 0.00};    // Warna 2

int wKeyPressCount = 0; // Jumlah penekanan tombol 'w'

int cKeyPressCount = 0; // Jumlah penekanan tombol 'c'

GLfloat separationDistance = 20.0;

struct imageFile
{
    int width;
    int height;
    unsigned char *data;
};

imageFile *getBMP(std::string fileName)
{
    int offset, // No. of bytes to start of image data in input BMP file.
        w,      // Width in pixels of input BMP file.
        h;      // Height in pixels of input BMP file.

    // Initialize imageFile objects.
    imageFile *tempStore = new imageFile; // Temporary storage.
    imageFile *outRGB = new imageFile;    // RGB output file.
    imageFile *outRGBA = new imageFile;   // RGBA output file.

    // Initialize input stream.
    std::ifstream inFile(fileName.c_str(), std::ios::binary);

    // Get start point of image data in input BMP file.
    inFile.seekg(10);
    inFile.read((char *)&offset, 4);

    // Get image width and height.
    inFile.seekg(18);
    inFile.read((char *)&w, 4);
    inFile.read((char *)&h, 4);

    // Determine the length of padding of the pixel rows
    // (each pixel row of a BMP file is 4-byte aligned by padding with zero bytes).
    int padding = (3 * w) % 4 ? 4 - (3 * w) % 4 : 0;

    // Allocate storage for temporary input file, read in image data from the BMP file, close input stream.
    tempStore->data = new unsigned char[(3 * w + padding) * h];
    inFile.seekg(offset);
    inFile.read((char *)tempStore->data, (3 * w + padding) * h);
    inFile.close();

    // Set image width and height and allocate storage for image in output RGB file.
    outRGB->width = w;
    outRGB->height = h;
    outRGB->data = new unsigned char[3 * w * h];

    // Copy data from temporary input file to output RGB file adjusting for padding and performing BGR to RGB conversion.
    int tempStorePos = 0;
    int outRGBpos = 0;
    for (int j = 0; j < h; j++)
        for (int i = 0; i < 3 * w; i += 3)
        {
            tempStorePos = (3 * w + padding) * j + i;
            outRGBpos = 3 * w * j + i;
            outRGB->data[outRGBpos] = tempStore->data[tempStorePos + 2];
            outRGB->data[outRGBpos + 1] = tempStore->data[tempStorePos + 1];
            outRGB->data[outRGBpos + 2] = tempStore->data[tempStorePos];
        }

    // Set image width and height and allocate storage for image in output RGBA file.
    outRGBA->width = w;
    outRGBA->height = h;
    outRGBA->data = new unsigned char[4 * w * h];

    // Copy image data from output RGB file to output RGBA file, setting all A values to 1.
    for (int j = 0; j < 4 * w * h; j += 4)
    {
        outRGBA->data[j] = outRGB->data[(j / 4) * 3];
        outRGBA->data[j + 1] = outRGB->data[(j / 4) * 3 + 1];
        outRGBA->data[j + 2] = outRGB->data[(j / 4) * 3 + 2];
        outRGBA->data[j + 3] = 0xFF;
    }

    // Release temporary storage and the output RGB file and return the RGBA version.
    delete[] tempStore;
    delete[] outRGB;
    return outRGBA;
}

void loadTextures()
{
    imageFile *image[1];
    image[0] = getBMP("Textures/Logo.bmp");

    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image[0]->width, image[0]->height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, image[0]->data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

// Deklarasi pengaturan lembar kerja agar object 3d yang dibuat saat diputar atau di geser tidak kemana mana
void ukur(int lebar, int tinggi)
{
    if (tinggi == 0)
        tinggi = 1;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, lebar / tinggi, 5, 450);
    glTranslatef(0, 0, -300); // jarak object dari lembar kerja
    glMatrixMode(GL_MODELVIEW);
}

void myinit(void)
{
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glMatrixMode(GL_PROJECTION);
    glEnable(GL_DEPTH_TEST);

    // Create texture ids.
    glGenTextures(1, texture);

    // Load texture.
    loadTextures();

    // Turn on OpenGL texturing.
    glEnable(GL_TEXTURE_2D);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glMatrixMode(GL_MODELVIEW);
    glPointSize(10.0);
    glLineWidth(7.0f);
}

// Fungsi mouse
void mouse(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        mouseDown = true;
        xdiff = x - yrot;
        ydiff = -y + xrot;
    }
    else
        mouseDown = false;
}

void mouseMotion(int x, int y)
{
    if (mouseDown)
    {
        yrot = x - xdiff;
        xrot = y + ydiff;
        glutPostRedisplay();
    }
}

void toggleLamp()
{
    lampOn = !lampOn;

    if (lampOn)
        glEnable(GL_LIGHT0);
    else
        glDisable(GL_LIGHT0);

    glutPostRedisplay();
}

void changetableColor(unsigned char key, int x, int y)
{
    switch (key)
    {
        // Color 1
    case 'w':
        wKeyPressCount++;

        // Mengubah warna secara random hanya jika tombol 'w' ditekan kurang dari 5 kali
        if (wKeyPressCount <= 4)
        {
            tableColor1[0] = (GLfloat)rand() / RAND_MAX;
            tableColor1[1] = (GLfloat)rand() / RAND_MAX;
            tableColor1[2] = (GLfloat)rand() / RAND_MAX;
        }
        else
        {
            // Jika tombol 'w' ditekan lebih dari 5 kali, kembali ke warna semula
            tableColor1[0] = 250.0;
            tableColor1[1] = 245.0;
            tableColor1[2] = 239.0;
            wKeyPressCount = 0; // Reset hitungan penekanan tombol 'w'
        }
        break;

        // Color 2
    case 'c':
        cKeyPressCount++;

        // Mengubah warna secara random hanya jika tombol 'a' ditekan kurang dari 5 kali
        if (cKeyPressCount <= 4)
        {
            tableColor2[0] = (GLfloat)rand() / RAND_MAX;
            tableColor2[1] = (GLfloat)rand() / RAND_MAX;
            tableColor2[2] = (GLfloat)rand() / RAND_MAX;
        }
        else
        {
            // Jika tombol 'c' ditekan lebih dari 5 kali, kembali ke warna semula
            tableColor2[0] = 0.36;
            tableColor2[1] = 0.18;
            tableColor2[2] = 0.00;
            cKeyPressCount = 0; // Reset hitungan penekanan tombol 'a'
        }
        break;

    case 't':
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glEnable(GL_COLOR_MATERIAL);
        break;
    case 'y':
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
        glDisable(GL_LIGHT0);
        glDisable(GL_COLOR_MATERIAL);
        break;
    case 'f':
        idles = true;
        break;
    case 'g':
        idles = false;
        break;
    case 'o':
        changeMode = 90;
        break;
    case 'p':
        changeMode = 0;
        break;
    case 27:
        exit(0);
        break;
    default:
        break;
    }

    glutPostRedisplay();
}

void SetupLighting()
{
    GLfloat lightPosition[] = {1.0f, 1.0f, 1.0f, 0.0f}; // Light position (directional light from top-right)
    GLfloat ambientColor[] = {.2, .2, .2, 1};           // Ambient light color (gray)
    GLfloat diffuseColor[] = {1, 1, 1, 1};              // Diffuse light color (white)
    GLfloat specularColor[] = {1.0f, 1.0f, 1.0f, 1.0f}; // Specular light color (white)

    // Set light properties
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientColor);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseColor);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularColor);

    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE); // Set material properties to follow glColor
}

void reshape(int w, int h)
{
    if (h == 0)
        h = 1;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, (float)w / h, 5, 450);
    glTranslatef(0, 0, -300);

    glMatrixMode(GL_MODELVIEW);
}

void drawTable(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(0, 0, 3, 0, 0, 0, 0, 1, 0);
    glRotatef(xrot, 1, 0, 0);
    glRotatef(yrot, 0, 1, 0);
    glPushMatrix();

    // ALAS MEJA
    glBegin(GL_POLYGON); // BAGIAN ATAS
    glColor3fv(tableColor1);
    glVertex3f(-52, 15, 58);
    glVertex3f(52, 15, 58);
    glVertex3f(52, 15, -28);
    glVertex3f(-52, 15, -28);
    glEnd();

    glBegin(GL_POLYGON); // BAGIAN BAWAH
    glColor3fv(tableColor2);
    glVertex3f(-52, 14, 58);
    glVertex3f(52, 14, 58);
    glVertex3f(52, 14, -28);
    glVertex3f(-52, 14, -28);
    glEnd();

    glBegin(GL_POLYGON); // SAMPING KANAN
    glColor3fv(tableColor2);
    glVertex3f(50, 13, 58);
    glVertex3f(50, 15, 58);
    glVertex3f(50, 15, -28);
    glVertex3f(50, 13, -28);
    glEnd();

    glBegin(GL_POLYGON); // SAMPING KIRI
    glColor3fv(tableColor2);
    glVertex3f(-50, 13, 58);
    glVertex3f(-50, 15, 58);
    glVertex3f(-50, 15, -28);
    glVertex3f(-50, 13, -28);
    glEnd();

    glBegin(GL_POLYGON); // BAGIAN DEPAN
    glColor3fv(tableColor2);
    glVertex3f(-50, 13, 58);
    glVertex3f(-50, 15, 58);
    glVertex3f(50, 15, 58);
    glVertex3f(50, 13, 58);
    glEnd();

    glBegin(GL_POLYGON); // BAGIAN BELAKANG
    glColor3fv(tableColor2);
    glVertex3f(-50, 13, -28);
    glVertex3f(-50, 15, -28);
    glVertex3f(50, 15, -28);
    glVertex3f(50, 13, -28);
    glEnd();

    // KAKI MEJA 1
    glPushMatrix();
    glRotatef(changeMode, 1, 0, 0);

    glBegin(GL_POLYGON); // ALAS BAWAH
    glColor3fv(tableColor1);
    glVertex3f(-45, -57, 55);
    glVertex3f(-50, -57, 55);
    glVertex3f(-45, -57, 50);
    glVertex3f(-50, -57, 50);
    glEnd();

    glBegin(GL_POLYGON); // BAGIAN DEPAN
    glColor3fv(tableColor1);
    glVertex3f(-50, 13, 55);
    glVertex3f(-45, 13, 55);
    glVertex3f(-45, -57, 55);
    glVertex3f(-50, -57, 55);
    glEnd();

    glBegin(GL_POLYGON); // BAGIAN BELAKANG
    glColor3fv(tableColor1);
    glVertex3f(-50, 13, 50);
    glVertex3f(-45, 13, 50);
    glVertex3f(-45, -57, 50);
    glVertex3f(-50, -57, 50);
    glEnd();

    glBegin(GL_POLYGON); // SAMPING KIRI
    glColor3fv(tableColor1);
    glVertex3f(-50, 13, 55);
    glVertex3f(-50, -57, 55);
    glVertex3f(-50, -57, 50);
    glVertex3f(-50, 13, 50);
    glEnd();

    glBegin(GL_POLYGON); // SAMPING KANAN
    glColor3fv(tableColor1);
    glVertex3f(-45, 13, 55);
    glVertex3f(-45, -57, 55);
    glVertex3f(-45, -57, 50);
    glVertex3f(-45, 13, 50);
    glEnd();
    glPopMatrix();

    // KAKI MEJA 2
    glPushMatrix();
    glRotatef(changeMode, 1, 0, 0);

    glBegin(GL_POLYGON); // ALAS BAWAH
    glColor3fv(tableColor1);
    glVertex3f(50, -57, 55);
    glVertex3f(45, -57, 55);
    glVertex3f(45, -57, 50);
    glVertex3f(50, -57, 50);
    glEnd();

    glBegin(GL_POLYGON); // DEPAN
    glColor3fv(tableColor1);
    glVertex3f(50, 13, 55);
    glVertex3f(45, 13, 55);
    glVertex3f(45, -57, 55);
    glVertex3f(50, -57, 55);
    glEnd();

    glBegin(GL_POLYGON); // BAGIAN BELAKANG
    glColor3fv(tableColor1);
    glVertex3f(50, 13, 50);
    glVertex3f(45, 13, 50);
    glVertex3f(45, -57, 50);
    glVertex3f(50, -57, 50);
    glEnd();

    glBegin(GL_POLYGON); // SAMPING KIRI
    glColor3fv(tableColor1);
    glVertex3f(50, 13, 50);
    glVertex3f(50, -57, 50);
    glVertex3f(50, -57, 55);
    glVertex3f(50, 13, 55);
    glEnd();

    glBegin(GL_POLYGON); // SAMPING KANAN
    glColor3fv(tableColor1);
    glVertex3f(45, 13, 50);
    glVertex3f(45, -57, 50);
    glVertex3f(45, -57, 55);
    glVertex3f(45, 13, 55);
    glEnd();

    glPopMatrix();

    glPushMatrix();
    glRotatef(changeMode, 1, 0, 0);
    // KAKI MEJA 3
    glBegin(GL_POLYGON); // ALAS BAWAH
    glColor3fv(tableColor1);
    glVertex3f(-50, -57, -20);
    glVertex3f(-45, -57, -20);
    glVertex3f(-45, -57, -25);
    glVertex3f(-50, -57, -25);
    glEnd();

    glBegin(GL_POLYGON); // DEPAN
    glColor3fv(tableColor1);
    glVertex3f(-50, 13, -20);
    glVertex3f(-45, 13, -20);
    glVertex3f(-45, -57, -20);
    glVertex3f(-50, -57, -20);
    glEnd();

    glBegin(GL_POLYGON); // BELAKANG
    glColor3fv(tableColor1);
    glVertex3f(-50, 13, -25);
    glVertex3f(-45, 13, -25);
    glVertex3f(-45, -57, -25);
    glVertex3f(-50, -57, -25);
    glEnd();

    glBegin(GL_POLYGON); // KIRI
    glColor3fv(tableColor1);
    glVertex3f(-50, 13, -20);
    glVertex3f(-50, -57, -20);
    glVertex3f(-50, -57, -25);
    glVertex3f(-50, 13, -25);
    glEnd();

    glBegin(GL_POLYGON); // KANAN
    glColor3fv(tableColor1);
    glVertex3f(-45, 13, -20);
    glVertex3f(-45, -57, -20);
    glVertex3f(-45, -57, -25);
    glVertex3f(-45, 13, -25);
    glEnd();

    glPopMatrix();

    glPushMatrix();
    glRotatef(changeMode, 1, 0, 0);
    // KAKI MEJA 4
    glBegin(GL_POLYGON); // ALAS BAWAH
    glColor3fv(tableColor1);
    glVertex3f(50, -57, -20);
    glVertex3f(45, -57, -20);
    glVertex3f(45, -57, -25);
    glVertex3f(50, -57, -25);
    glEnd();

    glBegin(GL_POLYGON); // DEPAN
    glColor3fv(tableColor1);
    glVertex3f(50, 13, -20);
    glVertex3f(45, 13, -20);
    glVertex3f(45, -57, -20);
    glVertex3f(50, -57, -20);
    glEnd();

    glBegin(GL_POLYGON); // BELAKANG
    glColor3fv(tableColor1);
    glVertex3f(50, 13, -25);
    glVertex3f(45, 13, -25);
    glVertex3f(45, -57, -25);
    glVertex3f(50, -57, -25);
    glEnd();

    glBegin(GL_POLYGON); // KIRI
    glColor3fv(tableColor1);
    glVertex3f(50, 13, -20);
    glVertex3f(50, -57, -20);
    glVertex3f(50, -57, -25);
    glVertex3f(50, 13, -25);
    glEnd();

    glBegin(GL_POLYGON); // KANAN
    glColor3fv(tableColor1);
    glVertex3f(45, 13, -20);
    glVertex3f(45, -57, -20);
    glVertex3f(45, -57, -25);
    glVertex3f(45, 13, -25);
    glEnd();

    glPopMatrix();

    // Sandaran Meja

    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    // logo
    glTexCoord2f(0.0, 0.0);
    glVertex3f(-10, 0, -26);

    glTexCoord2f(1.0, 0.0);
    glVertex3f(-10, -20, -26);

    glTexCoord2f(1.0, 1.0);
    glVertex3f(10, -20, -26);

    glTexCoord2f(0.0, 1.0);
    glVertex3f(10, 0, -26);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    // Bagian belakang
    glBegin(GL_QUADS);
    glColor3fv(tableColor2);
    glVertex3f(-45, 9, -25);
    glVertex3f(-45, -30, -25);
    glVertex3f(45, -30, -25);
    glVertex3f(45, 9, -25);
    glEnd();

    // Bagian bawah
    glBegin(GL_QUADS);
    glColor3fv(tableColor1);
    glVertex3f(-45, -13, -25);
    glVertex3f(45, -30, -25);
    glVertex3f(45, -30, -20);
    glVertex3f(-45, -13, -20);
    glEnd();

    // Bagian atas
    glBegin(GL_QUADS);
    glColor3fv(tableColor1);
    glVertex3f(-45, 9, -25);
    glVertex3f(45, 9, -25);
    glVertex3f(45, 9, -20);
    glVertex3f(-45, 9, -20);
    glEnd();

    // Bagian kiri
    glBegin(GL_QUADS);
    glColor3fv(tableColor1);
    glVertex3f(-45, 9, -25);
    glVertex3f(-45, -30, -25);
    glVertex3f(-45, -30, -20);
    glVertex3f(-45, 9, -20);
    glEnd();

    // Bagian kanan
    glBegin(GL_QUADS);
    glColor3fv(tableColor1);
    glVertex3f(45, 9, -25);
    glVertex3f(45, -30, -25);
    glVertex3f(45, -30, -20);
    glVertex3f(45, 9, -20);
    glEnd();

    // Bagian depan
    glBegin(GL_QUADS);
    glColor3fv(tableColor2);
    glVertex3f(-45, 9, -20);
    glVertex3f(-45, -30, -20);
    glVertex3f(45, -30, -20);
    glVertex3f(45, 9, -20);
    glEnd();

    // Kotak kecil 1
    glBegin(GL_QUADS);
    glColor3fv(tableColor1);
    glVertex3f(-45, 9, -19.5);
    glVertex3f(-45, 1, -19.5);
    glVertex3f(-38, 1, -19.5);
    glVertex3f(-38, 9, -19.5);
    glEnd();

    // Kotak kecil 2
    glBegin(GL_QUADS);
    glColor3fv(tableColor1);
    glVertex3f(38, 9, -19.5);
    glVertex3f(38, 1, -19.5);
    glVertex3f(45, 1, -19.5);
    glVertex3f(45, 9, -19.5);
    glEnd();

    // Kotak kecil 3
    glBegin(GL_QUADS);
    glColor3fv(tableColor1);
    glVertex3f(-45, -30, -19.5);
    glVertex3f(-45, -22, -19.5);
    glVertex3f(-38, -22, -19.5);
    glVertex3f(-38, -30, -19.5);
    glEnd();

    // Kotak kecil 4
    glBegin(GL_QUADS);
    glColor3fv(tableColor1);
    glVertex3f(38, -30, -19.5);
    glVertex3f(38, -22, -19.5);
    glVertex3f(45, -22, -19.5);
    glVertex3f(45, -30, -19.5);
    glEnd();

    // Balok Kecil Atas 1
    glBegin(GL_QUADS);
    // Bagian depan
    glColor3fv(tableColor1);
    glVertex3f(-45, 13, -20);
    glVertex3f(-45, 9, -20);
    glVertex3f(45, 9, -20);
    glVertex3f(45, 13, -20);

    // Bagian belakang
    glColor3fv(tableColor1);
    glVertex3f(-45, 13, -25);
    glVertex3f(-45, 9, -25);
    glVertex3f(45, 9, -25);
    glVertex3f(45, 13, -25);

    // Bagian atas
    glColor3fv(tableColor1);
    glVertex3f(-45, 13, -25);
    glVertex3f(45, 9, -25);
    glVertex3f(45, 9, -20);
    glVertex3f(-45, 13, -20);

    // Bagian bawah
    glColor3fv(tableColor1);
    glVertex3f(-45, 9, -25);
    glVertex3f(45, 9, -25);
    glVertex3f(45, 9, -20);
    glVertex3f(-45, 9, -20);

    // Bagian kiri
    glColor3fv(tableColor1);
    glVertex3f(-45, 13, -25);
    glVertex3f(-45, 9, -25);
    glVertex3f(-45, 9, -20);
    glVertex3f(-45, 13, -20);

    // Bagian kanan
    glColor3fv(tableColor1);
    glVertex3f(45, 13, -25);
    glVertex3f(45, 9, -25);
    glVertex3f(45, 9, -20);
    glVertex3f(45, 13, -20);
    glEnd();

    // Balok Kecil Atas 2
    glBegin(GL_QUADS);
    // Bagian depan
    glColor3fv(tableColor1);
    glVertex3f(-45, -30, -20);
    glVertex3f(-45, -34, -20);
    glVertex3f(45, -34, -20);
    glVertex3f(45, -30, -20);

    // Bagian belakang
    glColor3fv(tableColor1);
    glVertex3f(-45, -30, -25);
    glVertex3f(-45, -34, -25);
    glVertex3f(45, -34, -25);
    glVertex3f(45, -30, -25);

    // Bagian atas
    glColor3fv(tableColor1);
    glVertex3f(-50, 13, 50);
    glVertex3f(-45, 13, 50);
    glVertex3f(-45, 13, -20);
    glVertex3f(-50, 13, -20);

    // Bagian bawah
    glColor3fv(tableColor1);
    glVertex3f(-50, 9, 50);
    glVertex3f(-45, 9, 50);
    glVertex3f(-45, 9, -20);
    glVertex3f(-50, 9, -20);

    // Bagian kiri
    glColor3fv(tableColor1);
    glVertex3f(-45, 13, 50);
    glVertex3f(-45, 9, 50);
    glVertex3f(-45, 9, -20);
    glVertex3f(-45, 13, -20);

    // Bagian kanan
    glColor3fv(tableColor1);
    glVertex3f(-50, 13, 50);
    glVertex3f(-50, 9, 50);
    glVertex3f(-50, 9, -20);
    glVertex3f(-50, 13, -20);
    glEnd();

    // Balok Kecil Atas 3
    glBegin(GL_QUADS);
    // Bagian depan
    glColor3fv(tableColor1);
    glVertex3f(-45, -53, 50);
    glVertex3f(-45, -57, 50);
    glVertex3f(-45, -57, -20);
    glVertex3f(-45, -53, -20);

    // Bagian belakang
    glColor3fv(tableColor1);
    glVertex3f(-50, -53, 50);
    glVertex3f(-50, -57, 50);
    glVertex3f(-50, -57, -20);
    glVertex3f(-50, -53, -20);

    // Bagian atas
    glColor3fv(tableColor1);
    glVertex3f(50, 13, 50);
    glVertex3f(45, 13, 50);
    glVertex3f(45, 13, -20);
    glVertex3f(50, 13, -20);

    // Bagian bawah
    glColor3fv(tableColor1);
    glVertex3f(50, 9, 50);
    glVertex3f(45, 9, 50);
    glVertex3f(45, 9, -20);
    glVertex3f(50, 9, -20);

    // Bagian kiri
    glColor3fv(tableColor1);
    glVertex3f(50, 13, 50);
    glVertex3f(50, 9, 50);
    glVertex3f(50, 9, -20);
    glVertex3f(50, 13, -20);

    // Bagian kanan
    glColor3fv(tableColor1);
    glVertex3f(45, 13, 50);
    glVertex3f(45, 9, 50);
    glVertex3f(45, 9, -20);
    glVertex3f(45, 13, -20);

    // Balok Kecil Atas 4
    glBegin(GL_QUADS);
    // Bagian depan
    glColor3fv(tableColor1);
    glVertex3f(-45, 13, 55);
    glVertex3f(-45, 9, 55);
    glVertex3f(45, 9, 55);
    glVertex3f(45, 13, 55);

    // Bagian belakang
    glColor3fv(tableColor1);
    glVertex3f(-45, 13, 50);
    glVertex3f(-45, 9, 50);
    glVertex3f(45, 9, 50);
    glVertex3f(45, 13, 50);

    // Bagian atas
    glColor3fv(tableColor1);
    glVertex3f(-45, 13, 55);
    glVertex3f(45, 13, 55);
    glVertex3f(45, 13, 50);
    glVertex3f(-45, 13, 50);

    // Bagian bawah
    glColor3fv(tableColor1);
    glVertex3f(-45, 9, 55);
    glVertex3f(45, 9, 55);
    glVertex3f(45, 9, 50);
    glVertex3f(-45, 9, 50);

    // Bagian kiri
    glColor3fv(tableColor1);
    glVertex3f(-45, 13, 55);
    glVertex3f(-45, 9, 55);
    glVertex3f(-45, 9, 50);
    glVertex3f(-45, 13, 50);

    // Bagian kanan
    glColor3fv(tableColor1);
    glVertex3f(45, 13, 55);
    glVertex3f(45, 9, 55);
    glVertex3f(45, 9, 50);
    glVertex3f(45, 13, 50);
    glEnd();

    // Balok Kecil Bawah 1
    glBegin(GL_QUADS);
    // Bagian depan
    glColor3fv(tableColor1);
    glVertex3f(-45, -30, -20);
    glVertex3f(-45, -34, -20);
    glVertex3f(45, -34, -20);
    glVertex3f(45, -30, -20);

    // Bagian belakang
    glColor3fv(tableColor1);
    glVertex3f(-45, -30, -25);
    glVertex3f(-45, -34, -25);
    glVertex3f(45, -34, -25);
    glVertex3f(45, -30, -25);

    // Bagian atas
    glColor3fv(tableColor1);
    glVertex3f(-50, -53, 50);
    glVertex3f(-45, -53, 50);
    glVertex3f(-45, -53, -20);
    glVertex3f(-50, -53, -20);

    // Bagian bawah
    glColor3fv(tableColor1);
    glVertex3f(-50, -57, 50);
    glVertex3f(-45, -57, 50);
    glVertex3f(-45, -57, -20);
    glVertex3f(-50, -57, -20);

    // Bagian kiri
    glColor3fv(tableColor1);
    glVertex3f(-45, -30, -25);
    glVertex3f(-45, -34, -25);
    glVertex3f(-45, -34, -20);
    glVertex3f(-45, -30, -20);

    // Bagian kanan
    glColor3fv(tableColor1);
    glVertex3f(45, -30, -25);
    glVertex3f(45, -34, -25);
    glVertex3f(45, -34, -20);
    glVertex3f(45, -30, -20);
    glEnd();

    // Balok Kecil Bawah 2
    glBegin(GL_QUADS);
    // Bagian depan
    glColor3fv(tableColor1);
    glVertex3f(-45, -53, 50);
    glVertex3f(-45, -57, 50);
    glVertex3f(-45, -57, -20);
    glVertex3f(-45, -53, -20);

    // Bagian belakang
    glColor3fv(tableColor1);
    glVertex3f(-50, -53, 50);
    glVertex3f(-50, -57, 50);
    glVertex3f(-50, -57, -20);
    glVertex3f(-50, -53, -20);

    // Bagian atas
    glColor3fv(tableColor1);
    glVertex3f(50, -53, 50);
    glVertex3f(45, -53, 50);
    glVertex3f(45, -53, -20);
    glVertex3f(50, -53, -20);

    // Bagian bawah
    glColor3fv(tableColor1);
    glVertex3f(50, -57, 50);
    glVertex3f(45, -57, 50);
    glVertex3f(45, -57, -20);
    glVertex3f(50, -57, -20);

    // Bagian kiri
    glColor3fv(tableColor1);
    glVertex3f(-50, -53, -25);
    glVertex3f(-45, -57, -25);
    glVertex3f(-45, -57, -20);
    glVertex3f(-50, -53, -20);

    // Bagian kanan
    glColor3fv(tableColor1);
    glVertex3f(-50, -53, -25);
    glVertex3f(-45, -57, -25);
    glVertex3f(-45, -57, -20);
    glVertex3f(-50, -53, -20);

    // Balok Kecil Bawah 3
    glBegin(GL_QUADS);
    // Bagian depan
    glColor3fv(tableColor1);
    glVertex3f(45, -53, 50);
    glVertex3f(45, -57, 50);
    glVertex3f(45, -57, -20);
    glVertex3f(45, -53, -20);

    // Bagian belakang
    glColor3fv(tableColor1);
    glVertex3f(50, -53, 50);
    glVertex3f(50, -57, 50);
    glVertex3f(50, -57, -20);
    glVertex3f(50, -53, -20);

    // Bagian atas
    glColor3fv(tableColor1);
    glVertex3f(-50, -53, -25);
    glVertex3f(-50, -57, -25);
    glVertex3f(-45, -57, -20);
    glVertex3f(-45, -53, -20);

    // Bagian bawah
    glColor3fv(tableColor1);
    glVertex3f(-50, -34, -25);
    glVertex3f(45, -34, -25);
    glVertex3f(45, -34, -20);
    glVertex3f(-50, -34, -20);

    // Bagian kiri
    glColor3fv(tableColor1);
    glVertex3f(-45, -53, -25);
    glVertex3f(-45, -57, -25);
    glVertex3f(-45, -57, -20);
    glVertex3f(-45, -53, -20);

    // Bagian kanan
    glColor3fv(tableColor1);
    glVertex3f(-45, -53, -25);
    glVertex3f(-45, -57, -25);
    glVertex3f(-45, -57, -20);
    glVertex3f(-45, -53, -20);
    glEnd();

    glColor3fv(tableColor1);
    glutSolidCube(1.0);

    glPopMatrix();
    glutSwapBuffers();
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    gluLookAt(0, 0, 3, 0, 0, 0, 0, 1, 0);

    // Gambar meja pertama
    drawTable();

    // Pindahkan koordinat untuk gambar meja kedua
    glTranslatef(separationDistance, 0.0, 0.0);

    // Gambar meja kedua
    drawTable();

    glRotatef(xrot, 1, 0, 0);
    glRotatef(yrot, 0, 1, 0);

    glutSwapBuffers();
}

void idleAnimation()
{
    if (!mouseDown && idles)
    {
        xrot += .01f;
        yrot += .015f;
    }

    glutPostRedisplay();
}

// Coding untuk membuat object

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowPosition(240, 80);
    glutInitWindowSize(750, 600);
    glutCreateWindow("3D MEJA LAB GT 303 PENS");

    myinit();
    glutDisplayFunc(drawTable);
    glutMouseFunc(mouse);
    glutMotionFunc(mouseMotion);
    glutReshapeFunc(ukur);
    glutIdleFunc(idleAnimation);

    // Callback fungsi untuk mengubah warna latar belakang
    glutKeyboardFunc(changetableColor);

    glutMainLoop();

    return 0;
}
